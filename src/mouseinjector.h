#pragma once

#include <QString>

// A small uinput mouse used for express-key actions such as "left click" and
// "scroll up". The helper runs as the desktop user, so /dev/uinput needs the
// uaccess tag installed by the package (or a distribution equivalent).
class MouseInjector {
public:
    MouseInjector() = default;
    ~MouseInjector();

    MouseInjector(const MouseInjector &) = delete;
    MouseInjector &operator=(const MouseInjector &) = delete;

    bool ensureReady(QString *error = nullptr);
    bool trigger(const QString &action, QString *error = nullptr);
    bool isReady() const { return m_fd >= 0; }

private:
    bool emitEvent(int type, int code, int value, QString *error = nullptr);
    bool click(int button, QString *error);
    bool scroll(int axis, int amount, QString *error);

    int m_fd = -1;
};
