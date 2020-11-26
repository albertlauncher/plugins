The applications extension makes desktop applications runnable by albert. The concept of desktop applications is described in the [freedesktop.org Desktop Entry Specification](https://specifications.freedesktop.org/desktop-entry-spec/latest/).

Desktop entries are looked up in the _applications_ directory in the paths set in `$XDG_DATA_HOME` and `$XDG_DATA_DIRS` environment variables. If they are not set, they default to `~/.local/share` and `/usr/local/share:/usr/share`.

Every application is represented by a desktop file which has an ID. The desktop file ID is built using the path of the desktop file relative to the applications directory and turning '/' into '-'. If multiple files have the same desktop file ID, the first one in the precedence order given by the paths above is used. The standard action of an application item is to start the application, the alternative actions are defined by the desktop actions in the desktop entry.

The fuzzy option in the settings dialog allows error tolerant matching of the items.
