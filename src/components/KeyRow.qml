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

    implicitHeight: 70
    color: highlighted ? Qt.alpha(theme.accent, 0.09) : "transparent"
    radius: 8

    Rectangle {
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 1
        color: "#202026"
    }

    RowLayout {
        anchors.fill: parent
        anchors.leftMargin: 10
        anchors.rightMargin: 10
        spacing: 14

        RowLayout {
            Layout.preferredWidth: 200
            spacing: 10

            Rectangle {
                width: 26
                height: 26
                radius: 13
                color: root.highlighted ? theme.accent : "#202027"

                Text {
                    anchors.centerIn: parent
                    text: root.keyIndex + 1
                    color: root.highlighted ? theme.accentForeground : "#92929c"
                    font.pixelSize: 11
                    font.weight: Font.DemiBold
                }
            }

            ColumnLayout {
                spacing: 1
                Layout.fillWidth: true

                Text {
                    text: root.comboLabel
                    color: "#e5e5ea"
                    font.pixelSize: 12
                    font.weight: Font.Medium
                    elide: Text.ElideRight
                    Layout.fillWidth: true
                }

                Text {
                    text: qsTr("Express key %1").arg(root.keyIndex + 1)
                    color: "#6d6d77"
                    font.pixelSize: 10
                }
            }
        }

        Rectangle {
            Layout.preferredWidth: 296
            Layout.preferredHeight: 32
            radius: 8
            color: "#101013"
            border.width: 1
            border.color: "#26262c"

            RowLayout {
                anchors.fill: parent
                anchors.margins: 2
                spacing: 2

                Repeater {
                    model: [
                        { value: "pass", label: qsTr("Pass") },
                        { value: "keys", label: qsTr("Keys") },
                        { value: "command", label: qsTr("Command") },
                        { value: "nothing", label: qsTr("Off") }
                    ]

                    delegate: Rectangle {
                        id: actionOption

                        required property var modelData
                        property bool chosen: modelData.value === root.action

                        Layout.fillWidth: true
                        Layout.fillHeight: true
                        radius: 6
                        color: chosen ? theme.accent : (actionHover.hovered ? "#1d1d23" : "transparent")

                        Text {
                            anchors.centerIn: parent
                            text: actionOption.modelData.label
                            color: actionOption.chosen ? theme.accentForeground : "#a9a9b3"
                            font.pixelSize: 11
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
            Layout.minimumWidth: 140
            implicitHeight: 34

            TextField {
                id: valueField

                anchors.fill: parent
                visible: root.action === "keys" || root.action === "command"
                text: root.value
                placeholderText: root.action === "keys" ? qsTr("ctrl+shift+t") : qsTr("shell command")
                color: "#ececf0"
                placeholderTextColor: "#5d5d67"
                font.pixelSize: 12
                leftPadding: 10
                rightPadding: 10
                selectByMouse: true
                onEditingFinished: {
                    root.valueEdited(text)
                    text = root.value
                }

                background: Rectangle {
                    radius: 7
                    color: "#0f0f12"
                    border.width: 1
                    border.color: valueField.activeFocus ? theme.accent : "#2a2a31"
                }
            }

            Text {
                anchors.fill: parent
                visible: !valueField.visible
                verticalAlignment: Text.AlignVCenter
                leftPadding: 2
                text: root.action === "pass" ? qsTr("Replays %1").arg(root.comboLabel) : qsTr("Does nothing")
                color: "#63636d"
                font.pixelSize: 11
                elide: Text.ElideRight
            }

            Connections {
                target: root
                function onValueEdited() {
                    if (!valueField.activeFocus)
                        valueField.text = root.value
                }
            }
        }
    }
}
