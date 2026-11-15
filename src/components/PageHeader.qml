import QtQuick
import QtQuick.Layouts

RowLayout {
    id: root

    property string title
    property string subtitle
    default property alias actions: actionsRow.data

    Layout.fillWidth: true
    spacing: 16

    ColumnLayout {
        Layout.fillWidth: true
        spacing: 4

        Text {
            text: root.title
            color: "#f3f3f6"
            font.pixelSize: 25
            font.weight: Font.DemiBold
        }

        Text {
            text: root.subtitle
            Layout.fillWidth: true
            color: "#86868f"
            font.pixelSize: 12
            lineHeight: 1.3
            wrapMode: Text.WordWrap
        }
    }

    RowLayout {
        id: actionsRow
        spacing: 8
        Layout.alignment: Qt.AlignTop
    }
}
