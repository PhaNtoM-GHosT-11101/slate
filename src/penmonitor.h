#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

class QSocketNotifier;

// Reads the pen interface directly from evdev so the test pad works even when
// the pointer is not over the window. It never grabs the device, so Hyprland
// keeps receiving pen events normally.
class PenMonitor : public QObject {
    Q_OBJECT
    Q_PROPERTY(bool available READ available NOTIFY stateChanged)
    Q_PROPERTY(QString error READ error NOTIFY stateChanged)
    Q_PROPERTY(QString devicePath READ devicePath NOTIFY stateChanged)
    Q_PROPERTY(double x READ x NOTIFY frame)
    Q_PROPERTY(double y READ y NOTIFY frame)
    Q_PROPERTY(double pressure READ pressure NOTIFY frame)
    Q_PROPERTY(double tiltX READ tiltX NOTIFY frame)
    Q_PROPERTY(double tiltY READ tiltY NOTIFY frame)
    Q_PROPERTY(bool hovering READ hovering NOTIFY frame)
    Q_PROPERTY(bool touching READ touching NOTIFY frame)
    Q_PROPERTY(bool barrel READ barrel NOTIFY frame)
    Q_PROPERTY(bool secondary READ secondary NOTIFY frame)
    Q_PROPERTY(bool eraser READ eraser NOTIFY frame)

public:
    explicit PenMonitor(QObject *parent = nullptr);
    ~PenMonitor() override;

    bool available() const { return m_available; }
    QString error() const { return m_error; }
    QString devicePath() const { return m_devicePath; }
    double x() const { return m_x; }
    double y() const { return m_y; }
    double pressure() const { return m_pressure; }
    double tiltX() const { return m_tiltX; }
    double tiltY() const { return m_tiltY; }
    bool hovering() const { return m_hovering; }
    bool touching() const { return m_touching; }
    bool barrel() const { return m_barrel; }
    bool secondary() const { return m_secondary; }
    bool eraser() const { return m_eraser; }

signals:
    void stateChanged();
    void frame();

private:
    void tryOpen();
    void closeDevice();
    void readEvents();
    void fail(const QString &error);

    int m_fd = -1;
    QSocketNotifier *m_notifier = nullptr;
    QTimer m_retry;
    QString m_devicePath;

    bool m_available = false;
    QString m_error;

    double m_x = 0.5;
    double m_y = 0.5;
    double m_pressure = 0;
    double m_tiltX = 0;
    double m_tiltY = 0;
    bool m_hovering = false;
    bool m_touching = false;
    bool m_barrel = false;
    bool m_secondary = false;
    bool m_eraser = false;
    bool m_dirty = false;
};
