#include "keymap.h"

#include <QHash>
#include <QRegularExpression>

#include <algorithm>
#include <linux/input-event-codes.h>

namespace SlateKeys {
namespace {

const QStringList kModifierOrder{"ctrl", "shift", "alt", "super"};

QString modifierLabel(const QString &modifier)
{
    if (modifier == "ctrl")
        return QStringLiteral("Ctrl");
    if (modifier == "shift")
        return QStringLiteral("Shift");
    if (modifier == "alt")
        return QStringLiteral("Alt");
    if (modifier == "super")
        return QStringLiteral("Super");
    return modifier;
}

const QHash<int, QString> &keyTable()
{
    static const QHash<int, QString> table = [] {
        QHash<int, QString> keys;
        auto add = [&keys](int code, const char *token) { keys.insert(code, QString::fromLatin1(token)); };

        add(KEY_A, "a");
        add(KEY_B, "b");
        add(KEY_C, "c");
        add(KEY_D, "d");
        add(KEY_E, "e");
        add(KEY_F, "f");
        add(KEY_G, "g");
        add(KEY_H, "h");
        add(KEY_I, "i");
        add(KEY_J, "j");
        add(KEY_K, "k");
        add(KEY_L, "l");
        add(KEY_M, "m");
        add(KEY_N, "n");
        add(KEY_O, "o");
        add(KEY_P, "p");
        add(KEY_Q, "q");
        add(KEY_R, "r");
        add(KEY_S, "s");
        add(KEY_T, "t");
        add(KEY_U, "u");
        add(KEY_V, "v");
        add(KEY_W, "w");
        add(KEY_X, "x");
        add(KEY_Y, "y");
        add(KEY_Z, "z");

        add(KEY_0, "0");
        add(KEY_1, "1");
        add(KEY_2, "2");
        add(KEY_3, "3");
        add(KEY_4, "4");
        add(KEY_5, "5");
        add(KEY_6, "6");
        add(KEY_7, "7");
        add(KEY_8, "8");
        add(KEY_9, "9");

        add(KEY_F1, "f1");
        add(KEY_F2, "f2");
        add(KEY_F3, "f3");
        add(KEY_F4, "f4");
        add(KEY_F5, "f5");
        add(KEY_F6, "f6");
        add(KEY_F7, "f7");
        add(KEY_F8, "f8");
        add(KEY_F9, "f9");
        add(KEY_F10, "f10");
        add(KEY_F11, "f11");
        add(KEY_F12, "f12");
        add(KEY_F13, "f13");
        add(KEY_F14, "f14");
        add(KEY_F15, "f15");
        add(KEY_F16, "f16");
        add(KEY_F17, "f17");
        add(KEY_F18, "f18");
        add(KEY_F19, "f19");
        add(KEY_F20, "f20");
        add(KEY_F21, "f21");
        add(KEY_F22, "f22");
        add(KEY_F23, "f23");
        add(KEY_F24, "f24");

        add(KEY_ESC, "escape");
        add(KEY_TAB, "tab");
        add(KEY_ENTER, "enter");
        add(KEY_SPACE, "space");
        add(KEY_BACKSPACE, "backspace");
        add(KEY_DELETE, "delete");
        add(KEY_INSERT, "insert");
        add(KEY_HOME, "home");
        add(KEY_END, "end");
        add(KEY_PAGEUP, "pageup");
        add(KEY_PAGEDOWN, "pagedown");
        add(KEY_LEFT, "left");
        add(KEY_RIGHT, "right");
        add(KEY_UP, "up");
        add(KEY_DOWN, "down");

        add(KEY_MINUS, "minus");
        add(KEY_EQUAL, "equal");
        add(KEY_LEFTBRACE, "bracketleft");
        add(KEY_RIGHTBRACE, "bracketright");
        add(KEY_BACKSLASH, "backslash");
        add(KEY_SEMICOLON, "semicolon");
        add(KEY_APOSTROPHE, "apostrophe");
        add(KEY_GRAVE, "grave");
        add(KEY_COMMA, "comma");
        add(KEY_DOT, "period");
        add(KEY_SLASH, "slash");
        return keys;
    }();
    return table;
}

const QSet<QString> &knownKeys()
{
    static const QSet<QString> keys = [] {
        QSet<QString> set;
        for (auto it = keyTable().cbegin(); it != keyTable().cend(); ++it)
            set.insert(it.value());
        return set;
    }();
    return keys;
}

QString keyLabel(const QString &key)
{
    if (key.size() == 1)
        return key.toUpper();
    if (key.startsWith('f') && key.mid(1).toInt() > 0)
        return key.toUpper();
    if (key == "pageup")
        return QStringLiteral("Page Up");
    if (key == "pagedown")
        return QStringLiteral("Page Down");
    if (key == "escape")
        return QStringLiteral("Esc");
    if (key == "enter")
        return QStringLiteral("Enter");
    if (key == "space")
        return QStringLiteral("Space");
    if (key == "backspace")
        return QStringLiteral("Backspace");
    if (key == "delete")
        return QStringLiteral("Delete");
    if (key == "insert")
        return QStringLiteral("Insert");
    if (key == "home")
        return QStringLiteral("Home");
    if (key == "end")
        return QStringLiteral("End");
    if (key == "up" || key == "down" || key == "left" || key == "right")
        return key.left(1).toUpper() + key.mid(1);
    if (key == "minus")
        return QStringLiteral("Minus");
    if (key == "equal")
        return QStringLiteral("Equal");
    if (key == "bracketleft")
        return QStringLiteral("[");
    if (key == "bracketright")
        return QStringLiteral("]");
    if (key == "backslash")
        return QStringLiteral("\\");
    if (key == "semicolon")
        return QStringLiteral(";");
    if (key == "apostrophe")
        return QStringLiteral("'");
    if (key == "grave")
        return QStringLiteral("`");
    if (key == "comma")
        return QStringLiteral(",");
    if (key == "period")
        return QStringLiteral(".");
    if (key == "slash")
        return QStringLiteral("/");
    return key;
}

QString wtypeKey(const QString &key)
{
    if (key.size() == 1)
        return key;
    if (key.startsWith('f') && key.mid(1).toInt() > 0)
        return key.toUpper();
    static const QHash<QString, QString> names{
        {QStringLiteral("escape"), QStringLiteral("Escape")},
        {QStringLiteral("tab"), QStringLiteral("Tab")},
        {QStringLiteral("enter"), QStringLiteral("Return")},
        {QStringLiteral("space"), QStringLiteral("space")},
        {QStringLiteral("backspace"), QStringLiteral("BackSpace")},
        {QStringLiteral("delete"), QStringLiteral("Delete")},
        {QStringLiteral("insert"), QStringLiteral("Insert")},
        {QStringLiteral("home"), QStringLiteral("Home")},
        {QStringLiteral("end"), QStringLiteral("End")},
        {QStringLiteral("pageup"), QStringLiteral("Page_Up")},
        {QStringLiteral("pagedown"), QStringLiteral("Page_Down")},
        {QStringLiteral("left"), QStringLiteral("Left")},
        {QStringLiteral("right"), QStringLiteral("Right")},
        {QStringLiteral("up"), QStringLiteral("Up")},
        {QStringLiteral("down"), QStringLiteral("Down")},
        {QStringLiteral("minus"), QStringLiteral("minus")},
        {QStringLiteral("equal"), QStringLiteral("equal")},
        {QStringLiteral("bracketleft"), QStringLiteral("bracketleft")},
        {QStringLiteral("bracketright"), QStringLiteral("bracketright")},
        {QStringLiteral("backslash"), QStringLiteral("backslash")},
        {QStringLiteral("semicolon"), QStringLiteral("semicolon")},
        {QStringLiteral("apostrophe"), QStringLiteral("apostrophe")},
        {QStringLiteral("grave"), QStringLiteral("grave")},
        {QStringLiteral("comma"), QStringLiteral("comma")},
        {QStringLiteral("period"), QStringLiteral("period")},
        {QStringLiteral("slash"), QStringLiteral("slash")},
    };
    return names.value(key);
}

void addModifier(Combo *combo, const QString &modifier)
{
    if (modifier.isEmpty() || combo->modifiers.contains(modifier))
        return;
    combo->modifiers.append(modifier);
    std::sort(combo->modifiers.begin(), combo->modifiers.end(), [](const QString &a, const QString &b) {
        return kModifierOrder.indexOf(a) < kModifierOrder.indexOf(b);
    });
}

} // namespace

QString Combo::toString() const
{
    if (!isValid())
        return {};
    QStringList parts = modifiers;
    parts.append(key);
    return parts.join('+');
}

QString keyToken(int evdevCode)
{
    return keyTable().value(evdevCode);
}

QString modifierForCode(int evdevCode)
{
    switch (evdevCode) {
    case KEY_LEFTCTRL:
    case KEY_RIGHTCTRL:
        return QStringLiteral("ctrl");
    case KEY_LEFTSHIFT:
    case KEY_RIGHTSHIFT:
        return QStringLiteral("shift");
    case KEY_LEFTALT:
    case KEY_RIGHTALT:
        return QStringLiteral("alt");
    case KEY_LEFTMETA:
    case KEY_RIGHTMETA:
        return QStringLiteral("super");
    default:
        return {};
    }
}

QString normalizeModifierToken(const QString &token)
{
    const QString normalized = token.trimmed().toLower();
    if (normalized == "ctrl" || normalized == "control")
        return QStringLiteral("ctrl");
    if (normalized == "shift")
        return QStringLiteral("shift");
    if (normalized == "alt" || normalized == "option" || normalized == "opt")
        return QStringLiteral("alt");
    if (normalized == "super" || normalized == "meta" || normalized == "win"
        || normalized == "logo" || normalized == "mod4")
        return QStringLiteral("super");
    return {};
}

QString normalizeKeyToken(const QString &token)
{
    QString normalized = token.trimmed().toLower();
    if (normalized.startsWith("key_"))
        normalized.remove(0, 4);
    normalized.replace(' ', '_');

    static const QHash<QString, QString> aliases{
        {QStringLiteral("esc"), QStringLiteral("escape")},
        {QStringLiteral("return"), QStringLiteral("enter")},
        {QStringLiteral("spacebar"), QStringLiteral("space")},
        {QStringLiteral("back_space"), QStringLiteral("backspace")},
        {QStringLiteral("bksp"), QStringLiteral("backspace")},
        {QStringLiteral("del"), QStringLiteral("delete")},
        {QStringLiteral("ins"), QStringLiteral("insert")},
        {QStringLiteral("pgup"), QStringLiteral("pageup")},
        {QStringLiteral("page_up"), QStringLiteral("pageup")},
        {QStringLiteral("prior"), QStringLiteral("pageup")},
        {QStringLiteral("pgdn"), QStringLiteral("pagedown")},
        {QStringLiteral("page_down"), QStringLiteral("pagedown")},
        {QStringLiteral("next"), QStringLiteral("pagedown")},
        {QStringLiteral("arrowup"), QStringLiteral("up")},
        {QStringLiteral("arrow_up"), QStringLiteral("up")},
        {QStringLiteral("arrowdown"), QStringLiteral("down")},
        {QStringLiteral("arrow_down"), QStringLiteral("down")},
        {QStringLiteral("arrowleft"), QStringLiteral("left")},
        {QStringLiteral("arrow_left"), QStringLiteral("left")},
        {QStringLiteral("arrowright"), QStringLiteral("right")},
        {QStringLiteral("arrow_right"), QStringLiteral("right")},
        {QStringLiteral("dot"), QStringLiteral("period")},
        {QStringLiteral("-"), QStringLiteral("minus")},
        {QStringLiteral("="), QStringLiteral("equal")},
        {QStringLiteral("["), QStringLiteral("bracketleft")},
        {QStringLiteral("]"), QStringLiteral("bracketright")},
        {QStringLiteral("\\"), QStringLiteral("backslash")},
        {QStringLiteral(";"), QStringLiteral("semicolon")},
        {QStringLiteral("'"), QStringLiteral("apostrophe")},
        {QStringLiteral("`"), QStringLiteral("grave")},
        {QStringLiteral(","), QStringLiteral("comma")},
        {QStringLiteral("."), QStringLiteral("period")},
        {QStringLiteral("/"), QStringLiteral("slash")},
    };
    normalized = aliases.value(normalized, normalized);
    return knownKeys().contains(normalized) ? normalized : QString();
}

QString displayName(const Combo &combo)
{
    if (!combo.isValid())
        return {};
    QStringList parts;
    for (const QString &modifier : combo.modifiers)
        parts.append(modifierLabel(modifier));
    parts.append(keyLabel(combo.key));
    return parts.join(QStringLiteral(" + "));
}

QString displayName(const QString &combo)
{
    Combo parsed;
    if (!parseCombo(combo, &parsed))
        return combo;
    return displayName(parsed);
}

QStringList wtypeArgs(const Combo &combo)
{
    QStringList args;
    for (const QString &modifier : combo.modifiers) {
        if (modifier == "super")
            args << QStringLiteral("-M") << QStringLiteral("logo");
        else
            args << QStringLiteral("-M") << modifier;
    }
    args << QStringLiteral("-k") << wtypeKey(combo.key);
    for (int i = combo.modifiers.size() - 1; i >= 0; --i) {
        const QString &modifier = combo.modifiers.at(i);
        args << QStringLiteral("-m") << (modifier == "super" ? QStringLiteral("logo") : modifier);
    }
    return args;
}

bool parseCombo(const QString &text, Combo *combo, QString *error)
{
    auto fail = [error](const QString &message) {
        if (error)
            *error = message;
        return false;
    };

    if (!combo)
        return false;

    const QString cleaned = text.trimmed();
    if (cleaned.isEmpty())
        return fail(QStringLiteral("Enter a key combination."));

    Combo result;
    const QStringList parts = cleaned.split('+', Qt::SkipEmptyParts);
    for (const QString &part : parts) {
        const QString modifier = normalizeModifierToken(part);
        if (!modifier.isEmpty()) {
            addModifier(&result, modifier);
            continue;
        }
        if (!result.key.isEmpty())
            return fail(QStringLiteral("Only one non-modifier key is allowed."));
        const QString key = normalizeKeyToken(part);
        if (key.isEmpty())
            return fail(QStringLiteral("Unknown key “%1”.").arg(part.trimmed()));
        result.key = key;
    }

    if (result.key.isEmpty())
        return fail(QStringLiteral("Add a key to the combination."));
    *combo = result;
    return true;
}

QVector<Combo> defaultCombos()
{
    static const QStringList defaults{
        QStringLiteral("ctrl+c"),
        QStringLiteral("ctrl+v"),
        QStringLiteral("ctrl+m"),
        QStringLiteral("ctrl+l"),
        QStringLiteral("b"),
        QStringLiteral("f5"),
        QStringLiteral("w"),
        QStringLiteral("e"),
        QStringLiteral("pageup"),
        QStringLiteral("pagedown"),
        QStringLiteral("ctrl+p"),
        QStringLiteral("ctrl+e"),
    };
    QVector<Combo> combos;
    combos.reserve(defaults.size());
    for (const QString &text : defaults) {
        Combo combo;
        if (parseCombo(text, &combo))
            combos.append(combo);
    }
    return combos;
}

std::optional<Combo> ComboTracker::feed(int type, int code, int value)
{
    if (type != EV_KEY)
        return std::nullopt;

    if (value == 0) {
        m_pressed.remove(code);
        return std::nullopt;
    }
    if (value != 1)
        return std::nullopt;
    if (m_pressed.contains(code))
        return std::nullopt;

    const QString modifier = modifierForCode(code);
    if (!modifier.isEmpty()) {
        m_pressed.insert(code);
        return std::nullopt;
    }

    m_pressed.insert(code);
    const QString key = keyToken(code);
    if (key.isEmpty())
        return std::nullopt;

    Combo combo;
    combo.key = key;
    for (int pressed : m_pressed) {
        const QString pressedModifier = modifierForCode(pressed);
        if (!pressedModifier.isEmpty())
            addModifier(&combo, pressedModifier);
    }
    return combo;
}

void ComboTracker::reset()
{
    m_pressed.clear();
}

} // namespace SlateKeys
