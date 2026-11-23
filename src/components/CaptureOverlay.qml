import QtQuick
import QtQuick.Layouts

// Full-window capture sheet. It is opened by a four-second press on a key's
// learn control, then turns the next keyboard or mouse press into an action.
Item {
    id: root

    anchors.fill: parent
    visible: false
    z: 200
    focus: visible

    property int keyIndex: -1
    property string comboLabel: ""
    property string modifierPreview: ""

    signal captured(string action, string value)
    signal cancelled()

    function open(index) {
        keyIndex = index
        modifierPreview = ""
        visible = true
        forceActiveFocus()
    }

    function close() {
        visible = false
        modifierPreview = ""
    }

    function modifierText(modifiers) {
        var parts = []
        if (modifiers & Qt.ControlModifier) parts.push(qsTr("Ctrl"))
        if (modifiers & Qt.ShiftModifier) parts.push(qsTr("Shift"))
        if (modifiers & Qt.AltModifier) parts.push(qsTr("Alt"))
        if (modifiers & Qt.MetaModifier) parts.push(qsTr("Super"))
        return parts.join(" + ")
    }

    function mouseAction(button) {
        if (button === Qt.LeftButton) return "left"
        if (button === Qt.RightButton) return "right"
        if (button === Qt.MiddleButton) return "middle"
        if (button === Qt.BackButton) return "back"
        if (button === Qt.ForwardButton) return "forward"
        return ""
    }

    function handleKey(event) {
        if (event.isAutoRepeat)
            return
        if (event.key === Qt.Key_Escape) {
            event.accepted = true
            root.cancelled()
            return
        }

        var combo = backend.comboFromKey(event.key, event.modifiers, event.text)
        if (combo.length === 0) {
            root.modifierPreview = root.modifierText(event.modifiers)
            event.accepted = true
            return
        }

        event.accepted = true
        root.captured("keys", combo)
    }

    Keys.onPressed: function(event) { root.handleKey(event) }

    Rectangle {
        anchors.fill: parent
        color: Qt.rgba(0.02, 0.02, 0.03, 0.86)
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton | Qt.MiddleButton
                         | Qt.BackButton | Qt.ForwardButton
        cursorShape: Qt.CrossCursor
        onPressed: function(mouse) {
            const action = root.mouseAction(mouse.button)
            if (action.length > 0)
                root.captured("mouse", action)
        }
        onWheel: function(wheel) {
            if (wheel.angleDelta.y > 0)
                root.captured("mouse", "wheelup")
            else if (wheel.angleDelta.y < 0)
                root.captured("mouse", "wheeldown")
            else if (wheel.angleDelta.x > 0)
                root.captured("mouse", "wheelright")
            else if (wheel.angleDelta.x < 0)
                root.captured("mouse", "wheelleft")
        }
    }

    Rectangle {
        id: sheet

        width: Math.min(580, root.width - 48)
        implicitHeight: content.implicitHeight + 56
        anchors.centerIn: parent
        radius: 16
        color: "#151519"
        border.width: 1
        border.color: "#33333c"

        ColumnLayout {
            id: content

            anchors.fill: parent
            anchors.margins: 28
            spacing: 18

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3

                    Text {
                        text: qsTr("Learn express key %1").arg(root.keyIndex + 1)
                        color: "#f3f3f6"
                        font.pixelSize: 20
                        font.weight: Font.DemiBold
                    }

                    Text {
                        text: root.comboLabel.length > 0
                            ? qsTr("Currently sends %1").arg(root.comboLabel)
                            : qsTr("Choose what this key should do")
                        color: "#8f8f99"
                        font.pixelSize: 11
                    }
                }

                Rectangle {
                    implicitWidth: cancelButton.implicitWidth + 24
                    implicitHeight: 28
                    radius: 14
                    color: cancelHover.hovered ? "#232329" : "#101014"
                    border.width: 1
                    border.color: "#2d2d35"

                    Text {
                        id: cancelButton
                        anchors.centerIn: parent
                        text: qsTr("Cancel")
                        color: "#b7b7c0"
                        font.pixelSize: 10
                        font.weight: Font.Medium
                    }

                    HoverHandler { id: cancelHover }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.cancelled()
                    }
                }
            }

            Rectangle {
                Layout.alignment: Qt.AlignHCenter
                width: 92
                height: 92
                radius: 46
                color: Qt.alpha(theme.accent, 0.10)
                border.width: 1
                border.color: Qt.alpha(theme.accent, 0.55)

                SequentialAnimation on scale {
                    running: root.visible
                    loops: Animation.Infinite
                    NumberAnimation { to: 1.04; duration: 900; easing.type: Easing.InOutSine }
                    NumberAnimation { to: 1.0; duration: 900; easing.type: Easing.InOutSine }
                }

                Text {
                    anchors.centerIn: parent
                    text: "⌨"
                    color: theme.accent
                    font.pixelSize: 34
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 6

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Press a key or mouse button")
                    color: "#f0f0f3"
                    font.pixelSize: 16
                    font.weight: Font.DemiBold
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Modifiers are supported. The next complete input becomes the new assignment.")
                    color: "#7f7f89"
                    font.pixelSize: 11
                    wrapMode: Text.WordWrap
                }
            }

            Rectangle {
                Layout.fillWidth: true
                implicitHeight: 44
                radius: 9
                color: "#0f0f12"
                border.width: 1
                border.color: root.modifierPreview.length > 0
                    ? Qt.alpha(theme.accent, 0.6) : "#27272e"

                Text {
                    anchors.centerIn: parent
                    text: root.modifierPreview.length > 0
                        ? root.modifierPreview + " + …"
                        : qsTr("Waiting for input")
                    color: root.modifierPreview.length > 0 ? theme.accent : "#6f6f79"
                    font.pixelSize: 12
                    font.weight: root.modifierPreview.length > 0 ? Font.DemiBold : Font.Normal
                }
            }

            Text {
                Layout.fillWidth: true
                horizontalAlignment: Text.AlignHCenter
                text: qsTr("Mouse: left, right, middle, back, forward, and scroll. Press Esc or Cancel to close.")
                color: "#5f5f69"
                font.pixelSize: 10
                wrapMode: Text.WordWrap
            }
        }
    }
}
