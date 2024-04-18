import QtQuick

TextInput {
    id: root

    property color cursorColor
    property color triggerColor
    property int triggerLength: 0
    property string _userText

    // fontMetrics.ascent * 0.9: hack. TODO use capHeight as soon as available
    FontMetrics { id: fontMetrics; font: root.font }
    leftPadding: padding + (height - fontMetrics.ascent * 0.9) / 2

    wrapMode: TextEdit.NoWrap
    // clip: true
    selectByMouse: true

    cursorDelegate: Rectangle {
        id: cursor
        color: cursorColor
        width: 1

        SequentialAnimation on opacity
        {
            id: animation
            loops: Animation.Infinite;
            NumberAnimation { to: 0; duration: 1000; easing.type: Easing.InOutExpo }
            NumberAnimation { to: 1; duration: 1000; easing.type: Easing.InOutExpo }
        }

        function restartAnimation(){ opacity=1; animation.restart() }

        Connections
        {
            target: root
            function onTextChanged() { restartAnimation() }
            function onCursorPositionChanged() { restartAnimation() }
        }
    }

    // onTriggerLengthChanged: root.setTriggerMarkup()

    // onTextChanged: {
    //     if (_blockTextEdited)
    //         _blockTextEdited = false
    //     else
    //         textEdited(getText(0, length))
    // }

    onTextEdited: {
        _userText = text
        root.setTriggerMarkup()
        history.resetIterator()
    }

    Window.onVisibilityChanged: {
        if (Window.visibility === Window.Hidden){
            history.add(text)
            history.resetIterator()
            _userText = ''
            selectAll()
            gc()
        }
    }

    Keys.onPressed: (event) => {
        albert.debug(`Key ${event.modifier} + ${event.key}  pressed`)
        if (event.matches(StandardKey.Undo))
        {
            albert.debug("undo")
            undo()
        }
    }

    function setTriggerMarkup() {
        // albert.critical(`setTriggerMarkup ${triggerColor}`)
        // let cp = cursorPosition
        // let t = getText(0, length)
        // _blockTextEdited = true
        // root.text = `<font color="${triggerColor}">${t.substring(0, triggerLength)}</font>${t.substring(triggerLength, length)}`
        // root.cursorPosition = cp
    }

    function historyNext() {
        let match = history.next(_userText)
        if (match)
            text = match
    }

    function historyPrev() {
        let match = history.prev(_userText)
        if (match)
            text = match
    }

    // Crucial. the focus should never change therefore FocusScopes
    // are overkill. Set the focus once and forever
    Component.onCompleted: forceActiveFocus()
}


