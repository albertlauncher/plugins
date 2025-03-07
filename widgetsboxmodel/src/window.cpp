// Copyright (c) 2022-2025 Manuel Schneider

#include "actiondelegate.h"
#include "debugoverlay.h"
#include "inputline.h"
#include "itemdelegate.h"
#include "resizinglist.h"
#include "resultitemmodel.h"
#include "settingsbutton.h"
#include "statetransitions.h"
#include "window.h"
#include <QApplication>
#include <QBoxLayout>
#include <QDir>
#include <QFrame>
#include <QGraphicsEffect>
#include <QKeyEvent>
#include <QMenu>
#include <QMessageBox>
#include <QMouseEvent>
#include <QPixmapCache>
#include <QPropertyAnimation>
#include <QSettings>
#include <QSignalBlocker>
#include <QStandardPaths>
#include <QStateMachine>
#include <QStringListModel>
#include <QStyleFactory>
#include <QTimer>
#include <QWindow>
#include <albert/albert.h>
#include <albert/logging.h>
#include <albert/plugininstance.h>
#include <albert/pluginloader.h>
#include <albert/pluginmetadata.h>
#include <albert/query.h>
using namespace albert;
using namespace std;



namespace  {

const uint    DEF_SHADOW_SIZE = 32;  // TODO user
const char*   STATE_WND_POS  = "windowPosition";

static const bool  DEF_ALWAYS_ON_TOP      = true;
static const bool  DEF_CENTERED           = true;
static const bool  DEF_CLEAR_ON_HIDE      = true;
static const bool  DEF_DEBUG              = false;
static const bool  DEF_DISPLAY_SCROLLBAR  = false;
static const bool  DEF_FOLLOW_CURSOR      = true;
static const bool  DEF_HIDE_ON_FOCUS_LOSS = true;
static const bool  DEF_HISTORY_SEARCH     = true;
static const bool  DEF_QUIT_ON_CLOSE      = false;
static const bool  DEF_SHADOW_CLIENT      = true;
static const bool  DEF_SHADOW_SYSTEM      = false;
static const char* DEF_THEME_DARK         = "Default System Palette";
static const char* DEF_THEME_LIGHT        = "Default System Palette";
static const uint  DEF_MAX_RESULTS        = 5;

static const char *CFG_ALWAYS_ON_TOP      = "alwaysOnTop";
static const char *CFG_CENTERED           = "showCentered";
static const char *CFG_CLEAR_ON_HIDE      = "clearOnHide";
static const char *CFG_DEBUG              = "debug";
static const char *CFG_DISPLAY_SCROLLBAR  = "displayScrollbar";
static const char *CFG_FOLLOW_CURSOR      = "followCursor";
static const char *CFG_HIDE_ON_FOCUS_LOSS = "hideOnFocusLoss";
static const char *CFG_HISTORY_SEARCH     = "historySearch";
static const char *CFG_MAX_RESULTS        = "itemCount";
static const char *CFG_QUIT_ON_CLOSE      = "quitOnClose";
static const char *CFG_SHADOW_CLIENT      = "clientShadow";
static const char *CFG_SHADOW_SYSTEM      = "systemShadow";
static const char *CFG_THEME_DARK         = "darkTheme";
static const char *CFG_THEME_LIGHT        = "lightTheme";

//constexpr Qt::KeyboardModifier mods_mod[] = {
//    Qt::ShiftModifier,
//    Qt::MetaModifier,
//    Qt::ControlModifier,
//    Qt::AltModifier
//};

constexpr Qt::Key mods_keys[] = {
    Qt::Key_Shift,
    Qt::Key_Meta,
    Qt::Key_Control,
    Qt::Key_Alt
};



static bool haveDarkSystemPalette()
{
    auto pal = QApplication::style()->standardPalette();
    return pal.color(QPalette::WindowText).lightness()
           > pal.color(QPalette::Window).lightness();
}

static map<QString, QString> findThemes(const QString &plugin_id)
{
    map<QString, QString> themes;

    QStringList pluginDataPaths = QStandardPaths::locateAll(
        QStandardPaths::AppDataLocation, plugin_id, QStandardPaths::LocateDirectory);

    for (const QString &pluginDataPath : pluginDataPaths)
        for (const auto &file_info : QDir(QString("%1/themes").arg(pluginDataPath)).entryInfoList(QStringList("*.qss"), QDir::Files | QDir::NoSymLinks))
            themes.emplace(file_info.baseName(), file_info.canonicalFilePath());

    if (themes.empty())
        throw runtime_error("No theme files found.");

    return themes;
}

}

Window::Window(PluginInstance *p):
    themes(findThemes(p->loader().metaData().id)),
    plugin(p),
    frame(new QFrame(this)),
    input_line(new InputLine(frame)),
    settings_button(new SettingsButton(this)),
    results_list(new ResizingList(frame)),
    actions_list(new ResizingList(frame)),
    item_delegate(new ItemDelegate(results_list)),
    action_delegate(new ActionDelegate(actions_list)),
    dark_mode(haveDarkSystemPalette()),
    current_query{nullptr}
{
    // Setup UI
    {

        results_list->setItemDelegate(item_delegate);
        actions_list->setItemDelegate(action_delegate);

        auto *window_layout = new QVBoxLayout(this);
        window_layout->addWidget(frame);

        auto *frame_layout = new QVBoxLayout(frame);
        frame_layout->addWidget(input_line,0); //, 0, Qt::AlignTop);
        frame_layout->addWidget(results_list,0); //, 0, Qt::AlignTop);
        frame_layout->addWidget(actions_list,0); //, 1, Qt::AlignTop);
        frame_layout->addStretch(1);

        // Identifiers for stylesheets
        frame->setObjectName("frame");
        settings_button->setObjectName("settingsButton");
        input_line->setObjectName("inputLine");
        results_list->setObjectName("resultsList");
        actions_list->setObjectName("actionList");

        window_layout->setContentsMargins(0,0,0,0);
        frame_layout->setContentsMargins(0,0,0,0);

        window_layout->setSizeConstraint(QLayout::SetFixedSize);

        input_line->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        results_list->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        actions_list->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

        settings_button->setFocusPolicy(Qt::NoFocus);
        results_list->setFocusPolicy(Qt::NoFocus);
        actions_list->setFocusPolicy(Qt::NoFocus);
        actions_list->setEditTriggers(QAbstractItemView::NoEditTriggers);

        results_list->hide();
        actions_list->hide();

        setWindowFlags(Qt::Tool|Qt::FramelessWindowHint);
        setAttribute(Qt::WA_TranslucentBackground);

        input_line->installEventFilter(this);

        // reproducible UX
        setStyle(QStyleFactory::create("Fusion"));

        connect(input_line, &InputLine::textChanged, this, &Window::inputChanged);
    }

    // Settings
    {
        auto s = plugin->settings();
        setAlwaysOnTop(s->value(CFG_ALWAYS_ON_TOP, DEF_ALWAYS_ON_TOP).toBool());
        setClearOnHide(s->value(CFG_CLEAR_ON_HIDE, DEF_CLEAR_ON_HIDE).toBool());
        setDisplayClientShadow(s->value(CFG_SHADOW_CLIENT, DEF_SHADOW_CLIENT).toBool());
        setDisplayScrollbar(s->value(CFG_DISPLAY_SCROLLBAR, DEF_DISPLAY_SCROLLBAR).toBool());
        setDisplaySystemShadow(s->value(CFG_SHADOW_SYSTEM, DEF_SHADOW_SYSTEM).toBool());
        setFollowCursor(s->value(CFG_FOLLOW_CURSOR, DEF_FOLLOW_CURSOR).toBool());
        setHideOnFocusLoss(s->value(CFG_HIDE_ON_FOCUS_LOSS, DEF_HIDE_ON_FOCUS_LOSS).toBool());
        setHistorySearchEnabled(s->value(CFG_HISTORY_SEARCH, DEF_HISTORY_SEARCH).toBool());
        setMaxResults(s->value(CFG_MAX_RESULTS, DEF_MAX_RESULTS).toUInt());
        setQuitOnClose(s->value(CFG_QUIT_ON_CLOSE, DEF_QUIT_ON_CLOSE).toBool());
        setShowCentered(s->value(CFG_CENTERED, DEF_CENTERED).toBool());
        setDebugMode(s->value(CFG_DEBUG, DEF_DEBUG).toBool());

        try {
            setThemeLight(s->value(CFG_THEME_LIGHT, DEF_THEME_LIGHT).toString());
        } catch (const out_of_range &) {
            setThemeLight(themes.begin()->first);  // okay, we know there is at least one theme
        }

        try {
            setThemeDark(s->value(CFG_THEME_DARK, DEF_THEME_DARK).toString());
        } catch (const out_of_range &) {
            setThemeDark(themes.begin()->first);  // okay, we know there is at least one theme
        }
    }

    // Actions
    {
        auto *a = new QAction(tr("Settings"), this);
        a->setShortcuts({QKeySequence("Ctrl+,")});
        a->setShortcutVisibleInContextMenu(true);
        addAction(a);
        connect(a, &QAction::triggered, this, [] { albert::showSettings(); });

        a = new QAction(tr("Hide on focus out"), this);
        a->setShortcuts({QKeySequence("Meta+h")});
        a->setShortcutVisibleInContextMenu(true);
        a->setCheckable(true);
        a->setChecked(hideOnFocusLoss());
        addAction(a);
        connect(a, &QAction::toggled, this, &Window::setHideOnFocusLoss);
        connect(this, &Window::hideOnFocusLossChanged, a, &QAction::setChecked);

        a = new QAction(tr("Show centered"), this);
        a->setShortcuts({QKeySequence("Meta+c")});
        a->setShortcutVisibleInContextMenu(true);
        a->setCheckable(true);
        a->setChecked(showCentered());
        addAction(a);
        connect(a, &QAction::toggled, this, &Window::setShowCentered);
        connect(this, &Window::showCenteredChanged, a, &QAction::setChecked);

        a = new QAction(tr("Clear on hide"), this);
        a->setShortcuts({QKeySequence("Meta+Alt+c")});
        a->setShortcutVisibleInContextMenu(true);
        a->setCheckable(true);
        a->setChecked(clearOnHide());
        addAction(a);
        connect(a, &QAction::toggled, this, &Window::setClearOnHide);
        connect(this, &Window::clearOnHideChanged, a, &QAction::setChecked);

        a = new QAction(tr("Debug mode"), this);
        a->setShortcuts({QKeySequence("Meta+d")});
        a->setShortcutVisibleInContextMenu(true);
        a->setCheckable(true);
        a->setChecked(debugMode());
        addAction(a);
        connect(a, &QAction::toggled, this, &Window::setDebugMode);
        connect(this, &Window::debugModeChanged, a, &QAction::setChecked);

        // mouse clicks
        connect(settings_button, &SettingsButton::clicked, this, [this](Qt::MouseButton b){
            if (b == Qt::LeftButton)
                albert::showSettings();
            else if (b == Qt::RightButton){
                auto *menu = new QMenu(this);
                menu->addActions(actions());
                menu->setAttribute(Qt::WA_DeleteOnClose);
                menu->popup(QCursor::pos());
            }
        });
    }

    // State
    {
        auto s = plugin->state();
        if (!showCentered()
            && s->contains(STATE_WND_POS)
            && s->value(STATE_WND_POS).canConvert(QMetaType(QMetaType::QPoint)))
           move(s->value(STATE_WND_POS).toPoint());
    }

    init_statemachine();
}

Window::~Window() = default;

void Window::init_statemachine()
{
    //
    // States
    //

    auto *s_root = new QState(QState::ParallelStates);

    auto *s_settings_button = new QState(s_root);
    auto *s_settings_button_hidden = new QState(s_settings_button);
    auto *s_settings_button_shown = new QState(s_settings_button);
    s_settings_button->setInitialState(s_settings_button_hidden);

    auto *s_results = new QState(s_root);

    auto *s_results_query_unset = new QState(s_results);
    auto *s_results_query_set = new QState(s_results);
    s_results->setInitialState(s_results_query_unset);

    auto *s_results_hidden = new QState(s_results_query_set);
    auto *s_results_disabled = new QState(s_results_query_set);
    auto *s_results_matches = new QState(s_results_query_set);
    auto *s_results_fallbacks = new QState(s_results_query_set);
    s_results_query_set->setInitialState(s_results_hidden);

    auto *s_results_match_items = new QState(s_results_matches);
    auto *s_results_match_actions = new QState(s_results_matches);
    s_results_matches->setInitialState(s_results_match_items);

    auto *s_results_fallback_items = new QState(s_results_fallbacks);
    auto *s_results_fallback_actions = new QState(s_results_fallbacks);
    s_results_fallbacks->setInitialState(s_results_fallback_items);

    auto display_delay_timer = new QTimer(this);
    display_delay_timer->setInterval(250);
    display_delay_timer->setSingleShot(true);

    //
    // Debug
    //

    // QObject::connect(s_settings_button_hidden, &QState::entered,
    //                  this, [](){ CRIT << "s_settings_button_hidden::enter"; });
    // QObject::connect(s_settings_button_shown, &QState::entered,
    //                  this, [](){ CRIT << "s_settings_button_shown::enter"; });
    // QObject::connect(s_results_query_unset, &QState::entered,
    //                  this, [](){ CRIT << "s_results_query_unset::enter"; });
    // QObject::connect(s_results_query_set, &QState::entered,
    //                  this, [](){ CRIT << "s_results_query_set::enter"; });
    // QObject::connect(s_results_hidden, &QState::entered,
    //                  this, [](){ CRIT << "s_results_hidden::enter"; });
    // QObject::connect(s_results_disabled, &QState::entered,
    //                  this, [](){ CRIT << "s_results_disabled::enter"; });
    // QObject::connect(s_results_match_items, &QState::entered,
    //                  this, [](){ CRIT << "s_results_match_items::enter"; });
    // QObject::connect(s_results_match_actions, &QState::entered,
    //                  this, [](){ CRIT << "s_results_match_actions::enter"; });
    // QObject::connect(s_results_fallback_items, &QState::entered,
    //                  this, [](){ CRIT << "s_results_fallback_items::enter"; });
    // QObject::connect(s_results_fallback_actions, &QState::entered,
    //                  this, [](){ CRIT << "s_results_fallback_actions::enter"; });

    // connect(input_line, &InputLine::textChanged, [](){ CRIT << "InputLine::textChanged";});
    // connect(this, &Window::queryChanged, [](){ CRIT << "Window::queryChanged";});
    // connect(this, &Window::queryFinished, [](){ CRIT << "Window::queryFinished";});

    //
    // Transitions
    //

    // settingsbutton hidden ->

    addTransition(s_settings_button_hidden, s_settings_button_shown,
                  settings_button, QEvent::Type::Enter);

    addTransition(s_settings_button_hidden, s_settings_button_shown,
                  this, &Window::queryChanged);

    addTransition(s_settings_button_hidden, s_settings_button_shown,
                  this, &Window::queryStateBusy);


    // settingsbutton visible ->

    addTransition(s_settings_button_shown, s_settings_button_hidden,
                  settings_button, QEvent::Type::Leave,
                  [this]{ return !current_query || !current_query->isActive(); });

    addTransition(s_settings_button_shown, s_settings_button_hidden,
                  this, &Window::queryStateIdle,
                  [this]{ return !settings_button->underMouse(); });


    // Query

    addTransition(s_results_query_unset, s_results_query_set,
                  this, &Window::queryChanged,
                  [this]{ return current_query != nullptr; });

    addTransition(s_results_query_set, s_results_query_unset,
                  this, &Window::queryChanged,
                  [this]{ return current_query == nullptr; });


    // hidden ->

    addTransition(s_results_hidden, s_results_matches,
                  this, &Window::queryMatchesAdded);

    addTransition(s_results_hidden, s_results_fallbacks,
                  input_line, QEvent::KeyPress, mods_keys[(int)mod_fallback],
                  [this]{ return current_query->fallbacks().size() > 0; });

    addTransition(s_results_hidden, s_results_fallbacks,
                  this, &Window::queryStateIdle,
                  [this]{ return current_query->fallbacks().size() > 0 && !current_query->isTriggered(); });


    // disabled ->

    addTransition(s_results_disabled, s_results_matches,
                  this, &Window::queryMatchesAdded);

    addTransition(s_results_disabled, s_results_hidden,
                  display_delay_timer, &QTimer::timeout);

    addTransition(s_results_disabled, s_results_hidden,
                  this, &Window::queryStateIdle,
                  [this]{ return current_query->fallbacks().size() == 0 || current_query->isTriggered(); });

    addTransition(s_results_disabled, s_results_fallbacks,
                  this, &Window::queryStateIdle,
                  [this]{ return current_query->fallbacks().size() > 0 && !current_query->isTriggered(); });


    // matches ->

    addTransition(s_results_matches, s_results_disabled,
                  this, &Window::queryChanged,
                  [this]{ return current_query != nullptr; });

    addTransition(s_results_matches, s_results_fallbacks,
                  input_line, QEvent::KeyPress, mods_keys[(int)mod_fallback],
                  [this]{ return current_query->fallbacks().size() > 0; });


    // fallbacks ->

    addTransition(s_results_fallbacks, s_results_disabled,
                  this, &Window::queryChanged,
                  [this]{ return current_query != nullptr; });


    addTransition(s_results_fallbacks, s_results_hidden,
                  input_line, QEvent::KeyRelease, mods_keys[(int)mod_fallback],
                  [this]{ return current_query->matches().size() == 0 && current_query->isActive(); });

    addTransition(s_results_fallbacks, s_results_matches,
                  input_line, QEvent::KeyRelease, mods_keys[(int)mod_fallback],
                  [this]{ return current_query->matches().size() > 0; });


    // Actions

    addTransition(s_results_match_items, s_results_match_actions,
                  input_line, QEvent::KeyPress, mods_keys[(int)mod_actions]);

    addTransition(s_results_match_actions, s_results_match_items,
                  input_line, QEvent::KeyRelease, mods_keys[(int)mod_actions]);

    addTransition(s_results_fallback_items, s_results_fallback_actions,
                  input_line, QEvent::KeyPress, mods_keys[(int)mod_actions]);

    addTransition(s_results_fallback_actions, s_results_fallback_items,
                  input_line, QEvent::KeyRelease, mods_keys[(int)mod_actions]);


    //
    // Behavior
    //

    // BUTTON

    auto *graphics_effect = new QGraphicsOpacityEffect(settings_button);
    settings_button->setGraphicsEffect(graphics_effect);  // QWidget takes ownership of effect.

    auto *opacity_animation = new QPropertyAnimation(graphics_effect, "opacity");
    connect(this, &QWidget::destroyed, opacity_animation, &QObject::deleteLater);

    QObject::connect(s_settings_button_shown, &QState::entered, this, [opacity_animation](){
        opacity_animation->stop();
        opacity_animation->setEndValue(0.999);  // Rounding issues on linux
        opacity_animation->start();
    });

    QObject::connect(s_settings_button_hidden, &QState::entered, this, [opacity_animation](){
        opacity_animation->stop();
        opacity_animation->setEndValue(0.0);
        opacity_animation->start();
    });


    // RESULTS

    QObject::connect(s_results_query_unset, &QState::entered, this, [this]{
        auto *sm = results_list->selectionModel();
        results_list->setModel(nullptr);
        delete sm;

        input_line->removeEventFilter(results_list);
    });

    QObject::connect(s_results_query_set, &QState::entered, this, [this]{
        // Eventfilters are processed in reverse order
        input_line->removeEventFilter(this);
        input_line->installEventFilter(results_list);
        input_line->installEventFilter(this);
    });

    QObject::connect(s_results_hidden, &QState::entered, this, [this]{
        results_list->hide();
        input_line->removeEventFilter(results_list);
    });

    QObject::connect(s_results_disabled, &QState::entered, this, [this, display_delay_timer]{
        display_delay_timer->start();
        results_list->setEnabled(false);
        input_line->removeEventFilter(results_list);
    });

    QObject::connect(s_results_disabled, &QState::exited, this, [this]{
        results_list->setEnabled(true);
    });

    QObject::connect(s_results_matches, &QState::entered, this, [this]{

        results_model = make_unique<MatchItemsModel>(*current_query);
        auto *m = results_model.get();

        auto *sm = results_list->selectionModel();
        results_list->setModel(m);
        delete sm;

        // let selection model currentChanged set input hint
        connect(results_list->selectionModel(), &QItemSelectionModel::currentChanged,
                this, [this](const QModelIndex &current, const QModelIndex&) {
                    if (results_list->currentIndex().isValid())
                        input_line->setInputHint(current.data((int)ItemRoles::InputActionRole).toString());
                });

        if (current_query->string().isEmpty()) {
            // avoid setting completion when synopsis should be shown
            const QSignalBlocker block(results_list->selectionModel());
            results_list->setCurrentIndex(m->index(0, 0));
        } else
            results_list->setCurrentIndex(m->index(0, 0));

        // Eventfilters are processed in reverse order
        input_line->removeEventFilter(this);
        input_line->installEventFilter(results_list);
        input_line->installEventFilter(this);

        results_list->show();
    });

    QObject::connect(s_results_fallbacks, &QState::entered, this, [this]{
        results_model = make_unique<FallbackItemsModel>(*current_query);
        auto *m = results_model.get();
        auto *sm = results_list->selectionModel();
        results_list->setModel(m);
        delete sm;
        results_list->setCurrentIndex(m->index(0, 0)); // should be okay since this state requires rc>0

        // Eventfilters are processed in reverse order
        input_line->removeEventFilter(this);
        input_line->installEventFilter(results_list);
        input_line->installEventFilter(this);

        results_list->show();
    });


    // ACTIONS

    auto hideActions = [this]
    {
        actions_list->hide();
        input_line->removeEventFilter(actions_list);

        // See QAbstractItemView::setModel documentation
        auto *sm = actions_list->selectionModel();
        actions_list->setModel(nullptr);
        delete sm;
    };

    auto showActions = [this]
    {
        // if item is selected and has actions display
        if (auto current_index = results_list->currentIndex();
            current_index.isValid())
        {
            if (auto action_names = current_index.data((int)ItemRoles::ActionsListRole).toStringList();
                !action_names.isEmpty())
            {
                // See QAbstractItemView::setModel documentation
                auto *sm = actions_list->selectionModel();
                auto *old_m = actions_list->model();
                auto m = new QStringListModel(action_names, actions_list);
                actions_list->setModel(m);
                delete sm;
                delete old_m;

                // Eventfilters are processed in reverse order
                actions_list->setCurrentIndex(m->index(0, 0)); // should be okay since this state requires rc>0
                input_line->installEventFilter(actions_list);
                actions_list->show();

            }
        }
    };

    QObject::connect(s_results_match_actions, &QState::entered, this, showActions);
    QObject::connect(s_results_match_actions, &QState::exited, this, hideActions);

    QObject::connect(s_results_fallback_actions, &QState::entered, this, showActions);
    QObject::connect(s_results_fallback_actions, &QState::exited, this, hideActions);


    // Activations

    auto activate = [this, s_results_matches, s_results_fallbacks](uint i, uint a)
    {
        if (s_results_matches->active())
            current_query->activateMatch(i, a);
        else if (s_results_fallbacks->active())
            current_query->activateFallback(i, a);
        else
            WARN << "Activated action in neither Match nor Fallback state.";

        hide();
    };

    QObject::connect(results_list, &ResizingList::activated,
                     [activate](const auto &index){activate(index.row(), 0);});

    QObject::connect(actions_list, &ResizingList::activated, this,
                     [this, activate](const auto &index){activate(results_list->currentIndex().row(),
                                                                    index.row());});


    auto *machine = new QStateMachine(this);
    machine->addState(s_root);
    machine->setInitialState(s_root);
    machine->start();
}

QString Window::input() const
{ return input_line->text(); }

void Window::setInput(const QString &text)
{ input_line->setText(text); }

void Window::setQuery(Query *q)
{
    if(current_query)
        disconnect(current_query, nullptr, this, nullptr);

    current_query = q;
    emit queryChanged();

    if(q)
    {
        input_line->setTriggerLength(q->trigger().length());
        // if (q->string().isEmpty())
        if (q->isTriggered() && q->string().isEmpty())
            input_line->setInputHint(q->synopsis());

        connect(current_query, &Query::matchesAdded,
                this, &Window::queryMatchesAdded);

        connect(current_query, &Query::activeChanged,
                this, [this](bool active){
                    active ? emit queryStateBusy() : emit queryStateIdle();
                });
    }
}

void Window::applyThemeFile(const QString& path)
{
    QFile f(path);
    if (f.open(QFile::ReadOnly))
    {
        setStyleSheet(f.readAll());
        f.close();
    }
    else
    {
        auto msg = QString("%1:\n\n%2\n\n%3")
                       .arg(tr("The theme file could not be opened"), path, f.errorString());
        WARN << msg;
        QMessageBox::warning(this, qApp->applicationDisplayName(), msg);
    }
}

bool Window::darkMode() const { return dark_mode; }

bool Window::event(QEvent *event)
{
    if (event->type() == QEvent::Resize)  // Let settingsbutton stay in top right corner of frame
        settings_button->move(frame->geometry().topRight() - QPoint(settings_button->width()-1,0));

    else if (event->type() == QEvent::MouseButtonPress)
        windowHandle()->startSystemMove();

    else if (event->type() == QEvent::Show)
    {
        // If showCentered or off screen (e.g. display disconnected) move into visible area
        if (showCentered_ || !screen())
        {
            QScreen *screen = nullptr;
            if (followCursor_){
                if (screen = QGuiApplication::screenAt(QCursor::pos()); !screen){
                    WARN << "Could not retrieve screen for cursor position. Using primary screen.";
                    screen = QGuiApplication::primaryScreen();
                }
            }
            else
                screen = QGuiApplication::primaryScreen();

            // move window  TODO remove debugging stuff heree
            auto geo = screen->geometry();

            auto win_width = frameSize().width();
            auto newX = geo.center().x() - win_width / 2;
            auto newY = geo.top() + geo.height() / 5;

            // DEBG << screen->name() << screen->manufacturer() << screen->model() << screen->devicePixelRatio() << geo;
            // DEBG << "win_width" << win_width  << "newX" << newX << "newY" << newY;

            move(newX, newY);
        }

#if not defined Q_OS_MACOS // steals focus on macos
        raise();
        activateWindow();
#endif
        emit visibleChanged(true);
    }

    else if (event->type() == QEvent::Hide)
    {
        plugin->state()->setValue(STATE_WND_POS, pos());

        /*
         * Prevent the flicker when the window is shown
         *
         * Qt sends a resize event when the window is shown.
         *
         * When the window was expanded on hide the resize happens on show which introduces ugly
         * flicker. Force the resize to happen before the window is hidden.
         *
         * This may be removed when the frontend does not use the input changed > set query smell
         * anymore.
         */
        setQuery(nullptr);
        // Setting the query seems not to be sufficient. Hide the lists manually.
        results_list->hide();
        actions_list->hide();
        // looks like the layoutsystem is not synchronous
        QCoreApplication::processEvents();

        QPixmapCache::clear();

        emit visibleChanged(false);
    }

    else if (event->type() == QEvent::ThemeChange)
    {
        auto have_dark_system_palette = haveDarkSystemPalette();

        if (dark_mode != have_dark_system_palette)
        {
#ifdef Q_OS_LINUX
            QApplication::setPalette(QApplication::style()->standardPalette());
#endif
            // at(): no catch, theme_dark_ theme_light_ should exist
            dark_mode = have_dark_system_palette;
            applyThemeFile(themes.at((dark_mode) ? theme_dark_ : theme_light_));
        }
    }

    else if (event->type() == QEvent::Close)
    {
        if(quitOnClose_)
            qApp->quit();
        else
            hide();
    }

    // else if (event->type() == QEvent::ApplicationActivate) CRIT << event;
    // else if (event->type() == QEvent::ApplicationActivated) CRIT << event;
    // else if (event->type() == QEvent::ApplicationDeactivate) CRIT << event;
    // else if (event->type() == QEvent::ApplicationDeactivated) CRIT << event;
    // else if (event->type() == QEvent::ApplicationStateChange) CRIT << event;
    // else if (event->type() == QEvent::WindowActivate) CRIT << event;
    // else if (event->type() == QEvent::WindowStateChange) CRIT << event;
    else if (event->type() == QEvent::WindowDeactivate && hideOnFocusLoss_)
        setVisible(false);

    else return QWidget::event(event);

    return true;
}

bool Window::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == input_line)
    {
        if (event->type() == QEvent::KeyPress)
        {
            auto *keyEvent = static_cast<QKeyEvent *>(event);
            switch (keyEvent->key()) {

            case Qt::Key_Tab:
                // Toggle insert completion string
                if (auto i = results_list->currentIndex(); i.isValid())
                    if (auto t = i.data((int)ItemRoles::InputActionRole).toString();
                        !(t.isNull() && t.isEmpty()))
                        input_line->setText(t);
                return true;

            case Qt::Key_Up:
                // Move up in the history
                if (!results_list->currentIndex().isValid()
                    || keyEvent->modifiers().testFlag(Qt::ShiftModifier)
                    || (results_list->currentIndex().row() == 0
                        && !keyEvent->isAutoRepeat()))  // ... and first row (non repeat)
                {
                    input_line->next(history_search_);
                    return true;
                }
                break;

            case Qt::Key_Down:
                // Move down in the history
                if (keyEvent->modifiers().testFlag(Qt::ShiftModifier))
                {
                    input_line->previous(history_search_);
                    return true;
                }
                break;

            case Qt::Key_P:
            case Qt::Key_K:
                if (keyEvent->modifiers().testFlag(Qt::ControlModifier)){
                    QKeyEvent e(QEvent::KeyPress, Qt::Key_Up, keyEvent->modifiers().setFlag(Qt::ControlModifier, false));
                    QApplication::sendEvent(input_line, &e);
                }
                break;

            case Qt::Key_N:
            case Qt::Key_J:
                if (keyEvent->modifiers().testFlag(Qt::ControlModifier)){
                    QKeyEvent e(QEvent::KeyPress, Qt::Key_Down, keyEvent->modifiers().setFlag(Qt::ControlModifier, false));
                    QApplication::sendEvent(input_line, &e);
                }
                break;

            case Qt::Key_H:
                if (keyEvent->modifiers().testFlag(Qt::ControlModifier)){
                    QKeyEvent e(QEvent::KeyPress, Qt::Key_Left, keyEvent->modifiers().setFlag(Qt::ControlModifier, false));
                    QApplication::sendEvent(input_line, &e);
                }
                break;

            case Qt::Key_L:
                if (keyEvent->modifiers().testFlag(Qt::ControlModifier)){
                    QKeyEvent e(QEvent::KeyPress, Qt::Key_Right, keyEvent->modifiers().setFlag(Qt::ControlModifier, false));
                    QApplication::sendEvent(input_line, &e);
                }
                break;

            case Qt::Key_Comma:{
                if (keyEvent->modifiers() == Qt::ControlModifier || keyEvent->modifiers() == Qt::AltModifier){
                    showSettings();
                    setVisible(false);
                    return true;
                }
                break;
            }

            case Qt::Key_Escape:{
                setVisible(false);
                break;
            }
            }
        }
    }
    return false;
}


//
//  PROPERTIES
//

const QString &Window::themeLight() const { return theme_light_; }
void Window::setThemeLight(const QString &val)
{
    if (themeLight() == val)
        return;

    // intended implicit test for existance
    auto theme_file = themes.at(val);
    if (!dark_mode)
        applyThemeFile(theme_file);

    theme_light_ = val;
    plugin->settings()->setValue(CFG_THEME_LIGHT, val);
    emit themeLightChanged(val);
}

const QString &Window::themeDark() const { return theme_dark_; }
void Window::setThemeDark(const QString &val)
{
    if (themeDark() == val)
        return;

    // intended implicit test for existance
    auto theme_file = themes.at(val);
    if (dark_mode)
        applyThemeFile(theme_file);

    theme_dark_ = val;theme_dark_ = val;
    plugin->settings()->setValue(CFG_THEME_DARK, val);
    emit themeDarkChanged(val);
}

bool Window::alwaysOnTop() const { return windowFlags() & Qt::WindowStaysOnTopHint; }
void Window::setAlwaysOnTop(bool val)
{
    if (alwaysOnTop() == val)
        return;

    setWindowFlags(windowFlags().setFlag(Qt::WindowStaysOnTopHint, val));
    plugin->settings()->setValue(CFG_ALWAYS_ON_TOP, val);
    emit clearOnHideChanged(val);
}

bool Window::clearOnHide() const { return input_line->clear_on_hide; }
void Window::setClearOnHide(bool val)
{
    if (clearOnHide() == val)
        return;

    input_line->clear_on_hide = val;
    plugin->settings()->setValue(CFG_CLEAR_ON_HIDE, val);
    emit clearOnHideChanged(val);
}

bool Window::displayClientShadow() const { return graphicsEffect() != nullptr; }
void Window::setDisplayClientShadow(bool val)
{
    if (displayClientShadow() == val)
        return;

    if (val)
    {
        // Properties
        auto* effect = new QGraphicsDropShadowEffect(this);
        effect->setBlurRadius(DEF_SHADOW_SIZE);
        effect->setColor(QColor(0, 0, 0, 92))  ;
        effect->setXOffset(0.0);
        effect->setYOffset(2.0);
        setGraphicsEffect(effect);  // takes ownership
        setContentsMargins(DEF_SHADOW_SIZE,DEF_SHADOW_SIZE,DEF_SHADOW_SIZE,DEF_SHADOW_SIZE);
    }
    else
    {
        setGraphicsEffect(nullptr);
        setContentsMargins(0,0,0,0);
    }

    plugin->settings()->setValue(CFG_SHADOW_CLIENT, val);
    emit displayClientShadowChanged(val);
}

bool Window::displayScrollbar() const
{ return results_list->verticalScrollBarPolicy() != Qt::ScrollBarAlwaysOff; }
void Window::setDisplayScrollbar(bool val)
{
    if (displayScrollbar() == val)
        return;

    results_list->setVerticalScrollBarPolicy(val ? Qt::ScrollBarAsNeeded : Qt::ScrollBarAlwaysOff);
    plugin->settings()->setValue(CFG_DISPLAY_SCROLLBAR, val);
    emit displayScrollbarChanged(val);
}

bool Window::displaySystemShadow() const
{ return !windowFlags().testFlag(Qt::NoDropShadowWindowHint); }
void Window::setDisplaySystemShadow(bool val)
{
    if (displaySystemShadow() == val)
        return;

    setWindowFlags(windowFlags().setFlag(Qt::NoDropShadowWindowHint, !val));
    plugin->settings()->setValue(CFG_SHADOW_SYSTEM, val);
    emit displaySystemShadowChanged(val);
}

bool Window::followCursor() const { return followCursor_; }
void Window::setFollowCursor(bool val)
{
    if (followCursor() == val)
        return;

    followCursor_ = val;
    plugin->settings()->setValue(CFG_FOLLOW_CURSOR, val);
    emit hideOnFocusLossChanged(val);
}

bool Window::hideOnFocusLoss() const { return hideOnFocusLoss_; }
void Window::setHideOnFocusLoss(bool val)
{
    if (hideOnFocusLoss() == val)
        return;

    hideOnFocusLoss_ = val;
    plugin->settings()->setValue(CFG_HIDE_ON_FOCUS_LOSS, val);
    emit hideOnFocusLossChanged(val);
}

bool Window::historySearchEnabled() const { return history_search_; }
void Window::setHistorySearchEnabled(bool val)
{
    if (historySearchEnabled() == val)
        return;

    history_search_ = val;
    plugin->settings()->setValue(CFG_HISTORY_SEARCH, val);
    emit maxResultsChanged(val);
}

uint Window::maxResults() const { return results_list->maxItems(); }
void Window::setMaxResults(uint val)
{
    if (maxResults() == val)
        return;

    results_list->setMaxItems(val);
    plugin->settings()->setValue(CFG_MAX_RESULTS, val);
    emit maxResultsChanged(val);
}

bool Window::quitOnClose() const { return quitOnClose_; }
void Window::setQuitOnClose(bool val)
{
    if (quitOnClose() == val)
        return;

    quitOnClose_ = val;
    plugin->settings()->setValue(CFG_QUIT_ON_CLOSE, val);
    emit quitOnCloseChanged(val);
}

bool Window::showCentered() const { return showCentered_; }
void Window::setShowCentered(bool val)
{
    if (showCentered() == val)
        return;

    showCentered_ = val;
    plugin->settings()->setValue(CFG_CENTERED, val);
    emit showCenteredChanged(val);
}

bool Window::debugMode() const { return debug_overlay_.get(); }
void Window::setDebugMode(bool val)
{
    if (debugMode() == val)
        return;

    if (val)
    {
        debug_overlay_ = make_unique<DebugOverlay>();
        debug_overlay_->recursiveInstallEventFilter(this);
    }
    else
        debug_overlay_.reset();

    plugin->settings()->setValue(CFG_DEBUG, val);
    update();
    emit debugModeChanged(val);
}
