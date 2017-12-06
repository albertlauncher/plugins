﻿import QtQuick 2.5
import QtQuick.Controls 1.0
import QtGraphicalEffects 1.0
import QtQuick.Window 2.0
import "themes.js" as Themes

Item {

    id: root
    width: frame.width+2*preferences.shadow_size
    height: Screen.height * 0.7

    layer.enabled: true
    layer.effect: DropShadow {
        transparentBorder: true
        verticalOffset: preferences.shadow_size/3
        radius: preferences.shadow_size
        samples: preferences.shadow_size*2
        color: preferences.shadow_color
    }

    Preferences {
        id: preferences
        objectName: "preferences"
    }

    FontLoader {
        source: "fonts/Roboto-Thin.ttf"
        onStatusChanged: if (loader.status === FontLoader.Ready) preferences.font_name = fontname
    }

    Rectangle {

        id: frame
        objectName: "frame" // for C++
        x: preferences.shadow_size;
        y: preferences.shadow_size
        width: preferences.window_width
        height: content.height+2*content.anchors.margins
        radius: preferences.radius
        color: preferences.background_color
        Behavior on color { ColorAnimation { duration: preferences.animation_duration; easing.type: Easing.OutCubic } }
        Behavior on border.color { ColorAnimation { duration: preferences.animation_duration; easing.type: Easing.OutCubic } }
        border.color: preferences.border_color
        border.width: preferences.border_size

        Column {

            id: content
            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: preferences.border_size+preferences.padding
            }
            spacing: preferences.spacing

            HistoryTextInput {
                id: historyTextInput
                anchors {
                    left: parent.left;
                    right: parent.right;
                }
                clip: true
                color: preferences.input_color
                focus: true
                font.pixelSize: preferences.input_fontsize
                font.family: preferences.font_name
                selectByMouse: true
                selectedTextColor: preferences.background_color
                selectionColor: preferences.selection_color
                Keys.forwardTo: [root, resultsList]
                cursorDelegate : Item {
                    Rectangle {
                        width: 1
                        height: parent.height
                        color: preferences.cursor_color
                    }
                    SequentialAnimation on opacity {
                        id: animation
                        loops: Animation.Infinite;
                        NumberAnimation { to: 0; duration: 750; easing.type: Easing.InOutExpo }
                        NumberAnimation { to: 1; duration: 750; easing.type: Easing.InOutExpo }
                    }
                    Connections {
                        target: historyTextInput
                        onTextChanged: { opacity=1; animation.restart() }
                        onCursorPositionChanged: { opacity=1; animation.restart() }
                    }
                }
                onTextChanged: { root.state="" }
            } // historyTextInput

            DesktopListView {
                id: resultsList
                width: parent.width
                model: resultsModel
                itemCount: preferences.max_items
                spacing: preferences.spacing
                delegate: Component {
                    ItemViewDelegate{
                        iconSize: preferences.icon_size
                        spacing: preferences.spacing
                        textSize: preferences.item_title_fontsize
                        descriptionSize: preferences.item_description_fontsize
                        textColor: preferences.foreground_color
                        highlightColor: preferences.highlight_color
                        fontName: preferences.font_name
                        animationDuration: preferences.animation_duration
                    }
                }
                Keys.onEnterPressed: (event.modifiers===Qt.NoModifier) ? activate() : activate(-event.modifiers)
                Keys.onReturnPressed: (event.modifiers===Qt.NoModifier) ? activate() : activate(-event.modifiers)
                onCurrentIndexChanged: if (root.state==="detailsView") root.state=""
            }  // resultsList (ListView)

            DesktopListView {
                id: actionsListView
                width: parent.width
                model: ListModel { id: actionsModel }
                itemCount: actionsModel.count
                spacing: preferences.spacing
                Behavior on visible {
                    SequentialAnimation {
                        PropertyAction  { }
                        NumberAnimation { target: actionsListView; property: "opacity"; from:0; to: 1; duration: preferences.animation_duration }
                    }
                }
                delegate: Text {
                    horizontalAlignment: Text.AlignHCenter
                    width: parent.width
                    text: name
                    textFormat: Text.PlainText
                    font.family: preferences.font_name
                    elide: Text.ElideRight
                    font.pixelSize: (preferences.item_description_fontsize+preferences.item_title_fontsize)/2
                    color: ListView.isCurrentItem ? preferences.highlight_color : preferences.foreground_color
                    Behavior on color { ColorAnimation{ duration: preferences.animation_duration } }
                    MouseArea {
                        anchors.fill: parent
                        onClicked: actionsListView.currentIndex = index
                        onDoubleClicked: activate(index)
                    }
                }
                visible: false
                Keys.onEnterPressed: activate(currentIndex)
                Keys.onReturnPressed: activate(currentIndex)
            }  // actionsListView (ListView)
        }  // content (Column)


        SettingsButton {
            id: settingsButton
            size: preferences.settingsbutton_size
            color: preferences.settingsbutton_color
            hoverColor: preferences.settingsbutton_hover_color
            onLeftClicked: settingsWidgetRequested()
            onRightClicked: menu.popup()
            anchors {
                top: parent.top
                right: parent.right
                topMargin: preferences.padding+preferences.border_size
                rightMargin: preferences.padding+preferences.border_size
            }

            Menu {
                id: menu
                MenuItem {
                    text: "Preferences"
                    shortcut: "Alt+,"
                    onTriggered: settingsWidgetRequested()
                }
                MenuItem {
                    text: "Quit"
                    shortcut: "Alt+F4"
                    onTriggered: Qt.quit()

                }
            }
        }
    }  // frame (Rectangle)

    onActiveFocusChanged: state=""

    // Key handling
    Keys.onPressed: {
        event.accepted = true
        if ( (event.key === Qt.Key_Up || event.key === Qt.Key_P && event.modifiers === Qt.ControlModifier )
                && state === "" && resultsList.currentIndex === 0 ) {
            historyTextInput.nextIteration()
        }
        else if ( event.key === Qt.Key_Up && event.modifiers === Qt.ControlModifier ) {
            state == ""
            historyTextInput.nextIteration()
        }
        else if ( event.key === Qt.Key_Down && event.modifiers === Qt.ControlModifier ) {
            state == ""
            historyTextInput.prevIteration()
        }
        else if ( event.key === Qt.Key_Meta ) {
            if (resultsList.currentIndex === -1)
                resultsList.currentIndex = 0
            state="fallback"
        }
        else if ( event.key === Qt.Key_Comma && event.modifiers === Qt.AltModifier ) {
            settingsWidgetRequested()
        }
        else if ( event.key === Qt.Key_Alt && resultsList.count > 0 ) {
            if (resultsList.currentIndex === -1)
                resultsList.currentIndex = 0
            state = "detailsView"
        }
        else if ( event.key === Qt.Key_Tab && resultsList.count > 0 ) {
            if ( resultsList.currentIndex === -1 )
                resultsList.currentIndex = 0
            historyTextInput.text = resultsList.currentItem.attachedModel.itemCompletionStringRole
        } else event.accepted = false
    }
    Keys.onReleased: {
        event.accepted = true
        if ( event.key === Qt.Key_Meta )
            state=""
        else if ( event.key === Qt.Key_Alt )
            state=""
        else event.accepted = false

    }

    states : [
        State {
            name: ""
        },
        State {
            name: "fallback"
        },
        State {
            name: "detailsView"
            PropertyChanges { target: actionsListView; visible: true  }
            PropertyChanges { target: historyTextInput; Keys.forwardTo: [root, actionsListView] }
            StateChangeScript {
                name: "actionLoaderScript"
                script: {
                    actionsModel.clear()
                    var actionTexts = resultsList.currentItem.actionsList();
                    for ( var i = 0; i < actionTexts.length; i++ )
                        actionsModel.append({"name": actionTexts[i]});
                    actionsListView.currentIndex = 0
                }
            }
        }
    ]

    Connections {
        target: mainWindow
        onVisibilityChanged: {
            state=""
            historyTextInput.selectAll()
            historyTextInput.clearIterator()
        }
    }

    Component.onCompleted: setTheme("Bright")


    // ▼ ▼ ▼ ▼ ▼ DO NOT CHANGE THIS UNLESS YOU KNOW WHAT YOU ARE DOING ▼ ▼ ▼ ▼ ▼

    /*
     * Currently the interface with the program logic comprises the following:
     *
     * These context properties are set:
     *   - mainWindow
     *   - resultsModel
     *   - history
     *
     * These properties must exist in root:
     *   - inputText (string, including the implicitly genreated signal)
     *
     * These functions must extist in root:
     *   - availableThemes()
     *   - setTheme(str)
     *
     * These signals must exist in root:
     *   - inputChanged(str)
     *   - settingsWidgetRequested()
     *
     * These object names with must exist somewhere:
     *   - frame (the visual root frame, i.e. withouth shadow)
     *   - preferences (QtObject containing only preference propterties)
     */
    property string interfaceVersion: "1.0-alpha" // Will not change until beta

    property alias inputText: historyTextInput.text
    signal settingsWidgetRequested()

    function activate(/*optional*/ action) {
        if ( resultsList.count > 0 ) {
            if ( resultsList.currentIndex === -1 )
                resultsList.currentIndex = 0
            resultsList.currentItem.activate(action)
            historyTextInput.pushTextToHistory()
            mainWindow.hide()
        }
    }

    function availableThemes() { return Object.keys(Themes.themes()) }
    function setTheme(themeName) {
        var themeObject = Themes.themes()[themeName]
        for (var property in themeObject)
            if (themeObject.hasOwnProperty(property))
                preferences[property] = themeObject[property]
    }
}
