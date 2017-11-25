import QtQuick 2.0

ListView {

    property int itemCount: 5

    width: parent.width
    height: (count === 0) ? 0 : Math.min(itemCount, count)*(itemAt(0,0).height+spacing)-spacing
    snapMode: ListView.SnapToItem
    clip: true
    keyNavigationWraps: false
    boundsBehavior: Flickable.StopAtBounds
    highlightMoveDuration : 250
    highlightMoveVelocity : 100

    Connections {
        target: model
        onModelReset: currentIndex = 0
    }

    Keys.onPressed: {
        if ( count === 0 )
            return

        event.accepted = true

        if ( event.key === Qt.Key_Up && event.modifiers === Qt.NoModifier
             || event.key === Qt.Key_P && event.modifiers === Qt.ControlModifier )
            decrementCurrentIndex()
        else if ( event.key === Qt.Key_Down && event.modifiers === Qt.NoModifier
                  || event.key === Qt.Key_N && event.modifiers === Qt.ControlModifier )
            incrementCurrentIndex()
        else if ( event.key === Qt.Key_PageUp && event.modifiers === Qt.NoModifier)
            currentIndex = Math.max(currentIndex - itemCount, 0)
        else if ( event.key === Qt.Key_PageDown && event.modifiers === Qt.NoModifier)
            currentIndex = Math.min(currentIndex + itemCount, count-1)
        else if ( event.key === Qt.Key_Home && event.modifiers === Qt.ControlModifier)
            currentIndex = 0
        else if ( event.key === Qt.Key_End && event.modifiers === Qt.ControlModifier)
            currentIndex = count-1
        else
            event.accepted = false
    }
}
