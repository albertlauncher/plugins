// Copyright (C) 2014-2018 Manuel Schneider

#include "externalextension.h"
#include <QByteArray>
#include <QFileInfo>
#include <QDebug>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QLabel>
#include <QProcess>
#include <QVBoxLayout>
#include <functional>
#include <vector>
#include "albert/util/standardactions.h"
#include "albert/util/standarditem.h"
#include "xdg/iconlookup.h"
using namespace std;
using namespace Core;

#define EXTERNAL_EXTENSION_IID "org.albert.extension.external/v3.0"

namespace {

const constexpr char* ALBERT_OP = "ALBERT_OP";
const constexpr char* ALBERT_QRY = "ALBERT_QUERY";
const constexpr uint PROC_TIMEOUT = 5000;

enum Message {
    Metadata,
    Initialize,
    Finalize,
    Query
};

QString OP_COMMANDS[] = {
    "METADATA",
    "INITIALIZE",
    "FINALIZE",
    "QUERY"
};


bool runProcess (QString path,
                 std::map<QString, QString> *variables,
                 QByteArray *out,
                 QString *errorString) {

    // Run the process
    QProcess process;
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    for ( auto & entry : *variables )
        env.insert(entry.first, entry.second);
    process.setProcessEnvironment(env);
    process.start(path);

    if ( !process.waitForFinished(PROC_TIMEOUT) ){
        *errorString = "Process timed out.";
        process.kill();
        return false;
    }

    if ( process.exitStatus() != QProcess::NormalExit ) {
        *errorString = "Process crashed.";
        return false;
    }

    if ( process.exitCode() != 0 ) {

        *errorString = QString("Exit code is %1.").arg(process.exitCode());

        QByteArray cout = process.readAllStandardOutput();
        QByteArray cerr = process.readAllStandardError();

        if (!cout.isEmpty())
            errorString->append(QString("\n%1").arg(QString(cout)).trimmed());

        if (!cerr.isEmpty())
            errorString->append(QString("\n%1").arg(QString(cerr)).trimmed());

        return false;
    }

    *out = process.readAllStandardOutput();

    return true;
}


bool parseJsonObject (const QByteArray &json,
                      QJsonObject *object,
                      QString *errorString) {

    // Parse stdout
    QJsonParseError error;
    QJsonDocument document = QJsonDocument::fromJson(json, &error);
    if ( document.isNull() ) {
        *errorString = QString("Invalid JSON at %1: %2").arg(error.offset).arg(error.errorString());
        return false;
    }

    if ( !document.isObject() ) {
        *errorString = "Output does not contain a JSON object.";
        return false;
    }

    *object = document.object();
    return true;
}


bool saveVariables (QJsonObject *object,
                    std::map<QString, QString> *variables,
                    QString *errorString) {

    variables->clear();

    if ( !object->contains("variables") )
        return true;

    if ( !object->operator[]("variables").isObject() ) {
        *errorString = "'variables' is not a JSON object";
        return false;
    }

    QJsonObject vars = object->operator[]("variables").toObject();
    for (auto it = vars.begin(); it != vars.end(); ++it)
        if ( it.value().isString() )
            variables->emplace(it.key(), it.value().toString());

    return true;
}

}

/** ***************************************************************************/
ExternalExtensions::ExternalExtension::ExternalExtension(const QString &path, const QString &id)
    : QueryHandler(id), path_(path), id_(id) {

    state_ = State::Error;
    name_ = id; // Will be overwritten when available

    /*
     * Get the metadata
     */

    QString preparedMessage = QString(OP_COMMANDS[Message::Metadata]).append(": %1 [%2]");

    // Run the process
    QByteArray out;
    variables_[ALBERT_OP] = OP_COMMANDS[Message::Metadata];
    if ( !runProcess(path_, &variables_, &out, &errorString_) ){
        qWarning() << qPrintable(preparedMessage.arg(errorString_, path_));
        return;
    }

    // Parse stdout
    QJsonObject object;
    if ( !parseJsonObject(out, &object,  &errorString_) ) {
        qWarning() << qPrintable(preparedMessage.arg(errorString_, path_));
        return;
    }

    // Check for a sane interface ID (IID)
    QJsonValue value = object["iid"];
    if (value.isNull()) {
        errorString_ = "Metadate does not contain an interface id (iid).";
        qWarning() << qPrintable(preparedMessage.arg(errorString_, path_));
        return;
    }

    QString iid = value.toString();
    if (iid != EXTERNAL_EXTENSION_IID) {
        errorString_ = QString("Interface id '%1' does not match '%2'.").arg(iid, EXTERNAL_EXTENSION_IID);
        qWarning() << qPrintable(preparedMessage.arg(errorString_, path_));
        return;
    }

    trigger_ = object["trigger"].toString();
    if ( trigger_.isEmpty() ){
        errorString_ = "No trigger defined in metadata.";
        qWarning() << qPrintable(preparedMessage.arg(errorString_, path_));
        return;
    }

    // Get opional data
    value = object["name"];
    name_ = value.isString() ? value.toString() : id_;

    value = object["version"];
    version_ = value.isString() ? value.toString() : "N/A";

    value = object["author"];
    author_ = value.isString() ? value.toString() : "N/A";

    value = object["description"];
    description_ = value.toString();

    value = object["usage_example"];
    usageExample_ = value.toString();

    value = object["dependencies"];
    if ( value.isArray() )
        for (const QJsonValue & val : value.toArray())
            dependencies_.append(val.toString());


    /*
     * Initialize the extension
     */

    preparedMessage = QString(OP_COMMANDS[Message::Initialize]).append(": %1 [%2]");

    // Run the process
    variables_[ALBERT_OP] = OP_COMMANDS[Message::Initialize];
    if ( !runProcess(path_, &variables_, &out, &errorString_) ){
        qWarning() << qPrintable(preparedMessage.arg(errorString_, path_));
        return;
    }

    if ( !out.isEmpty() ) {

        // Parse stdout
        if ( !parseJsonObject(out, &object,  &errorString_) ){
            qWarning() << qPrintable(preparedMessage.arg(errorString_, path_));
            return;
        }

        // Finally save the variables, if any
        if ( !saveVariables(&object, &variables_, &errorString_) ){
            qWarning() << qPrintable(preparedMessage.arg(errorString_, path_));
            return;
        }
    }

    state_ = State::Initialized;
}


/** ***************************************************************************/
ExternalExtensions::ExternalExtension::~ExternalExtension() {

    if ( state_ == State::Error )
        return;

    QJsonObject object;
    QByteArray out;

    QString preparedMessage = QString(OP_COMMANDS[Message::Finalize]).append(": %1 [%2]");

    // Run the process
    variables_[ALBERT_OP] = OP_COMMANDS[Message::Finalize];
    if ( !runProcess(path_, &variables_, &out, &errorString_) ) {
        qWarning() << qPrintable(preparedMessage.arg(errorString_, path_));
        return;
    }
}


/** ***************************************************************************/
void ExternalExtensions::ExternalExtension::handleQuery(Core::Query* query) const {

    Q_ASSERT(state_ != State::Error);

    // External extension must run only when triggered, since they are too ressource heavy
    if ( query->trigger().isEmpty() )
        return;

    // Never run the extension concurrent
    QMutexLocker lock (&processMutex_);
    if (!query->isValid())
        return;

    QByteArray out;
    QString errorString;
    QString preparedMessage = QString(OP_COMMANDS[Message::Query]).append(": %1 [%2]");

    // Build env
    QProcessEnvironment env = QProcessEnvironment::systemEnvironment();
    variables_[ALBERT_OP] = OP_COMMANDS[Message::Query];
    variables_[ALBERT_QRY] = query->string();
    for ( auto & entry : variables_ )
        env.insert(entry.first, entry.second);

    // Run the process
    QProcess process;
    process.setProcessEnvironment(env);
    process.start(path_);

    while( !process.waitForFinished(10) ) {
        if (!query->isValid()) {
            process.terminate();
            if ( !process.waitForFinished(PROC_TIMEOUT) )
                process.kill();
            return;
        }
    }

    if ( process.exitStatus() != QProcess::NormalExit ) {
        qWarning() << qPrintable(preparedMessage.arg("Process crashed.", path_));
        return;
    }

    if ( process.exitCode() != 0 ) {
        errorString = QString("Exit code is %1.").arg(process.exitCode()).append(process.readAllStandardError());
        qWarning() << qPrintable(preparedMessage.arg(errorString, path_));
        return;
    }

    out = process.readAllStandardOutput();

    // Parse stdout
    QJsonObject object;
    if ( !parseJsonObject(out, &object,  &errorString) ) {
        qWarning() << qPrintable(preparedMessage.arg(errorString, path_));
        return;
    }

    if ( out.isEmpty() )
        return;

    // Save the variables, if any
    if ( !saveVariables(&object, &variables_, &errorString) ) {
         qWarning() << qPrintable(preparedMessage.arg(errorString, path_));
         return;
    }

    // Quit if there are no items
    QJsonValue value = object["items"];
    if ( value.isNull() )
        return;

    // Check type of items
    if ( !value.isArray() ) {
        errorString = "'items' is not an array.";
        qWarning() << qPrintable(preparedMessage.arg(errorString, path_));
        return;
    }

    // Iterate over the results
    shared_ptr<StandardItem> item;
    vector<pair<shared_ptr<Core::Item>,short>> results;

    int i = 0;
    for (const QJsonValue & itemValue : value.toArray() ){

        if ( !itemValue.isObject() ) {
            qWarning() << qPrintable(QString("Returned item %1 Item is not a JSON object. (%2)").arg(i).arg(path_));
            return;
        }
        object = itemValue.toObject();

        // Build the item from the json object
        item = std::make_shared<StandardItem>(object["id"].toString());
        item->setText(object["name"].toString());
        item->setSubtext(object["description"].toString());
        item->setCompletion(object["completion"].toString());
        QString icon = XDG::IconLookup::iconPath({object["icon"].toString(), "unknown"});
        item->setIconPath(icon.isEmpty() ? ":unknown" : icon);

        // Build the actions
        for (const QJsonValue & value : object["actions"].toArray()){
            object = value.toObject();

            QString command = object["command"].toString();
            QStringList arguments;
            for (const QJsonValue & value : object["arguments"].toArray())
                 arguments.append(value.toString());

            item->addAction(make_shared<ProcAction>(object["name"].toString(),
                                                    QStringList(command)+arguments));
        }
        results.emplace_back(std::move(item), 0);
    }

    query->addMatches(make_move_iterator(results.begin()),
                      make_move_iterator(results.end()));
}

