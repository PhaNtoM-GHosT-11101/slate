import QtQuick
import QtQuick.Layouts

Rectangle {
    id: root

    default property alias content: body.data
    property string title
    property string subtitle
    property int padding: 20

    Layout.fillWidth: true
    implicitHeight: layout.implicitHeight + padding * 2
    color: "#151518"
    radius: theme.radius > 0 ? Math.min(theme.radius + 4, 14) : 10
    border.width: 1
    border.color: "#25252b"

    ColumnLayout {
        id: layout
        anchors.fill: parent
        anchors.margins: root.padding
        spacing: 16

        ColumnLayout {
            Layout.fillWidth: true
            spacing: 4
            visible: root.title.length > 0 || root.subtitle.length > 0

            Text {
                text: root.title
                visible: root.title.length > 0
                color: "#f0f0f3"
                font.pixelSize: 15
                font.weight: Font.DemiBold
            }

            Text {
                text: root.subtitle
                visible: root.subtitle.length > 0
                Layout.fillWidth: true
                color: "#8b8b95"
                font.pixelSize: 12
                lineHeight: 1.3
                wrapMode: Text.WordWrap
            }
        }

        ColumnLayout {
            id: body
            Layout.fillWidth: true
            spacing: 14
        }
    }
}
