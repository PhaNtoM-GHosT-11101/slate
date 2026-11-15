#pragma once

#include <QString>

// The subset of Hyprland input settings slate manages. It is persisted as a
// marked Lua block in ~/.config/hypr/input.lua so it lives with the rest of
// the user's Hyprland configuration and survives restarts.
struct TabletSettings {
    QString output = QStringLiteral("current");
    int transform = 0;
    bool leftHanded = false;

    // Physical active area in millimetres. Zero size means the full tablet.
    double activeX = 0;
    double activeY = 0;
    double activeW = 0;
    double activeH = 0;

    // Mapped screen region in pixels. Zero size means the whole output.
    double regionX = 0;
    double regionY = 0;
    double regionW = 0;
    double regionH = 0;
    bool absoluteRegionPosition = false;

    // -1 means the device default.
    double pressureMin = -1.0;
    double pressureMax = -1.0;

    bool operator==(const TabletSettings &other) const;
    bool operator!=(const TabletSettings &other) const { return !(*this == other); }
};

namespace SlateSettings {

extern const QString kBeginMarker;
extern const QString kEndMarker;

QString block(const TabletSettings &settings);
bool parse(const QString &lua, TabletSettings *settings);
QString upsert(const QString &lua, const TabletSettings &settings);
QString remove(const QString &lua);

} // namespace SlateSettings
