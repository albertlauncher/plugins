"""
.. https://docutils.sourceforge.io/docs/ref/rst/restructuredtext.html
.. https://www.sphinx-doc.org/en/master/usage/restructuredtext/basics.html

====================================================================================================
Albert Python interface v3.0
====================================================================================================

To be a valid Python plugin a Python module has to contain at least the mandatory metadata fields
and a class named ``Plugin`` that inherits the :class:`PluginInstance` class.

See Also:
    `Albert C++ Reference <https://albertlauncher.github.io/reference/namespacealbert.html>`_

----------------------------------------------------------------------------------------------------
Mandatory metadata variables
----------------------------------------------------------------------------------------------------

``md_iid`` : *str*
    Python interface version (<major>.<minor>)

``md_version`` : *str*
    Plugin version (<major>.<minor>)

``md_name`` : *str*
    Human readable name

``md_description`` : *str*
    A brief, imperative description. (Like "Launch apps" or "Open files")


----------------------------------------------------------------------------------------------------
Optional metadata variables
----------------------------------------------------------------------------------------------------

``md_license`` : *str*
    Short form e.g. MIT or BSD-2

``md_url`` : *str*
    Browsable source, issues etc

``md_authors`` : *List(str)*
    The authors. Preferably using mentionable Github usernames.

``md_bin_dependencies`` : *List(str)*
    Required executable(s). Have to match the name of the executable in $PATH.

``md_lib_dependencies`` : *List(str)*
    Required Python package(s). Have to match the PyPI package name.

``md_credits`` : *List(str)*
    Third party credit(s) and license notes


====================================================================================================
Changelog
====================================================================================================


- ``v3.0``

  - Drop metadata field ``md_id``.
  - ``PluginInstance``

    - Add ``extensions(…)``.
    - Drop ``__init__(…)`` parameter ``extensions``. Use ``extensions(…)``.
    - Drop ``de``-/``registerExtension(…)``. Use ``extensions(…)``.
    - Drop ``initialize(…)``/``finalize(…)``. Use ``__init__(…)``/``__del__(…)``.
    - Make property a method:

      - ``id``
      - ``name``
      - ``description``
      - ``cacheLocation``
      - ``configLocation``
      - ``dataLocation``

    - Do not implicitly create the directory in:

      - ``cacheLocation``
      - ``configLocation``
      - ``dataLocation``

  - Revert the property based approach of the extenision hierarchy. I.e. drop all properties and
    constructors in relevant classes:

    - ``Extension``
    - ``TriggerQueryHandler``
    - ``GlobalQueryHandler``
    - ``IndexQueryHandler``
    - ``FallbackHandler``

  - ``Item``: Make all readonly properties methods.
  - ``RankItem.__init__(…)`` add overload that takes a ``Match``.
  - ``MatchConfig``: Add new class.
  - ``Matcher.__init__(…)``: Add optional parameter ``config`` of type ``MatchConfig``.
  - ``runTerminal(…)``:

      - Drop parameter ``workdir``. Prepend ``cd <workdir>;`` to the script.
      - Drop parameter ``close_on_exit``. Append ``exec $SHELL;`` to the script.

  - Add ``openFile(…)``.

- ``v2.5``

  - Matcher now not considered experimental anymore.
  - Add ``Matcher.match(strings: List[str])``.
  - Add ``Matcher.match(*args: str)``.

- ``v2.4``

  - Deprecate parameter ``workdir`` of runTerminal. Prepend ``cd <workdir>;`` to your script.
  - Deprecate parameter ``close_on_exit`` of runTerminal. Append ``exec $SHELL;`` to your script.

- ``v2.3``

  - Module

    - Deprecate ``md_id``. Use ``PluginInstance.id``.

  - PluginInstance:

    - Add read only property ``id``.
    - Add read only property ``name``.
    - Add read only property ``description``.
    - Add instance method ``registerExtension(…)``.
    - Add instance method ``deregisterExtension(…)``.
    - Deprecate ``initialize(…)``. Use ``__init__(…)``.
    - Deprecate ``finalize(…)``. Use ``__del__(…)``.
    - Deprecate ``__init__`` extensions parameter. Use ``(de)``-/``registerExtension(…)``.
    - Auto(de)register plugin extension if ``Plugin`` is instance of ``Extension``.

  - Use ``Query`` instead of ``TriggerQuery`` and ``GlobalQuery``.

    - The interface is backward compatible, however type hints may break.

  - Add ``Matcher`` and ``Match`` convenience classes.
  - Notification:

    - Add property ``title``.
    - Add property ``text``.
    - Add instance method ``send()``.
    - Add instance method ``dismiss()``.
    - Note: Notification does not display unless ``send(…)`` has been called.

- ``v2.2``

  - ``PluginInstance.configWidget`` supports ``'label'``
  - ``__doc__`` is not used anymore, since 0.23 drops ``long_description`` metadata
  - Deprecate ``md_maintainers`` not used anymore
  - Add optional variable ``md_authors``

- ``v2.1``

  - Add ``PluginInstance.readConfig``
  - Add ``PluginInstance.writeConfig``
  - Add ``PluginInstance.configWidget``

"""

from abc import abstractmethod, ABC
from typing import Any
from typing import Callable
from typing import List
from typing import Optional
from typing import overload
from pathlib import Path

class PluginInstance(ABC):
    """
    See also:
        `PluginInstance C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1PluginInstance.html>`_
    """

    def id(self) -> str:
        """
        Returns the id from the plugin metadata.
        """

    def name(self) -> str:
        """
        Returns the name from the plugin metadata.
        """

    def description(self) -> str:
        """
        Returns the description from the plugin metadata.
        """

    def cacheLocation(self) -> Path:
        """
        Returns the recommended cache location for this plugin.
        """

    def configLocation(self) -> Path:
        """
        Returns the recommended config location for this plugin.
        """

    def dataLocation(self) -> Path:
        """
        Returns the recommended data location for this plugin.
        """

    def extensions(self) -> List[Extension]:
        """
        Returns the extensions of this plugin. You are responsible to keep the extensions alive for
        the lifetime of this plugin. The base class implementation returns ``self`` if the plugin
        is an instance of ``Extension``, otherwise an empty list.
        """

    def readConfig(self, key: str, type: type[str|int|float|bool]) -> str|int|float|bool|None:
        """
        Returns the config value for ``key`` from the Albert settings or ``None`` if the value does
        notexist or errors occurred. Due to limitations of QSettings on some platforms the type may
        be lost, therefore the ``type`` has to be passed.
        """

    def writeConfig(self, key: str, value: str|int|float|bool):
        """
        Writes ``value`` to ``key`` in the Albert settings.
        """

    def configWidget(self) -> List[dict]:
        """
        Returns a list of dicts, describing a form layout as described below.

        **Descriptive config widget factory.**

        Define a static config widget using a list of dicts, each defining a row in the resulting
        form layout. Each dict must contain key ``type`` having one of the supported types specified
        below. Each type may define further keys.

        **A note on widget properties**

        This is a dict setting the properties of a widget. Note that due to the restricted type
        conversion only properties of type ``str``, ``int``, ``float, ``bool`` are settable.

        **Supported row types**

        * ``label``

          Display text spanning both columns. Additional keys:

          - ``text``: The text to display
          - ``widget_properties``: `QLabel properties <https://doc.qt.io/qt-6/qlabel.html#properties>`_

        * ``checkbox``

          A form layout item to edit boolean properties. Additional keys:

          - ``label``: The text displayed in front of the the editor widget.
          - ``property``: The name of the property that will be set on changes.
          - ``widget_properties``: `QCheckBox properties <https://doc.qt.io/qt-6/qcheckbox.html#properties>`_

        * ``lineedit``

          A form layout item to edit string properties. Additional keys:

          - ``label``: The text displayed in front of the the editor widget.
          - ``property``: The name of the property that will be set on changes.
          - ``widget_properties``: `QLineEdit properties <https://doc.qt.io/qt-6/qlineedit.html#properties>`_

        * ``combobox``

          A form layout item to set string properties using a list of options. Additional keys:

          - ``label``: The text displayed in front of the the editor widget.
          - ``property``: The name of the property that will be set on changes.
          - ``items``: The list of strings used to populate the combobox.
          - ``widget_properties``: `QComboBox properties <https://doc.qt.io/qt-6/qcombobox.html#properties>`_

        * ``spinbox``

          A form layout item to edit integer properties. Additional keys:

          - ``label``: The text displayed in front of the the editor widget.
          - ``property``: The name of the property that will be set on changes.
          - ``widget_properties``: `QSpinBox properties <https://doc.qt.io/qt-6/qspinbox.html#properties>`_

        * ``doublespinbox``

          A form layout item to edit float properties. Additional keys:

          - ``label``: The text displayed in front of the the editor widget.
          - ``property``: The name of the property that will be set on changes.
          - ``widget_properties``: `QDoubleSpinBox properties <https://doc.qt.io/qt-6/qdoublespinbox.html#properties>`_
        """


class Action:
    """
    See also:
        `Action C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1Action.html>`_
    """

    def __init__(self,
                 id: str,
                 text: str,
                 callable: Callable):
        ...


class Item(ABC):
    """
    See also:
        `Item C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1Item.html>`_
    """

    @abstractmethod
    def id(self) -> str:
        """
        Returns the item identifier.
        """

    @abstractmethod
    def text(self) -> str:
        """
        Returns the item text.
        """

    @abstractmethod
    def subtext(self) -> str:
        """
        Returns the item subtext.
        """

    @abstractmethod
    def inputActionText(self) -> str:
        """
        Returns the item input action text.
        """

    @abstractmethod
    def iconUrls(self) -> List[str]:
        """
        Returns the item icon URLs.

        See also:
             `pixmapFromUrl() C++ Reference <https://albertlauncher.github.io/reference/namespacealbert.html#ab33e1e7fab94ddf6b1b7f4683577602c>`_
        """

    @abstractmethod
    def actions(self) -> List[Action]:
        """
        Returns the item actions.
        """


class StandardItem(Item):
    """
    A property based implementation of the ``Item`` interface.

    See also:
        `StandardItem C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1StandardItem.html>`_
    """

    def __init__(self,
                 id: str = '',
                 text: str = '',
                 subtext: str = '',
                 inputActionText: Optional[str] = '',
                 iconUrls: List[str] = [],
                 actions: List[Action] = []):
        ...

    id: str
    """
    The item identifier.
    """

    text: str
    """
    The item text.
    """

    subtext: str
    """
    The item subtext.
    """

    inputActionText: str
    """
    The item input action text.
    """

    iconUrls: List[str]
    """
    The item icon URLs.

    See also:
         `pixmapFromUrl() C++ Reference <https://albertlauncher.github.io/reference/namespacealbert.html#ab33e1e7fab94ddf6b1b7f4683577602c>`_
    """

    actions: List[Action]
    """
    The item actions.
    """


class Query:
    """
    See also:
        `Query C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1Query.html>`_
    """

    @property
    def trigger(self) -> str:
        """
        Returns the trigger of this query.
        """

    @property
    def string(self) -> str:
        """
        Returns the query string.
        """

    @property
    def isValid(self) -> bool:
        """
        Returns ``False`` if the query has been cancelled or invalidated, otherwise returns ``True``.
        """

    @overload
    def add(self, item: Item):
        """
        Adds ``item`` to the query results.

        Use list add if you can to avoid expensive locking and UI flicker.
        """

    @overload
    def add(self, item: List[Item]):
        """
        Adds ``items`` to the query results.
        """


class MatchConfig:
    """
    See also:
        `MatchConfig C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1MatchConfig.html>`_
    """

    def __init__(self,
                 fuzzy: bool = False,
                 ignore_case: bool = True,
                 ignore_word_order: bool = True,
                 ignore_diacritics: bool = True,
                 separator_regex: str = "[\s\\\/\-\[\](){}#!?<>\"'=+*.:,;_]+"):
        """
        Constructs a ``MatchConfig`` initialized with the values of ``fuzzy``, ``ignore_case``,
        ``ignore_diacritics``, ``ignore_word_order`` and ``separator_regex``. All parameters are
        optional.
        """

    fuzzy: bool
    """
    Match strings error tolerant.
    """

    ignore_case: bool
    """
    Match strings case insensitive.
    """

    ignore_word_order: bool
    """
    Match strings independent of their order.
    """

    ignore_diacritics: bool
    """
    Match strings normalized.
    """

    separator_regex: str
    """
    The separator regex used to tokenize the strings.
    """


class Matcher:
    """
    See also:
        `Matcher C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1Matcher.html>`_
    """

    def __init__(self,
                 string: str,
                 config: MatchConfig = MatchConfig()):
        """
        Constructs a ``Matcher`` for the given ``string`` and ``config``.
        """

    @overload
    def match(self, string: str) -> Match:
        """
        Returns a ``Match`` for the ``string``.
        """

    @overload
    def match(self, strings: List[str]) -> Match:
        """
        Returns the best ``Match`` for the ``strings``.
        """

    @overload
    def match(self, *args: str) -> Match:
        """
        Returns the best ``Match`` for the ``args``.
        """


class Match:
    """
    See also:
        `Match C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1Match.html>`_
    """

    def score(self) -> float:
        """
        The score of this match.
        """

    def isMatch(self) -> bool:
        """
        Returns ``True`` if this is a match, otherwise returns ``False``.
        """

    def isEmptyMatch(self) -> bool:
        """
        Returns ``True`` if this is a zero score match, otherwise returns ``False``.
        """

    def isExactMatch(self) -> bool:
        """
        Returns ``True`` if this is a perfect match, otherwise returns ``False``.
        """

    def __bool__(self) -> bool:
        """
        Converts the match to ``bool`` using ``isMatch()``.
        """

    def __float__(self) -> float:
        """
        Converts the match to ``float`` using ``score()``.
        """

class Extension(ABC):
    """
    See also:
        `Extension C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1Extension.html>`_
    """

    @abstractmethod
    def id(self) -> str:
        """
        Returns the extension identifier.
        """

    @abstractmethod
    def name(self) -> str:
        """
        Returns the pretty, human readable extension name.
        """

    @abstractmethod
    def description(self) -> str:
        """
        Returns the brief extension description.
        """


class TriggerQueryHandler(Extension):
    """
    See also:
        `TriggerQueryHandler C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1TriggerQueryHandler.html>`_
    """

    def synopsis(self, query: str) -> str:
        """
        Returns the input hint to be displayed on empty query.

        The base class implementation returns an empty string.
        """

    def allowTriggerRemap(self) -> bool:
        """
        Returns ``True`` if the user is allowed to set a custom trigger, otherwise returns ``False``.

        The base class implementation returns ``True``.
        """

    def defaultTrigger(self) -> str:
        """
        Returns the default trigger.

        The base class implementation returns an empty string.
        """

    def supportsFuzzyMatching(self) -> bool:
        """
        Returns ``True`` if the handler supports error tolerant matching, otherwise returns ``False``.

        The base class implementation returns ``False``.
        """

    def setTrigger(self, trigger: str):
        """
        Notifies about changes to the user defined ``trigger`` used to call the handler.

        The base class implementation does nothing.
        """

    def setFuzzyMatching(self, enabled: bool):
        """
        Sets the fuzzy matching mode to ``enabled``.

        The base class implementation does nothing.
        """

    @abstractmethod
    def handleTriggerQuery(self, query: Query):
        """
        Handles the triggered ``query``.
        """


class RankItem:
    """
    See also:
        `RankItem C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1RankItem.html>`_
    """

    def __init__(self,
                 item: Item,
                 score: float):
        ...

    item: Item
    score: float


class GlobalQueryHandler(TriggerQueryHandler):
    """
    See also:
        `GlobalQueryHandler C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1GlobalQueryHandler.html>`_
    """

    def handleTriggerQuery(self, query: Query) -> List[RankItem]:
        """
        Implements ``TriggerQueryHandler.handleTriggerQuery()``.

        Runs ``GlobalQueryHandler.handleGlobalQuery()``, applies usage scores, sorts and adds items to ``query``.
        """

    def applyUsageScore(self, rank_items: List[RankItem]):
        """
        Modifies the score of ``items`` according to the users usage in place.
        """

    @abstractmethod
    def handleGlobalQuery(self, query: Query) -> List[RankItem]:
        """
        Returns items that match ``query``.
        """


class IndexItem:
    """
    See also:
        `IndexItem C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1IndexItem.html>`_
    """

    def __init__(self,
                 item: Item,
                 string: str):
        ...

    item: Item
    string: str


class IndexQueryHandler(GlobalQueryHandler):
    """
    See also:
        `IndexQueryHandler C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1IndexQueryHandler.html>`_
    """

    def handleGlobalQuery(self, query: Query) -> List[RankItem]:
        """
        Implements ``GlobalQueryHandler.handleGlobalQuery()``.

        Returns items that match ``query`` using the index.
        """

    def setIndexItems(self, indexItems: List[IndexItem]):
        """
        Sets the items of the index.

        Meant to be called in ``updateIndexItems()``.
        """

    @abstractmethod
    def updateIndexItems(self):
        """
        Updates the index.

        Called when the index needs to be updated, i.e. for initialization, on user changes to the
        index config (fuzzy, etc…) and probably by the client itself if the items changed. This
        function should call setIndexItems(std::vector<IndexItem>&&) to update the index.

        Do not call this method in the constructor. It will be called on plugin initialization.
        """


class FallbackHandler(Extension):
    """
    See also:
        `FallbackHandler C++ Reference <https://albertlauncher.github.io/reference/classalbert_1_1FallbackHandler.html>`_
    """

    @abstractmethod
    def fallbacks(self, query: str) -> List[Item]:
        """
        Returns fallback items for ``query``.
        """


class Notification:
    """
    See also:
        `Notification C++ Reference <See https://albertlauncher.github.io/reference/classalbert_1_1Notification.html>`_
    """

    def __init__(self,
                 title: str = '',
                 text: str = ''):
        ...

    title: str

    text: str

    def send(self):
        ...

    def dismiss(self):
        ...



def debug(arg: Any):
    """
    Logs ``str(arg)`` as debug message in the logging category of this plugin.

    Note:
        This function is not part of the albert module and here for reference only.
        The attribute is attached to the module at load time.
    """


def info(arg: Any):
    """
    Logs ``str(arg)`` as info message in the logging category of this plugin.

    Note:
        This function is not part of the albert module and here for reference only.
        The attribute is attached to the module at load time.
    """

def warning(arg: Any):
    """
    Logs ``str(arg)`` as warning message in the logging category of this plugin.

    Note:
        This function is not part of the albert module and here for reference only.
        The attribute is attached to the module at load time.
    """


def critical(arg: Any):
    """
    Logs ``str(arg)`` as critical message in the logging category of this plugin.

    Note:
        This function is not part of the albert module and here for reference only.
        The attribute is attached to the module at load time.
    """


def setClipboardText(text: str):
    """
    Sets the system clipboard to ``text``.
    """


def setClipboardTextAndPaste(text: str):
    """
    Sets the system clipboard to ``text`` and paste the content to the front-most window.

    Note:
        Requires paste support. Check ``havePasteSupport()`` before using this function.
    """


def havePasteSupport() -> bool:
    """
    Returns ``True`` if the platform supports pasting, otherwise returns ``False``.

    Note:
        This is a requirement for ``setClipboardTextAndPaste(…)`` to work.
    """


def openUrl(url: str):
    """
    Opens the URL ``url`` with the default URL handler.
    """


def openFile(path: str):
    """
    Opens the file at ``path`` with the default application.
    """


def runDetachedProcess(cmdln: List[str], workdir: str = '') -> int:
    """
    Starts the ``commandline`` in a new process, and detaches from it. Returns the PID on success;
    otherwise returns 0. The process will be started in the directory ``working_dir``. If
    ``working_dir`` is empty, the working directory is the users home directory.
    """


def runTerminal(script: str):
    """
    Runs a ``script`` in the users shell and terminal.
    """
