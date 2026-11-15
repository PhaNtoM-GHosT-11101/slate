import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Flickable {
    id: page

    property string activeCombo: ""

    contentWidth: width
    contentHeight: column.implicitHeight + 56
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

    function lastActionText() {
        switch (backend.lastKeyAction) {
        case "pass":
            return qsTr("passed through")
        case "keys":
            return qsTr("sent a shortcut")
        case "command":
            return qsTr("ran a command")
        case "nothing":
            return qsTr("ignored")
        default:
            return qsTr("detected")
        }
    }

    Timer {
        id: clearHighlight
        interval: 2800
        onTriggered: page.activeCombo = ""
    }

    Connections {
        target: backend

        function onLastKeyChanged() {
            page.activeCombo = backend.lastKeyCombo
            clearHighlight.restart()
        }
    }

    ColumnLayout {
        id: column

        x: Math.max(28, (page.width - width) / 2)
        y: 26
        width: Math.min(920, page.width - 56)
        spacing: 16

        PageHeader {
            title: qsTr("Express keys")
            subtitle: qsTr("The tablet's 12 shortcut keys send fixed keyboard combinations. Slate can take them over and run actions of your own.")

            AppButton {
                text: qsTr("Reset bindings")
                onClicked: backend.resetKeys()
            }
        }

        Rectangle {
            visible: !backend.serviceInstalled
            Layout.fillWidth: true
            implicitHeight: helperRow.implicitHeight + 24
            radius: 10
            color: "#2a2114"
            border.width: 1
            border.color: "#54431f"

            RowLayout {
                id: helperRow
                anchors.fill: parent
                anchors.margins: 12
                spacing: 10

                Rectangle {
                    width: 7
                    height: 7
                    radius: 3.5
                    color: "#e0a94a"
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTr("The background helper is not installed yet. Install Slate as a package to enable express-key remapping.")
                    color: "#d7b978"
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }
        }

        Card {
            title: qsTr("Remapping")
            subtitle: qsTr("The helper grabs only the tablet's express-key interface. Your normal keyboard is never touched.")

            RowLayout {
                Layout.fillWidth: true
                spacing: 14

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        text: qsTr("Remap express keys")
                        color: "#e4e4e9"
                        font.pixelSize: 12
                    }

                    Text {
                        Layout.fillWidth: true
                        text: backend.remapEnabled
                            ? qsTr("The helper is running. Presses are handled by the bindings below.")
                            : qsTr("When off, the keys send their factory shortcuts exactly as before.")
                        color: "#71717b"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }

                Toggle {
                    checked: backend.remapEnabled
                    enabled: backend.serviceInstalled
                    onToggled: function(value) { backend.setRemapEnabled(value) }
                }
            }

            Rectangle {
                Layout.fillWidth: true
                height: 1
                color: "#222228"
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 10

                Rectangle {
                    width: 7
                    height: 7
                    radius: 3.5
                    color: !backend.serviceInstalled ? "#e0a94a"
                        : backend.remapEnabled ? theme.accent : "#6b6b75"
                }

                Text {
                    text: backend.serviceState
                    color: "#9a9aa4"
                    font.pixelSize: 11
                }

                Item { Layout.fillWidth: true }

                Rectangle {
                    implicitWidth: lastRow.implicitWidth + 20
                    implicitHeight: 28
                    radius: 14
                    color: "#101013"
                    border.width: 1
                    border.color: page.activeCombo.length > 0 ? Qt.alpha(theme.accent, 0.55) : "#27272e"

                    RowLayout {
                        id: lastRow
                        anchors.centerIn: parent
                        spacing: 8

                        Text {
                            text: page.activeCombo.length > 0
                                ? qsTr("Last: %1").arg(backend.lastKeyLabel)
                                : qsTr("Press a tablet key to test")
                            color: page.activeCombo.length > 0 ? "#e0e0e6" : "#74747e"
                            font.pixelSize: 10
                        }

                        Text {
                            visible: page.activeCombo.length > 0
                            text: page.lastActionText()
                            color: "#6f6f79"
                            font.pixelSize: 10
                        }
                    }
                }
            }
        }

        Card {
            title: qsTr("Bindings")
            subtitle: qsTr("Pass replays the factory shortcut. Keys sends a new combination. Command runs a shell command. Off does nothing.")

            Column {
                Layout.fillWidth: true
                spacing: 0

                Repeater {
                    model: backend.keyBindings

                    delegate: KeyRow {
                        width: parent.width
                        keyIndex: modelData.index
                        combo: modelData.combo
                        comboLabel: modelData.label
                        action: modelData.action
                        value: modelData.value
                        highlighted: page.activeCombo === modelData.combo

                        onActionPicked: backend.updateKeyAction(modelData.index, newAction, modelData.value)
                        onValueEdited: backend.updateKeyAction(modelData.index, modelData.action, newValue)
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("Bindings are stored in %1 and reloaded by the helper on every change.").arg(backend.keysPath)
                color: "#5f5f69"
                font.pixelSize: 10
                wrapMode: Text.WordWrap
            }
        }

        Item { Layout.preferredHeight: 2 }
    }
}
