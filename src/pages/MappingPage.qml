import QtQuick
import QtQuick.Controls
import QtQuick.Layouts
import "../components"

Flickable {
    id: page

    contentWidth: width
    contentHeight: column.implicitHeight + 56
    clip: true
    boundsBehavior: Flickable.StopAtBounds
    ScrollBar.vertical: ScrollBar { policy: ScrollBar.AsNeeded }

    property var outputOptions: {
        var options = [
            { value: "current", label: qsTr("Follow cursor"), detail: qsTr("Recommended") },
            { value: "", label: qsTr("All monitors"), detail: qsTr("Across the desktop") }
        ]
        for (var i = 0; i < backend.monitors.length; ++i) {
            var monitor = backend.monitors[i]
            options.push({
                value: monitor.name,
                label: monitor.name,
                detail: monitor.detail + (monitor.focused ? qsTr(" · focused") : "")
            })
        }
        return options
    }

    ColumnLayout {
        id: column

        x: Math.max(28, (page.width - width) / 2)
        y: 26
        width: Math.min(920, page.width - 56)
        spacing: 16

        PageHeader {
            title: qsTr("Screen mapping")
            subtitle: qsTr("Choose where the pen lands and how the tablet matches the screen.")

            AppButton {
                text: qsTr("Reset")
                onClicked: backend.resetMapping()
            }
        }

        Rectangle {
            visible: !backend.hyprlandAvailable
            Layout.fillWidth: true
            implicitHeight: warningRow.implicitHeight + 24
            radius: 10
            color: "#2a2114"
            border.width: 1
            border.color: "#54431f"

            RowLayout {
                id: warningRow
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
                    text: qsTr("Hyprland is not reachable. Mapping changes cannot be applied until it is running.")
                    color: "#d7b978"
                    font.pixelSize: 12
                    wrapMode: Text.WordWrap
                }
            }
        }

        Card {
            title: qsTr("Output")
            subtitle: qsTr("Follow cursor keeps the tablet on whichever monitor you are using. All monitors treats the desktop as one large canvas.")

            Choice {
                options: page.outputOptions
                value: backend.output
                onSelected: function(value) { backend.output = value }
            }
        }

        Card {
            title: qsTr("Orientation")
            subtitle: qsTr("Rotate the tablet's input to match how you hold it. Left-handed rotates the tablet by another 180°.")

            RowLayout {
                Layout.fillWidth: true
                spacing: 28

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.preferredWidth: 320
                    Layout.minimumWidth: 220
                    Layout.maximumWidth: 360
                    spacing: 8

                    Text {
                        text: qsTr("Rotation")
                        color: "#8b8b95"
                        font.pixelSize: 11
                    }

                    Choice {
                        options: [
                            { value: 0, label: "0°" },
                            { value: 1, label: "90°" },
                            { value: 2, label: "180°" },
                            { value: 3, label: "270°" }
                        ]
                        value: backend.transform
                        itemHeight: 38
                        onSelected: function(value) { backend.transform = value }
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.minimumWidth: 220
                    spacing: 3

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 10

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: qsTr("Left-handed")
                                color: "#e4e4e9"
                                font.pixelSize: 12
                            }

                            Text {
                                text: qsTr("Turn the tablet around for a left-handed grip.")
                                Layout.fillWidth: true
                                color: "#71717b"
                                font.pixelSize: 11
                                wrapMode: Text.WordWrap
                            }
                        }

                        Toggle {
                            checked: backend.leftHanded
                            onToggled: function(value) { backend.leftHanded = value }
                        }
                    }
                }
            }
        }

        Card {
            title: qsTr("Active area")
            subtitle: qsTr("Limit the tablet to the part of its surface you actually use. Widths and offsets are in millimetres.")

            RowLayout {
                Layout.fillWidth: true
                spacing: 26

                Item {
                    Layout.preferredWidth: Math.min(360, Math.max(220, page.width * 0.36))
                    Layout.preferredHeight: 244

                    Rectangle {
                        id: tabletBody

                        width: Math.min(parent.width - 16, 360)
                        height: width * 152.4 / 254
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 4
                        radius: 9
                        color: "#0d0d11"
                        border.width: 1
                        border.color: "#383842"

                        Rectangle {
                            id: activeArea

                            x: (backend.activeX / 254) * parent.width
                            y: (backend.activeY / 152.4) * parent.height
                            width: (backend.activeW > 0 ? backend.activeW / 254 : 1) * parent.width
                            height: (backend.activeH > 0 ? backend.activeH / 152.4 : 1) * parent.height
                            radius: 6
                            color: Qt.alpha(theme.accent, backend.activeW > 0 ? 0.14 : 0.05)
                            border.width: 1
                            border.color: theme.accent
                        }

                        Text {
                            anchors.centerIn: activeArea
                            text: backend.activeW > 0
                                ? Math.round(backend.activeW) + " × " + Math.round(backend.activeH) + " mm"
                                : qsTr("Full tablet")
                            color: theme.accent
                            font.pixelSize: 11
                            font.weight: Font.Medium
                        }

                        Text {
                            anchors.left: parent.left
                            anchors.bottom: parent.bottom
                            anchors.margins: 7
                            text: "254 mm"
                            color: "#5d5d67"
                            font.pixelSize: 9
                        }

                        Text {
                            anchors.right: parent.right
                            anchors.top: parent.top
                            anchors.margins: 7
                            text: "152.4 mm"
                            color: "#5d5d67"
                            font.pixelSize: 9
                        }
                    }

                    Text {
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.bottom: parent.bottom
                        text: backend.activeW > 0
                            ? qsTr("Offset %1, %2 mm from the top-left").arg(Math.round(backend.activeX)).arg(Math.round(backend.activeY))
                            : qsTr("The whole 254 × 152.4 mm surface")
                        color: "#6c6c76"
                        font.pixelSize: 10
                    }
                }

                ColumnLayout {
                    Layout.fillWidth: true
                    Layout.alignment: Qt.AlignTop
                    spacing: 12

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        NumberField {
                            label: qsTr("Width")
                            suffix: "mm"
                            value: backend.activeW > 0 ? backend.activeW : ""
                            placeholder: qsTr("Full")
                            minimum: 0
                            maximum: 254
                            decimals: 1
                            onEdited: function(value) { backend.activeW = isNaN(parseFloat(value)) ? 0 : parseFloat(value) }
                        }

                        NumberField {
                            label: qsTr("Height")
                            suffix: "mm"
                            value: backend.activeH > 0 ? backend.activeH : ""
                            placeholder: qsTr("Full")
                            minimum: 0
                            maximum: 152.4
                            decimals: 1
                            onEdited: function(value) { backend.activeH = isNaN(parseFloat(value)) ? 0 : parseFloat(value) }
                        }
                    }

                    RowLayout {
                        Layout.fillWidth: true
                        spacing: 12

                        NumberField {
                            label: qsTr("Offset X")
                            suffix: "mm"
                            value: backend.activeX
                            minimum: 0
                            maximum: 254
                            decimals: 1
                            onEdited: function(value) { backend.activeX = isNaN(parseFloat(value)) ? 0 : parseFloat(value) }
                        }

                        NumberField {
                            label: qsTr("Offset Y")
                            suffix: "mm"
                            value: backend.activeY
                            minimum: 0
                            maximum: 152.4
                            decimals: 1
                            onEdited: function(value) { backend.activeY = isNaN(parseFloat(value)) ? 0 : parseFloat(value) }
                        }
                    }

                    AppButton {
                        text: qsTr("Use full tablet")
                        implicitWidth: 134
                        onClicked: backend.resetActiveArea()
                    }

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Leave width and height empty for the full surface. Offsets place the active area from the top-left corner.")
                        color: "#6c6c76"
                        font.pixelSize: 10
                        wrapMode: Text.WordWrap
                    }
                }
            }
        }

        Card {
            title: qsTr("Screen region")
            subtitle: qsTr("Map the tablet to a rectangle of the screen instead of the whole display. This is optional.")

            RowLayout {
                Layout.fillWidth: true
                spacing: 14

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        text: qsTr("Use a screen region")
                        color: "#e4e4e9"
                        font.pixelSize: 12
                    }

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Useful when the tablet should cover only a drawing canvas or a game window.")
                        color: "#71717b"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }

                Toggle {
                    checked: backend.regionEnabled
                    onToggled: function(value) { backend.enableRegion(value) }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12
                visible: backend.regionEnabled

                NumberField {
                    label: qsTr("Region width")
                    suffix: "px"
                    value: backend.regionW
                    minimum: 0
                    maximum: 100000
                    decimals: 0
                    onEdited: function(value) { backend.regionW = isNaN(parseFloat(value)) ? 0 : parseFloat(value) }
                }

                NumberField {
                    label: qsTr("Region height")
                    suffix: "px"
                    value: backend.regionH
                    minimum: 0
                    maximum: 100000
                    decimals: 0
                    onEdited: function(value) { backend.regionH = isNaN(parseFloat(value)) ? 0 : parseFloat(value) }
                }

                NumberField {
                    label: qsTr("Position X")
                    suffix: "px"
                    value: backend.regionX
                    minimum: -100000
                    maximum: 100000
                    decimals: 0
                    onEdited: function(value) { backend.regionX = isNaN(parseFloat(value)) ? 0 : parseFloat(value) }
                }

                NumberField {
                    label: qsTr("Position Y")
                    suffix: "px"
                    value: backend.regionY
                    minimum: -100000
                    maximum: 100000
                    decimals: 0
                    onEdited: function(value) { backend.regionY = isNaN(parseFloat(value)) ? 0 : parseFloat(value) }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 14
                visible: backend.regionEnabled && backend.output === ""

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        text: qsTr("Absolute position")
                        color: "#e4e4e9"
                        font.pixelSize: 12
                    }

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Treat the position as a point in the whole desktop layout rather than on one monitor.")
                        color: "#71717b"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }

                Toggle {
                    checked: backend.absoluteRegion
                    onToggled: function(value) { backend.absoluteRegion = value }
                }
            }
        }

        Item { Layout.preferredHeight: 2 }
    }
}
