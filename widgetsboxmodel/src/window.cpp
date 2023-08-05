// Copyright (c) 2022 Manuel Schneider

#include "window.h"
#include "inputline.h"
#include "settingsbutton.h"
#include "itemslist.h"
#include "actionslist.h"
#include "albert/logging.h"
#include <QGuiApplication>
#include <QMouseEvent>
#include <QEvent>
#include <QBoxLayout>
#include <QSpacerItem>
#include <QFrame>
#ifdef Q_OS_MAC
#include <objc/objc-runtime.h>

#include <QCommonStyle>
#include <QProxyStyle>
#include <map>
#endif



class ProxyStyle : public QProxyStyle
{
public:

    ProxyStyle() : QProxyStyle(new QCommonStyle) { }  // Ownership of style is transferred to QProxyStyle.

    void drawControl(ControlElement element,
                     const QStyleOption *option,
                     QPainter *painter,
                     const QWidget *widget = nullptr) const override
    {
        if (element == ControlElement::CE_CustomBase){
            widget->style()->drawControl(QStyle::CE_ItemViewItem, option, painter, widget);
        }
        return QProxyStyle::drawControl(element, option, painter, widget);
    }
};


Window::Window():
    frame(new QFrame(this)),
    input_line(new InputLine(frame)),
    settings_button(new SettingsButton(this)),
    results_list(new ItemsList(frame)),
    actions_list(new ActionsList(frame))

{
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
    frame_layout->setSpacing(0);

    window_layout->setSizeConstraint(QLayout::SetFixedSize);

    input_line->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    results_list->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);
    actions_list->setSizePolicy(QSizePolicy::Policy::Expanding, QSizePolicy::Policy::Fixed);

    settings_button->setFocusPolicy(Qt::NoFocus);
    results_list->setFocusPolicy(Qt::NoFocus);
    actions_list->setFocusPolicy(Qt::NoFocus);
    actions_list->setEditTriggers(QAbstractItemView::NoEditTriggers);



    setStyle(new ProxyStyle);
    setWindowFlags(Qt::Tool|Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
//    setAttribute(Qt::WA_ShowWithoutActivating);

#ifdef Q_OS_MAC
    WId window_id = winId();
    auto call_objc = [](objc_object* object, const char* selector){
        return ((objc_object* (*)(::id, SEL))objc_msgSend)(object, sel_registerName(selector));
    };
    auto call_objc_1 = [](objc_object* object, const char *selector, const auto &param){
        return ((objc_object* (*)(::id, SEL, decltype(param)))objc_msgSend)(object, sel_registerName(selector), param);
    };
    auto *ns_view_object = reinterpret_cast<objc_object *>(window_id);
    objc_object *ns_window = call_objc(ns_view_object, "window");
//    call_objc_1(ns_window, "setCollectionBehavior:",
    ((objc_object* (*)(::id, SEL, int))objc_msgSend)(ns_window, sel_registerName("setCollectionBehavior:"),
                                                     0  // NSWindowCollectionBehaviorDefault - The window appears in only one space at a time.
                                                     //                | 1 << 0   // NSWindowCollectionBehaviorCanJoinAllSpaces - The window appears in all spaces.
                                                     | 1 << 1   // NSWindowCollectionBehaviorMoveToActiveSpace - When the window becomes active, move it to the active space instead of switching spaces.
//        | 1 << 2   // NSWindowCollectionBehaviorManaged - The window participates in Spaces and Exposé.
//        | 1 << 3   // NSWindowCollectionBehaviorTransient - The window floats in Spaces and hides in Exposé.
//        | 1 << 4   // NSWindowCollectionBehaviorStationary - Exposé doesn’t affect the window, so it stays visible and stationary, like the desktop window.
//        | 1 << 5   // NSWindowCollectionBehaviorParticipatesInCycle - The window participates in the window cycle for use with the Cycle Through Windows menu item.
//        | 1 << 6   // NSWindowCollectionBehaviorIgnoresCycle - The window isn’t part of the window cycle for use with the Cycle Through Windows menu item.
//        | 1 << 7   // NSWindowCollectionBehaviorFullScreenPrimary - The window can enter full-screen mode.
//        | 1 << 8   // NSWindowCollectionBehaviorFullScreenAuxiliary - The window can display on the same space as the full-screen window.
//        | 1 << 9   // NSWindowCollectionBehaviorFullScreenNone - The window doesn’t support full-screen mode.
//        | 1 << 11  // NSWindowCollectionBehaviorFullScreenAllowsTiling - The window can be a secondary full screen tile even if it can’t be a full screen window itself.
//        | 1 << 12  // NSWindowCollectionBehaviorFullScreenDisallowsTiling - The window doesn’t support being a full-screen tile window, but may support being a full-screen window.
    );
    call_objc_1(ns_window, "setAnimationBehavior:", 2);  // NSWindowAnimationBehaviorNone
//    ((objc_object* (*)(id, SEL, int))objc_msgSend)(ns_window, sel_registerName("setAnimationBehavior:"), 2);
#endif

}

bool Window::event(QEvent *event)
{
    if (event->type() == QEvent::Close){  // Never close
        hide();
        return true;
    }

    else if (event->type() == QEvent::Move) {
        auto *moveEvent = static_cast<QMoveEvent*>(event);
        DEBG << "moveEvent" << moveEvent->oldPos() << ">" << moveEvent->pos();
    }

    else if (event->type() == QEvent::Resize)  // Let settingsbutton be in top right corner of frame
        settings_button->move(frame->geometry().topRight() - QPoint(settings_button->width()-1,0));

    else if (event->type() == QEvent::MouseMove && !clickOffset_.isNull())
        this->move(static_cast<QMouseEvent*>(event)->globalPosition().toPoint() - clickOffset_);

    else if (event->type() == QEvent::MouseButtonPress)
        clickOffset_ = static_cast<QMouseEvent*>(event)->pos();

    else if (event->type() == QEvent::MouseButtonRelease)
        clickOffset_ = QPoint();  // isNull

    return QWidget::event(event);

//    else if (event->type() == QEvent::ApplicationStateChange) {
//        if (hideOnFocusLoss_){
//            auto* app_state = static_cast<QApplicationStateChangeEvent*>(event);
//            if (app_state->applicationState() == Qt::ApplicationInactive)
//                setVisible(false);
//        }
    //    }
}


void Window::applyTheme(const QString &style_file_path)
{
    QSettings stylefile(style_file_path, QSettings::IniFormat);

    std::map<QString, std::map<QString, QString>> style;  // group > (key > value)

    // Add sensible defaults
    style["*"]["border"] = "none";
    style["#frame"]["min-width"] = "640px";
    style["#frame"]["max-width"] = "640px";
    style["QListView"]["background"] = "transparent";
    static const char* handles = "QListView QScrollBar::add-line:vertical, QScrollBar::sub-line:vertical, QListView QScrollBar::up-arrow:vertical, QScrollBar::down-arrow:vertical, QListView QScrollBar::add-page:vertical, QScrollBar::sub-page:vertical";
    style[handles]["border"] = "0px";
    style[handles]["width"] = "0px";
    style[handles]["height"] = "0px";
    style[handles]["background"] = "transparent";

    // Read the style file
    for (const QString &group : stylefile.childGroups()) {
        stylefile.beginGroup(group);
        for (const QString &key : stylefile.childKeys())
            if (key == "description-font-size")
                style[group]["qproperty-description_font_size"] = stylefile.value(key).toString().remove("px");
            else
                style[group][key] = stylefile.value(key).toString();

        stylefile.endGroup();
    }
    for (const QString &key : stylefile.childKeys())
        style["*"][key] = stylefile.value(key).toString();  // general

    // Buils QStyleSheet string
    QString stylesheet;
    for (const auto &[group, items] : style){
        stylesheet.append(QString("%1 {\n").arg(group));
        for(const auto &[key, value] : items)
            stylesheet.append(QString("%1: %2;\n").arg(key, value));
        stylesheet.append("}\n");
    }

    DEBG << stylesheet;
    // Apply stylesheet
    setStyleSheet(stylesheet);

    // Extend stylesheet

}


