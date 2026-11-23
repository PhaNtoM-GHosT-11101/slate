#include <QtTest>

#include <QDir>
#include <QFile>
#include <QTemporaryDir>

#include <linux/input-event-codes.h>

#include "bindings.h"
#include "keymap.h"
#include "settings.h"
#include "theme.h"

using namespace SlateKeys;

class SlateTests : public QObject {
    Q_OBJECT

private slots:
    // keymap
    void parsesCombos_data();
    void parsesCombos();
    void rejectsBadCombos_data();
    void rejectsBadCombos();
    void formatsDisplayNames();
    void buildsWtypeArguments();
    void buildsXdotoolArguments();
    void capturesQtKeys();
    void mapsEvdevCodes();
    void tracksHeldCombos();
    void ignoresAutorepeatAndModifiers();
    void providesFactoryCombos();

    // settings
    void settingsRoundTrip();
    void settingsParsesEscapedOutput();
    void settingsUpsertPreservesSurroundings();
    void settingsUpsertReplacesExistingBlock();
    void settingsRemove();
    void settingsRejectsMissingBlock();

    // bindings
    void defaultsArePassThrough();
    void bindingsRoundTrip();
    void bindingsRoundTripMouse();
    void bindingsRejectInvalidShortcut();
    void bindingsRejectInvalidMouseAction();
    void bindingsRejectDuplicates();
    void bindingsFallBackForBadFile();

    // theme
    void readsAccentFromTheme();
    void fallsBackWithoutTheme();
};

void SlateTests::parsesCombos_data()
{
    QTest::addColumn<QString>("input");
    QTest::addColumn<QString>("normalized");
    QTest::addColumn<QString>("display");

    QTest::newRow("lowercase") << "ctrl+shift+t" << "ctrl+shift+t" << "Ctrl + Shift + T";
    QTest::newRow("aliases") << "Control + Page Up" << "ctrl+pageup" << "Ctrl + Page Up";
    QTest::newRow("order") << "shift+ctrl+alt+c" << "ctrl+shift+alt+c" << "Ctrl + Shift + Alt + C";
    QTest::newRow("super") << "meta+enter" << "super+enter" << "Super + Enter";
    QTest::newRow("single") << "F5" << "f5" << "F5";
    QTest::newRow("symbol") << "ctrl+/" << "ctrl+slash" << "Ctrl + /";
    QTest::newRow("repeated-modifier") << "ctrl+ctrl+v" << "ctrl+v" << "Ctrl + V";
}

void SlateTests::parsesCombos()
{
    QFETCH(QString, input);
    QFETCH(QString, normalized);
    QFETCH(QString, display);

    Combo combo;
    QString error;
    QVERIFY2(parseCombo(input, &combo, &error), qPrintable(error));
    QCOMPARE(combo.toString(), normalized);
    QCOMPARE(displayName(combo), display);
}

void SlateTests::rejectsBadCombos_data()
{
    QTest::addColumn<QString>("input");

    QTest::newRow("empty") << "   ";
    QTest::newRow("modifier-only") << "ctrl+shift";
    QTest::newRow("unknown") << "hyper+x";
    QTest::newRow("two-keys") << "ctrl+a+b";
    QTest::newRow("junk") << "ctrl+nope";
}

void SlateTests::rejectsBadCombos()
{
    QFETCH(QString, input);

    Combo combo;
    combo.key = QStringLiteral("unchanged");
    QString error;
    QVERIFY(!parseCombo(input, &combo, &error));
    QCOMPARE(combo.key, QStringLiteral("unchanged"));
    QVERIFY(!error.isEmpty());
}

void SlateTests::formatsDisplayNames()
{
    QCOMPARE(displayName(QStringLiteral("ctrl+alt+delete")), QStringLiteral("Ctrl + Alt + Delete"));
    QCOMPARE(displayName(QStringLiteral("not a combo")), QStringLiteral("not a combo"));
    QCOMPARE(displayName(Combo{}), QString());
}

void SlateTests::buildsWtypeArguments()
{
    Combo combo;
    QVERIFY(parseCombo("ctrl+shift+t", &combo));
    const QStringList args = wtypeArgs(combo);
    QCOMPARE(args, QStringList({"-M", "ctrl", "-M", "shift", "-k", "t", "-m", "shift", "-m", "ctrl"}));

    Combo function;
    QVERIFY(parseCombo("f5", &function));
    QCOMPARE(wtypeArgs(function), QStringList({"-k", "F5"}));
}

void SlateTests::buildsXdotoolArguments()
{
    Combo combo;
    QVERIFY(parseCombo("ctrl+shift+t", &combo));
    QCOMPARE(xdotoolArgs(combo), QStringList({"key", "--clearmodifiers", "ctrl+shift+t"}));

    Combo function;
    QVERIFY(parseCombo("ctrl+pageup", &function));
    QCOMPARE(xdotoolArgs(function), QStringList({"key", "--clearmodifiers", "ctrl+Page_Up"}));
}

void SlateTests::capturesQtKeys()
{
    QCOMPARE(comboFromQtKey(Qt::Key_C, Qt::ControlModifier, "c"), QStringLiteral("ctrl+c"));
    QCOMPARE(comboFromQtKey(Qt::Key_1, Qt::ShiftModifier, "!"), QStringLiteral("shift+1"));
    QCOMPARE(comboFromQtKey(Qt::Key_F5, Qt::NoModifier, {}), QStringLiteral("f5"));
    QCOMPARE(comboFromQtKey(Qt::Key_PageUp, Qt::AltModifier, {}), QStringLiteral("alt+pageup"));
    QCOMPARE(comboFromQtKey(Qt::Key_Slash, Qt::MetaModifier, "/"), QStringLiteral("super+slash"));
    QCOMPARE(comboFromQtKey(Qt::Key_Question, Qt::ShiftModifier, "?"), QStringLiteral("shift+slash"));
    QCOMPARE(comboFromQtKey(Qt::Key_Dollar, Qt::ShiftModifier, "$"), QStringLiteral("shift+4"));
    QCOMPARE(comboFromQtKey(Qt::Key_Underscore, Qt::ShiftModifier, "_"), QStringLiteral("shift+minus"));
    QCOMPARE(comboFromQtKey(Qt::Key_Control, Qt::ControlModifier, {}), QString());
    QCOMPARE(comboFromQtKey(Qt::Key_unknown, Qt::NoModifier, {}), QString());
}

void SlateTests::mapsEvdevCodes()
{
    QCOMPARE(keyToken(KEY_C), QStringLiteral("c"));
    QCOMPARE(keyToken(KEY_PAGEUP), QStringLiteral("pageup"));
    QCOMPARE(keyToken(KEY_LEFTMETA), QString());
    QCOMPARE(modifierForCode(KEY_LEFTCTRL), QStringLiteral("ctrl"));
    QCOMPARE(modifierForCode(KEY_RIGHTALT), QStringLiteral("alt"));
    QCOMPARE(modifierForCode(KEY_A), QString());
}

void SlateTests::tracksHeldCombos()
{
    ComboTracker tracker;
    QVERIFY(!tracker.feed(EV_KEY, KEY_LEFTCTRL, 1));
    QVERIFY(!tracker.feed(EV_KEY, KEY_LEFTSHIFT, 1));

    const auto combo = tracker.feed(EV_KEY, KEY_T, 1);
    QVERIFY(combo.has_value());
    QCOMPARE(combo->toString(), QStringLiteral("ctrl+shift+t"));

    QVERIFY(!tracker.feed(EV_KEY, KEY_T, 0));
    QVERIFY(!tracker.feed(EV_KEY, KEY_LEFTSHIFT, 0));
    QVERIFY(!tracker.feed(EV_KEY, KEY_LEFTCTRL, 0));

    const auto next = tracker.feed(EV_KEY, KEY_V, 1);
    QVERIFY(next.has_value());
    QCOMPARE(next->toString(), QStringLiteral("v"));
}

void SlateTests::ignoresAutorepeatAndModifiers()
{
    ComboTracker tracker;
    QVERIFY(!tracker.feed(EV_KEY, KEY_LEFTCTRL, 1));
    QVERIFY(!tracker.feed(EV_KEY, KEY_LEFTCTRL, 2));
    QVERIFY(!tracker.feed(EV_KEY, KEY_LEFTCTRL, 1));
    QVERIFY(!tracker.feed(EV_KEY, KEY_C, 2));
    QVERIFY(!tracker.feed(EV_SYN, SYN_REPORT, 0));
    QVERIFY(!tracker.feed(EV_KEY, KEY_LEFTMETA, 1));

    const auto combo = tracker.feed(EV_KEY, KEY_C, 1);
    QVERIFY(combo.has_value());
    QCOMPARE(combo->toString(), QStringLiteral("ctrl+super+c"));
}

void SlateTests::providesFactoryCombos()
{
    const QVector<Combo> combos = defaultCombos();
    QCOMPARE(combos.size(), 12);
    QCOMPARE(combos.first().toString(), QStringLiteral("ctrl+c"));
    QCOMPARE(combos.at(4).toString(), QStringLiteral("b"));
    QCOMPARE(combos.at(8).toString(), QStringLiteral("pageup"));
    QCOMPARE(combos.last().toString(), QStringLiteral("ctrl+e"));
}

void SlateTests::settingsRoundTrip()
{
    TabletSettings settings;
    settings.output = QStringLiteral("HDMI-A-1");
    settings.transform = 2;
    settings.leftHanded = true;
    settings.activeX = 12.5;
    settings.activeY = 3;
    settings.activeW = 200.25;
    settings.activeH = 100;
    settings.regionX = 100;
    settings.regionY = 50;
    settings.regionW = 800;
    settings.regionH = 600;
    settings.absoluteRegionPosition = true;
    settings.pressureMin = 120;
    settings.pressureMax = 7000;

    const QString lua = SlateSettings::block(settings);
    QVERIFY(lua.contains(SlateSettings::kBeginMarker));
    QVERIFY(lua.contains(SlateSettings::kEndMarker));

    TabletSettings parsed;
    QVERIFY(SlateSettings::parse(lua, &parsed));
    QCOMPARE(parsed, settings);
}

void SlateTests::settingsParsesEscapedOutput()
{
    TabletSettings settings;
    settings.output = QStringLiteral("A\"B\\C");
    const QString lua = SlateSettings::block(settings);

    TabletSettings parsed;
    QVERIFY(SlateSettings::parse(lua, &parsed));
    QCOMPARE(parsed.output, settings.output);
}

void SlateTests::settingsUpsertPreservesSurroundings()
{
    const QString before = QStringLiteral("-- keep me\nhl.config({ input = {} })\n");
    const QString after = QStringLiteral("-- and me\n");

    TabletSettings settings;
    settings.output = QStringLiteral("eDP-1");
    const QString inserted = SlateSettings::upsert(before + after, settings);
    QVERIFY(inserted.startsWith(before + after));
    QVERIFY(inserted.endsWith(SlateSettings::kEndMarker + QStringLiteral("\n")));
    QVERIFY(inserted.contains(SlateSettings::kBeginMarker));

    TabletSettings parsed;
    QVERIFY(SlateSettings::parse(inserted, &parsed));
    QCOMPARE(parsed.output, QStringLiteral("eDP-1"));
}

void SlateTests::settingsUpsertReplacesExistingBlock()
{
    TabletSettings first;
    first.output = QStringLiteral("current");
    TabletSettings second;
    second.output = QStringLiteral("HDMI-A-1");
    second.transform = 1;

    const QString original = QStringLiteral("top\n") + SlateSettings::block(first) + QStringLiteral("\nbottom\n");
    const QString updated = SlateSettings::upsert(original, second);

    QCOMPARE(updated.count(SlateSettings::kBeginMarker), 1);
    QVERIFY(updated.startsWith(QStringLiteral("top\n")));
    QVERIFY(updated.endsWith(QStringLiteral("bottom\n")));

    TabletSettings parsed;
    QVERIFY(SlateSettings::parse(updated, &parsed));
    QCOMPARE(parsed, second);
}

void SlateTests::settingsRemove()
{
    TabletSettings settings;
    const QString original = QStringLiteral("before\n\n") + SlateSettings::block(settings)
        + QStringLiteral("\n\nafter\n");
    QCOMPARE(SlateSettings::remove(original), QStringLiteral("before\n\nafter\n"));
    QCOMPARE(SlateSettings::remove(QStringLiteral("plain\n")), QStringLiteral("plain\n"));
}

void SlateTests::settingsRejectsMissingBlock()
{
    TabletSettings settings;
    QVERIFY(!SlateSettings::parse(QStringLiteral("hl.config({})\n"), &settings));
}

void SlateTests::defaultsArePassThrough()
{
    const QVector<KeyBinding> bindings = SlateBindings::defaults();
    QCOMPARE(bindings.size(), 12);
    for (const KeyBinding &binding : bindings) {
        QCOMPARE(binding.action, QStringLiteral("pass"));
        QVERIFY(binding.value.isEmpty());
        QVERIFY(!binding.combo.isEmpty());
    }
}

void SlateTests::bindingsRoundTrip()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("keys.json"));

    QVector<KeyBinding> bindings = SlateBindings::defaults();
    bindings[0].action = QStringLiteral("keys");
    bindings[0].value = QStringLiteral("ctrl+shift+t");
    bindings[1].action = QStringLiteral("command");
    bindings[1].value = QStringLiteral("  notify-send hello  ");
    bindings[2].action = QStringLiteral("nothing");

    QString error;
    QVERIFY2(SlateBindings::save(path, bindings, &error), qPrintable(error));

    const QVector<KeyBinding> loaded = SlateBindings::load(path, &error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QCOMPARE(loaded.size(), bindings.size());
    QCOMPARE(loaded[0].action, QStringLiteral("keys"));
    QCOMPARE(loaded[0].value, QStringLiteral("ctrl+shift+t"));
    QCOMPARE(loaded[1].action, QStringLiteral("command"));
    QCOMPARE(loaded[1].value, QStringLiteral("notify-send hello"));
    QCOMPARE(loaded[2].action, QStringLiteral("nothing"));
    QVERIFY(loaded[2].value.isEmpty());
}

void SlateTests::bindingsRoundTripMouse()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("keys.json"));

    QVector<KeyBinding> bindings = SlateBindings::defaults();
    bindings[0].action = QStringLiteral("mouse");
    bindings[0].value = QStringLiteral("left");
    bindings[1].action = QStringLiteral("mouse");
    bindings[1].value = QStringLiteral("wheelup");

    QString error;
    QVERIFY2(SlateBindings::save(path, bindings, &error), qPrintable(error));

    const QVector<KeyBinding> loaded = SlateBindings::load(path, &error);
    QVERIFY2(error.isEmpty(), qPrintable(error));
    QCOMPARE(loaded[0].action, QStringLiteral("mouse"));
    QCOMPARE(loaded[0].value, QStringLiteral("left"));
    QCOMPARE(loaded[1].value, QStringLiteral("wheelup"));
    QCOMPARE(SlateBindings::mouseActionLabel(loaded[1].value), QStringLiteral("Scroll up"));
}

void SlateTests::bindingsRejectInvalidMouseAction()
{
    QTemporaryDir dir;
    QVector<KeyBinding> bindings = SlateBindings::defaults();
    bindings[0].action = QStringLiteral("mouse");
    bindings[0].value = QStringLiteral("doubleclick");

    QString error;
    QVERIFY(!SlateBindings::save(dir.filePath(QStringLiteral("keys.json")), bindings, &error));
    QVERIFY(error.contains(QStringLiteral("mouse button")));
}

void SlateTests::bindingsRejectInvalidShortcut()
{
    QTemporaryDir dir;
    QVector<KeyBinding> bindings = SlateBindings::defaults();
    bindings[0].action = QStringLiteral("keys");
    bindings[0].value = QStringLiteral("ctrl+nope");

    QString error;
    QVERIFY(!SlateBindings::save(dir.filePath(QStringLiteral("keys.json")), bindings, &error));
    QVERIFY(error.contains(QStringLiteral("Ctrl + C")));
}

void SlateTests::bindingsRejectDuplicates()
{
    QTemporaryDir dir;
    QVector<KeyBinding> bindings = SlateBindings::defaults();
    bindings[1].combo = bindings[0].combo;

    QString error;
    QVERIFY(!SlateBindings::save(dir.filePath(QStringLiteral("keys.json")), bindings, &error));
    QVERIFY(error.contains(QStringLiteral("Duplicate")));
}

void SlateTests::bindingsFallBackForBadFile()
{
    QTemporaryDir dir;
    const QString path = dir.filePath(QStringLiteral("keys.json"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("{ not json");
    file.close();

    QString error;
    const QVector<KeyBinding> loaded = SlateBindings::load(path, &error);
    QCOMPARE(loaded.size(), 12);
    QCOMPARE(loaded[0].action, QStringLiteral("pass"));
    QVERIFY(!error.isEmpty());
}

void SlateTests::readsAccentFromTheme()
{
    QTemporaryDir dir;
    QVERIFY(dir.isValid());
    const QString path = dir.filePath(QStringLiteral("colors.toml"));
    QFile file(path);
    QVERIFY(file.open(QIODevice::WriteOnly));
    file.write("accent = \"#12AB34\"\n");
    file.close();

    QCOMPARE(Theme::readAccent(path), QStringLiteral("#12AB34"));
}

void SlateTests::fallsBackWithoutTheme()
{
    QCOMPARE(Theme::readAccent(QStringLiteral("/nonexistent/colors.toml")), QStringLiteral("#FFD60A"));
}

QTEST_GUILESS_MAIN(SlateTests)

#include "test_main.moc"
