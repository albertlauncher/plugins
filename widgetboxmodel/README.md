The main user interface can be customized using [QStyleSheets](http://doc.qt.io/qt-5/stylesheet.html). Qt Style Sheets are a powerful mechanism that allows you to customize the appearance of widgets. The concepts, terminology, and syntax of Qt Style Sheets are heavily inspired by HTML Cascading Style Sheets (CSS) but adapted to the world of widgets.

Before handcrafting your own style sheet you might want to consider the convenient [theme creator](//extensions/widgetboxmodel/themecreator/) written by Ricardo Fuhrmann. If you're not satisfied with the output, you can still customize the style sheet.

##### Writing style sheets

The user interface consists of five widgets: The window itself, the input line, the settings button the two list views for the items and their actions. In Qt terms a [QFrame](http://doc.qt.io/qt-5/qframe.html), a [QLineEdit](http://doc.qt.io/qt-5/qlineedit.html), a [QPushButton](http://doc.qt.io/qt-5/qpushbutton.html) and a [QListView](http://doc.qt.io/qt-5/qlistview.html). All of them support the [box model](http://doc.qt.io/qt-5/stylesheet-customizing.html).

To differentiate the views they have an ID Selector to refer to it, for the sake of completeness all other widgets too: `frame`, `inputLine`, `settingsButton`, `resultsList` (until v0.12 `proposalList`) and `actionList`. You can identify them like this :

```
QListView#actionList {
	font-size: 20px;
}
```

There is an excellent documentation on QStyleSheets:

- [Overview](http://doc.qt.io/qt-5/stylesheet.html)
  - [The Style Sheet Syntax](http://doc.qt.io/qt-5/stylesheet-syntax.html)
  - [Customizing Qt Widgets Using Style Sheets](http://doc.qt.io/qt-5/stylesheet-customizing.html)
  - [Qt Style Sheets Reference](http://doc.qt.io/qt-5/stylesheet-reference.html ), especially the sections
    - [Customizing QFrame](http://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qframe)
    - [Customizing QLineEdit](http://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qlineedit)
    - [Customizing QListView](http://doc.qt.io/qt-5/stylesheet-examples.html#customizing-qlistview)
  - [Qt Style Sheets Examples](http://doc.qt.io/qt-5/stylesheet-examples.html)

Further check the [existing themes](https://github.com/albertlauncher/plugins/tree/master/widgetboxmodel/share/themes).

If you're done place the file in "~/.local/share/albert/org.albert.frontend.widgetboxmodel/themes/" and restart albert. The restart is necessary because file list is read once at start. If the file is listed you can apply changes by switching to other themes and and back again.

##### Limitations

The item is printed using a custom QStyleItemDelegate and the regular items do not intend to display a second text. So there is no (proper) possibilty to design the subtexts font size.
