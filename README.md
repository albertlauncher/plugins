# C++/Qt plugins

A native plugin is a [Qt Plugin](https://doc.qt.io/qt-6/plugins-howto.html#the-low-level-api-extending-qt-applications), i.e. a shared library providing a particular interface.
Their distribution is not that trivial due to ABI compatibilitiy (system libraries, compiler(-flags), system architecture, Qt versions, etc…).
Therefore developing native plugins is rather worth it if you plan to get your code upstream and finally be shipped with the official plugins.
For personal workflows or less complex use cases Python plugins are the way to go.

## Getting started

The easiest way to build plugins is to checkout the source tree and build the entire app including plugins.
Open the CMake project in your favourite C++ IDE and make it build and run.
From there on you could simply copy an existing plugin.
The following gives a brief overview.
Details may change every now and then anyway.

### CMake

Having a standardized plugin project structure the `albert_plugin` macro takes care of most CMake boilerplate code you will need.
Read the documentation header of the [CMake module](https://raw.githubusercontent.com/albertlauncher/albert/main/cmake/albert-macros.cmake) before you proceed.

A minimal working CMakeLists.txt:

```cmake
cmake_minimum_required(VERSION 3.16)
project(my_plugin VERSION 1.0)
find_package(Albert REQUIRED)
albert_plugin()
```

Check the [CMakeLists.txt files](https://github.com/search?q=repo%3Aalbertlauncher%2Fplugins+path%3A**%2FCMakeLists.txt&type=code) of the official plugins for reference.

### Metadata

A minimal metadata file:

```json
{
    "name": "My Plugin",
    "description": "Do useful stuff",
    "authors": ["@myname"],
    "license": "MIT",
    "url": "https://github.com/myusername/my-albert-plugin",
}
```

Check the [metadata.json files](https://github.com/search?q=repo%3Aalbertlauncher%2Fplugins+path%3A**%2Fmetadata.json&type=code) of the official plugins for reference.

### C++

On the C++ side Qt plugins have to inherit QObject.
The relevant base classes do that for you. 

They also have to contain the `Q_OBJECT` macro and set an interface identifier and the metadata.
The `ALBERT_PLUGIN` macro takes care of this.

Albert expects plugins to inherit the
[`PluginInstance`](https://albertlauncher.github.io/reference/classalbert_1_1PluginInstance.html)
class. Usually you dont want to subclass PluginInstance directly but rather
[`ExtensionPlugin`](https://albertlauncher.github.io/reference/classalbert_1_1ExtensionPlugin.html)
which implements the 
[`Extension`](https://albertlauncher.github.io/reference/classalbert_1_1Extension.html)
interface using the metadata of PluginInstance. The Extension class is the virtual base of all
extension (See the inheritance diagram for the core interfaces).

A working example plugin implementing the GlobalQueryHandler interface:

```cpp
#pragma once
#include <albert/extensionplugin.h>
#include <albert/globalqueryhandler.h>
class Plugin : public albert::ExtensionPlugin,
               public albert::GlobalQueryHandler
{
    ALBERT_PLUGIN
public:
    std::vector<albert::RankItem> 
    handleGlobalQuery(const albert::Query*) const override
    { return {}; }
};
```

Check the [plugin header files](https://github.com/search?q=repo%3Aalbertlauncher%2Fplugins+path%3A**%2FPlugin.h&type=code) of the official plugins for reference.

Ultimately you want to display
[`Item`](https://albertlauncher.github.io/reference/classalbert_1_1Item.html)s and use their
[`Action`](https://albertlauncher.github.io/reference/classalbert_1_1Action.html)s.

Other classes worth noting:
- [`StandardItem`](https://albertlauncher.github.io/reference/classalbert_1_1StandardItem.html)
- [`Query`](https://albertlauncher.github.io/reference/classalbert_1_1Query.html)
- [`TriggerQueryHandler`](https://albertlauncher.github.io/reference/classalbert_1_1TriggerQueryHandler.html)
- [`IndexQueryHandler`](https://albertlauncher.github.io/reference/classalbert_1_1IndexQueryHandler.html)
- [`FallbackHandler`](https://albertlauncher.github.io/reference/classalbert_1_1FallbackHandler.html)

### What's next?

- There are a lot of utilities. Read/Skim through the [albert namespace reference](https://albertlauncher.github.io/reference/namespacealbert.html).
- See the [official native plugins](https://github.com/albertlauncher/plugins/tree/main/) as a reference.
- Read the [source code](https://github.com/albertlauncher/albert/tree/main/).
- Join the [community chats](https://albertlauncher.github.io/help/#chats).
- Build something cool and share it.
- Enjoy Albert!
