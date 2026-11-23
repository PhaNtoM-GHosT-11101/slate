import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

// A checkable row inside the mouse-action chooser. Keeping it in its own file
// avoids repeating the same styling for every button and scroll direction.
MenuItem {
    id: root

    property string actionValue: ""
    property bool selected: false

    implicitHeight: 32
    leftPadding: 8
    rightPadding: 8

    contentItem: RowLayout {
        spacing: 8

        Text {
            text: root.selected ? "✓" : " "
            color: theme.accent
            font.pixelSize: 11
            Layout.preferredWidth: 12
        }

        Text {
            Layout.fillWidth: true
            text: root.text
            color: root.enabled ? "#e2e2e8" : "#64646e"
            font.pixelSize: 11
        }
    }

    background: Rectangle {
        radius: 6
        color: root.hovered ? "#24242c" : "transparent"
    }
}
