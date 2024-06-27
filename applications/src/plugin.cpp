// Copyright (c) 2022-2024 Manuel Schneider

#include "plugin.h"
using namespace std;

QString Plugin::defaultTrigger() const { return QStringLiteral("apps "); }

void Plugin::commonInitialize(unique_ptr<QSettings> &s)
{
    fs_watcher_.addPaths(appDirectories());
    connect(&fs_watcher_, &QFileSystemWatcher::directoryChanged, this, &Plugin::updateIndexItems);

    restore_use_non_localized_name(s);
    connect(this, &Plugin::use_non_localized_name_changed, this, &Plugin::updateIndexItems);
}
