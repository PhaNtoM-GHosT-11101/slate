#include "penmonitor.h"

#include "device.h"

#include <QFile>
#include <QSocketNotifier>

#include <cerrno>
#include <cstring>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

namespace {
constexpr double kMaxX = 32767.0;
constexpr double kMaxY = 32767.0;
constexpr double kMaxPressure = 8191.0;
constexpr double kMaxTilt = 127.0;
}

PenMonitor::PenMonitor(QObject *parent)
    : QObject(parent)
{
    m_retry.setInterval(2000);
    connect(&m_retry, &QTimer::timeout, this, &PenMonitor::tryOpen);
    m_retry.start();
    tryOpen();
}

PenMonitor::~PenMonitor()
{
    closeDevice();
}

void PenMonitor::tryOpen()
{
    if (m_fd >= 0)
        return;

    const QString path = SlateDevice::penEventNode();
    if (path.isEmpty()) {
        const QString error = tr("Tablet not connected.");
        if (error != m_error) {
            m_error = error;
            emit stateChanged();
        }
        return;
    }

    const int fd = ::open(path.toLocal8Bit().constData(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        const QString error = tr("Cannot read %1: %2").arg(path, QString::fromLocal8Bit(std::strerror(errno)));
        if (error != m_error) {
            m_error = error;
            emit stateChanged();
        }
        return;
    }

    m_fd = fd;
    m_devicePath = path;
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &PenMonitor::readEvents);
    m_available = true;
    m_error.clear();
    emit stateChanged();
}

void PenMonitor::closeDevice()
{
    if (m_notifier) {
        m_notifier->setEnabled(false);
        m_notifier->deleteLater();
        m_notifier = nullptr;
    }
    if (m_fd >= 0) {
        ::close(m_fd);
        m_fd = -1;
    }
    m_available = false;
    m_devicePath.clear();
}

void PenMonitor::fail(const QString &error)
{
    closeDevice();
    m_hovering = false;
    m_touching = false;
    m_pressure = 0;
    m_barrel = false;
    m_secondary = false;
    m_eraser = false;
    m_error = error;
    emit stateChanged();
    emit frame();
    m_retry.start();
}

void PenMonitor::readEvents()
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
                if (event.type == EV_ABS) {
                    switch (event.code) {
                    case ABS_X:
                        m_x = qBound(0.0, event.value / kMaxX, 1.0);
                        m_dirty = true;
                        break;
                    case ABS_Y:
                        m_y = qBound(0.0, event.value / kMaxY, 1.0);
                        m_dirty = true;
                        break;
                    case ABS_PRESSURE:
                        m_pressure = qBound(0.0, event.value / kMaxPressure, 1.0);
                        m_dirty = true;
                        break;
                    case ABS_TILT_X:
                        m_tiltX = qBound(-kMaxTilt, static_cast<double>(event.value), kMaxTilt);
                        m_dirty = true;
                        break;
                    case ABS_TILT_Y:
                        m_tiltY = qBound(-kMaxTilt, static_cast<double>(event.value), kMaxTilt);
                        m_dirty = true;
                        break;
                    default:
                        break;
                    }
                } else if (event.type == EV_KEY) {
                    switch (event.code) {
                    case BTN_TOOL_PEN:
                        m_hovering = event.value != 0;
                        if (m_hovering)
                            m_eraser = false;
                        m_dirty = true;
                        break;
                    case BTN_TOOL_RUBBER:
                        m_hovering = event.value != 0;
                        if (m_hovering)
                            m_eraser = true;
                        m_dirty = true;
                        break;
                    case BTN_TOUCH:
                        m_touching = event.value != 0;
                        m_dirty = true;
                        break;
                    case BTN_STYLUS:
                        m_barrel = event.value != 0;
                        m_dirty = true;
                        break;
                    case BTN_STYLUS2:
                        m_secondary = event.value != 0;
                        m_dirty = true;
                        break;
                    default:
                        break;
                    }
                } else if (event.type == EV_SYN && event.code == SYN_REPORT && m_dirty) {
                    m_dirty = false;
                    emit frame();
                }
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

    if (disconnected)
        fail(tr("Tablet disconnected."));
}
