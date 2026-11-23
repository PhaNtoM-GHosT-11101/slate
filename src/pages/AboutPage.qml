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

    function openLink(url) {
        Qt.openUrlExternally(url)
    }

    ColumnLayout {
        id: column

        x: Math.max(28, (page.width - width) / 2)
        y: 26
        width: Math.min(920, page.width - 56)
        spacing: 16

        PageHeader {
            title: qsTr("About Slate")
            subtitle: qsTr("A focused settings app for the VEIKK A15, built with Qt Quick.")
        }

        Card {
            title: qsTr("Device")
            subtitle: qsTr("Slate finds the A15 by name, so event numbers can change freely.")

            GridLayout {
                Layout.fillWidth: true
                columns: 2
                columnSpacing: 32
                rowSpacing: 10

                Text { text: qsTr("Tablet"); color: "#86868f"; font.pixelSize: 11 }
                Text { text: backend.tabletName; color: "#e4e4e9"; font.pixelSize: 12 }

                Text { text: qsTr("Status"); color: "#86868f"; font.pixelSize: 11 }
                RowLayout {
                    spacing: 7
                    Rectangle {
                        width: 7
                        height: 7
                        radius: 3.5
                        color: backend.tabletConnected ? "#5ed38b" : "#e0a94a"
                    }
                    Text {
                        text: backend.tabletConnected ? qsTr("Connected") : qsTr("Not detected")
                        color: backend.tabletConnected ? "#bfe9cf" : "#d7b978"
                        font.pixelSize: 12
                    }
                }

                Text { text: qsTr("Active area"); color: "#86868f"; font.pixelSize: 11 }
                Text { text: backend.tabletSize; color: "#e4e4e9"; font.pixelSize: 12 }

                Text { text: qsTr("USB ID"); color: "#86868f"; font.pixelSize: 11 }
                Text { text: "2feb:0004"; color: "#e4e4e9"; font.pixelSize: 12 }

                Text { text: qsTr("Helper"); color: "#86868f"; font.pixelSize: 11 }
                Text { text: backend.serviceState; color: "#e4e4e9"; font.pixelSize: 12 }

                Text { text: qsTr("Version"); color: "#86868f"; font.pixelSize: 11 }
                Text { text: Qt.application.version; color: "#e4e4e9"; font.pixelSize: 12 }
            }
        }

        Card {
            title: qsTr("Open source")
            subtitle: qsTr("Slate is MIT licensed. Bug reports, device notes, and pull requests are welcome.")

            RowLayout {
                Layout.fillWidth: true
                spacing: 8

                AppButton {
                    text: qsTr("GitHub repository")
                    onClicked: page.openLink("https://github.com/PhaNtoM-GHosT-11101/slate")
                }

                AppButton {
                    text: qsTr("Report an issue")
                    onClicked: page.openLink("https://github.com/PhaNtoM-GHosT-11101/slate/issues")
                }

                AppButton {
                    text: qsTr("Contributing")
                    onClicked: page.openLink("https://github.com/PhaNtoM-GHosT-11101/slate/blob/main/CONTRIBUTING.md")
                }

                AppButton {
                    text: qsTr("Install guide")
                    onClicked: page.openLink("https://github.com/PhaNtoM-GHosT-11101/slate/blob/main/docs/installation.md")
                }

                AppButton {
                    text: qsTr("Security")
                    onClicked: page.openLink("https://github.com/PhaNtoM-GHosT-11101/slate/blob/main/SECURITY.md")
                }

                Item { Layout.fillWidth: true }
            }

            Text {
                Layout.fillWidth: true
                text: qsTr("If you find Slate useful, star the repository or share a device report. The project is designed to be forked: the UI, evdev helper, and packaging are all in this tree.")
                color: "#75757f"
                font.pixelSize: 11
                wrapMode: Text.WordWrap
            }
        }

        Card {
            title: qsTr("Platform support")
            subtitle: qsTr("Slate is a Linux-first app. The tablet mapping layer is tailored to Hyprland/Omarchy; express-key remapping works on other Linux desktops.")

            ColumnLayout {
                Layout.fillWidth: true
                spacing: 10

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Text { text: "Linux"; color: "#e4e4e9"; font.pixelSize: 12; Layout.preferredWidth: 92 }
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Full support: mapping, pen test, and express keys. Wayland uses wtype; X11 uses xdotool.")
                        color: "#8c8c96"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Text { text: "Windows"; color: "#e4e4e9"; font.pixelSize: 12; Layout.preferredWidth: 92 }
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("Use the official VEIKK driver or OpenTabletDriver. A native Windows filter driver needs a signed kernel component; see docs/platforms.md.")
                        color: "#8c8c96"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }

                RowLayout {
                    Layout.fillWidth: true
                    spacing: 10
                    Text { text: "macOS"; color: "#e4e4e9"; font.pixelSize: 12; Layout.preferredWidth: 92 }
                    Text {
                        Layout.fillWidth: true
                        text: qsTr("OpenTabletDriver is the recommended open-source option. Slate's helper is Linux-specific today.")
                        color: "#8c8c96"
                        font.pixelSize: 11
                        wrapMode: Text.WordWrap
                    }
                }
            }

            RowLayout {
                Layout.fillWidth: true
                spacing: 8
                AppButton {
                    text: qsTr("OpenTabletDriver")
                    onClicked: page.openLink("https://opentabletdriver.net/Tablets")
                }
                AppButton {
                    text: qsTr("Platform notes")
                    onClicked: page.openLink("https://github.com/PhaNtoM-GHosT-11101/slate/blob/main/docs/platforms.md")
                }
                Item { Layout.fillWidth: true }
            }
        }

        Card {
            title: qsTr("Credits")
            subtitle: qsTr("Slate stands on the shoulders of these projects.")

            Text {
                Layout.fillWidth: true
                text: qsTr("Qt 6 and Qt Quick · Omarchy's theme conventions · OpenTabletDriver for cross-platform tablet support · the Linux input and udev communities · VEIKK for the A15 hardware.")
                color: "#7e7e88"
                font.pixelSize: 11
                lineHeight: 1.4
                wrapMode: Text.WordWrap
            }
        }

        Item { Layout.preferredHeight: 2 }
    }
}
