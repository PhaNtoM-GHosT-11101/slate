#include "keymonitor.h"

#include "device.h"

#include <QSocketNotifier>

#include <cerrno>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>

KeyMonitor::KeyMonitor(QObject *parent)
    : QObject(parent)
{
    m_retry.setInterval(2000);
    connect(&m_retry, &QTimer::timeout, this, &KeyMonitor::tryOpen);
    m_retry.start();
    tryOpen();
}

KeyMonitor::~KeyMonitor()
{
    closeDevice();
}

void KeyMonitor::tryOpen()
{
    if (m_fd >= 0)
        return;

    const QString path = SlateDevice::keysEventNode();
    if (path.isEmpty())
        return;

    const int fd = ::open(path.toLocal8Bit().constData(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0)
        return;

    m_fd = fd;
    m_tracker.reset();
    m_notifier = new QSocketNotifier(m_fd, QSocketNotifier::Read, this);
    connect(m_notifier, &QSocketNotifier::activated, this, &KeyMonitor::readEvents);
}

void KeyMonitor::closeDevice()
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
}

void KeyMonitor::readEvents()
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
                    emit comboTriggered(combo->toString());
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
        closeDevice();
        m_retry.start();
    }
}
