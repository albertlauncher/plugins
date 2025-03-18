// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include <QEvent>
#include <QPoint>
#include <QTimer>
#include <QWidget>
namespace albert {
class Query;
class PluginInstance;
}
class ActionDelegate;
class ActionsList;
class DebugOverlay;
class Frame;
class InputLine;
class ItemDelegate;
class Plugin;
class QEvent;
class QFrame;
class QKeyCombination;
class QPropertyAnimation;
class QStateMachine;
class ResultItemsModel;
class ResultsList;
class SettingsButton;

class Window : public QWidget
{
    Q_OBJECT

public:

    Window(albert::PluginInstance *plugin);
    ~Window();

    QString input() const;
    void setInput(const QString&);

    void setQuery(albert::Query *query);

    const std::map<QString, QString> themes;
    void applyThemeFile(const QString& path);

    std::map<QString, QString> findPalettes() const;
    void applyPalette(const QString& palette_name);  // throws

    bool darkMode() const;

private:

    void initializeUi();
    void initializeProperties();
    void initializeWindowActions();
    void initializeStatemachine();

    bool haveMatches() const;
    bool haveFallbacks() const;

    void onSettingsButtonClicked(Qt::MouseButton button);
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    albert::PluginInstance const * const plugin;

    QStateMachine *state_machine;

    Frame *frame;
    Frame *input_frame;
    InputLine *input_line;
    SettingsButton *settings_button;
    ResultsList *results_list;
    ActionsList *actions_list;
    std::unique_ptr<ResultItemsModel> results_model;
    std::unique_ptr<DebugOverlay> debug_overlay_;
    std::unique_ptr<QPropertyAnimation> color_animation_;
    std::unique_ptr<QPropertyAnimation> speed_animation_;
    bool dark_mode;

    albert::Query *current_query;

    enum Mod {Shift, Meta, Contol, Alt};
    Mod mod_command = Mod::Contol;
    Mod mod_actions = Mod::Alt;
    Mod mod_fallback = Mod::Meta;

    QString theme_light_;
    QString theme_dark_;
    bool hideOnFocusLoss_;
    bool showCentered_;
    bool followCursor_;
    bool quitOnClose_;
    bool shadow_size_;
    bool shadow_offset_;
    QColor settings_button_color_;
    QColor settings_button_color_highlight_;

    enum EventType {
        ShowActions = QEvent::User,
        HideActions,
        ToggleActions,
        ShowFallbacks,
        HideFallbacks,
        SettingsButtonEnter,
        SettingsButtonLeave,
        InputFrameEnter,
        InputFrameLeave,
        QueryUnset,
        QuerySet,
        QueryBusy,
        QueryIdle,
        QueryHaveMatches
    };

    struct Event : public QEvent {
        Event(EventType eventType) : QEvent((QEvent::Type)eventType) {}
    };

    void postCustomEvent(EventType type);

signals:

    void inputChanged(QString);
    void visibleChanged(bool);

public:

    const QString &themeLight() const;
    void setThemeLight(const QString& theme);

    const QString &themeDark() const;
    void setThemeDark(const QString& theme);

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool alwaysOnTop);

    bool clearOnHide() const;
    void setClearOnHide(bool b = true);

    bool displayClientShadow() const;
    void setDisplayClientShadow(bool value);

    bool displayScrollbar() const;
    void setDisplayScrollbar(bool value);

    bool displaySystemShadow() const;
    void setDisplaySystemShadow(bool value);

    bool followCursor() const;
    void setFollowCursor(bool b = true);

    bool hideOnFocusLoss() const;
    void setHideOnFocusLoss(bool b = true);

    bool historySearchEnabled() const;
    void setHistorySearchEnabled(bool b = true);

    uint maxResults() const;
    void setMaxResults(uint max);

    bool quitOnClose() const;
    void setQuitOnClose(bool b = true);

    bool showCentered() const;
    void setShowCentered(bool b = true);

    bool debugMode() const;
    void setDebugMode(bool b = true);


    QBrush windowBackgroundBrush() const;
    void setWindowBackgroundBrush(QBrush);

    uint windowWidth() const;
    void setWindowWidth(uint);

    uint windowPadding() const;
    void setWindowPadding(uint);

    uint windowSpacing() const;
    void setWindowSpacing(uint);

    double windowBorderRadius() const;
    void setWindowBorderRadius(double);

    double windowBorderWidth() const;
    void setWindowBorderWidth(double);

    QBrush windowBorderBrush() const;
    void setWindowBorderBrush(QBrush);


    QBrush inputBackgroundBrush() const;
    void setInputBackgroundBrush(QBrush);

    QBrush inputBorderBrush() const;
    void setInputBorderBrush(QBrush);

    uint inputPadding() const;
    void setInputPadding(uint);

    double inputBorderRadius() const;
    void setInputBorderRadius(double);

    double inputBorderWidth() const;
    void setInputBorderWidth(double);


    uint inputFontSize() const;
    void setInputFontSize(uint);


    QColor settingsButtonColor() const;
    void setSettingsButtonColor(QColor);

    QColor settingsButtonHightlightColor() const;
    void setSettingsButtonHightlightColor(QColor);

    uint settingsButtonSize() const;
    void setSettingsButtonSize(uint);


    QBrush resultItemSelectionBackgroundBrush() const;
    void setResultItemSelectionBackgroundBrush(QBrush val);

    QBrush resultItemSelectionBorderBrush() const;
    void setResultItemSelectionBorderBrush(QBrush val);

    double resultItemSelectionBorderRadius() const;
    void setResultItemSelectionBorderRadius(double);

    double resultItemSelectionBorderWidth() const;
    void setResultItemSelectionBorderWidth(double);

    QColor resultItemTextColor() const;
    void setResultItemTextColor(QColor);

    QColor resultItemSubTextColor() const;
    void setResultItemSubTextColor(QColor);

    QColor resultItemSelectionTextColor() const;
    void setResultItemSelectionTextColor(QColor);

    QColor resultItemSelectionSubTextColor() const;
    void setResultItemSelectionSubTextColor(QColor);

    uint resultItemIconSize() const;
    void setResultItemIconSize(uint);

    uint resultItemTextFontSize() const;
    void setResultItemTextFontSize(uint);

    uint resultItemSubtextFontSize() const;
    void setResultItemSubtextFontSize(uint);

    uint resultItemHorizontalSpace() const;
    void setResultItemHorizontalSpace(uint);

    uint resultItemVerticalSpace() const;
    void setResultItemVerticalSpace(uint);

    uint resultItemPadding() const;
    void setResultItemPadding(uint);


    QBrush actionItemSelectionBackgroundBrush() const;
    void setActionItemSelectionBackgroundBrush(QBrush val);

    QBrush actionItemSelectionBorderBrush() const;
    void setActionItemSelectionBorderBrush(QBrush val);

    double actionItemSelectionBorderRadius() const;
    void setActionItemSelectionBorderRadius(double);

    double actionItemSelectionBorderWidth() const;
    void setActionItemSelectionBorderWidth(double);

    void setActionItemSelectionTextColor(QColor val);
    QColor actionItemSelectionTextColor() const;

    QColor actionItemTextColor() const;
    void setActionItemTextColor(QColor);

    uint actionItemFontSize() const;
    void setActionItemFontSize(uint);

    uint actionItemPadding() const;
    void setActionItemPadding(uint);

signals:

    void alwaysOnTopChanged(bool);
    void clearOnHideChanged(bool);
    void displayClientShadowChanged(bool);
    void displayScrollbarChanged(bool);
    void displaySystemShadowChanged(bool);
    void followCursorChanged(bool);
    void hideOnFocusLossChanged(bool);
    void historySearchEnabledChanged(bool);
    void maxResultsChanged(uint);
    void quitOnCloseChanged(bool);
    void showCenteredChanged(bool);
    void debugModeChanged(bool);
    void themeDarkChanged(QString);
    void themeLightChanged(QString);
};
