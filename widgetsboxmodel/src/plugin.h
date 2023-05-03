// Copyright (c) 2022-2023 Manuel Schneider

#pragma once
#include "albert.h"
#include "window.h"
#include <QString>
#include <map>
#include <memory>
class ItemsList;
class ActionsList;
class SettingsButton;
class InputLine;


class Plugin : public albert::ExtensionPlugin,
               public albert::TriggerQueryHandler,
               public albert::Frontend
{
    Q_OBJECT ALBERT_PLUGIN
public:
    Plugin();

    const std::map<QString, QString> &themes() const;
    const QString &theme() const;
    bool setTheme(const QString& theme);

    uint maxResults() const;
    void setMaxResults(uint max);

    bool showCentered() const;
    void setShowCentered(bool b = true);

    bool followCursor() const;
    void setFollowCursor(bool b = true);

    bool hideOnFocusLoss() const;
    void setHideOnFocusLoss(bool b = true);

    bool quitOnClose() const;
    void setQuitOnClose(bool b = true);

    bool historySearchEnabled() const;
    void setHistorySearchEnabled(bool b = true);

    bool clearOnHide() const;
    void setClearOnHide(bool b = true);

    bool alwaysOnTop() const;
    void setAlwaysOnTop(bool alwaysOnTop);

    bool displayIcons() const;
    void setDisplayIcons(bool value);

    bool displayScrollbar() const;
    void setDisplayScrollbar(bool value);

    bool displayClientShadow() const;
    void setDisplayClientShadow(bool value);

    bool displaySystemShadow() const;
    void setDisplaySystemShadow(bool value);

    bool showFallbacksOnEmptyMatches() const;
    void setShowFallbacksOnEmptyMatches(bool value);

    // albert::Plugin
    QWidget* buildConfigWidget() override;

    // albert::Frontend
    bool isVisible() const override;
    void setVisible(bool visible) override;
    QString input() const override;
    void setInput(const QString&) override;
    QWidget* createFrontendConfigWidget() override;

    // albert::QueryHandler
    QString defaultTrigger() const override;
    void handleTriggerQuery(TriggerQuery &query) const override;

private:
    void init_statemachine();
    bool eventFilter(QObject *watched, QEvent *event) override;

    Window window;
    std::shared_ptr<albert::Query> current_query;
    std::shared_ptr<albert::Query> displayed_query;
    QTimer display_delay_timer;
    albert::History history_;
    QString user_text; // used for history search
    std::map<QString, QString> themes_;
    std::list<std::shared_ptr<albert::Query>> queries_;
    QString theme_;
    bool show_fallbacks_on_empty_query = true;
    bool hideOnFocusLoss_{};
    bool showCentered_{};
    bool followCursor_{};
    bool quitOnClose_{};
    bool clearOnHide_{};
    bool history_search_{};

    enum class Mod {Shift, Meta, Contol, Alt};
    Mod mod_command = Mod::Contol;
    Mod mod_actions = Mod::Alt;
    Mod mod_fallback = Mod::Meta;

signals:  // Workaround for failing dynamic transitions
    void queryFinsished();
    void resultsReady();
};
