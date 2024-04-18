import QtQuick

Item{
    property color  color
    property color  color_current
    property alias  font_size: text.font.pixelSize
    property alias  padding: text.padding

    width: parent.width
    height: text.height

    Text { DebugRect{}
        id: text
        width: parent.width
        color: ListView.isCurrentItem ? parent.color_current : parent.color
        text: display
        textFormat: Text.PlainText
        elide: Text.ElideRight
        horizontalAlignment: Text.AlignHCenter
    }

    signal clicked(int index)
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: parent.clicked(index)
    }
}
