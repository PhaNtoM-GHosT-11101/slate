import QtQuick

Item {
    id: root

    property bool checked: false
    signal toggled(bool value)

    implicitWidth: 46
    implicitHeight: 26
    opacity: enabled ? 1 : 0.45
    activeFocusOnTab: enabled

    Keys.onSpacePressed: root.toggled(!root.checked)
    Keys.onReturnPressed: root.toggled(!root.checked)

    Rectangle {
        id: track
        anchors.fill: parent
        radius: height / 2
        color: root.checked ? theme.accent : "#25252c"
        border.width: 1
        border.color: root.activeFocus ? theme.accent : (root.checked ? theme.accent : "#3b3b44")
        Behavior on color { ColorAnimation { duration: 120 } }

        Rectangle {
            id: knob
            width: 20
            height: 20
            radius: 10
            y: 2
            x: root.checked ? track.width - width - 3 : 3
            color: root.checked ? theme.accentForeground : "#a7a7b1"
            Behavior on x { NumberAnimation { duration: 140; easing.type: Easing.OutCubic } }
            Behavior on color { ColorAnimation { duration: 120 } }
        }
    }

    MouseArea {
        anchors.fill: parent
        enabled: root.enabled
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            root.forceActiveFocus()
            root.toggled(!root.checked)
        }
    }
}
