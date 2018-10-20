import QtQuick 2.9 // Min Qt version 5.9 since ubuntu 18.04 LTS was released, needed for textEdited signal

TextInput {

    property string userText

    function pushTextToHistory() {
        history.add(text)
        text = ""
        resetHistoryMode()
    }

    function resetHistoryMode() {
        history.resetIterator()
    }

    function forwardSearchHistory() {
        var match = history.next(userText)
        if (match)
            text = match
    }

    function backwardSearchHistory() {
        var match = history.prev(userText)
        if (match)
            text = match
    }

    onTextEdited: {
        console.log("Text edited: " + text)
        userText = text
        history.resetIterator()
    }
}
