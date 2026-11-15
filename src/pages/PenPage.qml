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

    ColumnLayout {
        id: column

        x: Math.max(28, (page.width - width) / 2)
        y: 26
        width: Math.min(920, page.width - 56)
        spacing: 16

        PageHeader {
            title: qsTr("Pen")
            subtitle: qsTr("Tune the pen's pressure range and test its input live.")
        }

        Card {
            title: qsTr("Pressure range")
            subtitle: qsTr("Leave this off to use the tablet's native pressure range. A custom range can make the pen feel softer or firmer.")

            RowLayout {
                Layout.fillWidth: true
                spacing: 14

                ColumnLayout {
                    Layout.fillWidth: true
                    spacing: 2

                    Text {
                        text: qsTr("Custom pressure range")
                        color: "#e4e4e9"
                        font.pixelSize: 12
                    }

                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Set the pressure levels that count as the lightest and heaviest strokes.")
                        color: "#71717b"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }

                Toggle {
                    checked: backend.pressureCustom
                    onToggled: function(value) { backend.setPressureCustom(value) }
                }
            }

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 12
                visible: backend.pressureCustom

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Text {
                        text: qsTr("Minimum")
                        color: "#9a9aa4"
                        font.pixelSize: 11
                        Layout.preferredWidth: 64
                    }

                    Slider {
                        id: minSlider

                        Layout.fillWidth: true
                        from: 0.0
                        to: 1.0
                        stepSize: 0.01
                        value: backend.pressureMin
                        onMoved: function(value) { backend.pressureMin = value }

                        background: Rectangle {
                            x: minSlider.leftPadding
                            y: minSlider.topPadding + minSlider.availableHeight / 2 - height / 2
                            width: minSlider.availableWidth
                            height: 4
                            radius: 2
                            color: "#2a2a31"

                            Rectangle {
                                width: minSlider.visualPosition * parent.width
                                height: parent.height
                                radius: 2
                                color: theme.accent
                            }
                        }

                        handle: Rectangle {
                            x: minSlider.leftPadding + minSlider.visualPosition * (minSlider.availableWidth - width)
                            y: minSlider.topPadding + minSlider.availableHeight / 2 - height / 2
                            width: 16
                            height: 16
                            radius: 8
                            color: theme.accent
                            border.width: 2
                            border.color: "#0b0b0d"
                        }
                    }

                    Text {
                        text: Math.round(backend.pressureMin * 100) + "%"
                        color: "#d8d8de"
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignRight
                        Layout.preferredWidth: 42
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 12

                    Text {
                        text: qsTr("Maximum")
                        color: "#9a9aa4"
                        font.pixelSize: 11
                        Layout.preferredWidth: 64
                    }

                    Slider {
                        id: maxSlider

                        Layout.fillWidth: true
                        from: 0.0
                        to: 1.0
                        stepSize: 0.01
                        value: backend.pressureMax
                        onMoved: function(value) { backend.pressureMax = value }

                        background: Rectangle {
                            x: maxSlider.leftPadding
                            y: maxSlider.topPadding + maxSlider.availableHeight / 2 - height / 2
                            width: maxSlider.availableWidth
                            height: 4
                            radius: 2
                            color: "#2a2a31"

                            Rectangle {
                                width: maxSlider.visualPosition * parent.width
                                height: parent.height
                                radius: 2
                                color: theme.accent
                            }
                        }

                        handle: Rectangle {
                            x: maxSlider.leftPadding + maxSlider.visualPosition * (maxSlider.availableWidth - width)
                            y: maxSlider.topPadding + maxSlider.availableHeight / 2 - height / 2
                            width: 16
                            height: 16
                            radius: 8
                            color: theme.accent
                            border.width: 2
                            border.color: "#0b0b0d"
                        }
                    }

                    Text {
                        text: Math.round(backend.pressureMax * 100) + "%"
                        color: "#d8d8de"
                        font.pixelSize: 11
                        horizontalAlignment: Text.AlignRight
                        Layout.preferredWidth: 42
                    }
                }
            }

            Text {
                Layout.fillWidth: true
                visible: !backend.pressureCustom
                text: qsTr("Using the pen's native range (0–100%).")
                color: "#6c6c76"
                font.pixelSize: 11
            }
        }

        Card {
            title: qsTr("Live pen test")
            subtitle: qsTr("Hover and draw in the pad below. It reads the pen interface directly, so it works even when the app is not focused.")

            PenPad {
                Layout.fillWidth: true
                Layout.preferredHeight: 320
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 12

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 96
                    radius: 9
                    color: "#101013"
                    border.width: 1
                    border.color: "#26262c"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8

                        Text {
                            text: qsTr("Pressure")
                            color: "#84848e"
                            font.pixelSize: 10
                            font.letterSpacing: 0.8
                        }

                        Text {
                            text: Math.round(pen.pressure * 100) + "%"
                            color: "#f0f0f3"
                            font.pixelSize: 22
                            font.weight: Font.DemiBold
                        }

                        Rectangle {
                            Layout.fillWidth: true
                            height: 5
                            radius: 2.5
                            color: "#2a2a31"

                            Rectangle {
                                width: parent.width * pen.pressure
                                height: parent.height
                                radius: 2.5
                                color: theme.accent
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 96
                    radius: 9
                    color: "#101013"
                    border.width: 1
                    border.color: "#26262c"

                    RowLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 12

                        Rectangle {
                            width: 46
                            height: 46
                            radius: 23
                            color: "#0b0b0e"
                            border.width: 1
                            border.color: "#2d2d35"

                            Rectangle {
                                width: 5
                                height: 5
                                radius: 2.5
                                color: "#4a4a55"

                                anchors.horizontalCenter: parent.horizontalCenter
                                anchors.verticalCenter: parent.verticalCenter
                            }

                            Rectangle {
                                x: 18 + (pen.tiltX / 127) * 16
                                y: 18 + (pen.tiltY / 127) * 16
                                width: 10
                                height: 10
                                radius: 5
                                color: theme.accent
                            }
                        }

                        ColumnLayout {
                            Layout.fillWidth: true
                            spacing: 2

                            Text {
                                text: qsTr("Tilt")
                                color: "#84848e"
                                font.pixelSize: 10
                                font.letterSpacing: 0.8
                            }

                            Text {
                                text: "X " + Math.round(pen.tiltX) + "  ·  Y " + Math.round(pen.tiltY)
                                color: "#d8d8de"
                                font.pixelSize: 12
                            }

                            Text {
                                text: pen.eraser ? qsTr("Eraser") : (pen.touching ? qsTr("Touching") : (pen.hovering ? qsTr("Hovering") : qsTr("In range")))
                                color: "#6c6c76"
                                font.pixelSize: 10
                            }
                        }
                    }
                }

                Rectangle {
                    Layout.fillWidth: true
                    Layout.preferredHeight: 96
                    radius: 9
                    color: "#101013"
                    border.width: 1
                    border.color: "#26262c"

                    ColumnLayout {
                        anchors.fill: parent
                        anchors.margins: 12
                        spacing: 8

                        Text {
                            text: qsTr("Pen buttons")
                            color: "#84848e"
                            font.pixelSize: 10
                            font.letterSpacing: 0.8
                        }

                        RowLayout {
                            spacing: 6

                            Rectangle {
                                height: 24
                                width: 72
                                radius: 12
                                color: pen.barrel ? theme.accent : "#1b1b21"
                                border.width: 1
                                border.color: pen.barrel ? theme.accent : "#303038"

                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("Barrel")
                                    color: pen.barrel ? theme.accentForeground : "#9a9aa4"
                                    font.pixelSize: 10
                                }
                            }

                            Rectangle {
                                height: 24
                                width: 72
                                radius: 12
                                color: pen.secondary ? theme.accent : "#1b1b21"
                                border.width: 1
                                border.color: pen.secondary ? theme.accent : "#303038"

                                Text {
                                    anchors.centerIn: parent
                                    text: qsTr("Secondary")
                                    color: pen.secondary ? theme.accentForeground : "#9a9aa4"
                                    font.pixelSize: 10
                                }
                            }
                        }

                        Text {
                            text: pen.eraser ? qsTr("Eraser end active") : qsTr("Press a side button")
                            color: "#6c6c76"
                            font.pixelSize: 10
                        }
                    }
                }
            }
        }

        Item { Layout.preferredHeight: 2 }
    }

    Connections {
        target: backend

        function onPressureChanged() {
            if (!minSlider.pressed)
                minSlider.value = backend.pressureMin
            if (!maxSlider.pressed)
                maxSlider.value = backend.pressureMax
        }
    }
}
