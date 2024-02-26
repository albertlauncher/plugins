import QtQuick
import QtQuick.Shapes

Loader {
    id: loader
    active: debug
    property var target: parent
    property string name: target.objectName
    anchors.fill: target

    sourceComponent: Item{
        id: debugItem
        anchors.fill: parent

        property real hue: Math.random()
        property color color: Qt.hsva(hue, 1, 1, 0.5)

        z: 100

        Rectangle{
            x: Math.random()*(parent.width-width)
            y: Math.random()*(parent.height-height)
            width: text.width
            height: text.height
            color: Qt.hsva(hue, 1, 1, 0.3)
            Text{
                id: text
                text: name
                font.pixelSize: 8
            }
        }

        Shape {
            z: 100
            anchors.fill: parent
            ShapePath {
                dashPattern: [ 1, 2 ]
                fillColor: Qt.hsva(hue, 1, 1, 0.1)
                strokeColor: Qt.hsva(hue, 1, 1, 0.2)
                strokeStyle: ShapePath.DashLine
                strokeWidth: 1
                startX: 0; startY:0

                PathLine { relativeX: target.width; relativeY: 0 }
                PathLine { relativeX: 0; relativeY: target.height }
                PathLine { relativeX: -target.width; relativeY: 0 }
                PathLine { relativeX: 0; relativeY: -target.height }
            }
        }
    }
}
