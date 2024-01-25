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
Albert uses CMake and provides convenient macros, most notably the `albert_plugin` macro, you can utilize to get started without having to write a lot of CMake boilerplate code.
Read the documentation header of the [CMake module](https://raw.githubusercontent.com/albertlauncher/albert/master/cmake/albert-macros.cmake) before you proceed.

This basic CMakeLists.txt is sufficient to build a basic plugin without dependencies and translations:

```cmake
project(my_plugin VERSION 1.0)
albert_plugin(
    SOURCE_FILES
        src/*
)
```

Unless you specify `METADATA` in the `albert_plugin` macro, a metadata file is expected to be found at `metadata.json`.

Supported metadata keys:


|            Parameter |     Type     | Notes                                                                     |
|---------------------:|:------------:|---------------------------------------------------------------------------|
|                   id |              | Reserved. Added by CMake.                                                 |
|              version |              | Reserved. Added by CMake.                                                 |
|                 name | local string | Human readable name.                                                      |
|          description | local string | Brief, imperative description, e.g. "Open files".                         |
|              license |    string    | SPDX license identifier. E.g. BSD-2-Clause, MIT, LGPL-3.0-only, …         |
|                  url |    string    | Browsable online source code, issues etc.                                 |
|              authors | string list  | List of copyright holders. Preferably using mentionable GitHub usernames. |
| runtime_dependencies | string list  | Default: `[]`. Required libraries.                                        |
|  binary_dependencies | string list  | Default: `[]`. Required executables.                                      |
|  plugin_dependencies | string list  | Default: `[]`. Required plugins.                                      |
|              credits | string list  | Default: `[]`. Attributions, mentions, third party library licenses, …    |
|             loadtype |    string    | Default: `user`. `frontend` or `user`.                      |

A basic metadata file looks like this:

```json
{
    "name": "My Plugin",
    "description": "Do useful stuff",
    "authors": ["@myname"],
    "license": "MIT",
    "url": "https://github.com/myusername/my-albert-plugin",
}
```
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
#include "albert/extension/queryhandler/globalqueryhandler.h"
#include "albert/plugin.h"

class Plugin : public albert::plugin::ExtensionPlugin, public albert::GlobalQueryHandler
{
    Q_OBJECT ALBERT_PLUGIN
public:
    std::vector<albert::RankItem> handleGlobalQuery(const GlobalQuery*) const override;
    QWidget *buildConfigWidget() override;
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

Finally you may want to skim through the entire [albert namespace](https://albertlauncher.github.io/reference/namespacealbert.html).

If you need help, join our [community chats](https://albertlauncher.github.io/help/#chats).
