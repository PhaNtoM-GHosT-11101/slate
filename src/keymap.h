#pragma once

#include <QSet>
#include <QString>
#include <QStringList>
#include <QVector>

#include <optional>

// Keyboard combinations used by the express keys. A Combo is the normalized
// form of something like ctrl+shift+t: modifiers in a fixed order plus one
// non-modifier key. The same type is used for the tablet's factory shortcuts,
// user-entered actions, and the daemon's event matching.
namespace SlateKeys {

struct Combo {
    QStringList modifiers; // ctrl, shift, alt, super
    QString key;           // canonical token, e.g. c, f5, pageup

    bool isValid() const { return !key.isEmpty(); }
    QString toString() const;
};

// Canonical token for an evdev key code, or empty for modifiers and keys the
// app does not remap.
QString keyToken(int evdevCode);
QString modifierForCode(int evdevCode);

// Normalize a user-entered or evdev-derived key token.
QString normalizeKeyToken(const QString &token);
QString normalizeModifierToken(const QString &token);

// Human-readable forms: "Ctrl + C", "F5", "Page Up".
QString displayName(const Combo &combo);
QString displayName(const QString &combo);

// Arguments for wtype that press and release the combo.
QStringList wtypeArgs(const Combo &combo);

// Parse "ctrl+shift+t" and friends. On failure leaves combo untouched and
// fills error with a sentence suitable for the UI.
bool parseCombo(const QString &text, Combo *combo, QString *error = nullptr);

// The 12 factory shortcuts emitted by the VEIKK A15 express keys, in the order
// they were captured from the device.
QVector<Combo> defaultCombos();

// Turns raw evdev key events into combos for the daemon and the key monitor.
// It tracks modifiers and held keys, ignores autorepeat, and emits once when a
// non-modifier key goes down.
class ComboTracker {
public:
    std::optional<Combo> feed(int type, int code, int value);
    void reset();

private:
    QSet<int> m_pressed;
};

} // namespace SlateKeys
