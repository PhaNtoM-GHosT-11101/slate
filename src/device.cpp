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

namespace {

QString findA15Node(bool keyboard)
{
    const QDir input(QStringLiteral("/sys/class/input"));
    const QStringList entries = input.entryList({QStringLiteral("event*")}, QDir::Dirs | QDir::NoDotAndDotDot,
                                                QDir::Name);
    for (const QString &entry : entries) {
        QFile nameFile(input.filePath(entry + QStringLiteral("/device/name")));
        if (!nameFile.open(QIODevice::ReadOnly))
            continue;
        const QString name = QString::fromUtf8(nameFile.readAll()).trimmed();
        if (!name.contains(QStringLiteral("VEIKK"), Qt::CaseInsensitive)
            || !name.contains(QStringLiteral("A15"), Qt::CaseInsensitive))
            continue;
        const bool isKeyboard = name.contains(QStringLiteral("Keyboard"), Qt::CaseInsensitive);
        if (isKeyboard == keyboard)
            return QStringLiteral("/dev/input/") + entry;
    }
    return {};
}

} // namespace

QString penEventNode()
{
    const QString exact = findEventNode(QStringLiteral("VEIKK.INC A15"));
    return exact.isEmpty() ? findA15Node(false) : exact;
}

QString keysEventNode()
{
    const QString exact = findEventNode(QStringLiteral("VEIKK.INC A15 Keyboard"));
    return exact.isEmpty() ? findA15Node(true) : exact;
}

bool isConnected()
{
    return !penEventNode().isEmpty() || !keysEventNode().isEmpty();
}

} // namespace SlateDevice
