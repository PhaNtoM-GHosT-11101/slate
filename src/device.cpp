#include "device.h"

#include <QDir>
#include <QFile>

namespace SlateDevice {

QString findEventNode(const QString &deviceName)
{
    const QDir input(QStringLiteral("/sys/class/input"));
    const QStringList entries = input.entryList({QStringLiteral("event*")}, QDir::Dirs | QDir::NoDotAndDotDot,
                                                QDir::Name);
    for (const QString &entry : entries) {
        QFile nameFile(input.filePath(entry + QStringLiteral("/device/name")));
        if (!nameFile.open(QIODevice::ReadOnly))
            continue;
        if (QString::fromUtf8(nameFile.readAll()).trimmed() == deviceName)
            return QStringLiteral("/dev/input/") + entry;
    }
    return {};
}

QString penEventNode()
{
    return findEventNode(QStringLiteral("VEIKK.INC A15"));
}

QString keysEventNode()
{
    return findEventNode(QStringLiteral("VEIKK.INC A15 Keyboard"));
}

bool isConnected()
{
    return !penEventNode().isEmpty() || !keysEventNode().isEmpty();
}

} // namespace SlateDevice
