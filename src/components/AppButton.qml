import QtQuick
import QtQuick.Controls

Button {
    id: control

    property bool primary: false

    implicitHeight: 36
    implicitWidth: Math.max(84, label.implicitWidth + 32)
    padding: 0
    leftPadding: 0
    rightPadding: 0
    hoverEnabled: true

    background: Rectangle {
        radius: 8
        color: !control.enabled ? "#19191d"
            : control.primary ? (control.down ? Qt.darker(theme.accent, 1.15) : theme.accent)
            : control.hovered ? "#26262c" : "#1c1c21"
        border.width: control.primary ? 0 : 1
        border.color: control.primary ? "transparent" : (control.hovered ? "#3a3a43" : "#2a2a31")
        opacity: control.enabled ? 1 : 0.5
    }

    contentItem: Text {
        id: label
        text: control.text
        color: control.primary ? theme.accentForeground : "#e4e4e9"
        font.pixelSize: 12
        font.weight: Font.Medium
        horizontalAlignment: Text.AlignHCenter
        verticalAlignment: Text.AlignVCenter
        elide: Text.ElideRight
        opacity: control.enabled ? 1 : 0.7
    }
}
