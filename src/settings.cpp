#include "settings.h"

#include <QRegularExpression>

namespace SlateSettings {

const QString kBeginMarker = QStringLiteral("-- >>> slate: tablet settings >>>");
const QString kEndMarker = QStringLiteral("-- <<< slate: tablet settings <<<");

namespace {

QString number(double value)
{
    if (qFuzzyIsNull(value))
        return QStringLiteral("0");
    QString text = QString::number(value, 'f', 4);
    while (text.endsWith('0'))
        text.chop(1);
    if (text.endsWith('.'))
        text.chop(1);
    return text;
}

QString luaString(QString value)
{
    value.replace('\\', QStringLiteral("\\\\"));
    value.replace('"', QStringLiteral("\\\""));
    return '"' + value + '"';
}

QString unescapeString(QString value)
{
    value.replace(QStringLiteral("\\\""), QStringLiteral("\""));
    value.replace(QStringLiteral("\\\\"), QStringLiteral("\\"));
    return value;
}

QString stripNewlines(QString value, bool front)
{
    while (!value.isEmpty()) {
        const QChar ch = front ? value.front() : value.back();
        if (ch != '\n' && ch != '\r')
            break;
        if (front)
            value.remove(0, 1);
        else
            value.chop(1);
    }
    return value;
}

bool parsePair(const QString &text, double *first, double *second)
{
    static const QRegularExpression pair(R"(^\{\s*([-+0-9.eE]+)\s*,\s*([-+0-9.eE]+)\s*\}$)");
    const auto match = pair.match(text.trimmed());
    if (!match.hasMatch())
        return false;
    bool okFirst = false, okSecond = false;
    const double a = match.captured(1).toDouble(&okFirst);
    const double b = match.captured(2).toDouble(&okSecond);
    if (!okFirst || !okSecond)
        return false;
    *first = a;
    *second = b;
    return true;
}

} // namespace

} // namespace SlateSettings

bool TabletSettings::operator==(const TabletSettings &other) const
{
    return output == other.output
        && transform == other.transform
        && leftHanded == other.leftHanded
        && qFuzzyCompare(activeX + 1, other.activeX + 1)
        && qFuzzyCompare(activeY + 1, other.activeY + 1)
        && qFuzzyCompare(activeW + 1, other.activeW + 1)
        && qFuzzyCompare(activeH + 1, other.activeH + 1)
        && qFuzzyCompare(regionX + 1, other.regionX + 1)
        && qFuzzyCompare(regionY + 1, other.regionY + 1)
        && qFuzzyCompare(regionW + 1, other.regionW + 1)
        && qFuzzyCompare(regionH + 1, other.regionH + 1)
        && absoluteRegionPosition == other.absoluteRegionPosition
        && qFuzzyCompare(pressureMin + 1, other.pressureMin + 1)
        && qFuzzyCompare(pressureMax + 1, other.pressureMax + 1);
}

namespace SlateSettings {

QString block(const TabletSettings &settings)
{
    QStringList lines;
    lines << kBeginMarker;
    lines << QStringLiteral("-- Managed by slate. Edit tablet settings in the app; changes here are overwritten.");
    lines << QStringLiteral("hl.config({");
    lines << QStringLiteral("  input = {");
    lines << QStringLiteral("    tablet = {");
    lines << QStringLiteral("      output = %1,").arg(luaString(settings.output));
    lines << QStringLiteral("      transform = %1,").arg(settings.transform);
    lines << QStringLiteral("      left_handed = %1,").arg(settings.leftHanded ? QStringLiteral("true") : QStringLiteral("false"));
    lines << QStringLiteral("      active_area_size = { %1, %2 },").arg(number(settings.activeW), number(settings.activeH));
    lines << QStringLiteral("      active_area_position = { %1, %2 },").arg(number(settings.activeX), number(settings.activeY));
    lines << QStringLiteral("      region_size = { %1, %2 },").arg(number(settings.regionW), number(settings.regionH));
    lines << QStringLiteral("      region_position = { %1, %2 },").arg(number(settings.regionX), number(settings.regionY));
    lines << QStringLiteral("      absolute_region_position = %1,").arg(settings.absoluteRegionPosition ? QStringLiteral("true") : QStringLiteral("false"));
    lines << QStringLiteral("    },");
    lines << QStringLiteral("    tablettool = {");
    lines << QStringLiteral("      pressure_range_min = %1,").arg(number(settings.pressureMin));
    lines << QStringLiteral("      pressure_range_max = %1,").arg(number(settings.pressureMax));
    lines << QStringLiteral("    },");
    lines << QStringLiteral("  },");
    lines << QStringLiteral("})");
    lines << kEndMarker;
    return lines.join('\n');
}

bool parse(const QString &lua, TabletSettings *settings)
{
    if (!settings)
        return false;

    const int begin = lua.indexOf(kBeginMarker);
    if (begin < 0)
        return false;
    int end = lua.indexOf(kEndMarker, begin);
    if (end < 0)
        return false;
    end += kEndMarker.size();

    TabletSettings parsed;
    const QString blockText = lua.mid(begin, end - begin);
    const QStringList lines = blockText.split('\n');
    static const QRegularExpression assignment(R"(^\s*([A-Za-z_][A-Za-z0-9_]*)\s*=\s*(.*?)\s*,?\s*$)");

    for (const QString &rawLine : lines) {
        QString line = rawLine.trimmed();
        if (line.isEmpty() || line.startsWith(QStringLiteral("--")))
            continue;

        const auto match = assignment.match(line);
        if (!match.hasMatch())
            continue;

        const QString name = match.captured(1);
        QString value = match.captured(2).trimmed();
        if (value.endsWith(','))
            value.chop(1);

        if (name == QStringLiteral("output")) {
            if (value.size() >= 2 && value.startsWith('"') && value.endsWith('"'))
                parsed.output = unescapeString(value.mid(1, value.size() - 2));
            continue;
        }

        if (name == QStringLiteral("left_handed") || name == QStringLiteral("absolute_region_position")) {
            const bool boolean = value == QStringLiteral("true");
            if (name == QStringLiteral("left_handed"))
                parsed.leftHanded = boolean;
            else
                parsed.absoluteRegionPosition = boolean;
            continue;
        }

        if (name == QStringLiteral("transform")) {
            bool ok = false;
            const int transform = value.toInt(&ok);
            if (ok)
                parsed.transform = transform;
            continue;
        }

        double first = 0, second = 0;
        if (name == QStringLiteral("active_area_size") && parsePair(value, &first, &second)) {
            parsed.activeW = first;
            parsed.activeH = second;
        } else if (name == QStringLiteral("active_area_position") && parsePair(value, &first, &second)) {
            parsed.activeX = first;
            parsed.activeY = second;
        } else if (name == QStringLiteral("region_size") && parsePair(value, &first, &second)) {
            parsed.regionW = first;
            parsed.regionH = second;
        } else if (name == QStringLiteral("region_position") && parsePair(value, &first, &second)) {
            parsed.regionX = first;
            parsed.regionY = second;
        } else if (name == QStringLiteral("pressure_range_min")) {
            bool ok = false;
            const double pressure = value.toDouble(&ok);
            if (ok)
                parsed.pressureMin = pressure;
        } else if (name == QStringLiteral("pressure_range_max")) {
            bool ok = false;
            const double pressure = value.toDouble(&ok);
            if (ok)
                parsed.pressureMax = pressure;
        }
    }

    *settings = parsed;
    return true;
}

QString upsert(const QString &lua, const TabletSettings &settings)
{
    const QString generated = block(settings);
    const int begin = lua.indexOf(kBeginMarker);
    if (begin >= 0) {
        int end = lua.indexOf(kEndMarker, begin);
        if (end < 0) {
            // A block without its closing marker is broken; replace it through
            // the end rather than leaving a second copy behind.
            end = lua.size();
        } else {
            end += kEndMarker.size();
        }

        QString prefix = stripNewlines(lua.left(begin), false);
        QString suffix = stripNewlines(lua.mid(end), true);
        QString result = prefix;
        if (!result.isEmpty())
            result += QStringLiteral("\n\n");
        result += generated;
        if (!suffix.isEmpty())
            result += QStringLiteral("\n\n") + suffix;
        if (!result.endsWith('\n'))
            result += '\n';
        return result;
    }

    QString prefix = stripNewlines(lua, false);
    if (!prefix.isEmpty())
        prefix += QStringLiteral("\n\n");
    prefix += generated;
    if (!prefix.endsWith('\n'))
        prefix += '\n';
    return prefix;
}

QString remove(const QString &lua)
{
    const int begin = lua.indexOf(kBeginMarker);
    if (begin < 0)
        return lua;
    int end = lua.indexOf(kEndMarker, begin);
    if (end < 0)
        end = lua.size();
    else
        end += kEndMarker.size();

    QString prefix = stripNewlines(lua.left(begin), false);
    QString suffix = stripNewlines(lua.mid(end), true);
    QString result = prefix;
    if (!result.isEmpty() && !suffix.isEmpty())
        result += QStringLiteral("\n\n");
    result += suffix;
    if (!result.isEmpty() && !result.endsWith('\n'))
        result += '\n';
    return result;
}

} // namespace SlateSettings
