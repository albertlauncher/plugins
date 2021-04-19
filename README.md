# C++/Qt plugins

A native albert extension is a [Qt Plugin](http://doc.qt.io/qt-5/plugins-howto.html#the-low-level-api-extending-qt-applications.) which is nothing else but a special shared library. A plugin has to have the correct interface id (IID) and of course to implement this interfaces to be loaded, e.g. `Core::Extension` or `Core::Frontend`. The best way to to get an overview is to read the [core library interface](https://github.com/albertlauncher/albert/tree/master/include/albert) classes. The headers comments and the other plugins especially the [template extension](https://github.com/albertlauncher/plugins/tree/master/templateExtension) should get you started.

The internal API is still not final yet. If you want to write a plugin check the other extensions. There are some caveats and requirements you should know:

- Qt Plugin needs a metadata file
- The metadata needs a unique id to be loaded
- The metadata file name must match the name in the `Q_PLUGIN_METADATA` macro
- The interface id defined in `Q_PLUGIN_METADATA` must match the current one defined in the interface headers for the plugin to be loaded.

## Getting started

The best way to get started is to copy the [template extension](https://github.com/albertlauncher/plugins/tree/master/templateExtension) and adjust the contents.

To keep the code readable there are some conventions that are not strictly necessary, but the intention is to unify the filenames of the plugins.The main class of the extension is called `Extension` and if the extension returns a configuration widget the class shall be called `ConfigWidget`. The metadata file is called `metadata.json`. This would implicitly lead to naming conflicts, therefor all classes of an extensions live in a dedicated namespace having the name of the extension. In bullets:

- Copy the template extension.
- Adjust the values contents in `metadata.json` and project name in `CmakeLists.txt`.
- Rename the namespace. Remember to define the namespace in the `*.ui` files, too.
- Make sure to have checked all core library headers .
- Implement your extension.

Contact us if you need [help](/help/).

## Extension plugins

Implement the `Core::Extension` and `Core::QueryHandler` interface. Especially `Core::Extension` is not final yet. But this should not be a problem. Since changes would need you to just change a few lines of code.

`Core::QueryHandler` has several functions that will be called on special events. Most important is the virtual function `handleQuery(Query)` this function will be called when the user types in his queries.

The `Core::Query` object contains all necessary information and accepts objects of abstract type `Core::Item`. Subclass it or use `Core::StandardItem`. The items interface has a getter for actions of abstract type `Core::Action`. Again subclass it or use `Core::StandardAction`. Furter there is the `Core::IndexItem` interface with its standard implementation `Core::StandardIndexItem`. These items are for the use with the utility class `Core::OfflineIndex` which does basic offline indexing and searching for you.

To get a detailed description of the interfaces read the header files of the core library interface classes.

## Frontend plugins

Implement the `Core::Frontend` interface. Implementing a frontend is a cumbersome process with tons of caveats. I will rather not write a documentation on it. [Contact](/help/) us directly.


## The plugin metadata

The plugin metadata is a mandatory file that is needed to compile the plugin. Its content is *JSON* formatted and its name has to be equal to the the one specified in the `Q_PLUGIN_METADATA` in the extensions main class. The convention is to call it `metadata.json`. Its fields give the application information about the plugin without having to load the plugin.

Currently the plugin specification has the following keys:
- `id` is the unique identifier of the app. A plugin will not be loaded if its id has been registered already by an other plugin.
- `name` is the pretty printed name of the plugin.
- `version` is, well, the version of the plugin.
- `author` name of the developer of this plugin.
- `dependencies` is an array of dependencies of this plugin. These dependencies are mandatory for the plugin but optional for the application. The user is responsible to install them.

A plugin specification could look like the following:

```json
{
    "id" :              "org.albert.extension.bookmarks",
    "name" :            "Bookmarks",
    "version" :         "1.1",
    "author" :          "Manuel Schneider",
    "dependencies" :    []
}
```
