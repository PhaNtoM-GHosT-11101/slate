import QtQuick
import QtQuick.Controls

// A deliberate four-second press. A short click does nothing, which avoids
// accidental rebinds while still keeping the gesture discoverable.
Item {
    id: root

    property string label: qsTr("Hold 4s")
    property int holdMillis: 4000
    property real progress: 0
    property color accentColor: (typeof theme !== "undefined" && theme && theme.accent)
        ? theme.accent : "#FFD60A"
    signal triggered()

    implicitWidth: 52
    implicitHeight: 52
    activeFocusOnTab: true
    Accessible.name: label

    function begin() {
        if (holdTimer.running)
            return
        root.progress = 0
        progressAnim.restart()
        holdTimer.restart()
    }

    function cancel() {
        holdTimer.stop()
        progressAnim.stop()
        root.progress = 0
    }

    Timer {
        id: holdTimer
        interval: root.holdMillis
        onTriggered: {
            root.progress = 1
            root.triggered()
            root.cancel()
        }
    }

    NumberAnimation {
        id: progressAnim
        target: root
        property: "progress"
        from: 0
        to: 1
        duration: root.holdMillis
        easing.type: Easing.Linear
    }

    Keys.onSpacePressed: root.triggered()
    Keys.onReturnPressed: root.triggered()

    Canvas {
        id: ring

        anchors.fill: parent
        antialiasing: true
        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()
            var cx = width / 2
            var cy = height / 2
            var r = Math.min(width, height) / 2 - 2

            ctx.beginPath()
            ctx.arc(cx, cy, r, 0, Math.PI * 2)
            ctx.lineWidth = 1
            ctx.strokeStyle = hover.hovered ? "#4a4a55" : "#2d2d35"
            ctx.stroke()

            ctx.beginPath()
            ctx.arc(cx, cy, r, 0, Math.PI * 2)
            ctx.fillStyle = "#17171c"
            ctx.fill()

            if (root.progress > 0) {
                ctx.beginPath()
                ctx.arc(cx, cy, r, -Math.PI / 2, -Math.PI / 2 + Math.PI * 2 * root.progress)
                ctx.lineWidth = 3
                ctx.lineCap = "round"
                ctx.strokeStyle = root.accentColor
                ctx.stroke()
            }
        }

        Connections {
            target: hover
            function onHoveredChanged() { ring.requestPaint() }
        }

        Connections {
            target: root
            function onProgressChanged() { ring.requestPaint() }
        }
    }

    Text {
        anchors.centerIn: parent
        text: root.progress > 0 ? Math.round(root.progress * 100) + "%" : "4s"
        color: root.progress > 0 ? root.accentColor : (hover.hovered ? "#d4d4dc" : "#9a9aa4")
        font.pixelSize: root.progress > 0 ? 10 : 11
        font.weight: Font.DemiBold
    }

    HoverHandler { id: hover }

    MouseArea {
        id: area
        anchors.fill: parent
        cursorShape: Qt.PointingHandCursor
        onPressed: root.begin()
        onReleased: root.cancel()
        onCanceled: root.cancel()
    }

    ToolTip.visible: hover.hovered && root.progress === 0
    ToolTip.text: root.label
    ToolTip.delay: 500
}
