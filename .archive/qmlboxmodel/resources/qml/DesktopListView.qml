import QtQuick
import "albert.js" as Util

///
/// A ListView providing sane defaults and standard desktop navigation
///
ListView {
    id: resizingListView
    objectName: "resizingListView"
    property int maxItems: 5

    signal itemActivated(int index)

    boundsBehavior: Flickable.StopAtBounds
    clip: true
    highlightMoveDuration : 0
    highlightMoveVelocity : -1
    snapMode: ListView.SnapToItem
    onCountChanged: {
        // count is not guaranteed to equal the contentItem.children.length
        // Especially invisible do not necessarily have to exist
        // never let the list have height 0
        visible=(count !== 0)
        if (count !== 0)
        {
            height = Math.min(maxItems, count) * (contentItem.children[0].height + spacing) - spacing
            if (currentIndex < 0)
                currentIndex = 0
        }
    }

    Keys.onPressed: (event)=>{
        //Util.printKeyPress("DefaultListView", event)
        if (count === 0)
            return

        if (Util.testKey(event, Qt.Key_PageUp)){
            pgUp()

        } else if (Util.testKey(event, Qt.Key_PageDown)){
            pgDn()

        } else if (Util.testKeyCombination(event, Qt.ControlModifier, Qt.Key_Home)){
            currentIndex = 0

        } else if (Util.testKeyCombination(event, Qt.ControlModifier, Qt.Key_End)){
            currentIndex = count-1

        } else if (Util.testKey(event, Qt.Key_Return) ||
                   Util.testKey(event, Qt.Key_Enter) ||
                   Util.testKeyCombination(event, Qt.ControlModifier, Qt.Key_O)){
            itemActivated(currentIndex)
        }
    }

    function pgUp(){
        let topIndex = indexAt(0, contentY)
        if (currentIndex === topIndex){
            let bottomIndex = indexAt(0, contentY + height - 1)
            let pageLen = bottomIndex - topIndex + 1
            currentIndex = Math.max(currentIndex - pageLen, 0)
        } else
            currentIndex = topIndex
    }

    function pgDn(){
        let bottomIndex = indexAt(0, contentY + height - 1)
        if (currentIndex === bottomIndex){
            let topIndex = indexAt(0, contentY)
            let pageLen = bottomIndex - topIndex + 1
            currentIndex = Math.min(currentIndex + pageLen, count-1)
        } else
            currentIndex = bottomIndex
    }
}

