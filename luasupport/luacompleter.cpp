#include "luacompleter.h"

#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>

using namespace LuaSupport;

LuaCompleter::LuaCompleter()
{
    m_matcher.addBraceCharPair(QLatin1Char('('), QLatin1Char(')'));
    m_matcher.addBraceCharPair(QLatin1Char('['), QLatin1Char(']'));
    m_matcher.addBraceCharPair(QLatin1Char('{'), QLatin1Char('}'));
    m_matcher.addQuoteChar(QLatin1Char('"'));
    m_matcher.addQuoteChar(QLatin1Char('\''));
    m_matcher.addDelimiterChar(QLatin1Char(','));
    m_matcher.addDelimiterChar(QLatin1Char(';'));
}

LuaCompleter::~LuaCompleter()
{}

bool LuaCompleter::contextAllowsAutoParentheses(const QTextCursor &cursor, const QString &textToInsert) const
{
    if (textToInsert.isEmpty() || isInComment(cursor))
        return false;

    return m_matcher.isKnownChar(textToInsert.at(0)); //shouldInsertMatchingText(textToInsert.at(0));
}

bool LuaCompleter::contextAllowsElectricCharacters(const QTextCursor &cursor) const
{
    return !isInComment(cursor);
}

bool LuaCompleter::isInComment(const QTextCursor &cursor) const
{
    // FIXME: Join with luaeditor.cpp
    // FIXME: Detect multiline comments
    const QTextBlock block = cursor.block();
    const int hashPos = block.text().indexOf(QLatin1String("--"));
    if (hashPos >= 0 && hashPos < cursor.position() - block.position())
        return true;

    return false;
}

QString LuaCompleter::insertMatchingBrace(const QTextCursor &cursor, const QString &text, QChar la, int *skippedChars) const
{
//    MatchingText m;
    return m_matcher.insertMatchingBrace(cursor, text, la, skippedChars);
}

//QString LuaCompleter::insertParagraphSeparator(const QTextCursor &cursor) const
//{
//    MatchingText m;
//    return m_matcher.insertParagraphSeparator(cursor);
//}
