import QtQuick 2.9 // Min Qt version 5.9 since ubuntu 18.04 LTS was released, needed for textEdited signal

TextInput {

    property string userText

    function pushTextToHistory() {
        if (text.trim() !== "")
            history.add(text)
    }

    function resetHistoryMode() {
        history.resetIterator()
    }

    function forwardSearchHistory() {
        var match = history.next(userText)
        if (match === text)  // Simple hack to avoid the seemingly-noop-on-first-history-iteration on disabled clear-on-hide
            match = history.next(userText)
        if (match)
            text = match
    }

    function backwardSearchHistory() {
        var match = history.prev(userText)
        if (match === text)  // Simple hack to avoid the seemingly-noop-on-first-history-iteration on disabled clear-on-hide
            match = history.prev(userText)
        if (match)
            text = match
    }

    onTextEdited: {
        userText = text
        history.resetIterator()
    }
}
