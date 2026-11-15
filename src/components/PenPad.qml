import QtQuick

Rectangle {
    id: root

    property var trail: []

    color: "#0c0c0f"
    radius: 10
    border.width: 1
    border.color: "#26262c"
    clip: true

    onWidthChanged: canvas.requestPaint()
    onHeightChanged: canvas.requestPaint()

    Connections {
        target: pen

        function onFrame() {
            if (pen.touching) {
                var points = root.trail.slice(-220)
                points.push({ x: pen.x, y: pen.y, p: pen.pressure })
                root.trail = points
            } else if (!pen.hovering && root.trail.length > 0) {
                root.trail = []
            }
            canvas.requestPaint()
        }
    }

    Canvas {
        id: canvas
        anchors.fill: parent
        antialiasing: true

        onPaint: {
            var ctx = getContext("2d")
            ctx.reset()

            // Dot grid, so the pad reads as a surface rather than an empty box.
            ctx.fillStyle = "#1b1b22"
            for (var gx = 18; gx < width; gx += 24) {
                for (var gy = 18; gy < height; gy += 24) {
                    ctx.beginPath()
                    ctx.arc(gx, gy, 1, 0, Math.PI * 2)
                    ctx.fill()
                }
            }

            // The tablet's full active area is the pad; draw a faint border.
            ctx.strokeStyle = "#24242b"
            ctx.lineWidth = 1
            ctx.strokeRect(0.5, 0.5, width - 1, height - 1)

            // Pressure-scaled trail. Each segment keeps its own width so the
            // line visibly thickens and thins with the pen.
            for (var i = 1; i < root.trail.length; ++i) {
                var a = root.trail[i - 1]
                var b = root.trail[i]
                ctx.beginPath()
                ctx.moveTo(a.x * width, a.y * height)
                ctx.lineTo(b.x * width, b.y * height)
                ctx.lineWidth = 0.8 + ((a.p + b.p) * 0.5) * 5.5
                ctx.strokeStyle = theme.accent
                ctx.lineCap = "round"
                ctx.globalAlpha = 0.85
                ctx.stroke()
            }
            ctx.globalAlpha = 1

            if (pen.hovering) {
                var px = pen.x * width
                var py = pen.y * height
                var radius = 3 + pen.pressure * 10

                ctx.beginPath()
                ctx.arc(px, py, radius + 4, 0, Math.PI * 2)
                ctx.strokeStyle = theme.accent
                ctx.globalAlpha = 0.35
                ctx.lineWidth = 1
                ctx.stroke()

                ctx.beginPath()
                ctx.arc(px, py, radius, 0, Math.PI * 2)
                ctx.fillStyle = theme.accent
                ctx.globalAlpha = pen.touching ? 0.9 : 0.55
                ctx.fill()
                ctx.globalAlpha = 1
            }
        }
    }

    Rectangle {
        anchors.left: parent.left
        anchors.top: parent.top
        anchors.margins: 12
        height: 22
        width: statusRow.implicitWidth + 16
        radius: 11
        color: "#111116"
        border.width: 1
        border.color: "#27272e"

        Row {
            id: statusRow
            anchors.centerIn: parent
            spacing: 6

            Rectangle {
                anchors.verticalCenter: parent.verticalCenter
                width: 6
                height: 6
                radius: 3
                color: pen.available ? theme.accent : "#e0a94a"
            }

            Text {
                anchors.verticalCenter: parent.verticalCenter
                text: pen.available ? qsTr("LIVE PEN INPUT") : qsTr("PEN OFFLINE")
                color: pen.available ? "#84848e" : "#c8a45c"
                font.pixelSize: 9
                font.letterSpacing: 1.1
            }
        }
    }

    Text {
        visible: pen.hovering
        x: Math.min(parent.width - width - 12, Math.max(12, pen.x * parent.width + 18))
        y: Math.min(parent.height - height - 12, Math.max(12, pen.y * parent.height + 18))
        text: Math.round(pen.pressure * 100) + "%"
        color: theme.accent
        font.pixelSize: 11
        font.weight: Font.DemiBold
    }

    Column {
        visible: !pen.available
        anchors.centerIn: parent
        width: Math.min(parent.width - 48, 460)
        spacing: 8

        Text {
            anchors.horizontalCenter: parent.horizontalCenter
            text: qsTr("Pen input unavailable")
            color: "#c5c5ce"
            font.pixelSize: 14
            font.weight: Font.DemiBold
        }

        Text {
            width: parent.width
            horizontalAlignment: Text.AlignHCenter
            text: pen.error
            color: "#7c7c86"
            font.pixelSize: 11
            wrapMode: Text.WordWrap
        }
    }

    Behavior on color { ColorAnimation { duration: 120 } }
}
