// Copyright (c) 2022-2024 Manuel Schneider

#include "application.h"
#include "plugin.h"
#include "terminal.h"
#include "ui_configwidget.h"
#include <QDir>
#include <QMessageBox>
#include <QWidget>
#include <albert/logging.h>
using namespace albert;
using namespace std;

namespace {
static QStringList appDirectories()
{
    return {
        "/Applications",
        "/Applications/Utilities",
        "/System/Applications",
        "/System/Applications/Utilities",
        "/System/Cryptexes/App/System/Applications",  // Safari Home
        "/System/Library/CoreServices/Applications",
        "/System/Library/CoreServices/Finder.app/Contents/Applications"
    };
}
}


Plugin::Plugin()
{
    auto s = settings();
    commonInitialize(s);

    fs_watcher.addPaths(appDirectories());
    connect(&fs_watcher, &QFileSystemWatcher::directoryChanged, this, &Plugin::updateIndexItems);

    indexer.parallel = [this](const bool &abort)
    {
        vector<shared_ptr<applications::Application>> apps;

        apps.emplace_back(make_shared<Application>("/System/Library/CoreServices/Finder.app",
                                                   use_non_localized_name_));

        for (const auto &path : appDirectories())
            for (const auto &fi : QDir(path).entryInfoList({"*.app"}))
                if (abort)
                    return apps;
                else
                    try {
                        apps.emplace_back(make_shared<Application>(fi.absoluteFilePath(),
                                                                   use_non_localized_name_));
                    } catch (const runtime_error &e) {
                        WARN << e.what();
                    }

        ranges::sort(apps, [](const auto &a, const auto &b){ return a->id() < b->id(); });

        return apps;
    };

    indexer.finish = [this](auto &&result)
    {
        INFO << QString("Indexed %1 applications (%2 ms).")
                .arg(result.size()).arg(indexer.runtime.count());
        applications = ::move(result);

        // Add terminals (replace apps by polymorphic type)

        terminals.clear();

        auto binary_search = [](auto &apps, const auto &id)
        {
            auto lb = lower_bound(apps.begin(), apps.end(), id,
                                  [](const auto &app, const auto &id){ return app->id() < id; });
            if (lb != apps.end() && (*lb)->id() == id)
                return lb;
            return apps.end();
        };

        if (auto it = binary_search(applications, "com.apple.Terminal"); it != applications.end())
        {
            auto t = make_shared<Terminal>(
                *static_cast<::Application*>(it->get()),
                R"(tell application "Terminal" to activate
                   tell application "Terminal" to do script "exec %1")"
            );
            *it = static_pointer_cast<applications::Application>(t);
            terminals.emplace_back(t.get());
        }

        if (auto it = binary_search(applications, "com.googlecode.iterm2"); it != applications.end())
        {
            auto t = make_shared<Terminal>(
                *static_cast<::Application*>(it->get()),
                R"(tell application "iTerm" to create window with default profile command "%1")"
            );
            *it = static_pointer_cast<applications::Application>(t);
            terminals.emplace_back(t.get());
        }

        setUserTerminalFromConfig();

        setIndexItems(buildIndexItems());

        emit appsChanged();
    };
}

QWidget *Plugin::buildConfigWidget()
{
    auto *w = new QWidget;
    Ui::ConfigWidget ui;
    ui.setupUi(w);

    addBaseConfig(ui.formLayout);

    return w;
}
