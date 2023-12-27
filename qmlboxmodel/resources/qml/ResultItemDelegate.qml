import QtQuick
import QtQuick.Window 2.2  // Screen.devicePixelRatio

Item {
    property int icon_size
    property int animation_duration
    property color text_color
    property color text_color_current
    property int text_font_size
    property color subtext_color
    property color subtext_color_current
    property int subtext_font_size
    property alias spacing_vertical: listItemTextColumn.spacing
    property int spacing_horizontal
    property int padding

    width: ListView.view.width
    height: content.height + padding*2

    Item { DebugRect{ name: "content" }
        id: content
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.leftMargin: padding
        anchors.rightMargin: padding
        anchors.verticalCenter: parent.verticalCenter
        height: Math.max(listItemIcon.height, listItemTextColumn.height)


        Image { DebugRect{ name: "icon" }
            id: listItemIcon
            width: icon_size
            height: icon_size
            anchors.verticalCenter: parent.verticalCenter

            //asynchronous: dont! introduces ugly flicker
            cache: true
            fillMode: Image.PreserveAspectFit
            smooth: true
            source: "image://albert/" + itemIconUrls.join(',')
            sourceSize.width: icon_size * Screen.devicePixelRatio
            sourceSize.height: icon_size * Screen.devicePixelRatio
        }
        DebugRect{ target: listItemTextColumn; name: "listItemTextColumn" }
        Column {
            id: listItemTextColumn
            anchors.left: listItemIcon.right
            anchors.right: parent.right
            anchors.leftMargin: spacing_horizontal
            anchors.verticalCenter: listItemIcon.verticalCenter

            Text { DebugRect{ name: "listItemText" }
                width: listItemTextColumn.width
                text: itemText
                font.pixelSize: text_font_size
                elide: Text.ElideRight
                color: ListView.isCurrentItem ? text_color_current : text_color
                Behavior on color { ColorAnimation{ duration: animation_duration } }
            }

            Text { DebugRect{ name: "listItemSubText" }
                width: listItemTextColumn.width
                text: itemSubText
                font.pixelSize: subtext_font_size
                elide: Text.ElideRight
                color: ListView.isCurrentItem ? subtext_color_current : subtext_color
                Behavior on color { ColorAnimation{ duration: animation_duration } }
            }
        }
    }

//    component TextComponent: Text {
//        id: textComponent
//        width: listItemTextColumn.width
////        height: metrics.tightBoundingRect.height

////        TextMetrics { // Fix annoying top padding of some fonts
////            id: metrics
////            text: textComponent.text
////            font: textComponent.font
////        }
//        elide: Text.ElideRight
//        Behavior on color { ColorAnimation{ duration: animation_duration } }
//    }

    signal clicked(int index)
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: parent.clicked(index)
    }

    function getInputAction(){ return itemInputAction }
}
