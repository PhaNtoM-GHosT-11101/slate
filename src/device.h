#pragma once

#include <QString>

// Finds the tablet's event nodes through sysfs. The A15 exposes three input
// devices; the pen is "VEIKK.INC A15" and the express keys are
// "VEIKK.INC A15 Keyboard".
namespace SlateDevice {

QString findEventNode(const QString &deviceName);
QString penEventNode();
QString keysEventNode();
bool isConnected();

} // namespace SlateDevice
