# Python

The Python extension makes the application extendable by embedding Python modules. Since the name of the native extension providing this functionality is *Python extension* and a Python module in this context is called *Python extension* too, this article refers to the Python extensions by using the term *Python modules*.

In the settings of the Python extension you can find a list of installed Python extensions. This list with checkboxes works similar to the list of native extensions. Check the box of a Python module to automatically load it when the Python extension gets initialized. The icon represents the loading status. Dash means unloaded, the green checkmark stands for a successfully loaded module and a red cross indicates an error while loading the extension. In this case you can hover over the item to check its tooltip. There you find any error messages. You can also run Albert from terminal to check for error output.

## Known issues

The Python interpreter shuts down when the Python extension is unloaded. After this, enabling the extension will restart the interpreter. Some modules cannot be re-initialized safely and may cause segfaults after the interpreter has been restarted (numpy!). The issue is that Python itself cannot completely unload extension modules and there are several caveats with regard to interpreter restarting. In short, not all memory may be freed, either due to Python reference cycles or user-created global data. All the details can be found in the CPython documentation.

## Development

How the Python extension interacts with Python modules is defined in the versioned interface specifications. Devs interested in writing extensions should check the interface specification below. Note that interfaces may brake any time as long as albert is still in alpha stage (v0.*).

## Deployment

The extension checks its data directories for a directory called `modules`. The name of a data directory is the id of the extension. In the case of the Python extension this is `org.albert.extension.python`. The data directories reside in the data directories of the application defined by [Qt](http://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum). Hence on linux the modules would be looked up in the following directories (in this order):

```
~/.local/share/albert/org.albert.extension.python/modules
/usr/local/share/albert/org.albert.extension.python/modules
/usr/share/albert/org.albert.extension.python/modules
```

Ids are guaranteed to be unique. This means that if several of those paths contain a plugins with identical ids, the first plugin found will be used.

## Getting started (on Linux)

This example should get you started developing a Python module to extend the functionality of Albert. For more extension examples, see the [albert Python extensions repository](https://github.com/albertlauncher/python).

1. Duplicate the folder `~/.local/share/albert/org.albert.extension.python/modules/api-test`
1. Rename the module but stay consistent with the conventions.
1. Open Albert settings and navigate to Extensions > Python
1. If your module is not listed, reload the Python extension or restart Albert
1. Enable your module (Check the checkbox) next to your extension
1. Create your extension. The Python extension reloads modules on file changes.

Before you code check the Pull Requests. Maybe somebody had your idea already. Finally dont hesitate to share your extension!

## The Python interface specification (v0.4)

To work as an albert extension Python modules have to have a particular interface described below. The Python extension also defines an embedded module `albert`  which allows the Python modules to interact with the core. The interface specification and the built-in albert module are versioned together and form the versioned Python interface. The Python interface is _not_ final. They are prototypes and intended to be improved on user feedback.

### The Python module interface specification

This is the metadata albert reads from your module **before** loading it:

Variable | Description
--- | ---
`__doc__` | **Optional** _[string]_. The docstring of the module is used as description of the extension. This string will be displayed to the user.
`__title__` | _**MANDATORY** [string]_. This variable should hold the pretty name of the extension. This string will be displayed to the user.
`__version__` | _**MANDATORY** [string]_. The versioning scheme should be `[iid_major].[iid_minor].[version]`. The interface id (`iid_*`) should match the pyton interface version. `version` is the extensions version.<br><br>**Note** that within each `iid_maj` the API is backwards compatible, but as long as albert is in alpha stage `iid_major` will be `0` and API may brake any time.
`__triggers__` | **Optional** _[string, list of strings]_. If this extension should be run exclusively, this variable has to hold the trigger that causes the extension to be executed.
`__authors__` | **Optional** _[string, list of strings]_. This variable should hold the name of the author of the extension.
`__exec_deps__`| **Optional** _[string, list of strings]_. This string should contain any dependencies the extension needs to be used. The name of the dependency has to match the name of the executable in $PATH.
`__py_deps__` | **Optional** _[string, list of strings]_. This string should contain any dependencies the extension needs to be used. The name of the dependency has to match the name of the package in the PyPI.

These are the functions an albert extension must/may have:

Function | Description
--- | ---
`handleQuery(Query)` | ***MANDATORY***. This is the crucial part of a Python module. When the user types a query, this function is called with a query object representing the current query execution. This function should return a list of Item objects. See the Item class section below.
`initialize()` | **Optional**. This function is called when the extension is loaded. Although you could technically run your initialization code in global scope, it is recommended to initialize your extension in this function. If your extension fails to initialize you can raise exceptions here, which are displayed to the user.
`finalize()` | **Optional**. This function is called when the extension is unloaded.

### The built-in `albert` module

The built-in albert module exposes several functions and classes for use with Albert.

#### Functions

Function | Description
--- | ---
`debug(obj)`<br>`info(obj)`<br>`warning(obj)`<br>`critical(obj)`|Log a message to stdout. Note that `debug` is effectively a NOP in release builds. Puts the passed object into `str()` for convenience. The messages are logged using the QLoggingCategory of the python extension and therefore are subject to filter rules.
`iconLookup(iconName:str)`|Perform xdg icon lookup and return a path. Empty if nothing found.
`iconLookup(iconName:list)`|Same as above. With multiple candidate names.
`cacheLocation()`|Returns the writable cache location of the app. (E.g. $HOME/.cache/albert/ on Linux)
`configLocation()`|Returns the writable config location of the app. (E.g. $HOME/.config/albert/ on Linux)
`dataLocation()`|Returns the writable data location of the app. (E.g. $HOME/.local/share/albert/ on Linux)

#### The `Query` class

The query class represents a query execution. It holds the necessary information to handle a Query. It is passed to the handleQuery function. It holds the following read-only properties.

Attribute | Description
--- | ---
`string`|This is the actual query string without the trigger. If the query is not triggered this string equals rawstring.
`rawString`|This is the full query string including the trigger. If the query is not triggered this string equals string.
`trigger`|This is the trigger that has been used to start this extension.
`isTriggered`|Indicates that this query has been triggered.
`isValid`|This flag indicates if the query is valid. A query is valid untill the query manager cancels it. You should regularly check this flag and abort the query handling if the flag is `False` to release threads in the threadpool for the next query.
`disableSort()`|Indicates that this query has been triggered.

#### The `Item` class

The base class for all items is `ItemBase`. This is a wrapper for the internal Item interface. You should not need this unless you need the `Urgency` enum. The `Urgency` enum is defined in the `ItemBase` namespace and has the following enum members: `Alert`, `Notification` and `Normal`. The `Item` class represents a result item. Objects of this class are intended to be returned by the handleQuery function. The signature of the constructor is as follows:

```python
Item(id="",
     icon=":python_module",
     text="",
     subtext="",
     actions=[],
     completion=None:str,
     urgency=Urgency.Normal)
```

Note that the default icon path is `:python_module` which is an embedded resource icon depicting a Python script and the urgency defaults to normal.

Attribute | Description
--- | ---
`id`|The identifier string of the item. It is used for ranking algorithms and should not be empty.
`icon`|The path of the icon displayed in the item.
`text`|The primary text of the item.
`actions`|The actions of the item. See [action classes](#action-classes).
`subtext`|The secondary text of the item. This text should have informative character.
`completion`|The completion string of the item. This string will be used to replace the input line when the user hits the Tab key on an item. Note that the semantics may vary depending on the context.
`urgency`|The urgency of the item. Note that the Urgency enum is defined in the ItemBase class. See the Urgency enum.
`addAction(Action)`|Add an action to the item.

#### Action classes

The base class for all actions is `ActionBase`. This is a wrapper for the internal Action interface. You should not need it ( If you think you do I‘d be interested why. Please contact me in this case.). There is also a set of standard actions subclassing `ActionBase` which should cover virtually every usecases.

##### ClipAction
```python
class albert.ClipAction(text="", clipboardText="")
```
This class copies the given text to the clipboard on activation.

##### UrlAction
```python
class albert.UrlAction(text="", url="")
```
This class opens the given URL with the systems default URL handler for the scheme of the URL on activation.

##### ProcAction
```python
class albert.ProcAction(text="", commandline=[], cwd=None:str)
```
This class executes the given commandline as a detached process on activation. Optionally the working directory of the process can be set.

##### TermAction
```python
class albert.TermAction(text="", commandline=[], cwd=None:str)
```
This class executes the given commandline in the terminal set in the preferences. Optionally the working directory of the process can be set.

```python
class albert.TermAction(text="", script="", behavior=CloseBehavior.CloseOnSuccess, cwd=None:str)
```
Convenience wrapper for shell scripts. Executes the script in the user shell in the terminal.

`TermAction` contains the enum `CloseBehavior`, which can be used to specify the desired behavior on command termination. The values should be self-explanatory.
```python
class CloseBehavior(Enum):
    CloseOnSuccess = 0
    CloseOnExit = 1
    DoNotClose = 2
```

##### FuncAction
```python
class albert.FuncAction(text="", callable:callable=lambda:pass)
```
This class is a general purpose action. On activation the callable is executed.

#### Some examples
```python
ClipAction(text='This action description',
           clipboardText='This goes to the cb')

UrlAction(text='This simply opens google',
          url='https://www.google.com/')

ProcAction(text='This action runs sth.',
           commandline=['jupyter', 'notebook'],
           cwd='notebooks/nb1')

TermAction(text='This action runs a command in the terminal.',
           commandline=['jupyter', 'notebook'],
           cwd='~/notebooks/nb1')

TermAction(text='This action runs a script in the user shell in the terminal.',
           script="echo Shell hun | tr 'h' 'f' | cat - ",
           behavior=TermAction.CloseBehavior.DoNotClose
           cwd='~')

def do_sth():
     albert.info("Hello on stdout!")

FuncAction(text="Prints Hello on stdout!", callable=do_sth)

# An item example
Item(id='google',
     text='Google the query',
     subtext='Handy shortcut to google the query',
     actions=[
          UrlAction(text='This simply opens a google search',
                    url='https://www.google.com/search?q=%s' % query.string)
          ClipAction(text='Nifty for dudes assuming you are Google',
                     clipboardText='Dude, https://www.google.com/search?q=%s')
     ])
```
