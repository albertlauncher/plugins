// Copyright (c) 2022-2024 Manuel Schneider

#include "plugin.h"
#include <pwd.h>
using namespace std;

QString Plugin::defaultTrigger() const { return QStringLiteral("apps "); }

void Plugin::commonInitialize(unique_ptr<QSettings> &s)
{
    fs_watcher_.addPaths(appDirectories());
    connect(&fs_watcher_, &QFileSystemWatcher::directoryChanged, this, &Plugin::updateIndexItems);

    restore_use_non_localized_name(s);
    connect(this, &Plugin::use_non_localized_name_changed, this, &Plugin::updateIndexItems);
}

#if defined(Q_OS_UNIX)
QString Plugin::userShell()
{
    // Get the user shell (passwd must not be freed)
    passwd *pwd = getpwuid(geteuid());
    if (pwd == nullptr){
        CRIT << "Could not retrieve user shell. Terminal dysfunctional.";
        return {};
    }
    return {pwd->pw_shell};
}
#endif
