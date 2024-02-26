// Copyright (c) 2023-2024 Manuel Schneider

#include "albert/albert.h"
// #include "albert/extension/frontend/frontend.h"
#include "albert/extension/frontend/query.h"
#include "albert/logging.h"
// #include "albert/plugin.h"
#include "imageprovider.h"
// #include "imageprovider.h"
// #include "plugin.h"
#include "window.h"
#include "qmlinterface.h"
// #include "propertyeditor.h"
// #include "ui_configwidget.h"
#include <QApplication>
// #include <QCursor>
// #include <QDebug>
// #include <QDir>
// #include <QDirIterator>
// #include <QFileInfo>
// #include <QFocusEvent>
// #include <QInputDialog>
// #include <QJsonDocument>
// #include <QJsonObject>
// #include <QMessageBox>
// #include <QPoint>
// #include <QQmlComponent>
#include <QQmlContext>
// #include <QQmlEngine>
#include <QQuickItem>
// #include <QSettings>
// #include <QSettings>
// #include <QShortcut>
// #include <QStandardItemModel>
// #include <QStandardPaths>
// #include <memory>
// #include <vector>
// class Plugin;
// class QAbstractItemModel;
// class QQmlEngine;
// class QQuickWindow;
// class QueryExecution;
ALBERT_LOGGING_CATEGORY("qml")
using namespace std;
using namespace albert;

namespace {
static const char* DEFAULT_STYLE_FILE_PATH = "qrc:/DefaultStyle.qml";
static const char* PREF_OBJ_NAME = "style";
static const char* INPUT_OBJ_NAME = "input";
}

Window::Window(QmlInterface &qmlif)
{
    // WINDOW

    setColor(Qt::transparent);
    setFlags(Qt::Tool
             | Qt::WindowStaysOnTopHint
             | Qt::FramelessWindowHint
             | Qt::WindowCloseButtonHint // No close event w/o this
             );

    // ENGINE

    qmlRegisterUncreatableType<albert::Query*>("Albert", 1, 0, "Query", "");
    engine_.addImageProvider(QLatin1String("albert"), image_provider_ = new ImageProvider); // The QQmlEngine takes ownership of provider.
    connect(&engine_, &QQmlEngine::quit, qApp, &QApplication::quit);

    // CONTEXT

    auto *root_context = engine_.rootContext();
    root_context->setContextProperty("albert", &qmlif);
    root_context->setContextProperty("history", &history_);
    root_context->setContextProperty("mainWindow", this);
    // Hack to get version branching in qml
    root_context->setContextProperty("QT_VERSION_MAJOR", QT_VERSION_MAJOR);
    root_context->setContextProperty("QT_VERSION_MINOR", QT_VERSION_MINOR);


    loadRootComponent(DEFAULT_STYLE_FILE_PATH);
}

Window::~Window() {  }

QString Window::input() const
{ return input_->property("text").toString(); }

void Window::setInput(const QString &input)
{ input_->setProperty("text", input); }

// void Window::setVisible(bool visible)
// {
//     QQuickWindow::setVisible(visible);
//     if (visible){
// #if not defined Q_OS_MACOS // steals focus on macos
//         raise();
//         requestActivate();
// #endif
//     } else {
//         qml_interface_.clearQueries();
//     }
// }

void Window::loadRootComponent(const QString &path)
{
    auto root_component = make_unique<QQmlComponent>(&engine_, path, QQmlComponent::PreferSynchronous);
    if (root_component->isError())
        qFatal("Failed loading QML component\n%s", root_component->errorString().toLatin1().constData());

    root_object_.reset(qobject_cast<QQuickItem*>(root_component->createWithInitialProperties({}, engine_.rootContext())));
    root_object_->setParentItem(contentItem());

    connect(root_object_.get(), &QQuickItem::widthChanged, this, [this](){ setWidth(root_object_->width()); });
    connect(root_object_.get(), &QQuickItem::heightChanged, this, [this](){ setHeight(root_object_->height()); });
    setWidth(root_object_->width());
    setHeight(root_object_->height());


    if (!(input_ = root_object_->findChild<QObject*>(INPUT_OBJ_NAME)))
        qFatal("Failed to get object: %s", INPUT_OBJ_NAME);

    connect(input_, SIGNAL(textChanged()),
            this, SIGNAL(inputChanged()));

//    applyTheme(availableThemes()[0].filePath());


//    qApp->installEventFilter(this);



    //    applyTheme(":Default.theme");

    //    // Load theme
    //    auto theme_prop_obj = getThemePropertiesObject();
    //    for (const auto &property : settableThemeProperties())
    //        if (s->contains(property))
    //            theme_prop_obj->setProperty(property.toLatin1().constData(), s->value(property));

    //    PropertyEditor *pe = new PropertyEditor(this);
    //    pe->setWindowModality(Qt::WindowModality::WindowModal);
    //    pe->show();

//    connect(this, &QQuickWindow::activeFocusItemChanged, [this](){
//        DEBG << "activeFocusItemChanged" << activeFocusItem();
//    });
}



//bool Plugin::eventFilter(QObject *obj, QEvent *event)
//{

//    if (event->type() == QEvent::Timer)
//        return false;
//    if (event->type() == QEvent::UpdateRequest)
//        return false;

//    DEBG << "eventFilter: " << obj << obj->objectName() << event->type();

//    if (event->type() == QEvent::KeyPress){
//        auto *keyEvent = static_cast<QKeyEvent*>(event);
//        CRIT << "APPWIDE C++ PRESS" << QKeySequence(keyEvent->keyCombination()).toString();
//    }

//    return QObject::eventFilter(obj, event);
//}


bool Window::event(QEvent *event)
{
    switch (event->type())
    {
    // Quit on Alt+F4
    case QEvent::Close:
        (hide_on_close) ? setVisible(false) : qApp->quit();
        return true;

    case QEvent::FocusOut:
        if (hide_on_focus_loss)
            albert::hide();  // calls setvisible and therefore deactivates
        break;

//    case QEvent::Move: {
//        auto *moveEvent = static_cast<QMoveEvent*>(event);
//        DEBG << "moveEvent" << moveEvent->oldPos() << ">" << moveEvent->pos();
//        break;
//    }

    case QEvent::MouseButtonPress:
        if (static_cast<QMouseEvent*>(event)->modifiers() == Qt::ControlModifier)
            clickOffset_ = static_cast<QMouseEvent*>(event)->pos();
        break;

    case QEvent::MouseButtonRelease:
        clickOffset_ = QPoint();  // isNull
        break;

    case QEvent::MouseMove:
        if (!clickOffset_.isNull())
            setPosition(static_cast<QMouseEvent*>(event)->globalPosition().toPoint() - clickOffset_);
        break;

    case QEvent::Hide:
        image_provider_->clearCache();
        if (clear_on_hide)
            this->setInput("");
        break;

    case QEvent::Show:
        if (show_centered || !screen()) {
            QScreen *screen = nullptr;
            if (!follow_mouse || !(screen = QGuiApplication::screenAt(QCursor::pos())))
                screen = QGuiApplication::primaryScreen();
            auto geo = screen->geometry();
            auto win_width = width();
            auto newX = geo.center().x() - win_width / 2;
            auto newY = geo.top() + geo.height() / 8;
            DEBG << screen->name() << screen->manufacturer() << screen->model() << screen->devicePixelRatio() << geo;
            DEBG << "win_width" << win_width  << "newX" << newX << "newY" << newY;
            setPosition(newX, newY);
        }
        break;

    case QEvent::KeyPress:{
        auto *keyEvent = static_cast<QKeyEvent*>(event);
        //CRIT << "C++ PRESS" << QKeySequence(keyEvent->keyCombination()).toString();
        if (keyEvent->modifiers() == Qt::NoModifier && keyEvent->key() == Qt::Key_Escape ){
            setVisible(false);
            return true;
        }
        break;
    }

    case QEvent::KeyRelease:{
        auto *keyEvent = static_cast<QKeyEvent*>(event);
        //CRIT << "C++ RELEASE" << QKeySequence(keyEvent->keyCombination()).toString();
        // Unfortunately Cmd+Alt seems to be swallowed by macos inputs
        // Synthesize keypresses on keyrelease
        // limitation:  no autorepeat
        if (keyEvent->modifiers() & Qt::ControlModifier){  // Nerd key nav
            if (cm_vim && syntheticVimNavigation(keyEvent)) return true;
            if (cm_emacs && syntheticEmacsNavigation(keyEvent)) return true;
        }
        break;
    }

//    case QEvent::Shortcut:{
//        auto *e = static_cast<QShortcutEvent*>(event);
//        CRIT << "Shortcut"<<  e->key().toString();
//        return true;
//    }

//    case QEvent::ShortcutOverride:{
//        auto *e = static_cast<QKeyEvent*>(event);
//        CRIT << "ShortcutOverride" << QKeySequence(e->keyCombination()).toString();
//        return true;
//        break;
//    }

//    case QEvent::UpdateRequest:
//        break; // mute debug

    default:
//        DEBG << "QEvent " << event->type();
        break;
    }

    return QQuickWindow::event(event);
}

bool Window::syntheticVimNavigation(QKeyEvent *keyEvent)
{
    switch (keyEvent->key()) {
    case Qt::Key_H: return sendSyntheticKey(keyEvent, Qt::Key_Left);
    case Qt::Key_J: return sendSyntheticKey(keyEvent, Qt::Key_Down);
    case Qt::Key_K: return sendSyntheticKey(keyEvent, Qt::Key_Up);
    case Qt::Key_L: return sendSyntheticKey(keyEvent, Qt::Key_Right);
    default: return false;
    }
}

bool Window::syntheticEmacsNavigation(QKeyEvent *keyEvent)
{
    switch (keyEvent->key()) {
    case Qt::Key_N: return sendSyntheticKey(keyEvent, Qt::Key_Down);
    case Qt::Key_P: return sendSyntheticKey(keyEvent, Qt::Key_Up);
    default: return false;
    }
}

bool Window::sendSyntheticKey(QKeyEvent *event, int key)
{
    QKeyEvent synth(QEvent::KeyPress,  //event->type(), hackis
                    key,
                    event->modifiers() & ~Qt::ControlModifier,
                    event->nativeScanCode(),
                    event->nativeVirtualKey(),
                    event->nativeModifiers(),
                    event->text(),
                    event->isAutoRepeat(),
                    event->count(),
                    event->device());
    return QApplication::sendEvent(this, &synth);
}

QObject *Window::getThemePropertiesObject() const
{
    if (auto obj = root_object_->findChild<QObject*>(PREF_OBJ_NAME); obj)
        return obj;
    else
        qFatal("Failed to get style object");
}

// QStringList Window::settableThemeProperties()
// {
//     if(const QMetaObject *meta_obj = getThemePropertiesObject()->metaObject(); !meta_obj)
//         qFatal("Failed to get style meta object");
//     else {
//         QStringList settableProperties;
//         for (int i = 0; i < meta_obj->propertyCount(); i++)
//             settableProperties.append(meta_obj->property(i).name());
//         settableProperties.removeAll("objectName");
//         return settableProperties;
//     }
// }

// QVariant Window::property(const QString &name) const
// {
//     return getThemePropertiesObject()->property(name.toLocal8Bit().constData());
// }

// void Window::setProperty(const QString &name, const QVariant &value)
// {
//     getThemePropertiesObject()->setProperty(name.toLocal8Bit().constData(), value);

// //    CRIT << name << value;
//     settings()->setValue(name, value);
// //    engine_->evaluate("console.warn(root.findChild('style').result_item_subtext_color)");
// }

// QFileInfoList Window::availableThemes() const
// {
//     QFileInfoList theme_files;
//     theme_files << QFileInfo(":SystemPaletteTheme.js");
//     theme_files << QFileInfo(":SystemPaletteTheme2.js");
//     theme_files << dataDir().entryInfoList({"*.theme"}, QDir::Files);
//     return theme_files;
// }

// void Window::saveThemeAsFile(const QString &theme_name)
// {
//     QSettings theme_file(configDir().filePath(theme_name+".theme"), QSettings::IniFormat);
//     auto *theme_properties_object = getThemePropertiesObject();
//     for (const QString &prop : settableThemeProperties())
//         theme_file.setValue(prop, theme_properties_object->property(prop.toLatin1().data()));
// }

// #include <QJSValueIterator>
// void Window::applyTheme(const QString &theme_file_path)
// {
//     CRIT << theme_file_path;



//     QFile file(theme_file_path);
//     if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
//         qDebug() << "Failed to open the file:" << file.errorString();
//         return;
//     }

//     QTextStream in(&file);
//     auto jsscript = in.readAll();
//     file.close();

//     DEBG << jsscript;

// //    engine_->globalObject().setProperty("palette", qApp->palette());

// //    CRIT << engine_->evaluate("palette.window").toString();
// //    CRIT << engine_->globalObject().property("palette").toString();

//     QStringList trace;
//     QJSValue module = engine_.importModule(theme_file_path);
// //    QJSValue module = engine_->evaluate(jsscript, {}, 1, &trace);
//     if (module.isError())
//         CRIT << "IMPORT ERROR" << module.toString() << trace;
//     else
//         WARN << module.toString();


//     QJSValue func = module.property("theme");
//     if (func.isError())
//         CRIT  << "PROPERTY ERROR"<< func.toString();
//     else
//         WARN   << "PROPERTY "<< func.toString();


//     QJSValue js_theme_object = func.call();
//     if (js_theme_object.isError())
//         CRIT << "FUNC CALL ERROR" << js_theme_object.toString();
//     else
//         WARN <<    "FUNC CALL " << js_theme_object.toString();



//     QJSValueIterator it(js_theme_object);
//     while (it.hasNext()) {
//         it.next();
//         CRIT << it.name() << ": " << it.value().toString();
//     }

//     CRIT <<  js_theme_object.toString();

//     auto *style = getThemePropertiesObject();
//     for (const auto &property : settableThemeProperties()){
//         if (QJSValue value = js_theme_object.property(property); !value.isUndefined()){
//             DEBG << property << value.toString();
//             style->setProperty(property.toUtf8().constData(), value.toVariant());
//         }
//     }









//    QJSValue module = engine_->importModule(theme_file_path);
//    if (module.isError())
//        WARN << module.toString();

//    QJSValue func = module.property("applyTheme");
//    if (func.isError())
//        WARN << func.toString();

//    QJSValue result = func.call({engine_->newQObject(getThemePropertiesObject())});
//    if (result.isError())
//        WARN << result.toString();


//    QJSValue fun = engine_->evaluate("(function(style) { style.window_background_color='#000000'; })");
//    QJSValue threeAgain = fun.call({engine_->newQObject(getThemePropertiesObject())});




//    QSettings theme_file(theme_file_path, QSettings::IniFormat);

//    CRIT << theme_file_path << theme_file.allKeys();

//    for (const QString &prop : settableThemeProperties())
//        if (theme_file.contains(prop))
//            setProperty(prop, theme_file.value(prop));
//    //    QMetaObject::invokeMethod(root_object_, "setTheme", Q_ARG(QVariant, QVariant::fromValue(name)));









//    auto style = getThemePropertiesObject();
//    QJSValue js_style_object = engine_->evaluate("var style={}; style.window_background_color='#000000'; return style;");


//    for (const auto &property : js_style_object.proper){

//        for (const auto &property : settableThemeProperties()){
//            if (QJSValue value = js_style_object.property(property); !value.isNull()){
//                DEBG << property << value.toString();
//                style->setProperty(property.toUtf8().constData(), value.toVariant());
//            }

//        }


        //    if (auto *style = root_object_->findChild<QObject*>("style"); style){



        ////        auto ret = engine_->evaluate("root.style.window_background_color='#000000'; console.warn('hello')");
        ////        CRIT << ret.isError() << ret.toString();

        ////        QJSValue fun = engine_->evaluate("(function(style) { style.window_background_color='#000000'; })");
        ////        QJSValue threeAgain = fun.call({engine_->newQObject(style)});


        //        QJSValue styleJSValue = engine_->evaluate("var style={}; style.window_background_color='#000000'; return style;");
        //        auto *style2 = styleJSValue.toQObject();



        //    }

// }
