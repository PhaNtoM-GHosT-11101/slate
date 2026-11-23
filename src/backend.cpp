#include "backend.h"

#include "device.h"
#include "keymap.h"

#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSaveFile>
#include <QStandardPaths>
#include <QTimer>

#include <cmath>

namespace {

constexpr double kTabletWidth = 254.0;
constexpr double kTabletHeight = 152.4;

bool runCommand(const QString &program, const QStringList &arguments, QByteArray *output, QString *error,
                int timeout = 5000)
{
    if (error)
        error->clear();
    if (output)
        output->clear();

    QProcess process;
    process.start(program, arguments);
    if (!process.waitForStarted(timeout)) {
        if (error)
            *error = QStringLiteral("Could not run %1: %2").arg(program, process.errorString());
        return false;
    }
    if (!process.waitForFinished(timeout)) {
        process.kill();
        process.waitForFinished(500);
        if (error)
            *error = QStringLiteral("%1 timed out.").arg(program);
        return false;
    }

    if (output)
        *output = process.readAllStandardOutput();

    if (process.exitStatus() != QProcess::NormalExit || process.exitCode() != 0) {
        if (error) {
            const QString message = QString::fromUtf8(process.readAllStandardError()).trimmed();
            *error = message.isEmpty()
                ? QStringLiteral("%1 exited with code %2.").arg(program).arg(process.exitCode())
                : message;
        }
        return false;
    }
    return true;
}

bool readTextFile(const QString &path, QString *text, QString *error)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        if (error)
            *error = QStringLiteral("Could not read %1: %2").arg(QDir::toNativeSeparators(path), file.errorString());
        return false;
    }
    *text = QString::fromUtf8(file.readAll());
    return true;
}

bool writeTextFile(const QString &path, const QString &text, QString *error)
{
    const QFileInfo info(path);
    if (!QDir().mkpath(info.absolutePath())) {
        if (error)
            *error = QStringLiteral("Could not create %1.").arg(QDir::toNativeSeparators(info.absolutePath()));
        return false;
    }

    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly)) {
        if (error)
            *error = QStringLiteral("Could not write %1: %2").arg(QDir::toNativeSeparators(path), file.errorString());
        return false;
    }
    const QByteArray data = text.toUtf8();
    if (file.write(data) != data.size() || !file.commit()) {
        if (error)
            *error = QStringLiteral("Could not write %1: %2").arg(QDir::toNativeSeparators(path), file.errorString());
        return false;
    }
    return true;
}

bool readVec2(const QJsonObject &object, double *first, double *second)
{
    const QJsonArray values = object.value(QStringLiteral("vec2")).toArray();
    if (values.size() != 2)
        return false;
    *first = values.at(0).toDouble();
    *second = values.at(1).toDouble();
    return true;
}

} // namespace

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_keyMonitor(this)
{
    connect(&m_keyMonitor, &KeyMonitor::comboTriggered, this, &Backend::noteKeyPreview);

    refreshDeviceState();
    refreshMonitors();

    QString error;
    if (!readHyprlandSettings(&error)) {
        // Hyprland may not be running, or the config may be unavailable. Show
        // the last saved values so the app still opens and can be inspected.
        QString lua;
        TabletSettings saved;
        if (readTextFile(hyprlandConfigPath(), &lua, nullptr) && SlateSettings::parse(lua, &saved))
            setMapping(saved, false);
        else
            setMapping(TabletSettings{}, false);
        if (!error.isEmpty())
            setMessage(error, true);
    }

    loadBindings();
    refreshService();
    startTimers();
    if (m_message.isEmpty())
        setMessage(tr("Ready."), false);
}

Backend::~Backend() = default;

QString Backend::tabletSize() const
{
    return QStringLiteral("254 × 152.4 mm");
}

QString Backend::hyprlandConfigPath() const
{
    const QString base = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    return (base.isEmpty() ? QDir::homePath() + QStringLiteral("/.config") : base)
        + QStringLiteral("/hypr/input.lua");
}

void Backend::refreshDeviceState()
{
    const bool connected = SlateDevice::isConnected();
    if (connected == m_tabletConnected)
        return;
    m_tabletConnected = connected;
    emit tabletChanged();
}

void Backend::refreshMonitors()
{
    QByteArray output;
    QString error;
    if (!runCommand(QStringLiteral("hyprctl"), {QStringLiteral("monitors"), QStringLiteral("-j")}, &output, &error)) {
        if (m_hyprlandAvailable) {
            m_hyprlandAvailable = false;
            emit stateChanged();
        }
        return;
    }

    const QJsonDocument document = QJsonDocument::fromJson(output);
    if (!document.isArray()) {
        if (m_hyprlandAvailable) {
            m_hyprlandAvailable = false;
            emit stateChanged();
        }
        return;
    }

    QVariantList monitors;
    for (const QJsonValue &value : document.array()) {
        const QJsonObject monitor = value.toObject();
        const int width = monitor.value(QStringLiteral("width")).toInt();
        const int height = monitor.value(QStringLiteral("height")).toInt();
        QVariantMap entry;
        entry.insert(QStringLiteral("name"), monitor.value(QStringLiteral("name")).toString());
        entry.insert(QStringLiteral("label"), monitor.value(QStringLiteral("name")).toString());
        entry.insert(QStringLiteral("detail"), QStringLiteral("%1 × %2").arg(width).arg(height));
        entry.insert(QStringLiteral("width"), width);
        entry.insert(QStringLiteral("height"), height);
        entry.insert(QStringLiteral("focused"), monitor.value(QStringLiteral("focused")).toBool());
        entry.insert(QStringLiteral("x"), monitor.value(QStringLiteral("x")).toInt());
        entry.insert(QStringLiteral("y"), monitor.value(QStringLiteral("y")).toInt());
        monitors.append(entry);
    }

    if (monitors != m_monitors) {
        m_monitors = monitors;
        emit monitorsChanged();
    }

    if (!m_hyprlandAvailable) {
        m_hyprlandAvailable = true;
        emit stateChanged();
    }
}

bool Backend::readHyprlandSettings(QString *error)
{
    auto option = [error](const QString &name, QJsonObject *object) {
        QByteArray output;
        QString why;
        if (!runCommand(QStringLiteral("hyprctl"),
                        {QStringLiteral("-j"), QStringLiteral("getoption"), name}, &output, &why)) {
            if (error)
                *error = QStringLiteral("Hyprland is not available: %1").arg(why);
            return false;
        }
        const QJsonDocument document = QJsonDocument::fromJson(output);
        if (!document.isObject()) {
            if (error)
                *error = QStringLiteral("Hyprland returned an unexpected value for %1.").arg(name);
            return false;
        }
        *object = document.object();
        return true;
    };

    TabletSettings settings;
    QJsonObject object;

    if (!option(QStringLiteral("input:tablet:output"), &object))
        return false;
    settings.output = object.value(QStringLiteral("str")).toString();

    if (!option(QStringLiteral("input:tablet:transform"), &object))
        return false;
    settings.transform = object.value(QStringLiteral("int")).toInt();

    if (!option(QStringLiteral("input:tablet:left_handed"), &object))
        return false;
    settings.leftHanded = object.value(QStringLiteral("bool")).toBool();

    if (!option(QStringLiteral("input:tablet:active_area_size"), &object))
        return false;
    if (!readVec2(object, &settings.activeW, &settings.activeH))
        return false;

    if (!option(QStringLiteral("input:tablet:active_area_position"), &object))
        return false;
    if (!readVec2(object, &settings.activeX, &settings.activeY))
        return false;

    if (!option(QStringLiteral("input:tablet:region_size"), &object))
        return false;
    if (!readVec2(object, &settings.regionW, &settings.regionH))
        return false;

    if (!option(QStringLiteral("input:tablet:region_position"), &object))
        return false;
    if (!readVec2(object, &settings.regionX, &settings.regionY))
        return false;

    if (!option(QStringLiteral("input:tablet:absolute_region_position"), &object))
        return false;
    settings.absoluteRegionPosition = object.value(QStringLiteral("bool")).toBool();

    if (!option(QStringLiteral("input:tablettool:pressure_range_min"), &object))
        return false;
    settings.pressureMin = object.value(QStringLiteral("float")).toDouble(-1.0);

    if (!option(QStringLiteral("input:tablettool:pressure_range_max"), &object))
        return false;
    settings.pressureMax = object.value(QStringLiteral("float")).toDouble(-1.0);

    setMapping(settings, false);
    return true;
}

bool Backend::applyMapping(QString *error)
{
    const QString path = hyprlandConfigPath();
    QString original;
    QFile existing(path);
    if (existing.exists() && !readTextFile(path, &original, error))
        return false;

    const QString updated = SlateSettings::upsert(original, m_settings);
    if (updated == original) {
        setMapping(m_settings, false);
        return true;
    }

    QByteArray beforeErrors;
    runCommand(QStringLiteral("hyprctl"), {QStringLiteral("configerrors")}, &beforeErrors, nullptr);

    if (!writeTextFile(path, updated, error))
        return false;

    QString reloadError;
    if (!runCommand(QStringLiteral("hyprctl"), {QStringLiteral("reload")}, nullptr, &reloadError)) {
        if (original.isEmpty())
            QFile::remove(path);
        else
            writeTextFile(path, original, nullptr);
        runCommand(QStringLiteral("hyprctl"), {QStringLiteral("reload")}, nullptr, nullptr);
        if (error)
            *error = tr("Could not reload Hyprland: %1").arg(reloadError);
        return false;
    }

    QByteArray afterErrors;
    QString configError;
    if (runCommand(QStringLiteral("hyprctl"), {QStringLiteral("configerrors")}, &afterErrors, &configError)) {
        const QString before = QString::fromUtf8(beforeErrors).trimmed();
        const QString after = QString::fromUtf8(afterErrors).trimmed();
        if (!after.isEmpty() && after != before) {
            if (original.isEmpty())
                QFile::remove(path);
            else
                writeTextFile(path, original, nullptr);
            runCommand(QStringLiteral("hyprctl"), {QStringLiteral("reload")}, nullptr, nullptr);
            if (error)
                *error = tr("Hyprland rejected the tablet settings:\n%1").arg(after);
            return false;
        }
    }

    setMapping(m_settings, false);
    return true;
}

void Backend::loadBindings()
{
    QString error;
    m_bindings = SlateBindings::load(SlateBindings::keysPath(), &error);
    rebuildKeyRows();
    setKeysDirty(false);
    if (!error.isEmpty())
        setMessage(error, true);
}

bool Backend::saveBindings(QString *error)
{
    if (!SlateBindings::save(SlateBindings::keysPath(), m_bindings, error))
        return false;
    QString loadError;
    m_bindings = SlateBindings::load(SlateBindings::keysPath(), &loadError);
    rebuildKeyRows();
    setKeysDirty(false);
    return true;
}

void Backend::rebuildKeyRows()
{
    QVariantList rows;
    for (int i = 0; i < m_bindings.size(); ++i) {
        const KeyBinding &binding = m_bindings.at(i);
        QVariantMap row;
        row.insert(QStringLiteral("index"), i);
        row.insert(QStringLiteral("combo"), binding.combo);
        row.insert(QStringLiteral("label"), SlateKeys::displayName(binding.combo));
        row.insert(QStringLiteral("action"), binding.action);
        row.insert(QStringLiteral("value"), binding.value);
        rows.append(row);
    }
    m_keyBindings = rows;
    emit keyBindingsChanged();
}

bool Backend::saveKeys()
{
    if (!m_keysDirty) {
        setMessage(tr("Key bindings are already saved."), false);
        return true;
    }
    QString error;
    if (!saveBindings(&error)) {
        setMessage(error, true);
        return false;
    }
    setMessage(tr("Key bindings saved."), false);
    return true;
}

QString Backend::comboFromKey(int key, int modifiers, const QString &text) const
{
    return SlateKeys::comboFromQtKey(key, modifiers, text);
}

QString Backend::comboLabel(const QString &combo) const
{
    return SlateKeys::displayName(combo);
}

QString Backend::mouseLabel(const QString &value) const
{
    return SlateBindings::mouseActionLabel(value);
}

bool Backend::applyAll()
{
    if (!m_mappingDirty && !m_keysDirty) {
        setMessage(tr("No changes to apply."), false);
        return true;
    }

    QString error;
    if (m_mappingDirty && !applyMapping(&error)) {
        setMessage(error, true);
        return false;
    }
    if (m_keysDirty && !saveBindings(&error)) {
        setMessage(error, true);
        return false;
    }

    setMessage(tr("Saved."), false);
    return true;
}

void Backend::revert()
{
    refreshDeviceState();
    refreshMonitors();

    QString error;
    const bool loaded = readHyprlandSettings(&error);
    if (!loaded) {
        QString lua;
        TabletSettings saved;
        if (readTextFile(hyprlandConfigPath(), &lua, nullptr) && SlateSettings::parse(lua, &saved))
            setMapping(saved, false);
        else
            setMapping(TabletSettings{}, false);
    }

    loadBindings();
    refreshService();
    if (loaded)
        setMessage(tr("Reverted to the settings saved in Hyprland."), false);
    else
        setMessage(error, true);
}

void Backend::resetMapping()
{
    TabletSettings settings;
    setMapping(settings, true);
    setMessage(tr("Tablet settings reset. Apply to save them."), false);
}

void Backend::resetKeys()
{
    m_bindings = SlateBindings::defaults();
    rebuildKeyRows();
    setKeysDirty(true);
    setMessage(tr("Express keys reset. Apply to save them."), false);
}

void Backend::resetActiveArea()
{
    TabletSettings next = m_settings;
    next.activeX = 0;
    next.activeY = 0;
    next.activeW = 0;
    next.activeH = 0;
    setMapping(next, true);
}

void Backend::enableRegion(bool enabled)
{
    TabletSettings next = m_settings;
    if (enabled) {
        if (next.regionW > 0 && next.regionH > 0)
            return;
        double width = 800;
        double height = 600;
        for (const QVariant &value : m_monitors) {
            const QVariantMap monitor = value.toMap();
            if (monitor.value(QStringLiteral("focused")).toBool()) {
                width = monitor.value(QStringLiteral("width")).toDouble();
                height = monitor.value(QStringLiteral("height")).toDouble();
                break;
            }
        }
        next.regionX = 0;
        next.regionY = 0;
        next.regionW = width;
        next.regionH = height;
    } else {
        if (next.regionW <= 0 && next.regionH <= 0)
            return;
        next.regionX = 0;
        next.regionY = 0;
        next.regionW = 0;
        next.regionH = 0;
    }
    setMapping(next, true);
}

void Backend::setPressureCustom(bool custom)
{
    if (custom == pressureCustom())
        return;
    TabletSettings next = m_settings;
    if (custom) {
        next.pressureMin = 0.0;
        next.pressureMax = 1.0;
    } else {
        next.pressureMin = -1.0;
        next.pressureMax = -1.0;
    }
    setMapping(next, true);
}

void Backend::updateKeyAction(int index, const QString &action, const QString &value)
{
    if (index < 0 || index >= m_bindings.size())
        return;
    if (!SlateBindings::isValidAction(action))
        return;

    KeyBinding &binding = m_bindings[index];

    QString newValue = value;
    if (action == QStringLiteral("keys")) {
        SlateKeys::Combo parsed;
        if (SlateKeys::parseCombo(newValue, &parsed)) {
            newValue = parsed.toString();
        } else if (binding.action == QStringLiteral("keys") && !binding.value.isEmpty()) {
            // Keep the previous valid shortcut when an edit cannot be parsed.
            newValue = binding.value;
        } else {
            // Switching to Keys starts at the factory shortcut. The user can
            // replace it with the hold gesture or by typing.
            newValue = binding.combo;
        }
    } else if (action == QStringLiteral("mouse")) {
        newValue = newValue.trimmed().toLower();
        if (!SlateBindings::isValidMouseAction(newValue)) {
            newValue = binding.action == QStringLiteral("mouse")
                && SlateBindings::isValidMouseAction(binding.value)
                ? binding.value : QStringLiteral("left");
        }
    } else if (action == QStringLiteral("command")) {
        // Do not carry a shortcut or mouse action into the shell command
        // field when the user switches the row to Command.
        if (binding.action != QStringLiteral("command"))
            newValue.clear();
    } else {
        newValue.clear();
    }
    if (binding.action == action && binding.value == newValue)
        return;

    binding.action = action;
    binding.value = newValue;
    rebuildKeyRows();
    setKeysDirty(true);
}

void Backend::setRemapEnabled(bool enabled)
{
    if (enabled == m_remapEnabled)
        return;

    if (enabled && !m_serviceInstalled) {
        setMessage(tr("The express-key helper is not installed yet. Install Slate as a package first."), true);
        refreshService();
        return;
    }

    if (enabled && m_keysDirty) {
        QString error;
        if (!saveBindings(&error)) {
            setMessage(error, true);
            return;
        }
    }

    QString error;
    if (enabled) {
        runCommand(QStringLiteral("systemctl"), {QStringLiteral("--user"), QStringLiteral("daemon-reload")},
                   nullptr, nullptr);
        if (!runCommand(QStringLiteral("systemctl"),
                        {QStringLiteral("--user"), QStringLiteral("enable"), QStringLiteral("--now"),
                         QStringLiteral("slate.service")},
                        nullptr, &error)) {
            setMessage(tr("Could not start the express-key helper: %1").arg(error), true);
            refreshService();
            return;
        }
        setMessage(tr("Express-key remapping is on."), false);
    } else {
        if (!runCommand(QStringLiteral("systemctl"),
                        {QStringLiteral("--user"), QStringLiteral("disable"), QStringLiteral("--now"),
                         QStringLiteral("slate.service")},
                        nullptr, &error)) {
            setMessage(tr("Could not stop the express-key helper: %1").arg(error), true);
            refreshService();
            return;
        }
        setMessage(tr("Express-key remapping is off."), false);
    }

    refreshService();
}

void Backend::refreshService()
{
    const QString configBase = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    const QString userUnit = (configBase.isEmpty() ? QDir::homePath() + QStringLiteral("/.config") : configBase)
        + QStringLiteral("/systemd/user/slate.service");
    const QString dataBase = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    const QString localDataUnit = (dataBase.isEmpty() ? QDir::homePath() + QStringLiteral("/.local/share") : dataBase)
        + QStringLiteral("/systemd/user/slate.service");
    const bool installed = QFileInfo::exists(QStringLiteral("/usr/lib/systemd/user/slate.service"))
        || QFileInfo::exists(QStringLiteral("/usr/local/lib/systemd/user/slate.service"))
        || QFileInfo::exists(QStringLiteral("/etc/systemd/user/slate.service"))
        || QFileInfo::exists(userUnit)
        || QFileInfo::exists(localDataUnit);

    bool active = false;
    QString state;
    if (!installed) {
        state = tr("Not installed");
    } else {
        QByteArray output;
        QString error;
        runCommand(QStringLiteral("systemctl"),
                   {QStringLiteral("--user"), QStringLiteral("is-active"), QStringLiteral("slate.service")},
                   &output, &error);
        const QString status = QString::fromUtf8(output).trimmed();
        active = status == QStringLiteral("active") || status == QStringLiteral("activating")
            || status == QStringLiteral("reloading");
        if (status == QStringLiteral("failed"))
            state = tr("Failed");
        else if (active)
            state = tr("Running");
        else
            state = tr("Stopped");
    }

    if (installed == m_serviceInstalled && active == m_remapEnabled && state == m_serviceState)
        return;

    m_serviceInstalled = installed;
    m_remapEnabled = active;
    m_serviceState = state;
    emit serviceChanged();
}

void Backend::refreshState()
{
    refreshDeviceState();
    refreshMonitors();
    refreshService();
    pollStatusFile();
}

void Backend::noteKeyPreview(const QString &combo)
{
    if (combo.isEmpty())
        return;
    noteKey(combo, SlateKeys::displayName(combo), QStringLiteral("preview"),
            QDateTime::currentMSecsSinceEpoch());
}

void Backend::noteKey(const QString &combo, const QString &label, const QString &action, qint64 at)
{
    if (combo.isEmpty())
        return;
    if (combo == m_lastKeyCombo && at == m_lastKeyAt)
        return;

    m_lastKeyCombo = combo;
    m_lastKeyLabel = label.isEmpty() ? SlateKeys::displayName(combo) : label;
    m_lastKeyAction = action;
    m_lastKeyAt = at;
    ++m_lastKeySerial;
    emit lastKeyChanged();
}

const KeyBinding *Backend::bindingFor(const QString &combo) const
{
    for (const KeyBinding &binding : m_bindings) {
        if (binding.combo == combo)
            return &binding;
    }
    return nullptr;
}

void Backend::pollStatusFile()
{
    const QFileInfo info(SlateBindings::statusPath());
    if (!info.exists()) {
        m_statusStamp = -1;
        m_statusSize = -1;
        return;
    }

    const qint64 stamp = info.lastModified().toMSecsSinceEpoch();
    const qint64 size = info.size();
    if (stamp == m_statusStamp && size == m_statusSize)
        return;
    m_statusStamp = stamp;
    m_statusSize = size;

    QFile file(SlateBindings::statusPath());
    if (!file.open(QIODevice::ReadOnly))
        return;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll());
    if (!document.isObject())
        return;
    const QJsonObject object = document.object();
    const QString combo = object.value(QStringLiteral("combo")).toString();
    if (combo.isEmpty())
        return;

    const QString label = object.value(QStringLiteral("label")).toString();
    const QString action = object.value(QStringLiteral("action")).toString();
    const qint64 at = static_cast<qint64>(object.value(QStringLiteral("at")).toDouble());
    noteKey(combo, label, action, at);
}

void Backend::startTimers()
{
    m_statusPoll = new QTimer(this);
    m_statusPoll->setInterval(800);
    connect(m_statusPoll, &QTimer::timeout, this, &Backend::pollStatusFile);
    m_statusPoll->start();

    m_servicePoll = new QTimer(this);
    m_servicePoll->setInterval(10000);
    connect(m_servicePoll, &QTimer::timeout, this, &Backend::refreshService);
    m_servicePoll->start();

    m_devicePoll = new QTimer(this);
    m_devicePoll->setInterval(3000);
    connect(m_devicePoll, &QTimer::timeout, this, &Backend::refreshDeviceState);
    m_devicePoll->start();

    pollStatusFile();
}

void Backend::setMapping(const TabletSettings &settings, bool dirty)
{
    const TabletSettings previous = m_settings;
    m_settings = settings;

    if (previous.output != settings.output)
        emit outputChanged();
    if (previous.transform != settings.transform)
        emit transformChanged();
    if (previous.leftHanded != settings.leftHanded)
        emit leftHandedChanged();

    if (!qFuzzyCompare(previous.activeX + 1, settings.activeX + 1)
        || !qFuzzyCompare(previous.activeY + 1, settings.activeY + 1)
        || !qFuzzyCompare(previous.activeW + 1, settings.activeW + 1)
        || !qFuzzyCompare(previous.activeH + 1, settings.activeH + 1))
        emit activeAreaChanged();

    if (!qFuzzyCompare(previous.regionX + 1, settings.regionX + 1)
        || !qFuzzyCompare(previous.regionY + 1, settings.regionY + 1)
        || !qFuzzyCompare(previous.regionW + 1, settings.regionW + 1)
        || !qFuzzyCompare(previous.regionH + 1, settings.regionH + 1)
        || previous.absoluteRegionPosition != settings.absoluteRegionPosition)
        emit regionChanged();

    if (!qFuzzyCompare(previous.pressureMin + 1, settings.pressureMin + 1)
        || !qFuzzyCompare(previous.pressureMax + 1, settings.pressureMax + 1))
        emit pressureChanged();

    if (m_mappingDirty != dirty) {
        m_mappingDirty = dirty;
        emit dirtyChanged();
    }
}

void Backend::setKeysDirty(bool dirty)
{
    if (m_keysDirty == dirty)
        return;
    m_keysDirty = dirty;
    emit dirtyChanged();
}

void Backend::setMessage(const QString &message, bool error)
{
    if (m_message == message && m_messageError == error)
        return;
    m_message = message;
    m_messageError = error;
    emit messageChanged();
}

void Backend::setOutput(const QString &output)
{
    if (m_settings.output == output)
        return;
    if (!output.isEmpty() && output != QStringLiteral("current")) {
        bool known = false;
        for (const QVariant &value : m_monitors) {
            if (value.toMap().value(QStringLiteral("name")).toString() == output) {
                known = true;
                break;
            }
        }
        if (!known)
            return;
    }
    TabletSettings next = m_settings;
    next.output = output;
    setMapping(next, true);
}

void Backend::setTransform(int transform)
{
    transform = qBound(0, transform, 6);
    if (m_settings.transform == transform)
        return;
    TabletSettings next = m_settings;
    next.transform = transform;
    setMapping(next, true);
}

void Backend::setLeftHanded(bool leftHanded)
{
    if (m_settings.leftHanded == leftHanded)
        return;
    TabletSettings next = m_settings;
    next.leftHanded = leftHanded;
    setMapping(next, true);
}

void Backend::setActiveX(double value)
{
    double clamped = qBound(0.0, value, kTabletWidth);
    if (m_settings.activeW > 0)
        clamped = qMin(clamped, kTabletWidth - m_settings.activeW);
    if (qFuzzyCompare(m_settings.activeX + 1, clamped + 1))
        return;
    TabletSettings next = m_settings;
    next.activeX = clamped;
    setMapping(next, true);
}

void Backend::setActiveY(double value)
{
    double clamped = qBound(0.0, value, kTabletHeight);
    if (m_settings.activeH > 0)
        clamped = qMin(clamped, kTabletHeight - m_settings.activeH);
    if (qFuzzyCompare(m_settings.activeY + 1, clamped + 1))
        return;
    TabletSettings next = m_settings;
    next.activeY = clamped;
    setMapping(next, true);
}

void Backend::setActiveW(double value)
{
    const double clamped = qBound(0.0, value, kTabletWidth - m_settings.activeX);
    if (qFuzzyCompare(m_settings.activeW + 1, clamped + 1))
        return;
    TabletSettings next = m_settings;
    next.activeW = clamped;
    setMapping(next, true);
}

void Backend::setActiveH(double value)
{
    const double clamped = qBound(0.0, value, kTabletHeight - m_settings.activeY);
    if (qFuzzyCompare(m_settings.activeH + 1, clamped + 1))
        return;
    TabletSettings next = m_settings;
    next.activeH = clamped;
    setMapping(next, true);
}

void Backend::setRegionX(double value)
{
    const double clamped = qBound(-100000.0, value, 100000.0);
    if (qFuzzyCompare(m_settings.regionX + 1, clamped + 1))
        return;
    TabletSettings next = m_settings;
    next.regionX = clamped;
    setMapping(next, true);
}

void Backend::setRegionY(double value)
{
    const double clamped = qBound(-100000.0, value, 100000.0);
    if (qFuzzyCompare(m_settings.regionY + 1, clamped + 1))
        return;
    TabletSettings next = m_settings;
    next.regionY = clamped;
    setMapping(next, true);
}

void Backend::setRegionW(double value)
{
    const double clamped = qBound(0.0, value, 100000.0);
    if (qFuzzyCompare(m_settings.regionW + 1, clamped + 1))
        return;
    TabletSettings next = m_settings;
    next.regionW = clamped;
    setMapping(next, true);
}

void Backend::setRegionH(double value)
{
    const double clamped = qBound(0.0, value, 100000.0);
    if (qFuzzyCompare(m_settings.regionH + 1, clamped + 1))
        return;
    TabletSettings next = m_settings;
    next.regionH = clamped;
    setMapping(next, true);
}

void Backend::setAbsoluteRegion(bool absolute)
{
    if (m_settings.absoluteRegionPosition == absolute)
        return;
    TabletSettings next = m_settings;
    next.absoluteRegionPosition = absolute;
    setMapping(next, true);
}

void Backend::setPressureMin(double value)
{
    if (value < 0)
        value = 0;
    const double maximum = m_settings.pressureMax >= 0 ? m_settings.pressureMax : 1.0;
    value = qBound(0.0, value, qMax(0.0, maximum - 0.01));
    TabletSettings next = m_settings;
    next.pressureMin = value;
    if (next.pressureMax < 0)
        next.pressureMax = 1.0;
    setMapping(next, true);
}

void Backend::setPressureMax(double value)
{
    const double minimum = m_settings.pressureMin >= 0 ? m_settings.pressureMin : 0.0;
    value = qBound(qMin(1.0, minimum + 0.01), value, 1.0);
    TabletSettings next = m_settings;
    next.pressureMax = value;
    if (next.pressureMin < 0)
        next.pressureMin = 0.0;
    setMapping(next, true);
}
