import QtQuick 2.5
import QtGraphicalEffects 1.0

Item {

    id: listItem

    property variant attachedModel: model
    property int iconSize
    property int spacing
    property int textSize
    property int descriptionSize
    property color textColor
    property color highlightColor
    property string fontName
    property int animationDuration: 150

    width: parent.width
    height: Math.max(listItemIconArea.height, listItemTextArea.height)

    MouseArea {
        anchors.fill: parent
        onClicked: resultsList.currentIndex = index
        onDoubleClicked:  (mouse.modifiers===Qt.NoModifier) ? root.activate(resultsList.currentIndex) : root.activate(resultsList.currentIndex, -mouse.modifiers)
    }

    Item {
        id: listItemIconArea
        width: iconSize
        height: iconSize + spacing

        Image {
            id: listItemIcon
            source: {
                var path = itemDecorationRole
                return ( path[0] === ":" ) ? "qrc"+path : path
            }
            visible: false
            asynchronous: true
            width: listItem.iconSize
            height: listItem.iconSize
            sourceSize.width: listItem.iconSize*2
            sourceSize.height: listItem.iconSize*2
            cache: true
            fillMode: Image.PreserveAspectFit
        }
        InnerShadow  {
            id: sunkenListItemIcon
            source: listItemIcon
            anchors.fill: listItemIcon
            horizontalOffset: listItem.ListView.isCurrentItem ? 0 : 2
            verticalOffset: listItem.ListView.isCurrentItem ? 0 : 2
            radius: listItem.ListView.isCurrentItem ? 0 : 6
            samples: 8
            color: "#f0000000"
            Behavior on verticalOffset { NumberAnimation{ duration: animationDuration } }
            Behavior on horizontalOffset { NumberAnimation{ duration: animationDuration } }
            Behavior on radius { NumberAnimation{ duration: animationDuration } }
        }
    }


    Column {
        id: listItemTextArea
        anchors {
            left: listItemIconArea.right
            leftMargin: listItem.spacing
            right: parent.right
            verticalCenter: parent.verticalCenter
        }
        Text {
            id: textId
            width: parent.width
            text: itemTextRole
            elide: Text.ElideRight
            color: listItem.ListView.isCurrentItem ? listItem.highlightColor : listItem.textColor
            font.family: listItem.fontName
            font.pixelSize: listItem.textSize
            Behavior on color { ColorAnimation{ duration: animationDuration } }
        }
        Text {
            id: subTextId
            width: parent.width
            text: (listItem.ListView.isCurrentItem && root.state==="fallback") ? itemFallbackRole : itemToolTipRole
            elide: Text.ElideRight
            color: listItem.ListView.isCurrentItem ? listItem.highlightColor : listItem.textColor
            font.family: listItem.fontName
            font.pixelSize: listItem.descriptionSize
            Behavior on color { ColorAnimation{ duration: animationDuration } }
            Behavior on text {
                SequentialAnimation {
                    NumberAnimation { target: subTextId; property: "opacity"; from:1; to: 0; duration: animationDuration/2 }
                    PropertyAction  { }
                    NumberAnimation { target: subTextId; property: "opacity"; from:0; to: 1; duration: animationDuration/2 }
                }
            }
        }
    }  // listItemTextArea (Column)

    Text {
        id: shortcutId
        anchors.right: listItemTextArea.right
        anchors.verticalCenter: listItemTextArea.verticalCenter
        text: {
            var num = 1 + index - Math.max(0, listItem.ListView.view.indexAt(1, listItem.ListView.view.contentY+1))
            return (num < 10) ? "Ctrl+" + num : ''
        }
        font.family: listItem.fontName
        font.pixelSize: listItem.textSize
        color: root.ctrl ? listItem.textColor : Qt.rgba(listItem.textColor.r, listItem.textColor.g, listItem.textColor.b, 0)
        Behavior on color { ColorAnimation{ duration: animationDuration } }
    }

    Glow {
        anchors.fill: shortcutId
        source: shortcutId
        color: root.ctrl ? frame.color : Qt.rgba(frame.color.r, frame.color.g, frame.color.b, 0)
        Behavior on color { ColorAnimation{ duration: animationDuration } }
        radius: 8
        samples: 17
        spread: 0.7
    }


    /*
     * The function to activate an item
     * Currently work as follows:
     * action is undefined -> default action
     * action 0<= are the alternative actions
     * action 0> activation while modifier is pressed (-action is the number of the modifier)
     *   currently only Meta is supported
     */
    function activate(/*optional*/ action){
        if (typeof action === 'undefined')
            itemActionRole = 0
        else
            if (action < 0 && -action==Qt.MetaModifier)
                itemFallbackRole = 0
            else
                itemAltActionsRole = action
    }

    function actionsList() {
        return itemAltActionsRole
    }

}  // listItem (MouseArea)
