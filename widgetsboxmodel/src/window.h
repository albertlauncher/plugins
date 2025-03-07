// Copyright (c) 2022-2025 Manuel Schneider

#pragma once
#include <QPoint>
#include <QWidget>
namespace albert {
class Query;
class PluginInstance;
}
class ActionDelegate;
class DebugOverlay;
class InputLine;
class ItemDelegate;
class Plugin;
class QEvent;
class QFrame;
class ResizingList;
class ResultItemsModel;
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
    void applyThemeFile(const QString& path);

    const std::map<QString, QString> themes;

    bool darkMode() const;

private:

    void initializeUi();
    void initializeProperties();
    void initializeWindowActions();
    void initializeStatemachine();

    void onSettingsButtonClicked(Qt::MouseButton button);
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    albert::PluginInstance const * const plugin;

    QFrame *frame;
    InputLine *input_line;
    SettingsButton *settings_button;
    ResizingList *results_list;
    ResizingList *actions_list;
    ItemDelegate *item_delegate;
    ActionDelegate *action_delegate;
    std::unique_ptr<ResultItemsModel> results_model;
    std::unique_ptr<DebugOverlay> debug_overlay_;
    bool dark_mode;

    albert::Query *current_query;

    enum class Mod {Shift, Meta, Contol, Alt};
    Mod mod_command = Mod::Contol;
    Mod mod_actions = Mod::Alt;
    Mod mod_fallback = Mod::Meta;

signals:

    void inputChanged(QString);
    void visibleChanged(bool);
    void queryChanged();
    void queryMatchesAdded();
    void queryStateBusy();
    void queryStateIdle();

    // Properties

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

private:

    QString theme_light_;
    QString theme_dark_;
    bool hideOnFocusLoss_;
    bool showCentered_;
    bool followCursor_;
    bool quitOnClose_;
    bool history_search_;

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

