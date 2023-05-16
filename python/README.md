# Python

The Python plugin makes the app extendable by Python modules. 

## The Python interface specification

A Python plugin module is required to have the metadata described below and contain a class named `Plugin` which will be instanciated when the plugin is loaded.   

### Plugin metadata

Variable | Description
--- | ---
`md_iid` | **MANDATORY [str]**`<major>.<minor>` The Python plugin interface version this plugin implements. Currently 0.5
`md_version` | **MANDATORY [str]**`<major>.<minor>`  The plugin version 
`md_id` | **Optional [str]** `[a-zA-Z0-9_]` Use to overwrite the default (module name). Note `__name__` gets `albert.` prepended to avoid conflicts.
`md_name` | **MANDATORY [str]** Human readable name
`md_description` | **MANDATORY [str]** A brief, imperative description like "Launchs apps" or "Open files"
`__doc__` | **Recommended [str]** The docstring of the module is used as long description/readme of the extension.
`md_license` | **Recommended [str]** Short form e.g. BSD-2-Clause or GPL-3.0
`md_url` | **Recommended [str]** Browsable source, issues etc
`md_maintainers` | **Optional [str, List(str)]** Active maintainers. Preferrably using mentionable Github usernames.
`md_bin_dependencies` | **Optional [str, List(str)]** Required executables. Have to match the name of the executable in $PATH.
`md_lib_dependencies` | **Optional [str, List(str)]** Required Python packages. Have to match the PyPI package name.
`md_credits` | **Optional [str, List(str)]** Third party credits and license notes 

### Plugin class

The plugin class is the entry point for a plugin. Use it to provide your extensions by subclassing extension base classes provided by the built-in `albert` module. Do not use the constructor, since PyBind11 imposes some inconvenient boilerplate on them. Instead use a `initialize()` and `finalize()` instance function. 

```python
"""
This extension provides a quick introduction on how to use the new Python pluging interface.
Hope you like it.
"""

from albert import *

md_iid = "0.5"
md_version = "1.2"
#md_id = "overwrite"
md_name = "Fancy Name"
md_description = "Do brief, imperative things"
md_license = "BSD-2"
md_url = "https://url.com/to/upstream/sources/and/maybe/issues"
md_maintainers = "@preferrablyYourGithubName"

class Plugin(QueryHandler):
    def id(self):
        return md_id

    def name(self):
        return md_name

    def description(self):
        return md_description

    def initialize(self):
        info('initialize')

    def finalize(self):
        info('finalize')

    def handleQuery(self, query):
        query.add(Item(
            id="Id",
            text="Text",
            subtext="Subtext",
            icon=["xdg:some-xdg-icon-name",
                  "qfip:/path/to/file/a/file/icon/provider/can/handle",
                  ":resource-path",
                  "/full/path/to/icon/file"],
            actions=[Action(
                "trash-open",
                "Open trash",
                lambda path=trash_path: openUrl(path)
            )]
        ))
```


### The built-in `albert` module

The built-in albert module exposes several functions and classes for use with Albert.

#### `debug(obj)`, `info(obj)`, `warning(obj)`, `critical(obj)`

Log a message to stdout. Note that `debug` is effectively a NOP in release builds. Puts the passed object into `str()` for convenience. The messages are logged using the QLoggingCategory of the python extension and therefore are subject to filter rules.

#### `cacheLocation()`, `configLocation()`, `dataLocation()`

Returns the writable cache, config and data location of the app. E.g. on Linux *$HOME/.cache/albert/*, *$HOME/.config/albert/* and *$HOME/.local/share/albert/*.

#### `setClipboardText(text:str='')`

#### `openUrl(url:str='')`

#### `runDetachedProcess(cmdln:list(str)=[], workdir:str='')`

#### `runTerminal(script:str='', workdir:str='', close_on_exit:bool=False)`

#### `sendTrayNotification(title:str='', msg:str='', ms:int=10000)`

#### `albert.Action(id:str, text:str, callable:object)`

Represents the internal `albert::Action` class.

#### `albert.Item(id:str='', text:str='', subtext:str='', completion:str='', icon:list(str)=[], actions:list(Action)=[])`

See the C++ documentation on [`albert::Item`](https://github.com/albertlauncher/albert/blob/master/include/albert/extensions/item.h) interface for more info. Corresponds to the internal class [`albert::StandardItem`](https://github.com/albertlauncher/albert/blob/master/include/albert/util/standarditem.h) 

#### `albert.Extension()`

Corresponds to the internal class [`albert::Extension`](https://github.com/albertlauncher/albert/blob/master/include/albert/extension.h) which is a virtual base class for all extensions. You _have to_ override the following functions in all subclasses.

Function| Description
--- | ---
`albert.Extension.id()` | ***MANDATORY*** Return the extension id
`albert.Extension.name()` | ***MANDATORY*** Return the human readable extension name 
`albert.Extension.description()` | ***MANDATORY*** Return an imperative, brief description 

#### `albert.QueryHandler(albert.Extension)`

Corresponds to the internal extension class [`albert::QueryHandler`](https://github.com/albertlauncher/albert/blob/master/include/albert/extensions/queryhandler.h). Subclass it to provide a query handling extension.

Function| Description
--- | ---
`albert.QueryHandler.handleQuery(Query)` | ***MANDATORY***. When the user types a query, this function is called with a query object representing the current query execution. See the Query section below.
`albert.QueryHandler.synopsis()` | **Optional** Return a synopsis to display on empty queries
`albert.QueryHandler.defaultTrigger()` | **Optional** Return a default trigger overwrite. Defaults to extension id.
`albert.QueryHandler.allowTriggerRemap()` | **Optional** Return a bool indicating if the user is allowed to remap the trigger. Defaults to True.

#### `albert.Query()`

The query class represents a user query and is passed to the handleQuery function when the user starts a query.

Attirbute | Description
--- | ---
`albert.Query.trigger()` | Returns the trigger used to trigger this query
`albert.Query.string()` | Returns the query string _without_ the trigger
`albert.Query.isValid()` | This flag indicates if the query is valid. A query is valid until the query manager cancels it. You should regularly check this flag and abort the query handling if the flag is `False` to release threads in the threadpool for the next query.
`albert.Query.add(Item)` | Adds a single item to the query
`albert.Query.add(list(Item))` | Adds a list of items to the query

## Deployment

The Python plugin scans `$DATADIR/python/plugins` for Python modules implementing the Python interface spec (See [AppDataLocation](http://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum) for `$DATADIR` values).
Ids are guaranteed to be unique. If several of those paths contain a plugins with identical ids, the first plugin found will be used.

## Known issues

The Python interpreter shuts down when the Python extension is unloaded. After this, enabling the extension will restart the interpreter. Some modules cannot be re-initialized safely and may cause segfaults after the interpreter has been restarted (numpy!). The issue is that Python itself cannot completely unload extension modules and there are several caveats with regard to interpreter restarting. In short, not all memory may be freed, either due to Python reference cycles or user-created global data. All the details can be found in the CPython documentation.
