import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

Rectangle {
    id: root

    property int keyIndex: 0
    property string combo: ""
    property string comboLabel: ""
    property string action: "pass"
    property string value: ""
    property bool highlighted: false
    signal actionPicked(string newAction)
    signal valueEdited(string newValue)
    signal learnRequested(int index)

    implicitHeight: 78
    radius: 10
    color: highlighted ? Qt.alpha(theme.accent, 0.10)
        : rowHover.hovered ? "#17171c" : "transparent"

    Behavior on color { ColorAnimation { duration: 100 } }

    property var actionOptions: [
        { value: "pass", label: qsTr("Pass") },
        { value: "keys", label: qsTr("Keys") },
        { value: "mouse", label: qsTr("Mouse") },
        { value: "command", label: qsTr("Command") },
        { value: "nothing", label: qsTr("Off") }
    ]

    function valueText() {
        if (root.action === "mouse")
            return backend.mouseLabel(root.value)
        if (root.action === "keys")
            return root.value.length > 0 ? backend.comboLabel(root.value) : qsTr("Hold learn to set")
        if (root.action === "command")
            return root.value.length > 0 ? root.value : qsTr("No command")
        if (root.action === "pass")
            return qsTr("Replays %1").arg(root.comboLabel)
        return qsTr("Swallowed")
    }

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: "#202026"
    }

    HoverHandler { id: rowHover }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 12
        anchors.rightMargin: 8
        spacing: 14

        RowLayout {
            Layout.preferredWidth: 200
            Layout.minimumWidth: 150
            spacing: 11

            Rectangle {
                width: 30
                height: 30
                radius: 9
                color: root.highlighted ? theme.accent : "#202027"
                border.width: 1
                border.color: root.highlighted ? theme.accent : "#2c2c34"

                Text {
                    anchors.centerIn: parent
                    text: root.keyIndex + 1
                    color: root.highlighted ? theme.accentForeground : "#9a9aa4"
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                }
            }

            ColumnLayout {
                spacing: 2
                Layout.fillWidth: true

                Text {
                    text: root.comboLabel
                    color: "#ececf0"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Text {
                    text: qsTr("Express key %1").arg(root.keyIndex + 1)
                    color: "#686872"
                    font.pixelSize: 10
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 320
            Layout.minimumWidth: 260
            Layout.preferredHeight: 34
            radius: 9
            color: "#101013"
            border.width: 1
            border.color: "#282830"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 2

                Repeater {
                    model: root.actionOptions

                    delegate: Rectangle {
                        id: actionOption

                        required property var modelData
                        property bool chosen: modelData.value === root.action

                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 7
                        color: chosen ? theme.accent : (actionHover.hovered ? "#1e1e24" : "transparent")

                        Behavior on color { ColorAnimation { duration: 90 } }

                        Text {
                            anchors.centerIn: parent
                            text: actionOption.modelData.label
                            color: actionOption.chosen ? theme.accentForeground : "#a9a9b3"
                            font.pixelSize: 10
                            font.weight: actionOption.chosen ? Font.DemiBold : Font.Normal
                        }

                        HoverHandler { id: actionHover }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: root.actionPicked(actionOption.modelData.value)
                        }
                    }
                }
            }
        }

        Item {
            Layout.fillWidth: true
            Layout.minimumWidth: 90
            implicitHeight: 36

            TextField {
                id: valueField

                anchors.fill: parent
                visible: root.action === "keys" || root.action === "command"
                text: root.action === "keys" ? backend.comboLabel(root.value) : root.value
                placeholderText: root.action === "keys"
                    ? qsTr("Hold learn to set a shortcut")
                    : qsTr("shell command")
                color: "#ececf0"
                placeholderTextColor: "#5d5d67"
                font.pixelSize: 11
                leftPadding: 10
                rightPadding: 10
                selectByMouse: true
                onEditingFinished: {
                    // The field accepts the same syntax as the daemon. A
                    // malformed value is left for the backend to reject on
                    // save instead of silently changing it here.
                    root.valueEdited(text)
                    text = root.action === "keys" ? backend.comboLabel(root.value) : root.value
                }

                background: Rectangle {
                    radius: 8
                    color: "#0f0f12"
                    border.width: 1
                    border.color: valueField.activeFocus ? theme.accent : "#2a2a31"
                }
            }

            Rectangle {
                anchors.fill: parent
                visible: root.action === "mouse"
                radius: 8
                color: mouseHover.hovered ? "#16161b" : "#0f0f12"
                border.width: 1
                border.color: mouseHover.hovered ? "#3a3a44" : "#2f2f38"

                Behavior on color { ColorAnimation { duration: 90 } }

                RowLayout {
                    anchors.fill: parent
                    anchors.leftMargin: 10
                    anchors.rightMargin: 10
                    spacing: 8

                    Text {
                        text: "◉"
                        color: theme.accent
                        font.pixelSize: 12
                    }

                    Text {
                        Layout.fillWidth: true
                        text: root.valueText()
                        color: "#d8d8de"
                        font.pixelSize: 11
                        elide: Text.ElideRight
                    }

                    Text {
                        text: "⌄"
                        color: mouseHover.hovered ? "#d4d4dc" : "#686872"
                        font.pixelSize: 13
                    }
                }

                HoverHandler { id: mouseHover }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        // Anchor the chooser under the value field instead of
                        // letting it open at the window origin.
                        const p = mapToItem(root, width / 2, height + 4)
                        mouseMenu.x = Math.max(4, Math.min(p.x - mouseMenu.width / 2,
                                                         root.width - mouseMenu.width - 4))
                        mouseMenu.y = p.y
                        mouseMenu.popup()
                    }
                }
            }

            Text {
                anchors.fill: parent
                visible: root.action === "pass" || root.action === "nothing"
                verticalAlignment: Text.AlignVCenter
                leftPadding: 2
                text: root.valueText()
                color: "#63636d"
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            Connections {
                target: root
                function onValueEdited() {
                    if (!valueField.activeFocus)
                        valueField.text = root.action === "keys" ? backend.comboLabel(root.value) : root.value
                }
            }
        }

        HoldButton {
            label: root.action === "keys"
                ? qsTr("Hold for 4 seconds, then press a key or mouse button")
                : root.action === "mouse"
                    ? qsTr("Hold for 4 seconds, then click a mouse button")
                    : qsTr("Hold for 4 seconds to learn a key or mouse assignment")
            onTriggered: root.learnRequested(root.keyIndex)
        }
    }

    Menu {
        id: mouseMenu

        width: 188
        padding: 5

        background: Rectangle {
            implicitWidth: 188
            color: "#17171c"
            radius: 10
            border.width: 1
            border.color: "#34343d"
        }

        MouseMenuItem {
            text: qsTr("Left click")
            actionValue: "left"
            selected: root.value === actionValue
            onTriggered: root.valueEdited(actionValue)
        }
        MouseMenuItem {
            text: qsTr("Right click")
            actionValue: "right"
            selected: root.value === actionValue
            onTriggered: root.valueEdited(actionValue)
        }
        MouseMenuItem {
            text: qsTr("Middle click")
            actionValue: "middle"
            selected: root.value === actionValue
            onTriggered: root.valueEdited(actionValue)
        }
        MouseMenuItem {
            text: qsTr("Back button")
            actionValue: "back"
            selected: root.value === actionValue
            onTriggered: root.valueEdited(actionValue)
        }
        MouseMenuItem {
            text: qsTr("Forward button")
            actionValue: "forward"
            selected: root.value === actionValue
            onTriggered: root.valueEdited(actionValue)
        }
        MenuSeparator {}
        MouseMenuItem {
            text: qsTr("Scroll up")
            actionValue: "wheelup"
            selected: root.value === actionValue
            onTriggered: root.valueEdited(actionValue)
        }
        MouseMenuItem {
            text: qsTr("Scroll down")
            actionValue: "wheeldown"
            selected: root.value === actionValue
            onTriggered: root.valueEdited(actionValue)
        }
        MouseMenuItem {
            text: qsTr("Scroll left")
            actionValue: "wheelleft"
            selected: root.value === actionValue
            onTriggered: root.valueEdited(actionValue)
        }
        MouseMenuItem {
            text: qsTr("Scroll right")
            actionValue: "wheelright"
            selected: root.value === actionValue
            onTriggered: root.valueEdited(actionValue)
        }
    }
}
