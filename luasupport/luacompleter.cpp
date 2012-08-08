#include "luacompleter.h"

#include "future/utils/bracematcher.h"

#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>

Q_GLOBAL_STATIC_WITH_INITIALIZER(Utils::BraceMatcher, braceMatcher, {
    x->addBraceCharPair(QLatin1Char('('), QLatin1Char(')'));
    x->addBraceCharPair(QLatin1Char('['), QLatin1Char(']'));
    x->addQuoteChar(QLatin1Char('"'));
    x->addQuoteChar(QLatin1Char('\''));
    x->addDelimiterChar(QLatin1Char(','));
    x->addDelimiterChar(QLatin1Char(';'));
})

using namespace LuaSupport;

LuaCompleter::LuaCompleter()
{
}

LuaCompleter::~LuaCompleter()
{}

bool LuaCompleter::contextAllowsAutoParentheses(const QTextCursor &cursor, const QString &textToInsert) const
{
    if (textToInsert.isEmpty() || isInComment(cursor))
        return false;

    return braceMatcher()->isKnownChar(textToInsert.at(0));
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
    return braceMatcher()->insertMatchingBrace(cursor, text, la, skippedChars);
}

//QString LuaCompleter::insertParagraphSeparator(const QTextCursor &cursor) const
//{
//    MatchingText m;
//    return m_matcher.insertParagraphSeparator(cursor);
//}
