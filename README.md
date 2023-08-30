# C++/Qt plugins

Native plugins are based on [Qt Plugins](https://doc.qt.io/qt-6/plugins-howto.html#the-low-level-api-extending-qt-applications).

## Getting started

Define a project in your CMakeLists.txt. Use the Albert CMake macro `albert_plugin`. This macro implements a lot of necessary CMake boilerplate code for you. See the table below for parameters you can pass. If you are keen on seeing the details check the `cmake` dir in the project root.

|         Parameter |  Type  | Notes                                                                                  |
|------------------:|:------:|----------------------------------------------------------------------------------------|
|              NAME | value  | **MANDATORY** Human readable name                                                      |
|       DESCRIPTION | value  | **MANDATORY** Brief, imperative description                                            |
|  LONG_DESCRIPTION | value  | *Optional* Longer description or absolute file path to text file (supports Markdown).  |
|           LICENSE | value  | **MANDATORY** Short form e.g. BSD-2-Clause or GPL-3.0                                  |
|               URL | value  | **MANDATORY** Browsable source, issues etc                                             |
|          FRONTEND | option | *Optional* Indicates that this plugin implements the frontend interface                |
|       MAINTAINERS |  list  | *Optional* Active maintainers. Preferrably using mentionable GitHub usernames          |
|   QT_DEPENDENCIES |  list  | *Optional* Qt::Core is exported from albert, auto import and link                      |
|  LIB_DEPENDENCIES |  list  | *Optional* Required libraries                                                          |
| EXEC_DEPENDENCIES |  list  | *Optional* Required executables                                                        |

A CMakeLists.txt of an example plugin could look like this:
```cmake
project(plugin_identifier VERSION 1.0)
albert_plugin(
    NAME "Plugin pretty name"
    DESCRIPTION "Brief description"
    LICENSE GPL
    URL https://mydomain.com/myurl
    MAINTAINERS @yourname
)
```

A minimal working example of an Albert plugin looks like:

```cpp
#pragma once
#include "albert.h"

class Plugin : public albert::Plugin
{
    Q_OBJECT ALBERT_PLUGIN
};
```

Let your plugin inherit extension classes to extend the application by loading this plugin. Check the [API reference](https://albertlauncher.github.io/reference/namespacealbert.html) or the [library interface headers](https://github.com/albertlauncher/albert/tree/master/include/albert). Probably the official [plugins](https://github.com/albertlauncher/plugins/tree/master/) are a good source to get you started. If you need help, join our [community chats](https://albertlauncher.github.io/help/#chats).
