The main user interface can be customized using QStyleSheets.
You might want to consider the convenient [theme creator](https://albertlauncher.github.io/themecreator/) written by Ricardo Fuhrmann. 
If you're not satisfied with the output, you can still customize the style sheet.

##### Writing style sheets

The user interface consists of five widgets: The container, the input line, the settings button the two list views for the items and their actions.
In Qt terms a [QFrame](http://doc.qt.io/qt-6/qframe.html), a [QLineEdit](http://doc.qt.io/qt-6/qlineedit.html), a [QPushButton](http://doc.qt.io/qt-6/qpushbutton.html) and a [QListView](http://doc.qt.io/qt-6/qlistview.html). 
All of them support the box model.

To differentiate the views they have an ID Selector to refer to it, for the sake of completeness all other widgets too: `frame`, `inputLine`, `settingsButton`, `resultsList` and `actionList`. You can identify them like this:

```css
QListView#actionList {
	font-size: 20px;
}
```

There is an excellent documentation on QStyleSheets:

- [Overview](http://doc.qt.io/qt-6/stylesheet.html)
  - [The Style Sheet Syntax](http://doc.qt.io/qt-6/stylesheet-syntax.html)
  - [Customizing Qt Widgets Using Style Sheets](http://doc.qt.io/qt-6/stylesheet-customizing.html)
  - [Qt Style Sheets Reference](http://doc.qt.io/qt-6/stylesheet-reference.html ), especially the sections
    - [Customizing QFrame](http://doc.qt.io/qt-6/stylesheet-examples.html#customizing-qframe)
    - [Customizing QLineEdit](http://doc.qt.io/qt-6/stylesheet-examples.html#customizing-qlineedit)
    - [Customizing QListView](http://doc.qt.io/qt-6/stylesheet-examples.html#customizing-qlistview)
  - [Qt Style Sheets Examples](http://doc.qt.io/qt-6/stylesheet-examples.html)

Further check the [existing themes](https://github.com/albertlauncher/plugins/tree/master/widgetsboxmodel/themes).

If you're done place the file in "$datapath/widgetsboxmodel/themes/" and restart albert.
The restart is necessary because file list is read once at start.
If the file is listed you can apply changes by switching to other themes and and back again.

##### Limitations

The item is printed using a custom QStyleItemDelegate and the regular items do not intend to display a second text.
So there is no (straightforward) possibilty to design the subtexts font size.
