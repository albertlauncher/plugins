import QtQuick
import Qt5Compat.GraphicalEffects
import QtQuick.Window 2.2  // Screen.devicePixelRatio

MouseArea {
    id: root
    hoverEnabled: true
    propagateComposedEvents: true
    acceptedButtons: Qt.LeftButton | Qt.RightButton
    property alias color: gearcolor.color
    property int size

    Image {
        id: gearmask
        anchors.centerIn: parent
        width: root.size; height: root.size
        cache: true
        smooth: true
        source: "qrc:gear.svg"
        sourceSize.width: width * Screen.devicePixelRatio
        sourceSize.height: height * Screen.devicePixelRatio
        visible: false
    }

    Rectangle {
        id: gearcolor
        anchors.fill: gearmask
        visible: false
        Behavior on color { ColorAnimation { } }
    }

    OpacityMask {
        id: gear
        anchors.fill: gearmask
        smooth: true
        source: gearcolor
        maskSource: gearmask
        RotationAnimation on rotation {
            id: rotationAnimation
            duration: 1000
            from: 0; to: 59  // gear has 6 teeth
            loops: Animation.Infinite;
            running: root.visible
        }
    }

    onClicked: (mouse) => {
        if (mouse.button === Qt.RightButton)
            debug = !debug
        else if (mouse.button === Qt.LeftButton)
            albert.showSettings()
    }
}
