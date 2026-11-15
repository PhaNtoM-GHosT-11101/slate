#pragma once

#include <QObject>
#include <QString>
#include <QVariantList>
#include <QVector>

#include "bindings.h"
#include "keymonitor.h"
#include "settings.h"

class QTimer;

// The GUI's view of the system: Hyprland's live tablet options, the monitor
// list, the express-key bindings, and the background helper service.
class Backend : public QObject {
    Q_OBJECT

    Q_PROPERTY(bool tabletConnected READ tabletConnected NOTIFY tabletChanged)
    Q_PROPERTY(QString tabletName READ tabletName NOTIFY tabletChanged)
    Q_PROPERTY(QString tabletSize READ tabletSize NOTIFY tabletChanged)
    Q_PROPERTY(bool hyprlandAvailable READ hyprlandAvailable NOTIFY stateChanged)
    Q_PROPERTY(QVariantList monitors READ monitors NOTIFY monitorsChanged)

    Q_PROPERTY(QString output READ output WRITE setOutput NOTIFY outputChanged)
    Q_PROPERTY(int transform READ transform WRITE setTransform NOTIFY transformChanged)
    Q_PROPERTY(bool leftHanded READ leftHanded WRITE setLeftHanded NOTIFY leftHandedChanged)

    Q_PROPERTY(double activeX READ activeX WRITE setActiveX NOTIFY activeAreaChanged)
    Q_PROPERTY(double activeY READ activeY WRITE setActiveY NOTIFY activeAreaChanged)
    Q_PROPERTY(double activeW READ activeW WRITE setActiveW NOTIFY activeAreaChanged)
    Q_PROPERTY(double activeH READ activeH WRITE setActiveH NOTIFY activeAreaChanged)

    Q_PROPERTY(double regionX READ regionX WRITE setRegionX NOTIFY regionChanged)
    Q_PROPERTY(double regionY READ regionY WRITE setRegionY NOTIFY regionChanged)
    Q_PROPERTY(double regionW READ regionW WRITE setRegionW NOTIFY regionChanged)
    Q_PROPERTY(double regionH READ regionH WRITE setRegionH NOTIFY regionChanged)
    Q_PROPERTY(bool regionEnabled READ regionEnabled NOTIFY regionChanged)
    Q_PROPERTY(bool absoluteRegion READ absoluteRegion WRITE setAbsoluteRegion NOTIFY regionChanged)

    Q_PROPERTY(double pressureMin READ pressureMin WRITE setPressureMin NOTIFY pressureChanged)
    Q_PROPERTY(double pressureMax READ pressureMax WRITE setPressureMax NOTIFY pressureChanged)
    Q_PROPERTY(bool pressureCustom READ pressureCustom NOTIFY pressureChanged)

    Q_PROPERTY(bool mappingDirty READ mappingDirty NOTIFY dirtyChanged)
    Q_PROPERTY(bool keysDirty READ keysDirty NOTIFY dirtyChanged)
    Q_PROPERTY(bool dirty READ dirty NOTIFY dirtyChanged)

    Q_PROPERTY(QVariantList keyBindings READ keyBindings NOTIFY keyBindingsChanged)
    Q_PROPERTY(bool remapEnabled READ remapEnabled NOTIFY serviceChanged)
    Q_PROPERTY(bool serviceInstalled READ serviceInstalled NOTIFY serviceChanged)
    Q_PROPERTY(QString serviceState READ serviceState NOTIFY serviceChanged)

    Q_PROPERTY(QString lastKeyCombo READ lastKeyCombo NOTIFY lastKeyChanged)
    Q_PROPERTY(QString lastKeyLabel READ lastKeyLabel NOTIFY lastKeyChanged)
    Q_PROPERTY(QString lastKeyAction READ lastKeyAction NOTIFY lastKeyChanged)
    Q_PROPERTY(qint64 lastKeyAt READ lastKeyAt NOTIFY lastKeyChanged)
    Q_PROPERTY(int lastKeySerial READ lastKeySerial NOTIFY lastKeyChanged)

    Q_PROPERTY(QString message READ message NOTIFY messageChanged)
    Q_PROPERTY(bool messageError READ messageError NOTIFY messageChanged)
    Q_PROPERTY(QString keysPath READ keysPath CONSTANT)
    Q_PROPERTY(QString hyprlandConfigPath READ hyprlandConfigPath CONSTANT)

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend() override;

    bool tabletConnected() const { return m_tabletConnected; }
    QString tabletName() const { return QStringLiteral("VEIKK A15"); }
    QString tabletSize() const;
    bool hyprlandAvailable() const { return m_hyprlandAvailable; }
    QVariantList monitors() const { return m_monitors; }

    QString output() const { return m_settings.output; }
    int transform() const { return m_settings.transform; }
    bool leftHanded() const { return m_settings.leftHanded; }

    double activeX() const { return m_settings.activeX; }
    double activeY() const { return m_settings.activeY; }
    double activeW() const { return m_settings.activeW; }
    double activeH() const { return m_settings.activeH; }

    double regionX() const { return m_settings.regionX; }
    double regionY() const { return m_settings.regionY; }
    double regionW() const { return m_settings.regionW; }
    double regionH() const { return m_settings.regionH; }
    bool regionEnabled() const { return m_settings.regionW > 0 && m_settings.regionH > 0; }
    bool absoluteRegion() const { return m_settings.absoluteRegionPosition; }

    double pressureMin() const { return m_settings.pressureMin; }
    double pressureMax() const { return m_settings.pressureMax; }
    bool pressureCustom() const { return m_settings.pressureMin >= 0 && m_settings.pressureMax >= 0; }

    bool mappingDirty() const { return m_mappingDirty; }
    bool keysDirty() const { return m_keysDirty; }
    bool dirty() const { return m_mappingDirty || m_keysDirty; }

    QVariantList keyBindings() const { return m_keyBindings; }
    bool remapEnabled() const { return m_remapEnabled; }
    bool serviceInstalled() const { return m_serviceInstalled; }
    QString serviceState() const { return m_serviceState; }

    QString lastKeyCombo() const { return m_lastKeyCombo; }
    QString lastKeyLabel() const { return m_lastKeyLabel; }
    QString lastKeyAction() const { return m_lastKeyAction; }
    qint64 lastKeyAt() const { return m_lastKeyAt; }
    int lastKeySerial() const { return m_lastKeySerial; }

    QString message() const { return m_message; }
    bool messageError() const { return m_messageError; }
    QString keysPath() const { return SlateBindings::keysPath(); }
    QString hyprlandConfigPath() const;

    Q_INVOKABLE bool applyAll();
    Q_INVOKABLE void revert();
    Q_INVOKABLE void resetMapping();
    Q_INVOKABLE void resetKeys();
    Q_INVOKABLE void resetActiveArea();
    Q_INVOKABLE void enableRegion(bool enabled);
    Q_INVOKABLE void setPressureCustom(bool custom);
    Q_INVOKABLE void updateKeyAction(int index, const QString &action, const QString &value);
    Q_INVOKABLE void setRemapEnabled(bool enabled);
    Q_INVOKABLE void refreshService();
    Q_INVOKABLE void refreshState();
    Q_INVOKABLE void noteKeyPreview(const QString &combo);

public slots:
    void setOutput(const QString &output);
    void setTransform(int transform);
    void setLeftHanded(bool leftHanded);
    void setActiveX(double value);
    void setActiveY(double value);
    void setActiveW(double value);
    void setActiveH(double value);
    void setRegionX(double value);
    void setRegionY(double value);
    void setRegionW(double value);
    void setRegionH(double value);
    void setAbsoluteRegion(bool absolute);
    void setPressureMin(double value);
    void setPressureMax(double value);

signals:
    void tabletChanged();
    void stateChanged();
    void monitorsChanged();
    void outputChanged();
    void transformChanged();
    void leftHandedChanged();
    void activeAreaChanged();
    void regionChanged();
    void pressureChanged();
    void dirtyChanged();
    void keyBindingsChanged();
    void serviceChanged();
    void lastKeyChanged();
    void messageChanged();

private:
    void setMapping(const TabletSettings &settings, bool dirty);
    void setKeysDirty(bool dirty);
    void setMessage(const QString &message, bool error = false);
    void refreshDeviceState();
    void refreshMonitors();
    bool readHyprlandSettings(QString *error);
    bool applyMapping(QString *error);
    bool saveBindings(QString *error);
    void loadBindings();
    void rebuildKeyRows();
    void pollStatusFile();
    void noteKey(const QString &combo, const QString &label, const QString &action, qint64 at);
    const KeyBinding *bindingFor(const QString &combo) const;
    void startTimers();

    TabletSettings m_settings;
    bool m_mappingDirty = false;
    bool m_keysDirty = false;

    bool m_tabletConnected = false;
    bool m_hyprlandAvailable = false;
    QVariantList m_monitors;

    QVector<KeyBinding> m_bindings;
    QVariantList m_keyBindings;

    bool m_remapEnabled = false;
    bool m_serviceInstalled = false;
    QString m_serviceState;

    QString m_lastKeyCombo;
    QString m_lastKeyLabel;
    QString m_lastKeyAction;
    qint64 m_lastKeyAt = 0;
    int m_lastKeySerial = 0;

    QString m_message;
    bool m_messageError = false;

    qint64 m_statusStamp = -1;
    qint64 m_statusSize = -1;

    KeyMonitor m_keyMonitor;
    QTimer *m_statusPoll = nullptr;
    QTimer *m_servicePoll = nullptr;
    QTimer *m_devicePoll = nullptr;
};
