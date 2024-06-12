// Copyright (c) 2022-2024 Manuel Schneider

#include "paletteeditor.h"
#include "plugin.h"
#include "primitives.h"
#include "styleeditor.h"
#include "util.h"
#include <QCommonStyle>
#include <QDir>
#include <QGraphicsEffect>
#include <QLayout>
#include <QMenu>
#include <QMessageBox>
#include <QPainter>
#include <QParallelAnimationGroup>
#include <QPixmapCache>
#include <QPropertyAnimation>
#include <QSequentialAnimationGroup>
#include <QStandardPaths>
#include <QStringListModel>
#include <QStyleFactory>
#include <QTimer>
#include <QWindow>
#include <albert/logging.h>
#include <albert/pluginloader.h>
#include <albert/pluginmetadata.h>
#include <albert/util.h>
using namespace albert;
using namespace std;

bool draw_debug_overlays = false;

namespace  {
const uint    DEF_SHADOW_SIZE = 24;  // TODO user
const char*   STATE_WND_POS  = "windowPosition";
}

Window::Window(Plugin *p):
    plugin(p),
    state_machine(this),
    current_query{nullptr}
{
    // Setup UI
    {
        // Reproducible UI
        // function<void(QWidget*, QStyle*)> setStyleRec = [&](QWidget *widget, QStyle *style){
        //     widget->setStyle(style);
        //     for (QObject *o : widget->children())
        //         if (QWidget *w = qobject_cast<QWidget *>(o); w)
        //             setStyleRec(w, style);
        // };
        // // setStyleRec(this, new QCommonStyle);
        // setStyleRec(this, QStyleFactory::create("Fusion"));

        auto *input_frame_layout = new QHBoxLayout(&input_frame);
        input_frame_layout->addWidget(&input_line);
        input_frame_layout->addWidget(&synopsis);
        input_frame_layout->addWidget(&settings_button);
        // input_frame_layout->setAlignment(&settings_button, Qt::AlignTop);

        // // hack to adjust button size
        // connect(input_line.document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged,
        //         this, [this](const QSizeF &newSize){
        //     settings_button.setFixedSize(newSize.height(), newSize.height());

        // });

        auto *layout = new QVBoxLayout(this);
        layout->addWidget(&input_frame);
        layout->addWidget(&results_list);
        layout->addWidget(&actions_list);

        results_list.hide();
        actions_list.hide();

        // Fix for nicely aligned text.
        // The location of this code is  hacky, but QTextEdit does not allow to set margins.
        // The text should be idented by the distance of the cap line to the top.
        QFont f;
        f.setPointSize(style_.input_line_font_size);
        QFontMetrics fm(f);
        auto font_margin_fix = (fm.lineSpacing() - fm.capHeight() - fm.tightBoundingRect("|").width())/2 ;
        auto px = style_.input_frame_padding + style_.input_frame_border_width;
        input_frame.layout()->setContentsMargins(px + font_margin_fix, px, px, px);


        CRIT << sizeHint();
        // setFixedWidth(700);
        layout->setSizeConstraint(QLayout::SetMinAndMaxSize);
        setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        setMinimumSize(0,0);
        adjustSize();
        input_frame.setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        input_line.setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        settings_button.setSizePolicy(QSizePolicy::Policy::Fixed, QSizePolicy::Policy::Fixed);
        results_list.setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
        actions_list.setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

        window_frame.setMouseTracking(true); //leave/enter events
        input_line.installEventFilter(this);

        settings_button.setFocusPolicy(Qt::NoFocus);
        results_list.setFocusPolicy(Qt::NoFocus);
        actions_list.setFocusPolicy(Qt::NoFocus);

        setWindowFlags(Qt::Tool|Qt::FramelessWindowHint|Qt::WindowStaysOnTopHint);
        setAttribute(Qt::WA_TranslucentBackground);

        connect(&input_line, &InputLine::plainTextChanged,
                this, [&](const QString &s){ emit inputChanged(s); });
    }

    // Actions
    {
        auto *a = new QAction(tr("Settings"), this);
        a->setShortcuts({QKeySequence("Ctrl+,")});
        a->setShortcutVisibleInContextMenu(true);
        addAction(a);
        connect(a, &QAction::triggered,
                this, []{ albert::showSettings(); });

        a = new QAction(tr("Hide on focus out"), this);
        a->setShortcuts({QKeySequence("Meta+h")});
        a->setShortcutVisibleInContextMenu(true);
        a->setCheckable(true);
        a->setChecked(hide_on_focus_loss());
        addAction(a);
        connect(a, &QAction::toggled,
                this, [this](bool b){ set_hide_on_focus_loss(b); });
        connect(this, &Window::hide_on_focus_loss_changed,
                a, [this, a]{ a->setChecked(hide_on_focus_loss()); });

        a = new QAction(tr("Show centered"), this);
        a->setShortcuts({QKeySequence("Meta+c")});
        a->setShortcutVisibleInContextMenu(true);
        a->setCheckable(true);
        a->setChecked(show_centered());
        addAction(a);
        connect(a, &QAction::toggled,
                this, [this](bool b){ set_show_centered(b); });
        connect(this, &Window::show_centered_changed,
                a, [this, a]{ a->setChecked(show_centered()); });

        a = new QAction(tr("Clear on hide"), this);
        a->setShortcuts({QKeySequence("Meta+Alt+c")});
        a->setShortcutVisibleInContextMenu(true);
        a->setCheckable(true);
        a->setChecked(clear_on_hide());
        addAction(a);
        connect(a, &QAction::toggled,
                this, [this](bool b){ set_clear_on_hide(b); });
        connect(this, &Window::clear_on_hide_changed,
                a, [this, a]{ a->setChecked(clear_on_hide()); });

        a = new QAction(tr("Toggle debug overlay"), this);
        a->setShortcuts({QKeySequence("Meta+d")});
        a->setShortcutVisibleInContextMenu(true);
        addAction(a);
        connect(a, &QAction::triggered, this,
                [this]{ style_.draw_debug_overlays = !style_.draw_debug_overlays; setStyle(style_); });

        // connect settingsbutton clicks
        connect(&settings_button, &SettingsButton::clicked, this, [this](Qt::MouseButton b){
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

    // Settings
    {
        auto s = plugin->settings();
        restore_light_style_file(s);
        restore_dark_style_file(s);
        restore_always_on_top(s);
        restore_clear_on_hide(s);
        restore_display_client_shadow(s);
        restore_display_system_shadow(s);
        restore_follow_cursor(s);
        restore_hide_on_focus_loss(s);
        restore_history_search(s);
        restore_max_results(s);
        restore_quit_on_close(s);
        restore_show_centered(s);
    }

    // State
    {
        auto s = plugin->state();
        if (!show_centered()
            && s->contains(STATE_WND_POS)
            && s->value(STATE_WND_POS).canConvert(QMetaType(QMetaType::QPoint)))
           move(s->value(STATE_WND_POS).toPoint());
    }

    // Statemachine
    {
        #define onState(state, signal, slot) \
        QObject::connect(&state_machine.state, &QState::signal, this, slot);

        // BUTTON

        connect(&state_machine.settings_button_hidden, &QState::entered, this,
                [this]{ settings_button.setState(SettingsButton::Hidden); });

        connect(&state_machine.settings_button_visible, &QState::entered, this,
                [this]{ settings_button.setState(SettingsButton::Visible); });

        connect(&state_machine.settings_button_highlight, &QState::entered, this,
                [this]{ settings_button.setState(SettingsButton::Highlight); });


        // RESULTS

        onState(results_query_unset, entered, [this]{

            results_list.hide();

            auto *sm = results_list.selectionModel();
            results_list.setModel(nullptr);
            delete sm;

            input_line.removeEventFilter(&results_list);
        });

        onState(results_query_set, entered, [this]{
            // Eventfilters are processed in reverse order
            input_line.removeEventFilter(this);
            input_line.installEventFilter(&results_list);
            input_line.installEventFilter(this);
        });

        onState(results_hidden, entered, [this]{
            results_list.hide();
            input_line.removeEventFilter(&results_list);
        });

        onState(results_disabled, entered, [this]{
            // TODO introduces flicker. find a better solution.
            // results_list.setEnabled(false);
            input_line.removeEventFilter(&results_list);
        });

        // onState(results_disabled, exited, [this]{
        //     // results_list.setEnabled(true);
        // });

        onState(results_matches, entered, [this]{
            auto *m = current_query->matches();

            auto *sm = results_list.selectionModel();
            results_list.setModel(m);
            delete sm;

            // let selection model currentChanged set input hint
            connect(results_list.selectionModel(), &QItemSelectionModel::currentChanged,
                    this, [this](const QModelIndex &current, const QModelIndex&) {
                        if (results_list.currentIndex().isValid())
                        {
                            const auto t = current.data((int)ItemRoles::InputActionRole).toString();
                            input_line.setCompletion(t.isEmpty() ? QString() : t);
                        }
                    });

            results_list.setCurrentIndex(m->index(0, 0));

            // Eventfilters are processed in reverse order
            input_line.removeEventFilter(this);
            input_line.installEventFilter(&results_list);
            input_line.installEventFilter(this);

            results_list.show();
        });

        onState(results_fallbacks, entered, [this]{
            // Needed because fallback model may already be set
            if (auto *m = current_query->fallbacks();
                m != results_list.model())
            {
                auto *sm = results_list.selectionModel();
                results_list.setModel(m);
                delete sm;
                results_list.setCurrentIndex(m->index(0, 0)); // should be okay since this state requires rc>0
            }

            // Eventfilters are processed in reverse order
            input_line.removeEventFilter(this);
            input_line.installEventFilter(&results_list);
            input_line.installEventFilter(this);

            results_list.show();
        });

        // ACTIONS

        auto hideActions = [this]
        {
            actions_list.hide();
            input_line.removeEventFilter(&actions_list);

            // See QAbstractItemView::setModel documentation
            auto *sm = actions_list.selectionModel();
            actions_list.setModel(nullptr);
            delete sm;
        };

        auto showActions = [this]
        {
            // if item is selected and has actions display
            if (auto current_index = results_list.currentIndex();
                current_index.isValid())
            {
                if (auto action_names = current_index.data((int)ItemRoles::ActionsListRole).toStringList();
                    !action_names.isEmpty())
                {
                    // See QAbstractItemView::setModel documentation
                    auto *sm = actions_list.selectionModel();
                    auto *old_m = actions_list.model();
                    auto m = new QStringListModel(action_names, &actions_list);
                    actions_list.setModel(m);
                    delete sm;
                    delete old_m;

                    // Eventfilters are processed in reverse order
                    input_line.removeEventFilter(this);
                    input_line.installEventFilter(&actions_list);
                    input_line.installEventFilter(this);

                    actions_list.setCurrentIndex(m->index(0, 0)); // should be okay since this state requires rc>0
                    actions_list.show();

                }
            }
        };

        onState(results_match_actions, entered, showActions);
        onState(results_match_actions, exited, hideActions);

        onState(results_fallback_actions, entered, showActions);
        onState(results_fallback_actions, exited, hideActions);

        state_machine.start();


        // Activations

        auto activate = [this](uint i, uint a)
        {
            if (state_machine.results_matches.active())
                current_query->activateMatch(i, a);
            else if (state_machine.results_fallbacks.active())
                current_query->activateFallback(i, a);
            else
                WARN << "Activated action in neither Match nor Fallback state.";
            hide();
        };

        QObject::connect(&results_list, &ResizingList::activated,
                         [activate](const auto &index){activate(index.row(), 0);});

        QObject::connect(&actions_list, &ResizingList::activated, this,
                         [this, activate](const auto &index){activate(results_list.currentIndex().row(),
                                                                        index.row());});
    }

    // Style
    {


    // // Check if themes exist and apply current
    // {
    //     auto tr_message = tr("Theme '%1' does not exist. Check your config!");
    //     if (!themes.contains(theme_light_))
    //     {
    //         CRIT << tr_message.arg(theme_light_);
    //         QMessageBox::critical(nullptr, qApp->applicationDisplayName(), tr_message.arg(theme_light_));
    //         setLightTheme(themes.contains(DEF_THEME) ? QString(DEF_THEME) : themes.begin()->first);
    //     }
    //     if (!themes.contains(theme_dark_))
    //     {
    //         CRIT << tr_message.arg(theme_dark_);
    //         QMessageBox::critical(nullptr, qApp->applicationDisplayName(), tr_message.arg(theme_dark_));
    //         setDarkTheme(themes.contains(DEF_THEME) ? QString(DEF_THEME) : themes.begin()->first);
    //     }
    //     // applyThemeFile(themes.at((dark_mode_ = haveDarkPalette()) ? theme_dark_ : theme_light_));
    // }



    // // Load style
    // Style s;
    // s=s.read("/Users/manuel/Desktop/Nord.ini");
    // setStyle(s);

    // s.write("/Users/manuel/Desktop/style.ini");
    // s.write("/Users/manuel/Desktop/style_read.ini");

    // // auto img = QImage("/Users/manuel/Desktop/nasa/cool2.png");
    // // pal.setBrush(QPalette::Window, QBrush(img));
    // setPalette(pal);

    // savePalette(palette(), "/Users/manuel/Desktop/palette.ini");
    // // setPalette(qApp->palette());

    // // pal = loadPalette("/Users/manuel/Desktop/palette.ini");
    // // setPalette(pal);

        // auto *e = new PaletteEditor(palette(), this);
        // e->show();

    }
}

Window::~Window() = default;

QFileInfoList Window::findStyles() const
{
    using SP = QStandardPaths;
    QFileInfoList t;
    for (const QString &p : SP::locateAll(SP::AppDataLocation, plugin->loader().metaData().id, SP::LocateDirectory))
        for (const auto &fi : QDir(QStringLiteral("%1/styles").arg(p))
                                   .entryInfoList(QStringList("*.style"), QDir::Files))
            t.emplace_back(fi);
    return t;
}

QString Window::input() const
{ return input_line.text(); }

void Window::setInput(const QString &text)
{ input_line.setText(text); }

void Window::setQuery(Query *q)
{
    if(current_query)
        disconnect(current_query, nullptr, this, nullptr);

    current_query = q;
    emit queryChanged();

    if(q)
    {
        synopsis.setText(q->synopsis());
        input_line.setTriggerLength(q->trigger().length());
        connect(q->matches(), &QAbstractItemModel::rowsInserted, this, &Window::queryMatchesAdded);
        connect(q, &Query::finished, this, &Window::queryFinished);
    }
    else
        synopsis.clear();
}

bool Window::event(QEvent *event)
{
    switch (event->type())
    {

    case QEvent::MouseButtonPress:
    {
        if (parent() == nullptr) // dont drag if embedded
            windowHandle()->startSystemMove();
        break;
    }

    case QEvent::Show:
    {

        CRIT <<"SH" << sizeHint();
        // If showCentered or off screen (e.g. display disconnected) move into visible area
        if (show_centered() || !screen())
        {
            QScreen *screen = nullptr;
            if (follow_cursor()){
                if (screen = QGuiApplication::screenAt(QCursor::pos()); !screen){
                    WARN << "Could not retrieve screen for cursor position. Using primary screen.";
                    screen = QGuiApplication::primaryScreen();
                }
            }
            else
                screen = QGuiApplication::primaryScreen();

            auto screen_rect = screen->geometry();
            DEBG << "Show on:" << screen->name() << screen_rect << screen->devicePixelRatio();
            move(screen_rect.center().x() - frameSize().width() / 2,
                 screen_rect.top() + screen_rect.height() / 5);
        }

#if not defined Q_OS_MACOS // steals focus on macos
        raise();
        activateWindow();
#endif
        emit visibleChanged(true);
        break;
    }

    case QEvent::Hide:
    {
        plugin->state()->setValue(STATE_WND_POS, pos());
        QPixmapCache::clear();
        emit visibleChanged(false);
        break;
    }

    case QEvent::ApplicationPaletteChange:
    case QEvent::PaletteChange:
    case QEvent::StyleChange: {
        INFO << event->type() << haveDarkSystemPalette();
        // update();
        // frame.update();
        // return true;

        // at(): no catch, theme_dark_ theme_light_ should exist
        // applyThemeFile(themes.at((dark_mode_ = haveDarkPalette()) ? theme_dark_
        // : theme_light_)); return true;
        break;
    }

    case QEvent::Close:
    {
        if(quit_on_close())
            qApp->quit();
        else
            hide();
        break;
    }

    case QEvent::WindowDeactivate:
    {
        if (hide_on_focus_loss())
            setVisible(false);
        break;
    }

    default:
        break;
    }

    return QWidget::event(event);
}

bool Window::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == &input_line)
    {
        if (event->type() == QEvent::KeyPress)
        {
            auto *e = static_cast<QKeyEvent *>(event);
            switch (e->key()) {

            case Qt::Key_Up:
                // Move up in the history
                if (!results_list.currentIndex().isValid()
                    || e->modifiers().testFlag(Qt::ShiftModifier)
                    || (results_list.currentIndex().row() == 0
                        && !e->isAutoRepeat()))  // ... and first row (non repeat)
                {
                    input_line.next();
                    return true;
                }
                break;

            case Qt::Key_Down:
                // Move down in the history
                if (e->modifiers().testFlag(Qt::ShiftModifier))
                {
                    input_line.previous();
                    return true;
                }
                break;

            case Qt::Key_P:
            case Qt::Key_K:
                if (e->modifiers().testFlag(Qt::ControlModifier)) {
                    QKeyEvent syn(
                        QEvent::KeyPress, Qt::Key_Up,
                        e->modifiers().setFlag(Qt::ControlModifier, false),
                        e->text(), e->isAutoRepeat());
                    QApplication::sendEvent(&input_line, &syn);
                }
                break;

            case Qt::Key_N:
            case Qt::Key_J:
                if (e->modifiers().testFlag(Qt::ControlModifier)) {
                    QKeyEvent syn(
                        QEvent::KeyPress, Qt::Key_Down,
                        e->modifiers().setFlag(Qt::ControlModifier, false),
                        e->text(), e->isAutoRepeat());
                    QApplication::sendEvent(&input_line, &syn);
                }
                break;

            case Qt::Key_H:
                if (e->modifiers().testFlag(Qt::ControlModifier)) {
                    QKeyEvent syn(
                        QEvent::KeyPress, Qt::Key_Left,
                        e->modifiers().setFlag(Qt::ControlModifier, false),
                        e->text(), e->isAutoRepeat());
                    QApplication::sendEvent(&input_line, &syn);
                }
                break;

            case Qt::Key_L:
                if (e->modifiers().testFlag(Qt::ControlModifier)) {
                    QKeyEvent syn(
                        QEvent::KeyPress, Qt::Key_Right,
                        e->modifiers().setFlag(Qt::ControlModifier, false),
                        e->text(), e->isAutoRepeat());
                    QApplication::sendEvent(&input_line, &syn);
                }
                break;

            case Qt::Key_Comma:{
                if (e->modifiers() == Qt::ControlModifier
                    || e->modifiers() == Qt::AltModifier){
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

            if (e->modifiers() == command_mod
                && (e->key() == Qt::Key_Return || e->key() == Qt::Key_Enter)) {
                state_machine.toggleActionMode();
                return true;
            }

            else if (e->key() == actions_key)
                state_machine.enterActionMode();

            else if (e->key() == fallback_key)
                state_machine.enterFallbackMode();

        }
        else if (event->type() == QEvent::KeyRelease)
        {
            auto *keyEvent = static_cast<QKeyEvent *>(event);

            if (keyEvent->key() == actions_key)
                state_machine.exitActionMode();

            else if (keyEvent->key() == fallback_key)
                state_machine.exitFallbackMode();
        }
    }
    return false;
}

void Window::paintEvent(QPaintEvent *)
{
    // CRIT << "Window::paintEvent" << event->rect();

    QPainter p(this);
    QPixmap pm;

    if (const auto cache_key = QStringLiteral("_Window_%1x%2")
                                   .arg(width()).arg(height());
        !QPixmapCache::find(cache_key, &pm))
    {
        // auto dpr = devicePixelRatioF();
        // pm = pixelPerfectRoundedRect(size() * dpr,
        //                              style->frame_background_brush,
        //                              style->frame_border_radius * dpr,
        //                              style->frame_border_brush,
        //                              style->frame_border_width * dpr);
        // pm.setDevicePixelRatio(dpr);

        auto dpr = devicePixelRatioF();
        auto shadow_margins = QMargins(style_.window_shadow_size,
                                       style_.window_shadow_size - style_.window_shadow_voffset,
                                       style_.window_shadow_size,
                                       style_.window_shadow_size + style_.window_shadow_voffset);
        auto frame_rect = rect().marginsRemoved(shadow_margins);


        auto frame_pixmap = pixelPerfectRoundedRect(frame_rect.size() * dpr,
                                                    style_.window_background_brush,
                                                    style_.window_border_radius * dpr,
                                                    style_.window_border_brush,
                                                    style_.window_border_width * dpr);
        frame_pixmap.setDevicePixelRatio(dpr);
        // WARN << "FRAME >>>" << frame_pixmap.size() << frame_pixmap.deviceIndependentSize() << frame_pixmap.devicePixelRatio();

        QImage img(size() * dpr, QImage::Format_ARGB32_Premultiplied);
        img.setDevicePixelRatio(dpr);
        img.fill(Qt::transparent);
        // WARN << "IMAGE >>>" << img.size() << img.deviceIndependentSize() << img.devicePixelRatio();

        QPainter imgPainter(&img);
        imgPainter.drawPixmap(frame_rect.adjusted(0, style_.window_shadow_voffset,
                                                  0, style_.window_shadow_voffset),
                              frame_pixmap);//, QRectF(frame_pixmap.rect()));
        imgPainter.setCompositionMode(QPainter::CompositionMode_SourceIn);
        imgPainter.fillRect(img.rect(), style_.window_shadow_color.color());
        imgPainter.end();

        // for (int i = 0; i < style_.frame_shadow_size/5; i++)
        //     img = blurImage(img, 5, false, true);

        // p.drawImage(QPoint{}, img);

        pm = QPixmap(size()*dpr);
        pm.setDevicePixelRatio(dpr);
        pm.fill(Qt::transparent);

        QPainter buf(&pm);
        buf.save();
        qt_blurImage(&buf, img, style_.window_shadow_size*dpr, false, false);
        buf.restore();
        buf.drawPixmap(frame_rect, frame_pixmap);
        buf.end();

        // WARN << "COMPOSITE >>>" << pm.size() << pm.deviceIndependentSize() << pm.devicePixelRatio();


        // QPixmapCache::insert(cache_key, pm);
    }

    // Q_ASSERT(size() == pm.deviceIndependentSize());
    p.drawPixmap(0, 0, pm);
    if (style_.draw_debug_overlays)
    {
        // drawDebugRect(p, frame_rect, "frame_rect", Qt::magenta);
        drawDebugRect(p, rect(), QString("Window"));
        drawDebugRect(p, contentsRect(), "Window::contents", Qt::green);
        drawDebugRect(p, layout()->contentsRect(), "Window::layout", Qt::green);

    }
}

const Style &Window::style()
{ return style_; }

void Window::setStyle(const Style &s)
{

    CRIT << "SETSTSYLE";
    style_ = s;

    setPalette(s.palette);

    // input_frame.setStyle(&style_);
    input_line.setStyle(&style_);

    auto f = font();
    f.setPointSize(s.input_line_font_size);
    input_line.setFont(f);
    synopsis.setFont(f);

    auto palette = synopsis.palette();
    palette.setColor(QPalette::ColorRole::WindowText, QApplication::palette().placeholderText().color());
    synopsis.setPalette(palette);


    settings_button.setStyle(&style_);
    results_list.setStyle(&style_);
    results_list.updateGeometry();
    actions_list.setStyle(&style_);

    CRIT << style_.window_width + style_.window_shadow_size * 2;
    setFixedWidth(style_.window_width + style_.window_shadow_size * 2);
    // windowHandle()->setWidth(style_.window_width + style_.window_shadow_size * 2);

    // ???
    auto h = input_line.fontMetrics().lineSpacing() + 2 * input_line.document()->documentMargin();
    settings_button.setFixedSize(h, h);
    auto m = (h-s.settings_button_size)/2;
    settings_button.setContentsMargins(m,m,m,m);


    layout()->setSpacing(style_.window_padding);
    auto p = style_.window_padding
           + style_.window_border_width
           + style_.window_shadow_size;

    setContentsMargins(0,0,0,0);

    layout()->setContentsMargins(p, p - style_.window_shadow_voffset,
                                 p, p + style_.window_shadow_voffset);

    QPixmapCache::clear();
    update();
    updateGeometry();
    results_list.reset(); // needed to relayout items
}



// const QString &Window::darkStyle() const
// { return theme_dark_; }

// void Window::setDarkTheme(const QString &theme_name)
// {
//     // intended implicit test for existance
//     auto theme_file = themes.at(theme_name);
//     if (haveDarkSystemPalette())
//         applyThemeFile(theme_file);
//     plugin->settings()->setValue(CFG_THEME_DARK, theme_dark_ = theme_name);
// }

// void Window::applyThemeFile(const QString& path)
// {
//     QFile f(path);
//     if (f.open(QFile::ReadOnly))
//     {
//         setStyleSheet(f.readAll());
//         f.close();
//     }
//     else
//     {
//         auto msg = QString("%1:\n\n%2\n\n%3")
//                        .arg(tr("The theme file could not be opened"), path, f.errorString());
//         WARN << msg;
//         QMessageBox::warning(this, qApp->applicationDisplayName(), msg);
//     }
// }





//
//  PROPERTIES
//

QString Window::light_style_file() const
{ return light_style_file_; }

void Window::set_light_style_file_(QString path)
{
    light_style_file_ = path;
    if (!path.isEmpty() && !QFile(path).exists()){
        QMessageBox::warning(this, qApp->applicationDisplayName(),
                             tr("Theme file '%1' not found. Using default style.").arg(path));
        light_style_file_ = {};
    }

    if (!haveDarkSystemPalette())
        setStyle(light_style_file_.isEmpty() ? Style{} : Style::read(light_style_file_));
}

QString Window::dark_style_file() const
{ return dark_style_file_; }

void Window::set_dark_style_file_(QString path)
{
    dark_style_file_ = path;
    if (!path.isEmpty() && !QFile(path).exists()){
        QMessageBox::warning(this, qApp->applicationDisplayName(),
                             tr("Theme file '%1' not found. Using default style.").arg(path));
        dark_style_file_ = {};
    }

    if (haveDarkSystemPalette())
        setStyle(dark_style_file_.isEmpty() ? Style{} : Style::read(dark_style_file_));
}

std::unique_ptr<QSettings> Window::settings() const
{ return plugin->settings(); }

bool Window::always_on_top() const
{ return windowFlags() & Qt::WindowStaysOnTopHint; }

void Window::set_always_on_top_(bool value)
{ setWindowFlags(windowFlags().setFlag(Qt::WindowStaysOnTopHint, value)); }

class MyShadow : public QGraphicsDropShadowEffect
{
    using QGraphicsDropShadowEffect::QGraphicsDropShadowEffect;

    void draw(QPainter *painter) override
    {
        DEBG << "MyShadow::draw" << painter->clipRegion() << painter->clipBoundingRect()  << painter->clipRegion().boundingRect() << painter->clipBoundingRect() << painter->clipBoundingRect().size();
        QGraphicsDropShadowEffect::draw(painter);
    }

    void sourceChanged(QGraphicsEffect::ChangeFlags flags) override
    {
           DEBG << "MyShadow::sourceChanged" << flags;
        if (flags.testAnyFlags(QGraphicsEffect::SourceInvalidated))
            return;
        else
            QGraphicsDropShadowEffect::sourceChanged(flags);
    }
};

bool Window::display_client_shadow() const
{ return graphicsEffect() != nullptr; }

void Window::set_display_client_shadow_(bool value)
{
    if (graphicsEffect() && !value)
        setGraphicsEffect(nullptr);

    if (!graphicsEffect() && value){
        // Properties
        auto* effect = new MyShadow(this);
        effect->setBlurRadius(DEF_SHADOW_SIZE);
        effect->setColor(QColor(0, 0, 0, 128))  ;
        effect->setXOffset(0.0);
        effect->setYOffset(2.0);
        setGraphicsEffect(effect);  // takes ownership
    }
    value
        ? setContentsMargins(DEF_SHADOW_SIZE,DEF_SHADOW_SIZE,DEF_SHADOW_SIZE,DEF_SHADOW_SIZE)
        : setContentsMargins(0,0,0,0);
}

bool Window::display_system_shadow() const
{ return !windowFlags().testFlag(Qt::NoDropShadowWindowHint); }

void Window::set_display_system_shadow_(bool value)
{ setWindowFlags(windowFlags().setFlag(Qt::NoDropShadowWindowHint, !value)); }

uint Window::max_results() const
{ return results_list.maxItems(); }

void Window::set_max_results_(uint maxItems)
{ results_list.setMaxItems(maxItems); }
