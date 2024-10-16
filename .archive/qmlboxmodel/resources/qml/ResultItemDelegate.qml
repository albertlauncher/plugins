import QtQuick
import QtQuick.Window 2.2  // Screen.devicePixelRatio

MouseArea
{
    id: root
    DebugRect{ name: "delegate" }

    property int iconSize
    property alias text_color: listItemText.color
    property alias text_font_size: listItemText.font.pixelSize
    property alias subtext_color: listItemSubText.color
    property alias subtext_font_size: listItemSubText.font.pixelSize
    property alias spacing_vertical: listItemTextColumn.spacing
    property alias spacing_horizontal: listItemTextColumn.anchors.leftMargin
    property int padding

    signal clicked(int index)
    onClicked: clicked(index)

    function getInputAction(){ return itemInputAction }
    function getActionsList(){ return itemActionsList }

    width: ListView.view.width
    height: padding * 2 + Math.max(iconSize, listItemTextColumn.height) //  childrenRect.height

    // onHeightChanged: albert.critical(`DELEGATE onHeightChanged ${height}  ${itemText}`)
    // onImplicitHeightChanged: albert.critical(`DELEGATE onImplicitHeightChanged ${implicitHeight} ${itemText}`)
    // Component.onCompleted: albert.critical(`DELEGATE onCompleted ${height} ${itemText}`)


    Item {
        id: listItemIconArea
        DebugRect{ name: "iconArea" }

        width: iconSize
        height: iconSize

        anchors.left: parent.left
        anchors.leftMargin: padding
        anchors.verticalCenter: parent.verticalCenter

        Image
        {
            id: listItemIcon
            DebugRect{ name: "icon" }

            anchors.centerIn: parent

            // width: …; height: …  // Do not scale!

            source: "image://albert/" + itemIconUrls.join(',')
            sourceSize.width: iconSize
            sourceSize.height: iconSize
            asynchronous: true
            cache: true
            // This is intended to not obscure transformations behind the scenes
            // Icon providers should return perfect fit icons
            fillMode: Image.Pad
            // Don't. Icons should be antialiased by icon provider
            smooth: false
        }
    }

    DebugRect{ target: listItemTextColumn; name: "listItemTextColumn" }
    Column
    {
        id: listItemTextColumn

        anchors.left: listItemIconArea.right
        anchors.right: parent.right
        anchors.rightMargin: padding
        anchors.verticalCenter: parent.verticalCenter

        // onHeightChanged: albert.warning(`COLUMN onHeightChanged ${height}  ${itemText}`)
        // onImplicitHeightChanged: albert.warning(`COLUMN onImplicitHeightChanged ${implicitHeight} ${itemText}`)
        // Component.onCompleted: albert.warning(`COLUMN onCompleted ${height} ${itemText}`)

        Text
        {
            id: listItemText
            DebugRect{ name: "listItemText" }
            width: listItemTextColumn.width
            text: itemText
            elide: Text.ElideRight
            textFormat: Text.PlainText
            // Behavior on color { ColorAnimation{} }

            // onHeightChanged: albert.warning(`TEXT onHeightChanged ${height}  ${itemText}`)
            // onImplicitHeightChanged: albert.warning(`TEXT onImplicitHeightChanged ${implicitHeight} ${itemText}`)
            // Component.onCompleted: albert.warning(`TEXT onCompleted ${height} ${itemText}`)
        }

        Text
        {
            id: listItemSubText
            DebugRect{ name: "listItemSubText" }
            width: listItemTextColumn.width

            text: itemSubText
            elide: Text.ElideRight
            textFormat: Text.PlainText
            // Behavior on color { ColorAnimation{} }

            // onHeightChanged: albert.warning(`SUBTEXT onHeightChanged ${height}  ${itemText}`)
            // onImplicitHeightChanged: albert.warning(`SUBTEXT onImplicitHeightChanged ${implicitHeight} ${itemText}`)
            // Component.onCompleted: albert.warning(`SUBTEXT onCompleted ${height} ${itemText}`)
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

}
