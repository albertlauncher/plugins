import QtQuick
import Qt5Compat.GraphicalEffects
import QtQuick.Window 2.2  // Screen.devicePixelRatio

MouseArea {
    id: mouseArea
    hoverEnabled: true
    property alias iconColor: gearcolor.color
    property bool busy: false

    Image {
        id: gearmask
        anchors.fill: parent
        smooth: true
        source: "qrc:gear.svg"
        sourceSize.width: width * Screen.devicePixelRatio
        sourceSize.height: height * Screen.devicePixelRatio
        visible: false
    }

    Rectangle {
        id: gearcolor
        anchors.fill: parent
        visible: false
    }

    OpacityMask {
        id: gear
        anchors.fill: parent
        smooth: true
        opacity: mouseArea.containsMouse || busy ? 1 : 0
        Behavior on opacity { NumberAnimation{} }
        cached: true
        source: gearcolor
        maskSource: gearmask
        RotationAnimator on rotation {
            loops: Animation.Infinite;
            from: 0; to: 360
            duration: 4000
        }
    }
}
