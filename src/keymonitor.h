#pragma once

#include <QObject>
#include <QString>
#include <QTimer>

#include "keymap.h"

class QSocketNotifier;

// Watches the express-key interface without grabbing it, purely so the app can
// show which tablet key was pressed. When the remapping daemon is running it
// holds the exclusive grab and this monitor goes quiet; the daemon's status
// file provides the feedback in that case.
class KeyMonitor : public QObject {
    Q_OBJECT

public:
    explicit KeyMonitor(QObject *parent = nullptr);
    ~KeyMonitor() override;

signals:
    void comboTriggered(const QString &combo);

private:
    void tryOpen();
    void closeDevice();
    void readEvents();

    int m_fd = -1;
    QSocketNotifier *m_notifier = nullptr;
    QTimer m_retry;
    SlateKeys::ComboTracker m_tracker;
};
