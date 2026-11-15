#include "bindings.h"

#include "keymap.h"

#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QHash>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPair>
#include <QSaveFile>
#include <QSet>
#include <QStandardPaths>

namespace SlateBindings {
namespace {

const QSet<QString> &actions()
{
    static const QSet<QString> values{
        QStringLiteral("pass"),
        QStringLiteral("keys"),
        QStringLiteral("command"),
        QStringLiteral("nothing"),
    };
    return values;
}

QString describe(const QString &path)
{
    return QDir::toNativeSeparators(path);
}

} // namespace

QString configDirectory()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    return (base.isEmpty() ? QDir::homePath() + QStringLiteral("/.config") : base)
        + QStringLiteral("/slate");
}

QString keysPath()
{
    return configDirectory() + QStringLiteral("/keys.json");
}

QString statusPath()
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::GenericStateLocation);
    return (base.isEmpty() ? QDir::homePath() + QStringLiteral("/.local/state") : base)
        + QStringLiteral("/slate/status.json");
}

bool isValidAction(const QString &action)
{
    return actions().contains(action);
}

QVector<KeyBinding> defaults()
{
    QVector<KeyBinding> bindings;
    const QVector<SlateKeys::Combo> combos = SlateKeys::defaultCombos();
    bindings.reserve(combos.size());
    for (const SlateKeys::Combo &combo : combos) {
        KeyBinding binding;
        binding.combo = combo.toString();
        binding.action = QStringLiteral("pass");
        bindings.append(binding);
    }
    return bindings;
}

QVector<KeyBinding> load(const QString &path, QString *error)
{
    if (error)
        error->clear();

    QVector<KeyBinding> bindings = defaults();
    QFile file(path);
    if (!file.exists())
        return bindings;
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("Could not read %1: %2").arg(describe(path), file.errorString());
        return bindings;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        if (error)
            *error = QStringLiteral("Could not read %1: %2").arg(describe(path), parseError.errorString());
        return bindings;
    }

    const QJsonArray saved = document.object().value(QStringLiteral("keys")).toArray();
    QHash<QString, QPair<QString, QString>> byCombo;
    for (const QJsonValue &value : saved) {
        const QJsonObject object = value.toObject();
        const QString comboText = object.value(QStringLiteral("combo")).toString();
        SlateKeys::Combo combo;
        if (!SlateKeys::parseCombo(comboText, &combo))
            continue;
        const QString action = object.value(QStringLiteral("action")).toString(QStringLiteral("pass"));
        byCombo.insert(combo.toString(), {isValidAction(action) ? action : QStringLiteral("pass"),
                                           object.value(QStringLiteral("value")).toString()});
    }

    for (KeyBinding &binding : bindings) {
        const auto it = byCombo.constFind(binding.combo);
        if (it == byCombo.cend())
            continue;
        binding.action = it->first;
        binding.value = it->second;

        if (binding.action == QStringLiteral("keys")) {
            SlateKeys::Combo output;
            QString why;
            if (!SlateKeys::parseCombo(binding.value, &output, &why)) {
                binding.action = QStringLiteral("pass");
                binding.value.clear();
                if (error)
                    *error = QStringLiteral("Key %1 has an invalid shortcut (%2); using pass-through.")
                                 .arg(SlateKeys::displayName(binding.combo), why);
            }
        } else if (binding.action == QStringLiteral("command") && binding.value.trimmed().isEmpty()) {
            binding.action = QStringLiteral("pass");
            binding.value.clear();
        }
    }

    return bindings;
}

bool save(const QString &path, const QVector<KeyBinding> &bindings, QString *error)
{
    auto fail = [error](const QString &message) {
        if (error)
            *error = message;
        return false;
    };
    if (error)
        error->clear();

    QVector<KeyBinding> normalized;
    QSet<QString> seen;
    for (const KeyBinding &binding : bindings) {
        SlateKeys::Combo combo;
        QString why;
        if (!SlateKeys::parseCombo(binding.combo, &combo, &why))
            return fail(QStringLiteral("Invalid tablet shortcut %1: %2").arg(binding.combo, why));
        if (seen.contains(combo.toString()))
            return fail(QStringLiteral("Duplicate tablet shortcut: %1").arg(SlateKeys::displayName(combo)));
        seen.insert(combo.toString());

        if (!isValidAction(binding.action))
            return fail(QStringLiteral("Unknown action “%1” for %2.").arg(binding.action, SlateKeys::displayName(combo)));

        KeyBinding normalizedBinding = binding;
        normalizedBinding.combo = combo.toString();
        if (normalizedBinding.action == QStringLiteral("keys")) {
            SlateKeys::Combo output;
            if (!SlateKeys::parseCombo(normalizedBinding.value, &output, &why))
                return fail(QStringLiteral("%1: %2").arg(SlateKeys::displayName(combo), why));
            normalizedBinding.value = output.toString();
        } else if (normalizedBinding.action == QStringLiteral("command")) {
            normalizedBinding.value = normalizedBinding.value.trimmed();
            if (normalizedBinding.value.isEmpty())
                return fail(QStringLiteral("%1: enter a command to run.").arg(SlateKeys::displayName(combo)));
        } else {
            normalizedBinding.value.clear();
        }
        normalized.append(normalizedBinding);
    }

    if (normalized.isEmpty())
        return fail(QStringLiteral("There are no key bindings to save."));

    QJsonArray keys;
    for (const KeyBinding &binding : normalized) {
        QJsonObject object;
        object.insert(QStringLiteral("combo"), binding.combo);
        object.insert(QStringLiteral("action"), binding.action);
        if (!binding.value.isEmpty())
            object.insert(QStringLiteral("value"), binding.value);
        keys.append(object);
    }
    QJsonObject root;
    root.insert(QStringLiteral("version"), 1);
    root.insert(QStringLiteral("keys"), keys);
    const QByteArray data = QJsonDocument(root).toJson(QJsonDocument::Indented);

    if (!QDir().mkpath(QFileInfo(path).absolutePath()))
        return fail(QStringLiteral("Could not create %1.").arg(describe(QFileInfo(path).absolutePath())));

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly))
        return fail(QStringLiteral("Could not write %1: %2").arg(describe(path), file.errorString()));
    if (file.write(data) != data.size() || !file.commit())
        return fail(QStringLiteral("Could not write %1: %2").arg(describe(path), file.errorString()));
    return true;
}

} // namespace SlateBindings
