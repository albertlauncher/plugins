#!/usr/bin/env python3
"""
Albert Python module interface specification v0.4

To work as an albert extension Python modules have to have a particular
interface described below. The Python extension also defines an embedded module
albert which allows the Python modules to interact with the core. The interface
specification and the built-in albert module are versioned together and form the
versioned Python interface. The Python interface is not final. They are
prototypes and intended to be improved on user feedback.

This file is a .pyi declaration of the interface, suitable for use with mypy
type checkers, as well as Python language servers which provide completion. It
contains inline documentation copied from the README [1].

[1]: https://github.com/albertlauncher/plugins/tree/master/python

The following portion of the docstring describes the variables and functions
which each Python extension module must implement. The remainder of this file's
contents are the declarations of the albert module itself.

Variables:

__doc__       Optional [string]. The docstring of the module is used as
              description of the extension. This string will be displayed to the
              user.
__title__     MANDATORY [string]. This variable should hold the pretty name of
              the extension. This string will be displayed to the user.
__version__   MANDATORY [string]. The versioning scheme should be
              [iid_major].[iid_minor].[verion]. The interface id (iid_*) should
              match the pyton interface version. version is the extensions
              version.
              Note that within each iid_maj the API is backwards compatible, but
              as long as albert is in alpha stage iid_major will be 0 and API
              may brake any time.
__triggers__  Optional [string, list of strings]. If this extension should be
              run exclusively, this variable has to hold the trigger that causes
              the extension to be executed.
__authors__   Optional [string, list of strings]. This variable should hold the
              name of the author of the extension.
__exec_deps__ Optional [string, list of strings]. This string should contain any
              dependencies the extension needs to be used. The name of the
              dependency has to match the name of the executable in $PATH.
__py_deps__   Optional [string, list of strings]. This string should contain any
              dependencies the extension needs to be used. The name of the
              dependency has to match the name of the package in the PyPI.

Functions:

handleQuery(Query) MANDATORY. This is the crucial part of a Python module. When
                   the user types a query, this function is called with a query
                   object representing the current query execution. This
                   function should return a list of Item objects.  See the Item
                   class section below.
initialize()       Optional. This function is called when the extension is
                   loaded. Although you could technically run your
                   initialization code in global scope, it is recommended to
                   initialize your extension in this function. If your extension
                   fails to initialize you can raise exceptions here, which are
                   displayed to the user.
finalize()         Optional. This function is called when the extension is
                   unloaded.
"""
from enum import Enum
from typing import Any
from typing import Callable
from typing import List
from typing import Optional
from typing import Union


def debug(arg: Any) -> None:
    """
    Log a message to stdout. Note that debug is effectively a NOP in release
    builds. Puts the passed object into str() for convenience. The messages are
    logged using the QLoggingCategory of the python extension and therefore are
    subject to filter rules.
    """


def info(arg: Any) -> None:
    """
    Log a message to stdout at the "info" log level. Puts the passed object into
    str() for convenience. The messages are logged using the QLoggingCategory of
    the python extension and therefore are subject to filter rules.
    """


def warning(arg: Any) -> None:
    """
    Log a message to stdout at the "info" log level. Puts the passed object into
    str() for convenience. The messages are logged using the QLoggingCategory of
    the python extension and therefore are subject to filter rules.
    """


def critical(arg: Any) -> None:
    """
    Log a message to stdout at the "info" log level. Puts the passed object into
    str() for convenience. The messages are logged using the QLoggingCategory of
    the python extension and therefore are subject to filter rules.
    """


def iconLookup(iconName: Union[str, List[str]]) -> Union[str, List[str]]:
    """
    Perform xdg icon lookup and return a path. Empty if nothing found.
    This function has two forms. Called with a single string argument, it
    returns a single (possibly empty) string. Called with a list of string
    arguments, it returns a list of (possibly empty) string results.
    """


def cacheLocation() -> str:
    """
    Returns the writable cache location of the app. (E.g. $HOME/.cache/albert/
    on Linux)
    """


def configLocation() -> str:
    """
    Returns the writable config location of the app. (E.g. $HOME/.cache/albert/
    on Linux)
    """


def dataLocation() -> str:
    """
    Returns the writable data location of the app. (E.g. $HOME/.cache/albert/
    on Linux)
    """


class Query(object):

    """
    The query class represents a query execution. It holds the necessary
    information to handle a Query. It is passed to the handleQuery function.
    """

    string: str
    """
    This is the actual query string without the trigger. If the query is not
    triggered this string equals rawstring.
    """

    rawString: str
    """
    This is the full query string including the trigger. If the query is not
    triggered this string equals string.
    """

    trigger: str
    """
    This is the trigger that has been used to start this extension.
    """

    isTriggered: bool
    """
    Indicates that this query has been triggered.
    """

    isValid: bool
    """
    This flag indicates if the query is valid. A query is valid untill the query
    manager cancels it. You should regularly check this flag and abort the query
    handling if the flag is False to release threads in the threadpool for the
    next query.
    """

    def disableSort(self) -> bool:
        """
        Indicates that this query has been triggered.
        """


class ItemBase(object):
    """
    The base class for all items. This is a wrapper for the internal Item
    interface. You should not need this unless you need the Urgency enum.
    """

    class Urgency(Enum):

        Alert = 0
        Notification = 1
        Normal = 2


class Item(object):
    """
    Represents a result item. Objects of this class are intended to be returned
    by the handleQuery function.
    """

    id: str
    """
    The identifier string of the item. It is used for ranking algorithms and
    should not be empty.
    """

    icon: str
    """
    The path of the icon displayed in the item.
    """

    text: str
    """
    The primary text of the item.
    """

    actions: List["ActionBase"]
    """
    The actions of the item. See action classes.
    """

    subtext: str
    """
    The secondary text of the item. This text should have informative character.
    """

    completion: str
    """
    The completion string of the item. This string will be used to replace the
    input line when the user hits the Tab key on an item. Note that the
    semantics may vary depending on the context.
    """

    urgency: ItemBase.Urgency
    """
    The urgency of the item. Note that the Urgency enum is defined in the
    ItemBase class. See the Urgency enum.
    """

    def __init__(
            self,
            id: str = "",
            icon: str = ":python_module",
            text: str = "",
            subtext: str = "",
            actions: List["ActionBase"] = [],
            completion: Optional[str] = None,
            urgency: ItemBase.Urgency = ItemBase.Urgency.Normal,
    ):
        """
        id: The identifier string of the item. It is used for ranking algorithms
            and should not be empty.
        icon: The path of the icon displayed in the item.
        text: The primary text of the item.
        actions: The actions of the item. See action classes.
        subtext: The secondary text of the item. This text should have
                 informative character.
        completion: The completion string of the item. This string will be used
                    to replace the input line when the user hits the Tab key on
                    an item.  Note that the semantics may vary depending on the
                    context.
        urgency: The urgency of the item. Note that the Urgency enum is defined
                 in the ItemBase class. See the Urgency enum.
        """

    def addAction(self, action: "ActionBase") -> None:
        """
        Add an action to the item.
        """


class ActionBase(object):
    """
    The base class for all actions is ActionBase. This is a wrapper for the
    internal Action interface. You should not need it (If you think you do I‘d
    be interested why. Please contact me in this case.). There is also a set of
    standard actions subclassing ActionBase which should cover virtually every
    usecases.
    """


class ClipAction(ActionBase):

    def __init__(self, text: str = "", clipboardText=""):
        """
        This class copies the given text to the clipboard on activation.
        """


class UrlAction(ActionBase):

    def __init__(self, text: str = "", url: str = ""):
        """
        This class opens the given URL with the systems default URL handler for
        the scheme of the URL on activation.
        """


class ProcAction(ActionBase):

    def __init__(
            self,
            text: str = "",
            commandline: List[str] = [],
            cwd: Optional[str] = None,
    ):
        """
        This class executes the given commandline as a detached process on
        activation. Optionally the working directory of the process can be set.
        """


class TermAction(ActionBase):

    class CloseBehavior(Enum):

        CloseOnSuccess = 0
        CloseOnExit = 1
        DoNotClose = 2

    def __init__(
            self,
            text: str = "",
            commandline: List[str] = [],
            cwd: Optional[str] = None,
            script: str = "",
            behavior: CloseBehavior = CloseBehavior.CloseOnSuccess,
    ):
        """
        This class executes the given commandline in the terminal set in the
        preferences. Optionally the working directory of the process can be set.
        This is the first form, which uses the commandline and cwd arguments.

        In the second form, script and behavior are used to execute a script of
        commands in the shell.
        """


class FuncAction(ActionBase):

    def __init__(
            self,
            text: str = "",
            callable: Callable[[], None] = lambda: None,
    ):
        """
        This class is a general purpose action. On activation the callable is
        executed.
        """
