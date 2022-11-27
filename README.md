# C++/Qt plugins

A native albert plugin is a Qt Plugin which is just a particular shared library. For the details on this see the docs on [Qt Plugins](https://doc.qt.io/qt-6/plugins-howto.html#the-low-level-api-extending-qt-applications). The library tries to abstract over the details as good as possible by providing helper C and CMake macros. 

## Getting started

### CMake

Albert is based on CMake. Use the macros defined in the Albert package to define a target for a plugin. You can either build a plugin in the source tree of the project using `albert_plugin` (for e.g upstream colaboration) use build a dedicated project and use the albert CMake package using `albert_downstream_plugin` and some more CMake directives. Both commands are more or less the same, while `albert_plugin` has some more convenience features. These macros do most of the necessary Qt boilerplate for you. Here are the parameters you can pass:

|         Parameter |  Type  | Notes                                                                             |
|------------------:|:------:|-----------------------------------------------------------------------------------|
|                ID | value  | optional, defaults to dirname                                                     |
|           VERSION | value  | required in in-project builds, overwrites ${PROJECT_VERSION} in downstream builds |
|              NAME | value  | required                                                                          |
|       DESCRIPTION | value  | required                                                                          |
|           LICENSE | value  | required                                                                          |
|               URL | value  | required, should contain code and and readme                                      |
|   NOUSER/FRONTEND | option | optional, FRONTEND implies NOUSER                                                 |
|       MAINTAINERS |  list  | optional                                                                          |
|           AUTHORS |  list  | optional, in source tree built from git log                                       |
|   QT_DEPENDENCIES |  list  | optional, Qt::Core is exported from albert, automated import and link             |
|  LIB_DEPENDENCIES |  list  | optional                                                                          |
| EXEC_DEPENDENCIES |  list  | optional                                                                          |

A CMakeLists.txt of a dedicated downstream plugin could look like this:

```cmake
cmake_minimum_required(VERSION 3.16)  # Needed for CMake top level projects

project(test VERSION 0.1)  # Needed for CMake top level projects

find_package(albert)

albert_downstream_plugin(
    NAME "Pretty name"
    DESCRIPTION "Brief description"
    LICENSE GPL
    URL https://mydomain.com/myurl
    MAINTAINERS @ManuelSchneid3r
)
```
while the equivalent in source plugin CMakeLists.txt would look like
```cmake
albert_plugin(
    VERSION 1.0
    NAME "Pretty name"
    DESCRIPTION "Brief description"
    LICENSE GPL
    URL https://mydomain.com/myurl
    MAINTAINERS @ManuelSchneid3r
)
```

Of course you can add additional CMake directives. If you are keen on seeing the details check the `cmake` dir in the project root. Thats it. Now let's get started writing some C++ code.

### C++

To build an albert plugin include `albert.h`, subclass `albert::Plugin` and put the `Q_OBJECT` and `ALBERT_PLUGIN` in the class declaration.

```cpp
#pragma once
#include "albert.h"

class Plugin : public QObject, public albert::Plugin
{
    Q_OBJECT ALBERT_PLUGIN
};

```

Compile. Done. Congratulations you built a plugin. Now go ahead and give it a purpose. Subclass base classes deriving `albert::Extension` to extend the application by loading this plugin. The most common use case for this app is the `albert::QueryHandler` and its subclasses. Not going into much detail here, since maintaining the documentation doubles the workload. Check the library interface classes as they are documented and always up to date. Also see some reference plugins like the Template and Debug plugins for basic understanding or the other plugins with a real purpose [here](https://github.com/albertlauncher/plugins/tree/master/). 

Join our [community chats](https://albertlauncher.github.io/help/#chats) if you need help.
