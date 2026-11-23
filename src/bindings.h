#pragma once

#include <QString>
#include <QStringList>
#include <QVector>

struct KeyBinding {
    QString combo;
    QString action = QStringLiteral("pass"); // pass, keys, mouse, command, nothing
    QString value;

    bool operator==(const KeyBinding &other) const
    {
        return combo == other.combo && action == other.action && value == other.value;
    }
};

// Express-key bindings stored in ~/.config/slate/keys.json. The daemon reads
// the file for every press, so edits take effect immediately.
namespace SlateBindings {

QString configDirectory();
QString keysPath();
QString statusPath();

QVector<KeyBinding> defaults();
QVector<KeyBinding> load(const QString &path, QString *error = nullptr);
bool save(const QString &path, const QVector<KeyBinding> &bindings, QString *error = nullptr);

bool isValidAction(const QString &action);

// Mouse actions understood by the helper: left, right, middle, back, forward,
// wheelup, wheeldown, wheelleft, wheelright.
QStringList mouseActions();
bool isValidMouseAction(const QString &value);
QString mouseActionLabel(const QString &value);

} // namespace SlateBindings
