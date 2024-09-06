// Copyright (c) 2024-2024 Manuel Schneider

#include "desktopentryparser.h"
#include <QFile>
#include <albert/logging.h>
using namespace std;

DesktopEntryParser::DesktopEntryParser(const QString &path)
{
    if (QFile file(path); file.open(QIODevice::ReadOnly| QIODevice::Text))
    {
        QTextStream stream(&file);
        QString currentGroup;
        for (QString line=stream.readLine(); !line.isNull(); line=stream.readLine())
        {
            line = line.trimmed();

            if (line.startsWith('#') || line.isEmpty())
                continue;

            if (line.startsWith("["))
            {
                currentGroup = line.mid(1,line.size()-2).trimmed();
                continue;
            }

            data[currentGroup].emplace(line.section('=', 0,0).trimmed(),
                                       line.section('=', 1, -1).trimmed());
        }
        file.close();
    }
    else
        throw runtime_error(QString("Failed opening file '%1': %2").arg(path, file.errorString()).toStdString());
}

QString DesktopEntryParser::getRawValue(const QString &section, const QString &key) const
{
    class SectionDoesNotExist : public std::out_of_range { using out_of_range::out_of_range; };
    class KeyDoesNotExist : public std::out_of_range { using out_of_range::out_of_range; };

    try {
        auto &s = data.at(section);
        try {
            return s.at(key);
        } catch (const out_of_range&) {
            throw KeyDoesNotExist(QString("Section '%1' does not contain a key '%2'.")
                                  .arg(section, key).toStdString());
        }
    } catch (const out_of_range&) {
        throw SectionDoesNotExist(QString("Desktop entry does not contain a section '%1'.")
                                  .arg(section).toStdString());
    }
}

QString DesktopEntryParser::getEscapedValue(const QString &section, const QString &key) const
{
    QString result;

    auto unescaped = getRawValue(section, key);
    for (auto it = unescaped.cbegin(); it != unescaped.cend();)
    {
        if (*it == '\\'){
            ++it;
            if (it == unescaped.cend())
                break;
            else if (*it=='s')
                result.append(' ');
            else if (*it=='n')
                result.append('\n');
            else if (*it=='t')
                result.append('\t');
            else if (*it=='r')
                result.append('\r');
            else if (*it=='\\')
                result.append('\\');
        }
        else
            result.append(*it);
        ++it;
    }

    return result;
}

QString DesktopEntryParser::getString(const QString &section, const QString &key) const
{
    return getEscapedValue(section, key);
}

QString DesktopEntryParser::getLocaleString(const QString &section, const QString &key)
{
    // https://wiki.ubuntu.com/UbuntuDevelopment/Internationalisation/Packaging#Desktop_Entries


    // TODO: Properly fetch the localestring
    //       (lang_COUNTRY@MODIFIER, lang_COUNTRY, lang@MODIFIER, lang, default value)

    try {
        return getEscapedValue(section, QString("%1[%2]").arg(key, locale.name()));
    } catch (const out_of_range&) { }

    try {
        return getEscapedValue(section, QString("%1[%2]").arg(key, locale.name().left(2)));
    } catch (const out_of_range&) { }

    QString unlocalized = getEscapedValue(section, key);

    try {
        auto domain = getEscapedValue(section, QStringLiteral("X-Ubuntu-Gettext-Domain"));
        // The resulting string is statically allocated and must not be modified or freed
        // Returns msgid on lookup failure
        // https://linux.die.net/man/3/dgettext
        return QString::fromUtf8(dgettext(domain.toStdString().c_str(),
                                          unlocalized.toStdString().c_str()));
    } catch (const out_of_range&) { }

    return unlocalized;
}

QString DesktopEntryParser::getIconString(const QString &section, const QString &key)
{
    return getEscapedValue(section, key);
}

bool DesktopEntryParser::getBoolean(const QString &section, const QString &key)
{
    auto raw = getRawValue(section, key);  // throws
    if (raw == QStringLiteral("true"))
        return true;
    else if (raw == QStringLiteral("false"))
        return false;
    else
        throw runtime_error(QString("Value for key '%1' in section '%2' is neither true nor false.")
                              .arg(key, section).toStdString());
}

double DesktopEntryParser::getNumeric(const QString &, const QString &)
{
    throw runtime_error("Not implemented.");
}

optional<QStringList> DesktopEntryParser::splitExec(const QString &s) noexcept
{
    QStringList tokens;
    QString token;
    auto c = s.begin();

    while (c != s.end())
    {
        if (*c == QChar::Space)  // separator
        {
            if (!token.isEmpty())
            {
                tokens << token;
                token.clear();
            }
        }

        else if (*c == '"')  // quote
        {
            ++c;

            while (c != s.end())
            {
                if (*c == '"')  // quote termination
                    break;

                else if (*c == '\\')  // escape
                {
                    ++c;
                    if(c == s.end())
                    {
                        WARN << QString("Unterminated escape in %1").arg(s);
                        return {};  // unterminated escape
                    }

                    else if (QStringLiteral(R"("`$\)").contains(*c))
                        token.append(*c);

                    else
                    {
                        WARN << QString("Invalid escape '%1' at '%2': %3")
                                    .arg(*c).arg(distance(c, s.begin())).arg(s);
                        return {};  // invalid escape
                    }
                }

                else
                    token.append(*c);  // regular char

                ++c;
            }

            if (c == s.end())
            {
                WARN << QString("Unterminated escape in %1").arg(s);
                return {};  // unterminated quote
            }
        }

        else
            token.append(*c);  // regular char

        ++c;

    }

    if (!token.isEmpty())
        tokens << token;

    return tokens;
}
