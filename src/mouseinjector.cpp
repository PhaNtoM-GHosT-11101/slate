#include "mouseinjector.h"

#include <QString>

#include <cerrno>
#include <chrono>
#include <cstring>
#include <fcntl.h>
#include <linux/input.h>
#include <linux/uinput.h>
#include <sys/ioctl.h>
#include <thread>
#include <unistd.h>

namespace {

constexpr const char *kDeviceName = "Slate Virtual Mouse";

bool writeEvent(int fd, int type, int code, int value, QString *error)
{
    input_event event{};
    event.type = static_cast<__u16>(type);
    event.code = static_cast<__u16>(code);
    event.value = value;
    if (::write(fd, &event, sizeof(event)) != static_cast<ssize_t>(sizeof(event))) {
        if (error)
            *error = QStringLiteral("Could not inject mouse input: %1")
                         .arg(QString::fromLocal8Bit(std::strerror(errno)));
        return false;
    }
    return true;
}

} // namespace

MouseInjector::~MouseInjector()
{
    if (m_fd >= 0) {
        ::ioctl(m_fd, UI_DEV_DESTROY);
        ::close(m_fd);
        m_fd = -1;
    }
}

bool MouseInjector::ensureReady(QString *error)
{
    if (error)
        error->clear();
    if (m_fd >= 0)
        return true;

    int fd = ::open("/dev/uinput", O_WRONLY | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0)
        fd = ::open("/dev/uinput", O_RDWR | O_NONBLOCK | O_CLOEXEC);
    if (fd < 0) {
        if (error)
            *error = QStringLiteral("Could not open /dev/uinput: %1. Install the Slate uinput udev rule "
                                    "or grant the desktop user access to /dev/uinput.")
                         .arg(QString::fromLocal8Bit(std::strerror(errno)));
        return false;
    }

    auto fail = [&](const QString &what) {
        if (error)
            *error = QStringLiteral("Could not create the virtual mouse: %1: %2")
                         .arg(what, QString::fromLocal8Bit(std::strerror(errno)));
        ::close(fd);
        return false;
    };

    if (::ioctl(fd, UI_SET_EVBIT, EV_KEY) < 0)
        return fail(QStringLiteral("EV_KEY"));
    for (int button : {BTN_LEFT, BTN_RIGHT, BTN_MIDDLE, BTN_SIDE, BTN_EXTRA}) {
        if (::ioctl(fd, UI_SET_KEYBIT, button) < 0)
            return fail(QStringLiteral("mouse button"));
    }
    if (::ioctl(fd, UI_SET_EVBIT, EV_REL) < 0)
        return fail(QStringLiteral("EV_REL"));
    for (int axis : {REL_WHEEL, REL_HWHEEL}) {
        if (::ioctl(fd, UI_SET_RELBIT, axis) < 0)
            return fail(QStringLiteral("wheel axis"));
    }

    uinput_setup setup{};
    setup.id.bustype = BUS_VIRTUAL;
    setup.id.vendor = 0x1209;
    setup.id.product = 0x0001;
    setup.id.version = 1;
    std::strncpy(setup.name, kDeviceName, UINPUT_MAX_NAME_SIZE - 1);
    if (::ioctl(fd, UI_DEV_SETUP, &setup) < 0)
        return fail(QStringLiteral("UI_DEV_SETUP"));
    if (::ioctl(fd, UI_DEV_CREATE) < 0)
        return fail(QStringLiteral("UI_DEV_CREATE"));

    m_fd = fd;
    // Give the compositor a moment to notice the new device before the first
    // click is emitted. This only happens once per helper session.
    std::this_thread::sleep_for(std::chrono::milliseconds(120));
    return true;
}

bool MouseInjector::emitEvent(int type, int code, int value, QString *error)
{
    if (m_fd < 0) {
        if (error)
            *error = QStringLiteral("The virtual mouse is not ready.");
        return false;
    }
    return writeEvent(m_fd, type, code, value, error);
}

bool MouseInjector::click(int button, QString *error)
{
    if (!emitEvent(EV_KEY, button, 1, error) || !emitEvent(EV_SYN, SYN_REPORT, 0, error))
        return false;
    std::this_thread::sleep_for(std::chrono::milliseconds(12));
    return emitEvent(EV_KEY, button, 0, error) && emitEvent(EV_SYN, SYN_REPORT, 0, error);
}

bool MouseInjector::scroll(int axis, int amount, QString *error)
{
    return emitEvent(EV_REL, axis, amount, error) && emitEvent(EV_SYN, SYN_REPORT, 0, error);
}

bool MouseInjector::trigger(const QString &action, QString *error)
{
    if (error)
        error->clear();
    const QString value = action.trimmed().toLower();
    if (value.isEmpty()) {
        if (error)
            *error = QStringLiteral("No mouse action was selected.");
        return false;
    }
    if (!ensureReady(error))
        return false;

    if (value == QStringLiteral("left"))
        return click(BTN_LEFT, error);
    if (value == QStringLiteral("right"))
        return click(BTN_RIGHT, error);
    if (value == QStringLiteral("middle"))
        return click(BTN_MIDDLE, error);
    if (value == QStringLiteral("back"))
        return click(BTN_SIDE, error);
    if (value == QStringLiteral("forward"))
        return click(BTN_EXTRA, error);
    if (value == QStringLiteral("wheelup"))
        return scroll(REL_WHEEL, 1, error);
    if (value == QStringLiteral("wheeldown"))
        return scroll(REL_WHEEL, -1, error);
    if (value == QStringLiteral("wheelleft"))
        return scroll(REL_HWHEEL, -1, error);
    if (value == QStringLiteral("wheelright"))
        return scroll(REL_HWHEEL, 1, error);

    if (error)
        *error = QStringLiteral("Unknown mouse action “%1”.").arg(action);
    return false;
}
