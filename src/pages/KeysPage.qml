import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Flickable {
    id: page

    property string activeCombo: ""
    signal learnRequested(int index, string comboLabel)

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
        case "mouse":
            return qsTr("sent a mouse action")
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
            subtitle: qsTr("Turn the tablet's twelve shortcut keys into your own command centre.")

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
                    text: qsTr("The background helper is not installed. Install Slate with the package or ./install.sh to enable express-key remapping.")
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

                Rectangle {
                    width: 38
                    height: 38
                    radius: 19
                    color: backend.remapEnabled ? Qt.alpha("#5ed38b", 0.12) : "#1a1a20"
                    border.width: 1
                    border.color: backend.remapEnabled ? Qt.alpha("#5ed38b", 0.45) : "#303038"

                    Rectangle {
                        anchors.centerIn: parent
                        width: 8
                        height: 8
                        radius: 4
                        color: backend.remapEnabled ? "#5ed38b" : "#71717b"
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        text: backend.remapEnabled ? qsTr("Helper running") : qsTr("Factory shortcuts")
                        color: "#e4e4e9"
                        font.pixelSize: 12
                        font.weight: Font.Medium
                    }

                    Text {
                        Layout.fillWidth: true
                        text: backend.remapEnabled
                            ? qsTr("Presses are handled by the assignments below.")
                            : qsTr("While off, the keys send their original shortcuts exactly as before.")
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

        Rectangle {
            Layout.fillWidth: true
            implicitHeight: learnRow.implicitHeight + 28
            radius: 11
            color: Qt.alpha(theme.accent, 0.055)
            border.width: 1
            border.color: Qt.alpha(theme.accent, 0.20)

            RowLayout {
                id: learnRow
                anchors.fill: parent
                anchors.margins: 14
                spacing: 14

                Rectangle {
                    width: 34
                    height: 34
                    radius: 17
                    color: Qt.alpha(theme.accent, 0.14)
                    border.width: 1
                    border.color: Qt.alpha(theme.accent, 0.35)

                    Text {
                        anchors.centerIn: parent
                        text: "⌨"
                        color: theme.accent
                        font.pixelSize: 15
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        text: qsTr("Learn by doing")
                        color: "#e9e9ee"
                        font.pixelSize: 12
                        font.weight: Font.DemiBold
                    }

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Hold the circular learn button on a key for 4 seconds, then press any key or mouse button. Shortcuts save immediately; commands still need Apply.")
                        color: "#8c8c96"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Card {
            title: qsTr("Key assignments")
            subtitle: qsTr("Pass replays the factory shortcut. Keys sends a shortcut. Mouse clicks or scrolls. Command runs a shell command. Off swallows the press.")

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

                        onActionPicked: function(newAction) {
                            backend.updateKeyAction(modelData.index, newAction, modelData.value)
                            if (newAction !== "command")
                                backend.saveKeys()
                        }
                        onValueEdited: function(newValue) {
                            backend.updateKeyAction(modelData.index, modelData.action, newValue)
                            if (modelData.action !== "command")
                                backend.saveKeys()
                        }
                        onLearnRequested: function(index) {
                            page.learnRequested(index, modelData.label)
                        }
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
