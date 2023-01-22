# C++/Qt plugins

Native plugins are based on [Qt Plugins](https://doc.qt.io/qt-6/plugins-howto.html#the-low-level-api-extending-qt-applications).

## Getting started

In your CMakeLists.txt define a project. PROJECT_NAME and PROJECT_VERSION define your plugin id and version. Use the Albert package CMake macro  `albert_plugin`. These macros do a lot of necessary Qt boilerplate for you. See the table below for parameters you can pass. If you are keen on seeing the details check the `cmake` dir in the project root. 

|         Parameter |  Type  | Notes                                                                         |
|------------------:|:------:|-------------------------------------------------------------------------------|
|              NAME | value  | **MANDATORY** Human readable name                                             |
|       DESCRIPTION | value  | **MANDATORY** Brief, imperative description                                   |
|  LONG_DESCRIPTION | value  | *Optional* Longer description or absolute file path to text file (supports Markdown).  |
|           LICENSE | value  | **MANDATORY** Short form e.g. BSD-2-Clause or GPL-3.0                         |
|               URL | value  | **MANDATORY** Browsable source, issues etc                                    |
|          FRONTEND | option | *Optional* Indicates that this plugin realizes the frontend interface         |
|       MAINTAINERS |  list  | *Optional* Active maintainers. Preferrably using mentionable GitHub usernames |
|   QT_DEPENDENCIES |  list  | *Optional* Qt::Core is exported from albert, auto import and link             |
|  LIB_DEPENDENCIES |  list  | *Optional* Required libraries                                                 |
| EXEC_DEPENDENCIES |  list  | *Optional* Required executables                                               |

A CMakeLists.txt of an example plugin could look like this:
```cmake
project(some_plugin_id VERSION 1.0)
albert_plugin(
    NAME "Some pretty name"
    DESCRIPTION "Brief description"
    LICENSE GPL
    URL https://mydomain.com/myurl
    MAINTAINERS @yourname
)
```

A minimal working example of an albert Plugin looks like:

```cpp
#pragma once
#include "albert.h"

class Plugin : public albert::Plugin
{
    Q_OBJECT ALBERT_PLUGIN
};
```

Subclass classes deriving `albert::Extension` to extend the application by loading this plugin. The most common use case is probably `albert::QueryHandler` or one of its subclasses. 


Check the [library interface classes](https://github.com/albertlauncher/albert/tree/master/include/albert) as they are documented and always up to date. Also see the [existing plugins](https://github.com/albertlauncher/plugins/tree/master/) to get a basic understanding. Join our [community chats](https://albertlauncher.github.io/help/#chats) if you need help.
