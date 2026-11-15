#include "daemon.h"

#include "device.h"

#include <QCoreApplication>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QSaveFile>
#include <QSocketNotifier>
#include <QStandardPaths>

#include <atomic>
#include <cerrno>
#include <csignal>
#include <cstring>
#include <fcntl.h>
#include <linux/input.h>
#include <sys/ioctl.h>
#include <unistd.h>

namespace {
std::atomic_bool g_quit{false};

void handleQuitSignal(int)
{
    g_quit.store(true);
}
}

Daemon::Daemon(QObject *parent)
    : QObject(parent)
{
    m_retry.setInterval(2000);
    connect(&m_retry, &QTimer::timeout, this, &Daemon::tryOpen);

    m_reloadDebounce.setSingleShot(true);
    m_reloadDebounce.setInterval(120);
    connect(&m_reloadDebounce, &QTimer::timeout, this, &Daemon::reloadBindings);
    connect(&m_watcher, &QFileSystemWatcher::fileChanged, this, [this] { m_reloadDebounce.start(); });
    connect(&m_watcher, &QFileSystemWatcher::directoryChanged, this, [this] { m_reloadDebounce.start(); });

    m_quitPoll.setInterval(150);
    connect(&m_quitPoll, &QTimer::timeout, this, [] {
        if (g_quit.load())
            QCoreApplication::quit();
    });
}

Daemon::~Daemon()
{
    closeDevice();
}

bool Daemon::start(QString *error)
{
    if (error)
        error->clear();

    g_quit.store(false);
    std::signal(SIGINT, handleQuitSignal);
    std::signal(SIGTERM, handleQuitSignal);
    m_quitPoll.start();

    ensureWaylandEnvironment();
    reloadBindings();

    tryOpen();
    m_retry.start();

    qInfo().noquote() << "slate: express-key helper started";
    return true;
}

void Daemon::ensureWaylandEnvironment()
{
    if (qEnvironmentVariableIsEmpty("XDG_RUNTIME_DIR")) {
        const QByteArray runtime = QByteArray("/run/user/") + QByteArray::number(::getuid());
        qputenv("XDG_RUNTIME_DIR", runtime);
    }
    if (!qEnvironmentVariableIsEmpty("WAYLAND_DISPLAY"))
        return;

    const QDir runtime(qEnvironmentVariable("XDG_RUNTIME_DIR"));
    const QStringList entries = runtime.entryList({QStringLiteral("wayland-*")},
                                                  QDir::Files | QDir::System | QDir::NoDotAndDotDot, QDir::Name);
    for (const QString &entry : entries) {
        if (entry.endsWith(QStringLiteral(".lock")))
            continue;
        qputenv("WAYLAND_DISPLAY", entry.toUtf8());
        qInfo().noquote() << "slate: using WAYLAND_DISPLAY" << entry;
        return;
    }
    qWarning().noquote() << "slate: no Wayland socket found; key replay may fail";
}

void Daemon::tryOpen()
{
    if (m_fd >= 0)
        return;

    const QString path = SlateDevice::keysEventNode();
    if (path.isEmpty())
        return;

    const int fd = ::open(path.toLocal8Bit().constData(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        qWarning().noquote() << "slate: cannot open" << path << "-" << QString::fromLocal8Bit(std::strerror(errno));
        return;
    }

    if (::ioctl(fd, EVIOCGRAB, 1) < 0) {
        qWarning().noquote() << "slate: cannot grab" << path << "-" << QString::fromLocal8Bit(std::strerror(errno));
        ::close(fd);
        return;
    }

    m_fd = fd;
    m_devicePath = path;
    m_tracker.reset();
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &Daemon::readEvents);
    qInfo().noquote() << "slate: remapping express keys on" << path;
}

void Daemon::closeDevice()
{
    if (m_notifier) {
        m_notifier->setEnabled(false);
        m_notifier->deleteLater();
        m_notifier = nullptr;
    }
    if (m_fd >= 0) {
        ::ioctl(m_fd, EVIOCGRAB, 0);
        ::close(m_fd);
        m_fd = -1;
    }
    m_devicePath.clear();
}

void Daemon::readEvents()
{
    if (m_fd < 0)
        return;

    input_event events[64];
    bool disconnected = false;
    while (true) {
        const ssize_t bytes = ::read(m_fd, events, sizeof(events));
        if (bytes > 0) {
            const ssize_t count = bytes / static_cast<ssize_t>(sizeof(input_event));
            for (ssize_t i = 0; i < count; ++i) {
                const input_event &event = events[i];
                if (const auto combo = m_tracker.feed(event.type, event.code, event.value))
                    trigger(*combo);
            }
            continue;
        }
        if (bytes < 0 && (errno == EAGAIN || errno == EWOULDBLOCK))
            break;
        if (bytes < 0 && errno == EINTR)
            continue;
        disconnected = true;
        break;
    }

    if (disconnected) {
        qInfo().noquote() << "slate: express-key interface disconnected";
        closeDevice();
        m_retry.start();
    }
}

void Daemon::reloadBindings()
{
    const QString path = SlateBindings::keysPath();
    const auto paths = m_watcher.files() + m_watcher.directories();
    if (!paths.isEmpty())
        m_watcher.removePaths(paths);

    const QString directory = QFileInfo(path).absolutePath();
    if (QFileInfo::exists(directory))
        m_watcher.addPath(directory);
    if (QFileInfo::exists(path))
        m_watcher.addPath(path);

    QString error;
    const QVector<KeyBinding> loaded = SlateBindings::load(path, &error);
    if (!error.isEmpty())
        qWarning().noquote() << "slate:" << error;

    m_bindings.clear();
    for (const KeyBinding &binding : loaded)
        m_bindings.insert(binding.combo, binding);
}

void Daemon::trigger(const SlateKeys::Combo &combo)
{
    const QString key = combo.toString();
    const KeyBinding binding = m_bindings.value(key, KeyBinding{key, QStringLiteral("pass"), {}});
    writeStatus(combo, binding);

    if (binding.action == QStringLiteral("nothing"))
        return;

    if (binding.action == QStringLiteral("command")) {
        if (!binding.value.trimmed().isEmpty())
            QProcess::startDetached(QStringLiteral("/bin/sh"), {QStringLiteral("-c"), binding.value});
        return;
    }

    if (binding.action == QStringLiteral("keys")) {
        SlateKeys::Combo output;
        QString why;
        if (!SlateKeys::parseCombo(binding.value, &output, &why)) {
            qWarning().noquote() << "slate:" << why;
            return;
        }
        runWtype(output);
        return;
    }

    runWtype(combo);
}

void Daemon::runWtype(const SlateKeys::Combo &combo)
{
    const QStringList args = SlateKeys::wtypeArgs(combo);
    if (!QProcess::startDetached(QStringLiteral("wtype"), args))
        qWarning().noquote() << "slate: could not start wtype for" << combo.toString();
}

void Daemon::writeStatus(const SlateKeys::Combo &combo, const KeyBinding &binding)
{
    QJsonObject object;
    object.insert(QStringLiteral("combo"), combo.toString());
    object.insert(QStringLiteral("label"), SlateKeys::displayName(combo));
    object.insert(QStringLiteral("action"), binding.action);
    object.insert(QStringLiteral("at"), static_cast<double>(QDateTime::currentMSecsSinceEpoch()));
    object.insert(QStringLiteral("pid"), static_cast<double>(QCoreApplication::applicationPid()));
    const QByteArray data = QJsonDocument(object).toJson(QJsonDocument::Compact);

    const QString path = SlateBindings::statusPath();
    if (!QDir().mkpath(QFileInfo(path).absolutePath())) {
        qWarning().noquote() << "slate: cannot create state directory for" << path;
        return;
    }
    QSaveFile file(path);
    if (!file.open(QIODevice::WriteOnly) || file.write(data) != data.size() || !file.commit())
        qWarning().noquote() << "slate: cannot write" << path << "-" << file.errorString();
}
