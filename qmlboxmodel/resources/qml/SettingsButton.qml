import QtQuick
import Qt5Compat.GraphicalEffects
import QtQuick.Window 2.2  // Screen.devicePixelRatio

Item {
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
        opacity: 0
        cached: true
        source: gearcolor
        maskSource: gearmask
        RotationAnimator on rotation {
            loops: Animation.Infinite;
            from: 0; to: 360
            duration: busy ? 5000 : 10000
            Behavior on duration { NumberAnimation {} }
        }
    }

    signal clicked()
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        acceptedButtons: Qt.LeftButton|Qt.RightButton
        onClicked: (mouse) => {
            if (mouse.button === Qt.RightButton)
                style.debug = !style.debug
            else if (mouse.button === Qt.LeftButton)
                parent.clicked()
        }
    }

    states :[
        State {
            name: ""
            PropertyChanges { target: gear; opacity: 0 }
        },
        State {
            name: "hovered"
            when: mouseArea.containsMouse || busy
            PropertyChanges { target: gear; opacity: 1 }
        }
    ]
    transitions: Transition {
        to: "*"
        NumberAnimation { properties: "opacity" }
    }
}
