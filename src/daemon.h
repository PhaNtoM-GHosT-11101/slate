#pragma once

#include <QFileSystemWatcher>
#include <QHash>
#include <QObject>
#include <QTimer>

#include "bindings.h"
#include "keymap.h"

class QSocketNotifier;

// The background half of slate. It grabs the tablet's express-key interface,
// suppresses the firmware shortcuts, and replays whatever the user assigned in
// ~/.config/slate/keys.json. Pass-through re-emits the original shortcut, so
// turning remapping on does not change behaviour until a key is edited.
class Daemon : public QObject {
    Q_OBJECT

public:
    explicit Daemon(QObject *parent = nullptr);
    ~Daemon() override;

    bool start(QString *error);

private:
    void tryOpen();
    void closeDevice();
    void readEvents();
    void reloadBindings();
    void trigger(const SlateKeys::Combo &combo);
    void runWtype(const SlateKeys::Combo &combo);
    void writeStatus(const SlateKeys::Combo &combo, const KeyBinding &binding);
    void ensureWaylandEnvironment();

    int m_fd = -1;
    QString m_devicePath;
    QSocketNotifier *m_notifier = nullptr;
    QTimer m_retry;
    QTimer m_quitPoll;
    SlateKeys::ComboTracker m_tracker;
    QHash<QString, KeyBinding> m_bindings;
    QFileSystemWatcher m_watcher;
    QTimer m_reloadDebounce;
};
