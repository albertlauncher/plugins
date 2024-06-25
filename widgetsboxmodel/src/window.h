// Copyright (c) 2022-2024 Manuel Schneider

#pragma once
#include <QPoint>
#include <QWidget>
namespace albert { class Query; }
class ActionDelegate;
class InputLine;
class ItemDelegate;
class QEvent;
class Plugin;
class QFrame;
class ResizingList;
class SettingsButton;

class Window : public QWidget
{
    Q_OBJECT

public:

    Window(Plugin *plugin);

    QString input() const;
    void setInput(const QString&);
    void setQuery(albert::Query *query);
    void applyThemeFile(const QString& path);

    // Properties

    const std::map<QString, QString> themes;

    const QString &lightTheme() const;
    void setLightTheme(const QString& theme);

    const QString &darkTheme() const;
    void setDarkTheme(const QString& theme);

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

private:

    void init_statemachine();
    bool event(QEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

    Plugin const * const plugin;

    QFrame *frame;
    InputLine *input_line;
    SettingsButton *settings_button;
    ResizingList *results_list;
    ResizingList *actions_list;
    ItemDelegate *item_delegate;
    ActionDelegate *action_delegate;

    QString theme_light_;
    QString theme_dark_;
    bool dark_mode_;
    bool hideOnFocusLoss_{};
    bool showCentered_{};
    bool followCursor_{};
    bool quitOnClose_{};
    bool history_search_{};

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
    void queryFinished();
};

