import QtQuick
import QtQuick.Controls
import QtQuick.Layouts

ColumnLayout {
    id: root

    property string label
    property var value: ""
    property string suffix
    property string placeholder
    property real minimum: 0
    property real maximum: 100000
    property int decimals: 1
    signal edited(string value)

    Layout.fillWidth: true
    spacing: 6

    Text {
        text: root.label
        color: "#8b8b95"
        font.pixelSize: 11
    }

    RowLayout {
        Layout.fillWidth: true
        spacing: 6

        TextField {
            id: input

            Layout.fillWidth: true
            implicitHeight: 34
            text: root.value === "" ? "" : String(root.value)
            placeholderText: root.placeholder
            color: "#ececf0"
            placeholderTextColor: "#5d5d67"
            font.pixelSize: 12
            leftPadding: 10
            rightPadding: 10
            selectByMouse: true
            validator: DoubleValidator {
                bottom: root.minimum
                top: root.maximum
                decimals: root.decimals
                notation: DoubleValidator.StandardNotation
            }

            background: Rectangle {
                radius: 7
                color: "#0f0f12"
                border.width: 1
                border.color: input.activeFocus ? theme.accent : "#2a2a31"
            }

            onEditingFinished: {
                root.edited(text)
                text = root.value === "" ? "" : String(root.value)
            }
        }

        Text {
            visible: root.suffix.length > 0
            text: root.suffix
            color: "#696973"
            font.pixelSize: 11
            Layout.alignment: Qt.AlignVCenter
            Layout.preferredWidth: 24
        }
    }

    Connections {
        target: root
        function onValueChanged() {
            if (!input.activeFocus)
                input.text = root.value === "" ? "" : String(root.value)
        }
    }
}
