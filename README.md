# C++/Qt plugins

Native plugins are basically shared libraries.
Their distribution is not that trivial due to ABI compatibilitiy (system libraries, compiler(-flags), system architecture, Qt versions, etc…).
Therefore developing native plugins is rather worth it if you plan to get your code upstream and finally be shipped with the official plugins.
For personal workflows or less complex use cases Python plugins are the way to go.

## Getting started

The easiest way to build plugins is to checkout the source tree and build the entire app including plugins.
Open the CMake project in your favourite C++ IDE and make it build and run.
There may be some caveats, so probably you want to join the chats and ask in case of questions.
From there on you could simply copy an existing plugin, e.g. the template plugin, and start implementing your extension. 
The following gives a brief overview.
Details may change every now and then anyway. 

## CMake

A native plugin is a [Qt Plugin](https://doc.qt.io/qt-6/plugins-howto.html#the-low-level-api-extending-qt-applications), i.e. a shared library providing a particular interface.
To build such a library you have to define CMake targets and create an appropiate metadata file.
The [CMake module](https://raw.githubusercontent.com/albertlauncher/albert/master/cmake/albert-macros.cmake) provides convenience macros for this purpose.
You should probably skim through this module once.
The `albert_plugin` macro should be sufficient for most trivial plugins:

```cmake
albert_plugin (
    NAME name 
    DESCRIPTION description
    LICENSE licencse
    URL url
    [LONG_DESCRIPTION long_description]
    [FRONTEND]
    [MAINTAINERS ...]
    [QT_DEPENDENCIES ...]
    [LIB_DEPENDENCIES ...]
    [EXEC_DEPENDENCIES ...]
)
```


|         Parameter |  Type  | Notes |
|------------------:|:------:|---|
|              NAME | value  | Human readable name. |
|       DESCRIPTION | value  | Brief, imperative description, e.g. "Open files". |
|           LICENSE | value  | Short form, e.g. BSD-2-Clause or GPL-3.0. |
|               URL | value  | Browsable online source, issues etc. |
|  LONG_DESCRIPTION | value  | Longer description or absolute file path to a text file (supports Markdown). |
|          FRONTEND | option | Indicates that this plugin implements the frontend interface. |
|       MAINTAINERS |  list  | A list of active maintainers. Preferrably using mentionable GitHub usernames. |
|   QT_DEPENDENCIES |  list  | Qt dependencies to import and link. Qt::Core is in the public interface of libalbert. |
|  LIB_DEPENDENCIES |  list  | Required libraries. Displayed to the user. |
| EXEC_DEPENDENCIES |  list  | Required executables. Displayed to the user. |


## C++

On the C++ side you have to tell the Qt MOC which interface the plugin implements and where the metadata is located.
The `ALBERT_PLUGIN` define takes care of this.
The MOC is triggered by the `QOBJECT` define.
The fundamental base class for _all_ plugins is `albert::PluginInstance`.
It is subclassed by the convenience classes for native plugins in the [`albert::plugin`](https://albertlauncher.github.io/reference/namespacealbert_1_1plugin.html) namespace.
Read their documentation before you proceed.
Check the inheritance diagram of the [`albert::Extension`](https://albertlauncher.github.io/reference/classalbert_1_1Extension.html) class for available extensions, especially the `albert::TriggerQueryHandler` and its subclasses.
By now you should understand a plugin class declaration like this: 

```cpp
#pragma once
#include "albert/extension/queryhandler/triggerqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin<albert::TriggerQueryHandler>
{
    Q_OBJECT ALBERT_PLUGIN
    ...
};
```

Now implement the virtual functions of the abstract classes you inherit.
Ultimately you want to display and activate items.
See 
 * the [`albert::Action`](https://albertlauncher.github.io/reference/classalbert_1_1Action.html),
 * the abstract [`albert::Item`](https://albertlauncher.github.io/reference/classalbert_1_1Item.html) and
 * the [`albert::StandardItem`](https://albertlauncher.github.io/reference/structalbert_1_1StandardItem.html) class.

Self explanatory examples serve way better as educational source than hundreds of lines of text.
See the [official native plugins](https://github.com/albertlauncher/plugins/tree/master/) as a reference.
The `debug`, `template` and `hash` plugins are good plugins to start reading. 

Finally you may want to skim through the entire [albert namespace](https://albertlauncher.github.io/reference/namespacealbert.html).

If you need help, join our [community chats](https://albertlauncher.github.io/help/#chats).
