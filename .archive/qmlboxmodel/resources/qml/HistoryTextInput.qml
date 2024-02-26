import QtQuick

TextInput {
    id: root
    property string userText
    property color cursorColor
    clip: true
    selectByMouse: true
    cursorDelegate: Rectangle {
        id: cursor
        color: cursorColor
        width: 1
        height: parent.height

        SequentialAnimation on opacity {
            id: animation
            loops: Animation.Infinite;
            NumberAnimation {
                to: 0
                duration: 750
                easing.type: Easing.InOutExpo
            }
            NumberAnimation {
                to: 1
                duration: 750
                easing.type: Easing.InOutExpo
            }
        }

        function restartAnimation(){
            opacity=1
            animation.restart()
        }

        Connections{
            target: root
            function onTextChanged() { restartAnimation() }
            function onCursorPositionChanged() { restartAnimation() }
        }
    }

    onTextEdited: {
        userText = text
        history.resetIterator()
    }

    Window.onVisibilityChanged: {
        if (Window.visibility === Window.Hidden){
            history.add(text)
            history.resetIterator()
            userText = ''  // for
            selectAll()
        }
    }

    function historyNext() {
        let match = history.next(userText)
        if (match) text = match
    }

    function historyPrev() {
        let match = history.prev(userText)
        if (match) text = match
    }

    // Crucial. the focus should never change therefore FocusScopes
    // are overkill. Set the focus once and forever
    Component.onCompleted: forceActiveFocus()
}
