import QtQuick
import QtQuick.Controls
import QtQuick.Controls.Material
import QtQuick.Layouts

import "components"
import "pages"

ApplicationWindow {
    id: win

    width: 1200
    height: 840
    minimumWidth: 1000
    minimumHeight: 640
    visible: true
    title: qsTr("Slate — VEIKK A15")
    color: "#0a0a0d"

    Material.theme: Material.Dark
    Material.accent: theme.accent
    Material.primary: theme.accent
    Material.background: "#0a0a0d"
    Material.foreground: "#e8e8ec"

    property int currentTab: initialPage
    property bool helpVisible: false

    property var navItems: [
        { label: qsTr("Mapping"), glyph: "▦", tab: 0 },
        { label: qsTr("Pen"), glyph: "✎", tab: 1 },
        { label: qsTr("Express keys"), glyph: "⌨", tab: 2 },
        { label: qsTr("About"), glyph: "ⓘ", tab: 3 }
    ]

    Shortcut { sequence: "Ctrl+Q"; context: Qt.ApplicationShortcut; enabled: !capture.visible; onActivated: Qt.quit() }
    Shortcut {
        sequence: "Ctrl+S"
        context: Qt.ApplicationShortcut
        enabled: !capture.visible
        onActivated: {
            win.contentItem.forceActiveFocus()
            backend.applyAll()
        }
    }
    Shortcut { sequence: "Ctrl+1"; context: Qt.ApplicationShortcut; enabled: !capture.visible; onActivated: win.currentTab = 0 }
    Shortcut { sequence: "Ctrl+2"; context: Qt.ApplicationShortcut; enabled: !capture.visible; onActivated: win.currentTab = 1 }
    Shortcut { sequence: "Ctrl+3"; context: Qt.ApplicationShortcut; enabled: !capture.visible; onActivated: win.currentTab = 2 }
    Shortcut { sequence: "Ctrl+4"; context: Qt.ApplicationShortcut; enabled: !capture.visible; onActivated: win.currentTab = 3 }
    Shortcut {
        sequence: "?"
        context: Qt.ApplicationShortcut
        // Do not steal "?" while the user is typing a command or shortcut.
        enabled: !capture.visible
            && (!win.activeFocusItem || win.activeFocusItem.text === undefined)
        onActivated: win.helpVisible = !win.helpVisible
    }
    Shortcut {
        sequence: "Escape"
        context: Qt.ApplicationShortcut
        onActivated: {
            if (capture.visible)
                capture.cancelled()
            else
                win.helpVisible = false
        }
    }

    Rectangle {
        anchors.fill: parent
        color: "#0a0a0d"
    }

    RowLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: sidebar

            Layout.fillHeight: true
            Layout.preferredWidth: 236
            color: "#0d0d10"

            Rectangle {
                anchors.right: parent.right
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                width: 1
                color: "#202027"
            }

            ColumnLayout {
                anchors.fill: parent
                anchors.margins: 16
                spacing: 14

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 11

                    Rectangle {
                        width: 38
                        height: 38
                        radius: 11
                        color: theme.accent

                        Text {
                            anchors.centerIn: parent
                            text: "s"
                            color: theme.accentForeground
                            font.pixelSize: 23
                            font.weight: Font.Bold
                        }
                    }

                    ColumnLayout {
                        spacing: 0

                        Text {
                            text: "slate"
                            color: "#f2f2f5"
                            font.pixelSize: 20
                            font.weight: Font.DemiBold
                        }

                        Text {
                            text: qsTr("VEIKK A15")
                            color: "#70707a"
                            font.pixelSize: 9
                            font.letterSpacing: 1.4
                        }
                    }

                    Item { Layout.fillWidth: true }
                }

                Rectangle {
                    Layout.fillWidth: true
                    height: 1
                    color: "#1d1d23"
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 3

                    Repeater {
                        model: win.navItems

                        delegate: Rectangle {
                            id: navItem

                            required property var modelData
                            property bool selected: win.currentTab === modelData.tab

                            Layout.fillWidth: true
                            implicitHeight: 42
                            radius: 9
                            color: selected ? Qt.alpha(theme.accent, 0.12)
                                : navHover.hovered ? "#17171c" : "transparent"

                            Behavior on color { ColorAnimation { duration: 100 } }

                            Rectangle {
                                visible: navItem.selected
                                width: 3
                                height: 20
                                radius: 1.5
                                anchors.left: parent.left
                                anchors.verticalCenter: parent.verticalCenter
                                color: theme.accent
                            }

                            RowLayout {
                                anchors.fill: parent
                                anchors.leftMargin: 14
                                anchors.rightMargin: 10
                                spacing: 11

                                Text {
                                    text: navItem.modelData.glyph
                                    color: navItem.selected ? theme.accent : "#7d7d88"
                                    font.pixelSize: 15
                                    Layout.preferredWidth: 20
                                }

                                Text {
                                    Layout.fillWidth: true
                                    text: navItem.modelData.label
                                    color: navItem.selected ? "#f0f0f3"
                                        : navHover.hovered ? "#d0d0d7" : "#a4a4ae"
                                    font.pixelSize: 12
                                    font.weight: navItem.selected ? Font.DemiBold : Font.Normal
                                    elide: Text.ElideRight
                                }

                                Rectangle {
                                    visible: navItem.modelData.tab === 2 && backend.remapEnabled
                                    width: 6
                                    height: 6
                                    radius: 3
                                    color: theme.accent
                                }
                            }

                            HoverHandler { id: navHover }

                            MouseArea {
                                anchors.fill: parent
                                cursorShape: Qt.PointingHandCursor
                                onClicked: win.currentTab = navItem.modelData.tab
                            }
                        }
                    }
                }

                Item { Layout.fillHeight: true }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: statusColumn.implicitHeight + 24
                    radius: 11
                    color: "#141418"
                    border.width: 1
                    border.color: "#25252c"

                    ColumnLayout {
                        id: statusColumn
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 9

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 7

                            Rectangle {
                                width: 7
                                height: 7
                                radius: 3.5
                                color: backend.tabletConnected ? "#5ed38b" : "#e0a94a"
                            }

                            Text {
                                Layout.fillWidth: true
                                text: backend.tabletConnected ? qsTr("Tablet connected") : qsTr("Tablet not found")
                                color: backend.tabletConnected ? "#c7e8d4" : "#d7b978"
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 1
                            color: "#222228"
                        }

                        RowLayout {
                            Layout.fillWidth: true
                            spacing: 7

                            Rectangle {
                                width: 7
                                height: 7
                                radius: 3.5
                                color: backend.remapEnabled ? theme.accent
                                    : backend.serviceInstalled ? "#6b6b75" : "#e0a94a"
                            }

                            Text {
                                Layout.fillWidth: true
                                text: backend.remapEnabled ? qsTr("Remapping on")
                                    : backend.serviceInstalled ? qsTr("Factory shortcuts") : qsTr("Helper not installed")
                                color: "#8f8f99"
                                font.pixelSize: 10
                                elide: Text.ElideRight
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    implicitHeight: 36
                    radius: 9
                    color: helpHover.hovered ? "#17171c" : "transparent"

                    RowLayout {
                        anchors.fill: parent
                        anchors.leftMargin: 10
                        anchors.rightMargin: 10
                        spacing: 9

                        Text {
                            text: "?"
                            color: "#7d7d88"
                            font.pixelSize: 13
                            font.weight: Font.DemiBold
                            Layout.preferredWidth: 18
                            horizontalAlignment: Text.AlignHCenter
                        }

                        Text {
                            Layout.fillWidth: true
                            text: qsTr("Keyboard shortcuts")
                            color: "#8f8f99"
                            font.pixelSize: 10
                            elide: Text.ElideRight
                        }

                        Text {
                            text: "?"
                            color: "#5c5c66"
                            font.pixelSize: 10
                        }
                    }

                    HoverHandler { id: helpHover }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: win.helpVisible = !win.helpVisible
                    }
                }

                Text {
                    Layout.fillWidth: true
                    horizontalAlignment: Text.AlignHCenter
                    text: qsTr("Version %1 · MIT").arg(Qt.application.version)
                    color: "#4e4e58"
                    font.pixelSize: 9
                }
            }
        }

        ColumnLayout {
            Layout.fillWidth: true
            Layout.fillHeight: true
            spacing: 0

            Rectangle {
                Layout.fillWidth: true
                Layout.fillHeight: true
                color: "#0a0a0d"

                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 280
                    gradient: Gradient {
                        GradientStop { position: 0.0; color: Qt.alpha(theme.accent, 0.035) }
                        GradientStop { position: 1.0; color: "transparent" }
                    }
                }

                StackLayout {
                    anchors.fill: parent
                    currentIndex: win.currentTab

                    MappingPage {}
                    PenPage {}
                    KeysPage {
                        id: keysPage
                        onLearnRequested: function(index, label) {
                            capture.open(index)
                            capture.comboLabel = label
                        }
                    }
                    AboutPage {}
                }
            }

            Rectangle {
                id: footer

                // The About page has nothing to apply, so give it the extra
                // room instead of showing a dead action bar.
                visible: win.currentTab !== 3
                Layout.fillWidth: true
                Layout.preferredHeight: 66
                color: "#0d0d10"

                Rectangle {
                    anchors.top: parent.top
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: 1
                    color: "#202027"
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
                        Layout.maximumWidth: 660
                        text: backend.message
                        color: backend.messageError ? "#ff9b9b" : "#8f8f99"
                        font.pixelSize: 11
                        maximumLineCount: 2
                        elide: Text.ElideRight
                        wrapMode: Text.WordWrap
                    }

                    Item { Layout.fillWidth: true }

                    Text {
                        visible: backend.dirty && win.width >= 1120
                        text: qsTr("Unsaved changes")
                        color: theme.accent
                        font.pixelSize: 10
                    }

                    Text {
                        visible: win.width >= 1080
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
        }
    }

    CaptureOverlay {
        id: capture
        onCaptured: function(action, value) {
            const index = capture.keyIndex
            backend.updateKeyAction(index, action, value)
            backend.saveKeys()
            capture.close()
        }
        onCancelled: capture.close()
    }

    Component.onCompleted: {
        if (initialCapture) {
            win.currentTab = 2
            capture.open(0)
            const rows = backend.keyBindings
            if (rows.length > 0)
                capture.comboLabel = rows[0].label
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
            width: Math.min(520, parent.width - 48)
            implicitHeight: helpColumn.implicitHeight + 48
            anchors.centerIn: parent
            radius: 16
            color: "#151519"
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

                Text {
                    Layout.fillWidth: true
                    text: qsTr("To remap a key, hold its circular learn button for 4 seconds, then press a key or mouse button. Esc cancels capture.")
                    color: theme.accent
                    font.pixelSize: 11
                    lineHeight: 1.35
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

                    Text { text: qsTr("Ctrl+1 … Ctrl+4"); color: "#d0d0d7"; font.pixelSize: 11 }
                    Text { text: qsTr("Switch pages"); color: "#777781"; font.pixelSize: 11 }
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
