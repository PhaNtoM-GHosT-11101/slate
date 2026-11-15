import QtQuick
import QtQuick.Layouts

// A segmented control. options is an array of { value, label, detail }.
Item {
    id: root

    property var options: []
    property var value: undefined
    property int itemHeight: 46
    signal selected(var value)

    implicitHeight: itemHeight
    Layout.fillWidth: true

    Rectangle {
        anchors.fill: parent
        radius: 9
        color: "#101013"
        border.width: 1
        border.color: "#27272e"

        RowLayout {
            anchors.fill: parent
            anchors.margins: 3
            spacing: 3

            Repeater {
                model: root.options

                delegate: Rectangle {
                    id: option

                    required property var modelData
                    property bool chosen: modelData.value === root.value

                    Layout.fillWidth: true
                    Layout.fillHeight: true
                    radius: 6
                    color: chosen ? theme.accent : (hover.hovered ? "#1d1d22" : "transparent")

                    Behavior on color { ColorAnimation { duration: 100 } }

                    Column {
                        anchors.centerIn: parent
                        spacing: 1

                        Text {
                            text: option.modelData.label
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: option.chosen ? theme.accentForeground : "#d2d2d9"
                            font.pixelSize: 12
                            font.weight: option.chosen ? Font.DemiBold : Font.Normal
                        }

                        Text {
                            visible: !!option.modelData.detail
                            text: option.modelData.detail || ""
                            anchors.horizontalCenter: parent.horizontalCenter
                            color: option.chosen ? theme.accentForeground : "#70707a"
                            font.pixelSize: 10
                            opacity: option.chosen ? 0.68 : 1
                        }
                    }

                    HoverHandler { id: hover }

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: root.selected(option.modelData.value)
                    }
                }
            }
        }
    }
}
