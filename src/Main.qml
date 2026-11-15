import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import "components"
import "pages"

ApplicationWindow {
    id: win

    width: 1040
    height: 800
    minimumWidth: 860
    minimumHeight: 620
    visible: true
    title: qsTr("Slate")
    color: "#0b0b0d"

    Material.theme: Material.Dark
    Material.accent: theme.accent
    Material.primary: theme.accent
    Material.background: "#0b0b0d"
    Material.foreground: "#e8e8ec"

    property int currentTab: 0
    property bool helpVisible: false
    property var tabs: [qsTr("Mapping"), qsTr("Pen"), qsTr("Express keys")]

    Shortcut { sequence: "Ctrl+Q"; context: Qt.ApplicationShortcut; onActivated: Qt.quit() }
    Shortcut {
        sequence: "Ctrl+S"
        context: Qt.ApplicationShortcut
        onActivated: {
            win.contentItem.forceActiveFocus()
            backend.applyAll()
        }
    }
    Shortcut { sequence: "Ctrl+1"; context: Qt.ApplicationShortcut; onActivated: win.currentTab = 0 }
    Shortcut { sequence: "Ctrl+2"; context: Qt.ApplicationShortcut; onActivated: win.currentTab = 1 }
    Shortcut { sequence: "Ctrl+3"; context: Qt.ApplicationShortcut; onActivated: win.currentTab = 2 }
    Shortcut {
        sequence: "?"
        context: Qt.ApplicationShortcut
        onActivated: win.helpVisible = !win.helpVisible
    }
    Shortcut {
        sequence: "Escape"
        context: Qt.ApplicationShortcut
        onActivated: win.helpVisible = false
    }

    Rectangle {
        anchors.fill: parent
        color: "#0b0b0d"
    }

    Rectangle {
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 260
        gradient: Gradient {
            GradientStop { position: 0.0; color: Qt.alpha(theme.accent, 0.045) }
            GradientStop { position: 1.0; color: "transparent" }
        }
    }

    Rectangle {
        id: header

        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        height: 74
        color: "transparent"

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 28
            anchors.rightMargin: 28
            spacing: 24

            RowLayout {
                spacing: 11

                Rectangle {
                    width: 34
                    height: 34
                    radius: 10
                    color: theme.accent

                    Text {
                        anchors.centerIn: parent
                        text: "s"
                        color: theme.accentForeground
                        font.pixelSize: 21
                        font.weight: Font.Bold
                    }
                }

                ColumnLayout {
                    spacing: 0

                    Text {
                        text: "slate"
                        color: "#f2f2f5"
                        font.pixelSize: 19
                        font.weight: Font.DemiBold
                    }

                    Text {
                        text: qsTr("TABLET SETTINGS")
                        color: "#6f6f79"
                        font.pixelSize: 9
                        font.letterSpacing: 1.5
                    }
                }
            }

            RowLayout {
                spacing: 4

                Repeater {
                    model: win.tabs

                    delegate: Rectangle {
                        required property int index
                        required property string modelData

                        width: tabLabel.implicitWidth + 28
                        height: 34
                        radius: 8
                        color: win.currentTab === index
                            ? Qt.alpha(theme.accent, 0.12)
                            : (tabHover.hovered ? "#16161a" : "transparent")

                        Behavior on color { ColorAnimation { duration: 100 } }

                        Text {
                            id: tabLabel
                            anchors.centerIn: parent
                            text: parent.modelData
                            color: win.currentTab === parent.index ? theme.accent : "#9a9aa4"
                            font.pixelSize: 12
                            font.weight: win.currentTab === parent.index ? Font.DemiBold : Font.Medium
                        }

                        HoverHandler { id: tabHover }

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor
                            onClicked: win.currentTab = parent.index
                        }
                    }
                }
            }

            Item { Layout.fillWidth: true }

            Rectangle {
                implicitWidth: tabletPill.implicitWidth + 20
                height: 28
                radius: 14
                color: "#111116"
                border.width: 1
                border.color: "#27272e"

                RowLayout {
                    id: tabletPill
                    anchors.centerIn: parent
                    spacing: 7

                    Rectangle {
                        width: 6
                        height: 6
                        radius: 3
                        color: backend.tabletConnected ? "#5ed38b" : "#e0a94a"
                    }

                    Text {
                        text: backend.tabletConnected ? backend.tabletName : qsTr("Tablet not found")
                        color: backend.tabletConnected ? "#b8b8c1" : "#c8a45c"
                        font.pixelSize: 10
                    }
                }
            }

            Rectangle {
                width: 30
                height: 30
                radius: 15
                color: helpHover.hovered ? "#1c1c22" : "transparent"
                border.width: 1
                border.color: helpHover.hovered ? "#33333b" : "transparent"

                Text {
                    anchors.centerIn: parent
                    text: "?"
                    color: "#83838d"
                    font.pixelSize: 13
                    font.weight: Font.DemiBold
                }

                HoverHandler { id: helpHover }

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: win.helpVisible = !win.helpVisible
                }
            }
        }
    }

    StackLayout {
        anchors.top: header.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: footer.top
        currentIndex: win.currentTab

        MappingPage {}
        PenPage {}
        KeysPage {}
    }

    Rectangle {
        id: footer

        anchors.left: parent.left
        anchors.right: parent.right
        anchors.bottom: parent.bottom
        height: 62
        color: "#0f0f13"

        Rectangle {
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            height: 1
            color: "#202026"
        }

        RowLayout {
            anchors.fill: parent
            anchors.leftMargin: 28
            anchors.rightMargin: 28
            spacing: 12

            Rectangle {
                width: 7
                height: 7
                radius: 3.5
                color: backend.messageError ? "#ff6b6b" : backend.dirty ? theme.accent : "#5c5c66"
            }

            Text {
                Layout.fillWidth: true
                Layout.maximumWidth: 620
                text: backend.message
                color: backend.messageError ? "#ff9b9b" : "#8f8f99"
                font.pixelSize: 11
                maximumLineCount: 2
                elide: Text.ElideRight
                wrapMode: Text.WordWrap
            }

            Item { Layout.fillWidth: true }

            Text {
                visible: backend.dirty
                text: qsTr("Unsaved changes")
                color: theme.accent
                font.pixelSize: 10
            }

            Text {
                visible: win.width >= 940
                text: qsTr("Ctrl+S")
                color: "#53535d"
                font.pixelSize: 10
            }

            AppButton {
                text: qsTr("Revert")
                Layout.minimumWidth: 84
                onClicked: backend.revert()
            }

            AppButton {
                text: qsTr("Apply changes")
                primary: true
                enabled: backend.dirty
                implicitWidth: 124
                Layout.minimumWidth: 124
                onClicked: {
                    win.contentItem.forceActiveFocus()
                    backend.applyAll()
                }
            }
        }
    }

    Rectangle {
        id: helpOverlay

        anchors.fill: parent
        visible: win.helpVisible
        z: 50
        color: Qt.rgba(0, 0, 0, 0.68)
        focus: visible

        Keys.onEscapePressed: win.helpVisible = false
        onVisibleChanged: if (visible) forceActiveFocus()

        MouseArea {
            anchors.fill: parent
            onClicked: win.helpVisible = false
        }

        Rectangle {
            width: Math.min(500, parent.width - 48)
            implicitHeight: helpColumn.implicitHeight + 48
            anchors.centerIn: parent
            radius: 14
            color: "#151518"
            border.width: 1
            border.color: "#2b2b32"

            MouseArea { anchors.fill: parent }

            ColumnLayout {
                id: helpColumn

                anchors.fill: parent
                anchors.margins: 24
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true

                    Text {
                        text: qsTr("Slate")
                        color: "#f2f2f5"
                        font.pixelSize: 20
                        font.weight: Font.DemiBold
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        text: qsTr("VEIKK A15")
                        color: "#65656f"
                        font.pixelSize: 10
                        font.letterSpacing: 0.8
                    }
                }

                Text {
                    Layout.fillWidth: true
                    text: qsTr("Mapping writes Hyprland tablet settings into your input.lua. Pen shows the live pressure range and raw tablet input. Express keys are handled by a small background helper that grabs only the tablet's shortcut interface.")
                    color: "#a2a2ac"
                    font.pixelSize: 12
                    lineHeight: 1.4
                    wrapMode: Text.WordWrap
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#26262d"
                }

                GridLayout {
                    Layout.fillWidth: true
                    columns: 2
                    columnSpacing: 24
                    rowSpacing: 8

                    Text { text: qsTr("Ctrl+1 · Ctrl+2 · Ctrl+3"); color: "#d0d0d7"; font.pixelSize: 11 }
                    Text { text: qsTr("Switch tabs"); color: "#777781"; font.pixelSize: 11 }
                    Text { text: qsTr("Ctrl+S"); color: "#d0d0d7"; font.pixelSize: 11 }
                    Text { text: qsTr("Apply changes"); color: "#777781"; font.pixelSize: 11 }
                    Text { text: qsTr("Ctrl+Q"); color: "#d0d0d7"; font.pixelSize: 11 }
                    Text { text: qsTr("Quit"); color: "#777781"; font.pixelSize: 11 }
                    Text { text: "?"; color: "#d0d0d7"; font.pixelSize: 11 }
                    Text { text: qsTr("Show this help"); color: "#777781"; font.pixelSize: 11 }
                }

                RowLayout {
                    Layout.fillWidth: true

                    Item { Layout.fillWidth: true }

                    AppButton {
                        text: qsTr("Close")
                        primary: true
                        implicitWidth: 92
                        onClicked: win.helpVisible = false
                    }
                }
            }
        }
    }
}
