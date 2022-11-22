// Copyright (c) 2022 Manuel Schneider

#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QDesktopWidget>
#include <QDir>
#include <QDirIterator>
#include <QJsonDocument>
#include <QJsonObject>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSettings>
#include <QStandardPaths>
#include <QTimer>
#include "frontendplugin.h"
#include "mainwindow.h"

namespace {
const QString CFG_CENTERED        = "showCentered";
const bool    DEF_CENTERED        = true;
const QString CFG_HIDEONFOCUSLOSS = "hideOnFocusLoss";
const bool    DEF_HIDEONFOCUSLOSS = true;
const QString CFG_ALWAYS_ON_TOP   = "alwaysOnTop";
const bool    DEF_ALWAYS_ON_TOP   = true;
const char*   CFG_HIDE_ON_CLOSE   = "hideOnClose";
const bool    DEF_HIDE_ON_CLOSE   = false;
const char*   CFG_CLEAR_ON_HIDE   = "clearOnHide";
const bool    DEF_CLEAR_ON_HIDE   = false;
const QString CFG_STYLEPATH       = "stylePath";
const QString CFG_WND_POS         = "windowPosition";
const QString PLUGIN_ID           = "org.albert.frontend.qmlboxmodel";
const QString STYLE_MAIN_NAME     = "MainComponent.qml";
const QString STYLE_CONFIG_NAME   = "style_properties.ini";
const QString PREF_OBJ_NAME       = "preferences";
const QString FRAME_OBJ_NAME      = "frame";
}

/** ***************************************************************************/
QmlBoxModel::MainWindow::MainWindow(FrontendPlugin *plugin, QWindow *parent) : QQuickView(parent) {
    setColor(Qt::transparent);
    setFlags(Qt::Tool
             | Qt::WindowStaysOnTopHint
             | Qt::FramelessWindowHint
             | Qt::WindowCloseButtonHint // No close event w/o this
             );

    plugin_ = plugin;

    // Set qml environment
    rootContext()->setContextProperty("mainWindow", this);
    rootContext()->setContextProperty("history", &history_);
    rootContext()->setContextProperty("resultsModel", &model_);

    // Quit application when qml signals quit
    connect(engine(), SIGNAL(quit()), QCoreApplication::instance(), SLOT(quit()));

    // When component is ready load the saved properties
    connect(this, &QQuickView::statusChanged, this, [this](QQuickView::Status status){
        if ( status == QQuickView::Status::Ready ){

            // Get root object
            if (!rootObject()){
                qWarning() << "Could not retrieve settableProperties: There is no root object.";
                return;
            }

            // Forward signals
            connect(rootObject(), SIGNAL(inputTextChanged()),
                    this, SIGNAL(inputChanged()));

            connect(rootObject(), SIGNAL(settingsWidgetRequested()),
                    this, SIGNAL(settingsWidgetRequested()));

            connect(rootObject(), SIGNAL(settingsWidgetRequested()),
                    this, SLOT(hide()));

            // Get preferences object
            QObject *preferencesObject = rootObject()->findChild<QObject*>(PREF_OBJ_NAME);
            if (!preferencesObject){
                qWarning() << qPrintable(QString("Could not retrieve settableProperties: "
                                                 "There is no object named '%1'.").arg(PREF_OBJ_NAME));
                return;
            }

            // Load the style properties in the group of this style id
            QSettings s(plugin_->configLocation().filePath(STYLE_CONFIG_NAME), QSettings::Format::IniFormat);
            s.beginGroup(QFileInfo(source().toString()).dir().dirName());
            for (const QString &prop : settableProperties())
                if (s.contains(prop))
                    preferencesObject->setProperty(prop.toLatin1().data(), s.value(prop));
        }
    });

    // Reload if source file changed
    connect(&watcher_, &QFileSystemWatcher::fileChanged, this, [this](){
        qDebug() << "QML file reloaded.";
        QUrl url = source();
        setSource(QUrl());
        engine()->clearComponentCache();
        setSource(url);
        watcher_.addPath(url.toString());
    });

    // Center window between each hide and show
    connect(this, &QQuickView::visibilityChanged, this, [this](QWindow::Visibility visibility){
        if ( visibility != QWindow::Visibility::Hidden )
            if ( showCentered_ ){
                QDesktopWidget *dw = QApplication::desktop();
                setPosition(dw->availableGeometry(dw->screenNumber(QCursor::pos()))
                            .center()-QPoint(width()/2,256));
            }
        if ( clearOnHide_ )
            this->setInput("");
    });

    QStringList pluginDataPaths = QStandardPaths::locateAll(QStandardPaths::AppDataLocation,
                                                            plugin->id(),
                                                            QStandardPaths::LocateDirectory);

//    // Add the shared modules to the lookup path
//    for (const QString &pluginDataPath : pluginDataPaths){
//        QDir pluginDataDir = QDir(pluginDataPath);
//        if ( pluginDataDir.exists("shared") )
//            engine()->addImportPath(pluginDataDir.filePath("shared"));
//    }

    // Get style files
    QFileInfoList styles;
    for (const QString &pluginDataPath : pluginDataPaths) {
        QDirIterator it(QString("%1/styles").arg(pluginDataPath), QDir::Dirs|QDir::NoDotAndDotDot);
        while ( it.hasNext() ) {
            QDir root = QDir(it.next());
            if ( root.exists(STYLE_MAIN_NAME) ){
                QmlStyleSpec style;
                style.mainComponent = root.filePath(STYLE_MAIN_NAME);
                style.name          = root.dirName();
                style.author        = "N/A";
                style.version       = "N/A";
                if ( root.exists("metadata.json") ) {
                    QFile file(root.filePath("metadata.json"));
                    if (file.open(QIODevice::ReadOnly)) {
                        QJsonObject metadata = QJsonDocument::fromJson(file.readAll()).object();
                        if (metadata.contains("name"))
                            style.name = metadata["name"].toString();
                        if (metadata.contains("author"))
                            style.author = metadata["author"].toString();
                        if (metadata.contains("version"))
                            style.version = metadata["version"].toString();
                    }
                }
                styles_.push_back(style);
            }
        }
    }

    if (styles_.empty())
        throw "No styles found.";


    auto storeWinPos = [this](){
        plugin_->settings().setValue(CFG_WND_POS, position());
    };
    connect(this, &MainWindow::xChanged, this, storeWinPos);
    connect(this, &MainWindow::yChanged, this, storeWinPos);

    // Load window settings
    setPosition(plugin_->settings().value(CFG_WND_POS).toPoint());
    setShowCentered(plugin_->settings().value(CFG_CENTERED, DEF_CENTERED).toBool());
    setClearOnHide(plugin_->settings().value(CFG_CLEAR_ON_HIDE, DEF_CLEAR_ON_HIDE).toBool());
    setHideOnFocusLoss(plugin_->settings().value(CFG_HIDEONFOCUSLOSS, DEF_HIDEONFOCUSLOSS).toBool());
    setAlwaysOnTop(plugin_->settings().value(CFG_ALWAYS_ON_TOP, DEF_ALWAYS_ON_TOP).toBool());
    setHideOnClose(plugin_->settings().value(CFG_HIDE_ON_CLOSE, DEF_HIDE_ON_CLOSE).toBool());
    if ( plugin_->settings().contains(CFG_STYLEPATH) && QFile::exists(plugin_->settings().value(CFG_STYLEPATH).toString()) )
        setSource(plugin_->settings().value(CFG_STYLEPATH).toString());
    else {
        setSource(styles_[0].mainComponent);
        plugin_->settings().setValue(CFG_STYLEPATH, styles_[0].mainComponent);
    }

}


/** ***************************************************************************/
QmlBoxModel::MainWindow::~MainWindow() {

}


/** ***************************************************************************/
QString QmlBoxModel::MainWindow::input() {

    // Get root object
    QObject *rootObj = rootObject();
    if (!rootObj){
        qWarning() << "Could not retrieve input: There is no root object.";
        return QString();
    }
    return rootObj->property("inputText").toString();
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setInput(const QString &input) {

    // Get root object
    QObject *rootObj = rootObject();
    if (!rootObj){
        qWarning() << "Could not retrieve input: There is no root object.";
        return;
    }
    rootObj->setProperty("inputText", input);
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setSource(const QUrl &url) {

    // Apply the source
    QQuickView::setSource(url);

    if ( url.isEmpty() )
        return;

    // Save the theme
    plugin_->settings().setValue(CFG_STYLEPATH, source().toString());

    // Watch this source file for modifications
    if ( !watcher_.files().isEmpty() )
        watcher_.removePaths(watcher_.files());
    watcher_.addPath(url.toString());
}


/** ***************************************************************************/
const std::vector<QmlBoxModel::QmlStyleSpec> &QmlBoxModel::MainWindow::availableStyles() const {
    return styles_;
}


/** ***************************************************************************/
QStringList QmlBoxModel::MainWindow::settableProperties() {

    // Get root object
    if (!rootObject()){
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return QStringList();
    }

    // Get preferences object
    const QObject *preferencesObject = rootObject()->findChild<QObject*>(PREF_OBJ_NAME);
    if (!preferencesObject){
        qWarning() << qPrintable(QString("Could not retrieve settableProperties: "
                                         "There is no object named '%1'.").arg(PREF_OBJ_NAME));
        return QStringList();
    }

    // Get preferences object's meta object (Reflection yieh…)
    const QMetaObject *preferencesMetaObject = preferencesObject->metaObject();
    if (!preferencesMetaObject){
        qWarning() << "Could not retrieve settableProperties: Fetching MetaObject failed.";
        return QStringList();
    }

    // Get all properties of the object
    QStringList settableProperties;
    for (int i = 0; i < preferencesMetaObject->propertyCount(); i++)
        settableProperties.append(preferencesMetaObject->property(i).name());

    // QtObject type has a single property "objectName". Remove it.
    settableProperties.removeAll("objectName");

    return settableProperties;
}


/** ***************************************************************************/
QVariant QmlBoxModel::MainWindow::property(const char *name) const {

    // Get root object
    if (!rootObject()){
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return QVariant();
    }

    // Get preferences object
    const QObject *preferencesObject = rootObject()->findChild<QObject*>(PREF_OBJ_NAME);
    if (!preferencesObject){
        qWarning() << qPrintable(QString("Could not retrieve settableProperties: "
                                         "There is no object named '%1'.").arg(PREF_OBJ_NAME));
        return QVariant();
    }

    return preferencesObject->property(name);
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setProperty(const char *attribute, const QVariant &value) {

    // Create the settings instance of the decicated file in config location
    QSettings s(plugin_->configLocation().filePath(STYLE_CONFIG_NAME), QSettings::Format::IniFormat);
    s.beginGroup(QFileInfo(source().toString()).dir().dirName());
    s.setValue(attribute, value);

    // Get root object
    if (!rootObject()) {
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return;
    }

    // Get preferences object
    QObject *preferencesObject = rootObject()->findChild<QObject*>(PREF_OBJ_NAME);
    if (!preferencesObject) {
        qWarning() << qPrintable(QString("Could not retrieve settableProperties: "
                                         "There is no object named '%1'.").arg(PREF_OBJ_NAME));
        return;
    }

    // Set the property
    preferencesObject->setProperty(attribute, value);
}


/** ***************************************************************************/
QStringList QmlBoxModel::MainWindow::availableThemes() {

    // Get root object
    if (!rootObject()){
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return QStringList();
    }

    QVariant returnedValue;
    QMetaObject::invokeMethod(rootObject(), "availableThemes", Q_RETURN_ARG(QVariant, returnedValue));
    return returnedValue.toStringList();
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setTheme(const QString &name){

    // Get root object
    if (!rootObject()) {
        qWarning() << "Could not retrieve settableProperties: There is no root object.";
        return;
    }

    // Let qml apply the theme
    QMetaObject::invokeMethod(rootObject(), "setTheme", Q_ARG(QVariant, QVariant::fromValue(name)));

    // Save all current poperties in the group with this style id
    QSettings s(plugin_->configLocation().filePath(STYLE_CONFIG_NAME), QSettings::Format::IniFormat);
    QString styleId = QFileInfo(source().toString()).dir().dirName();
    s.beginGroup(styleId);
    for (const QString &prop : settableProperties())
        s.setValue(prop, property(prop.toLatin1().data()));
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setModel(QAbstractItemModel *model) {
    model_.setSourceModel(model);
}


/** ***************************************************************************/
bool QmlBoxModel::MainWindow::event(QEvent *event) {
    switch (event->type())
    {
    // Quit on Alt+F4
    case QEvent::Close:
        ( hideOnClose_ ) ? setVisible(false) : qApp->quit();
        return true;

    // Hide window on escape key
    case QEvent::KeyPress:
        if ( static_cast<QKeyEvent*>(event)->modifiers() == Qt::NoModifier
             && static_cast<QKeyEvent*>(event)->key() == Qt::Key_Escape ){
            hide();
            return true;
        }
        break;
    default:break;
    }
    return QQuickView::event(event);
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::focusOutEvent(QFocusEvent *) {
    if (hideOnFocusLoss_)
        hide();
}


/** ***************************************************************************/
bool QmlBoxModel::MainWindow::alwaysOnTop() const {
    return flags() & Qt::WindowStaysOnTopHint;
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setAlwaysOnTop(bool alwaysOnTop) {

    plugin_->settings().setValue(CFG_ALWAYS_ON_TOP, alwaysOnTop);

    alwaysOnTop
            ? setFlags(flags() | Qt::WindowStaysOnTopHint)
            : setFlags(flags() & ~Qt::WindowStaysOnTopHint);
    // Flags changed. Update
    QQuickView::hide();
}


/** ***************************************************************************/
bool QmlBoxModel::MainWindow::hideOnFocusLoss() const {
    return hideOnFocusLoss_;
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setHideOnFocusLoss(bool hideOnFocusLoss) {
    plugin_->settings().setValue(CFG_HIDEONFOCUSLOSS, hideOnFocusLoss);
    hideOnFocusLoss_ = hideOnFocusLoss;
}


/** ***************************************************************************/
bool QmlBoxModel::MainWindow::showCentered() const {
    return showCentered_;
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setShowCentered(bool showCentered) {
    plugin_->settings().setValue(CFG_CENTERED, showCentered);
    showCentered_ = showCentered;
}


/** ***************************************************************************/
bool QmlBoxModel::MainWindow::hideOnClose() const {
    return hideOnClose_;
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setHideOnClose(bool hideOnClose) {
    plugin_->settings().setValue(CFG_HIDE_ON_CLOSE, hideOnClose);
    hideOnClose_ = hideOnClose;
}


/** ***************************************************************************/
bool QmlBoxModel::MainWindow::clearOnHide() const {
    return clearOnHide_;
}


/** ***************************************************************************/
void QmlBoxModel::MainWindow::setClearOnHide(bool b) {
    plugin_->settings().setValue(CFG_CLEAR_ON_HIDE, b);
    clearOnHide_ = b;
}
