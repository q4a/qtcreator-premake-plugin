#include "luacompleter.h"

#include <cplusplus/MatchingText.h>
#include <QDebug>

using namespace PremakeProjectManager;
using namespace CPlusPlus;

LuaCompleter::LuaCompleter()
{}

LuaCompleter::~LuaCompleter()
{}

bool LuaCompleter::contextAllowsAutoParentheses(const QTextCursor &cursor, const QString &textToInsert) const
{
    QChar ch;

    if (! textToInsert.isEmpty())
        ch = textToInsert.at(0);

    if (! (MatchingText::shouldInsertMatchingText(cursor)
           || ch == QLatin1Char('\'')
           || ch == QLatin1Char('"')))
        return false;
    else if (isInComment(cursor))
        return false;

    return true;
}

bool LuaCompleter::contextAllowsElectricCharacters(const QTextCursor &cursor) const
{
    Q_UNUSED(cursor)
    return true;
}

bool LuaCompleter::isInComment(const QTextCursor &cursor) const
{
    return false;
}

QString LuaCompleter::insertMatchingBrace(const QTextCursor &cursor, const QString &text, QChar la, int *skippedChars) const
{
    MatchingText m;
    return m.insertMatchingBrace(cursor, text, la, skippedChars);
}

QString LuaCompleter::insertParagraphSeparator(const QTextCursor &cursor) const
{
    MatchingText m;
    return m.insertParagraphSeparator(cursor);
}






