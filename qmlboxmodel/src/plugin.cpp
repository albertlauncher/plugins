// Copyright (c) 2023 Manuel Schneider

#include "albert/albert.h"
#include "albert/extension/frontend/query.h"
#include "albert/logging.h"
#include "imageprovider.h"
#include "plugin.h"
#include "propertyeditor.h"
#include "ui_configwidget.h"
#include <QApplication>
#include <QCursor>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QFocusEvent>
#include <QInputDialog>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>
#include <QQmlComponent>
#include <QQmlContext>
#include <QQmlEngine>
#include <QQuickItem>
#include <QSettings>
#include <QShortcut>
#include <QStandardItemModel>
#include <QStandardPaths>
ALBERT_LOGGING_CATEGORY("qml")
using namespace std;
using namespace albert;

namespace {
static const char* CFG_WND_POS = "windowPosition";
static const char* DEFAULT_STYLE_FILE_PATH = "qrc:/DefaultStyle.qml";
static const char* PREF_OBJ_NAME = "style";
static const char* INPUT_OBJ_NAME = "input";
}

Plugin::Plugin():
    qml_interface_(this)
{
    // WINDOW

    setColor(Qt::transparent);
    setFlags(Qt::Tool
             | Qt::WindowStaysOnTopHint
             | Qt::FramelessWindowHint
             | Qt::WindowCloseButtonHint // No close event w/o this
             );

    auto s = settings();
    setPosition(s->value(CFG_WND_POS).toPoint());
    restore_clearOnHide();
    restore_followMouse();
    restore_showCentered();
    restore_hideOnClose();
    restore_hideOnFocusLoss();
    restore_displaySystemShadow();
    restore_alwaysOnTop();

    // ENGINE

    qmlRegisterUncreatableType<albert::Query*>("Albert", 1, 0, "Query", "");
    engine_.addImageProvider(QLatin1String("albert"), image_provider_ = new ImageProvider); // The QQmlEngine takes ownership of provider.
    connect(&engine_, &QQmlEngine::quit, qApp, &QApplication::quit);

    // CONTEXT

    auto *root_context = engine_.rootContext();
    root_context->setContextProperty("albert", &qml_interface_);
    root_context->setContextProperty("history", &history_);
    root_context->setContextProperty("mainWindow", this);
    // Hack to get version branching in qml
    root_context->setContextProperty("QT_VERSION_MAJOR", QT_VERSION_MAJOR);
    root_context->setContextProperty("QT_VERSION_MINOR", QT_VERSION_MINOR);
}

Plugin::~Plugin() { settings()->setValue(CFG_WND_POS, position()); }

void Plugin::initialize(ExtensionRegistry &registry, map<QString,PluginInstance*> dependencies)
{
    PluginInstance::initialize(registry, dependencies);

    // Load in init because components may call query(), while QueryEngine is not set in ctor
    loadRootComponent(DEFAULT_STYLE_FILE_PATH);
}

void Plugin::loadRootComponent(const QString &path)
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


bool Plugin::event(QEvent *event)
{
    switch (event->type())
    {
    // Quit on Alt+F4
    case QEvent::Close:
        ( hideOnClose_ ) ? setVisible(false) : qApp->quit();
        return true;

    case QEvent::FocusOut:
        if (hideOnFocusLoss_)
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
        if (clearOnHide_)
            this->setInput("");
        break;

    case QEvent::Show:
        if (showCentered_ || !screen()) {
            QScreen *screen = nullptr;
            if (!followMouse_ || !(screen = QGuiApplication::screenAt(QCursor::pos())))
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

bool Plugin::syntheticVimNavigation(QKeyEvent *keyEvent)
{
    switch (keyEvent->key()) {
    case Qt::Key_H: return sendSyntheticKey(keyEvent, Qt::Key_Left);
    case Qt::Key_J: return sendSyntheticKey(keyEvent, Qt::Key_Down);
    case Qt::Key_K: return sendSyntheticKey(keyEvent, Qt::Key_Up);
    case Qt::Key_L: return sendSyntheticKey(keyEvent, Qt::Key_Right);
    default: return false;
    }
}

bool Plugin::syntheticEmacsNavigation(QKeyEvent *keyEvent)
{
    switch (keyEvent->key()) {
    case Qt::Key_N: return sendSyntheticKey(keyEvent, Qt::Key_Down);
    case Qt::Key_P: return sendSyntheticKey(keyEvent, Qt::Key_Up);
    default: return false;
    }
}

bool Plugin::sendSyntheticKey(QKeyEvent *event, int key)
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

QObject *Plugin::getThemePropertiesObject() const
{
    if (auto obj = root_object_->findChild<QObject*>(PREF_OBJ_NAME); obj)
        return obj;
    else
        qFatal("Failed to get style object");
}

QStringList Plugin::settableThemeProperties()
{
    if(const QMetaObject *meta_obj = getThemePropertiesObject()->metaObject(); !meta_obj)
        qFatal("Failed to get style meta object");
    else {
        QStringList settableProperties;
        for (int i = 0; i < meta_obj->propertyCount(); i++)
            settableProperties.append(meta_obj->property(i).name());
        settableProperties.removeAll("objectName");
        return settableProperties;
    }
}

QVariant Plugin::property(const QString &name) const
{
    return getThemePropertiesObject()->property(name.toLocal8Bit().constData());
}

void Plugin::setProperty(const QString &name, const QVariant &value)
{
    getThemePropertiesObject()->setProperty(name.toLocal8Bit().constData(), value);

//    CRIT << name << value;
    settings()->setValue(name, value);
//    engine_->evaluate("console.warn(root.findChild('style').result_item_subtext_color)");
}

QFileInfoList Plugin::availableThemes() const
{
    QFileInfoList theme_files;
    theme_files << QFileInfo(":SystemPaletteTheme.js");
    theme_files << QFileInfo(":SystemPaletteTheme2.js");
    theme_files << dataDir().entryInfoList({"*.theme"}, QDir::Files);
    return theme_files;
}

void Plugin::saveThemeAsFile(const QString &theme_name)
{
    QSettings theme_file(configDir().filePath(theme_name+".theme"), QSettings::IniFormat);
    auto *theme_properties_object = getThemePropertiesObject();
    for (const QString &prop : settableThemeProperties())
        theme_file.setValue(prop, theme_properties_object->property(prop.toLatin1().data()));
}

#include <QJSValueIterator>
void Plugin::applyTheme(const QString &theme_file_path)
{
    CRIT << theme_file_path;



    QFile file(theme_file_path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Failed to open the file:" << file.errorString();
        return;
    }

    QTextStream in(&file);
    auto jsscript = in.readAll();
    file.close();

    DEBG << jsscript;

//    engine_->globalObject().setProperty("palette", qApp->palette());

//    CRIT << engine_->evaluate("palette.window").toString();
//    CRIT << engine_->globalObject().property("palette").toString();

    QStringList trace;
    QJSValue module = engine_.importModule(theme_file_path);
//    QJSValue module = engine_->evaluate(jsscript, {}, 1, &trace);
    if (module.isError())
        CRIT << "IMPORT ERROR" << module.toString() << trace;
    else
        WARN << module.toString();


    QJSValue func = module.property("theme");
    if (func.isError())
        CRIT  << "PROPERTY ERROR"<< func.toString();
    else
        WARN   << "PROPERTY "<< func.toString();


    QJSValue js_theme_object = func.call();
    if (js_theme_object.isError())
        CRIT << "FUNC CALL ERROR" << js_theme_object.toString();
    else
        WARN <<    "FUNC CALL " << js_theme_object.toString();



    QJSValueIterator it(js_theme_object);
    while (it.hasNext()) {
        it.next();
        CRIT << it.name() << ": " << it.value().toString();
    }

    CRIT <<  js_theme_object.toString();

    auto *style = getThemePropertiesObject();
    for (const auto &property : settableThemeProperties()){
        if (QJSValue value = js_theme_object.property(property); !value.isUndefined()){
            DEBG << property << value.toString();
            style->setProperty(property.toUtf8().constData(), value.toVariant());
        }
    }









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

}


bool Plugin::alwaysOnTop() const { return flags() & Qt::WindowStaysOnTopHint; }
void Plugin::set_alwaysOnTop_(bool value)
{ setFlags(value ? flags() | Qt::WindowStaysOnTopHint : flags() & ~Qt::WindowStaysOnTopHint); }

bool Plugin::displaySystemShadow() const { return !flags().testFlag(Qt::NoDropShadowWindowHint); }
void Plugin::set_displaySystemShadow_(bool value)
{ setFlags(flags().setFlag(Qt::NoDropShadowWindowHint, !value)); }

bool Plugin::isVisible() const { return QQuickWindow::isVisible(); }
void Plugin::setVisible(bool visible)
{
    QQuickWindow::setVisible(visible);
    if (visible){
#if not defined Q_OS_MACOS // steals focus on macos
        raise();
        requestActivate();
#endif
    } else {
        qml_interface_.clearQueries();
    }
}

QString Plugin::input() const { return input_->property("text").toString(); }

void Plugin::setInput(const QString &input) { input_->setProperty("text", input); }

unsigned long long Plugin::winId() const { return QQuickWindow::winId(); }

QWidget* Plugin::createFrontendConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    ALBERT_PLUGIN_PROPERTY_CONNECT(this, followMouse, ui.checkBox_followMouse, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, hideOnFocusLoss, ui.checkBox_hideOnFocusOut, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, showCentered, ui.checkBox_center, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, hideOnClose, ui.checkBox_hideOnClose, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, clearOnHide, ui.checkBox_clearOnHide, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, displaySystemShadow, ui.checkBox_systemShadow, setChecked, toggled)
    ALBERT_PLUGIN_PROPERTY_CONNECT(this, alwaysOnTop, ui.checkBox_onTop, setChecked, toggled)

    // Themes

    // auto fillThemesCheckBox = [this, cb=ui.comboBox_themes](){
    //     QSignalBlocker b(cb);
    //     cb->clear();
    //     QStandardItemModel *model = qobject_cast<QStandardItemModel*>(cb->model());  // safe, see docs

    //     // Add disabled placeholder item
    //     auto *item = new QStandardItem;
    //     item->setText("Choose theme...");
    //     item->setEnabled(false);
    //     model->appendRow(item);

    //     cb->insertSeparator(1);

    //     // Add themes
    //     for (const QFileInfo &fi : availableThemes()){
    //         item = new QStandardItem;
    //         item->setText(fi.baseName());
    //         item->setToolTip(fi.absoluteFilePath());
    //         model->appendRow(item);
    //     }
    // };

    // fillThemesCheckBox();

    // connect(ui.comboBox_themes, &QComboBox::currentIndexChanged,
    //         this, [this, cb=ui.comboBox_themes](int i){
    //             auto theme_file_name = cb->model()->index(i,0).data(Qt::ToolTipRole).toString();
    //             applyTheme(theme_file_name);
    //         });

    // connect(ui.toolButton_propertyEditor, &QToolButton::clicked, this, [this, w](){
    //     PropertyEditor *pe = new PropertyEditor(this, w);
    //     pe->setWindowModality(Qt::WindowModality::WindowModal);
    //     pe->show();
    // });

    // connect(ui.toolButton_save, &QToolButton::clicked, this, [this, w, fillThemesCheckBox](){
    //     if (auto text = QInputDialog::getText(w, qApp->applicationDisplayName(), "Theme name:"); !text.isNull()){
    //         if (text.isEmpty())
    //             QMessageBox::warning(w, qApp->applicationDisplayName(), "Theme name must not be empty.");
    //         else if (auto dir = configDir(); dir.exists(text+".theme"))
    //             QMessageBox::warning(w, qApp->applicationDisplayName(), "Theme already exists.");
    //         else{
    //             saveThemeAsFile(dir.filePath(text));
    //             fillThemesCheckBox();
    //         }
    //     }
    // });


    return w;
}

///////////////////////////////////////////////////////////////////////////////

QmlInterface::QmlInterface(Plugin *plugin) : plugin_(plugin) { }

void QmlInterface::showSettings()
{
    plugin_->setVisible(false);
    albert::showSettings();
}

QObject *QmlInterface::query(const QString &query)
{
    if (!queries_.empty()){
        auto *q = queries_.back().get();
        q->disconnect(this);
        q->matches()->disconnect(this);
        q->cancel();
    }

    auto *q = queries_.emplace_back(plugin_->query(query)).get();
    q->setParent(this);  // important for qml ownership determination
    emit currentQueryChanged();

    connect(q, &albert::Query::finished, this, &QmlInterface::onQueryReady);
    connect(q->matches(), &QAbstractItemModel::rowsInserted, this, &QmlInterface::onQueryReady);
    connect(q, &albert::Query::finished, this, &QmlInterface::currentQueryFinished);

    return q;
}

QObject *QmlInterface::currentQuery()
{
    return (queries_.empty()) ? nullptr : queries_.back().get();
}

void QmlInterface::clearQueries()
{
    if (!queries_.empty()){
        auto *q = queries_.back().get();
        q->disconnect(this);
        q->matches()->disconnect(this);
        q->cancel();
        queries_.clear();
    }
    emit currentQueryChanged();
}

QString QmlInterface::kcString(int kc)
{
    return QKeySequence(kc).toString();
}

void QmlInterface::onQueryReady()
{
    auto *q = queries_.back().get();
    disconnect(q, &albert::Query::finished, this, &QmlInterface::onQueryReady);
    disconnect(q->matches(), &QAbstractItemModel::rowsInserted, this, &QmlInterface::onQueryReady);
    emit currentQueryReady();
}

void QmlInterface::onQueryFinished()
{
    auto *q = queries_.back().get();
    disconnect(q, &albert::Query::finished, this, &QmlInterface::currentQueryFinished);
    emit currentQueryFinished();
}

