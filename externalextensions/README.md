# External extensions

***Note: This scripting extension is deprecated as of v0.14.0. Use the Python extension instead***

An external extensions is a regular CGI executable which handles queries for albert.
The type of query is defined by the environment variable `$ALBERT_OP`.
The standard output stream (stdout) is used to return JSON formatted data.

To save state between executions add a dict of strings called 'variables' to the returned JSON dict.
The items of this dict will be set as environment variables in the next execution.

## External extensions communication protocol (v3)

The possible `$ALBERT_OP`s and the expected return values are as follows:

### `METADATA`
Return the metadata of the extension. It should have the
following keys:

|Key|Value|
|---|---|
|`iid`|(string, mandatory) The interface id `iid` (currently `org.albert.extension.external/v3.0` tells the application the type and version of the communication protocol . If the `iid` is incompatible this plugin will not show up in the plugins list. The remaining keys should be self-explanatory. Errors in this step are fatal: loading will not be continued.|
|`version`|(string, defaults to 'N/A')|
|`name`|(string, defaults to $id)|
|`trigger`|(string, defaults to 'empty')|
|`author`|(string, defaults to 'N/A')|
|`dependencies`|(array of strings, defaults to 'empty')|

### `INITIALIZE`
The request to initialize the plugin. The plugin should load potential state from persistant storage, check if all requirements are met and set the exit code accordingly. (Everything but zero is an error). Errors in this step are fatal: loading will not be continued.

### `FINALIZE`
The request to finalize the plugin. The plugin should save the state to persistent storage.

### `QUERY`
The request to handle a query. The environment variable `ALBERT_QUERY` contains the _complete_ query as the user entered it into the input box, i.e. including potential triggers.

**Note:** The process handling `QUERY` can be terminated at any time. Do _not_ change state in this code segment.

Return the results by an array "items" containing JSON objects representing the results. A result object has to contain the following entries: `id`, `name`, `description`, `icon` and `actions`.

|Key|Value|
|---|---|
|`id`| The plugin wide unique id of the result|
|`name`| The name of the result|
|`description`| The description of the result|
|`completion`| The completions string of the result|
|`icon`| The icon of the result (name or path)|
|`actions`| Array of objects representing the actions for the item.|

The `id` of the item will be used to sort the items by usage. The `name`, `icon` and `description` will be displayed together as an item in the results list. `completion` is the string that will be used to replace the text in the input box when the user pressed <kbd>Tab</kbd>. If `icon` is an absolute path, the given file will be used. If the `icon` is not an absolute path, the algorithm described in the [Icon Theme Specification](https://freedesktop.org/wiki/Specifications/icon-theme-spec/) will be used to locate the icon. An object representing an action has to contain the following values: `name`, `command` and `arguments`.

|Key|Value|
|---|---|
|`name`| The actions name|
|`command`| The program to be execute|
|`arguments`| An array of parameters for `command`|

An example:
```json
{
 "items": [{
   "id":"extension.wide.unique.id",
   "name":"An Item",
   "description":"Nice description.",
   "icon":"/path/to/icon",
   "actions":[{
     "name":"Action name 1",
     "command":"program",
     "arguments":["-a", "-b"]
   },{
     "name":"Action name 2",
     "command":"program2",
     "arguments":["-C", "-D"]
   }]
 }],
 "variables": {
   "some_var":"variable",
   "some_other_var":"cool state"
 }
}
```

## Deployment

> Note that the exteral extensions have to be executable to be used by albert

The extension check its data directories for a directory called `extensions`. The name of a data directory is the id of the extension. I the case of the external extension this is `org.albert.extension.externalextensions`. The data directories reside in the data directories of the application defined by [Qt](http://doc.qt.io/qt-5/qstandardpaths.html#StandardLocation-enum). Hence the external extensions would be looked up in the following directories (in this order):

* ~/.local/share/albert/org.albert.extension.externalextensions/extensions
* /usr/local/share/albert/org.albert.extension.externalextensions/extensions
* /usr/share/albert/org.albert.extension.externalextensions/extensions

Ids are guaranteed to be unique. This means that if several of those path contain a plugins with identical ids, only the first found plugin will be used.
