#include "luacompleter.h"

#include "future/utils/bracematcher.h"

#include <QTextBlock>
#include <QTextCursor>
#include <QTextDocument>

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

static bool shouldInsertNewline(const QTextCursor &tc)
{
    QTextDocument *doc = tc.document();
    int pos = tc.selectionEnd();

    // count the number of empty lines.
    int newlines = 0;
    for (int e = doc->characterCount(); pos != e; ++pos) {
        const QChar ch = doc->characterAt(pos);

        if (! ch.isSpace())
            break;
        else if (ch == QChar::ParagraphSeparator)
            ++newlines;
    }

    if (newlines <= 1 && doc->characterAt(pos) != QLatin1Char('}'))
        return true;

    return false;
}

QString LuaCompleter::insertParagraphSeparator(const QTextCursor &cursor) const
{
    if (shouldInsertNewline(cursor)) {
        QTextCursor selCursor = cursor;
        selCursor.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
        if (!selCursor.selectedText().trimmed().isEmpty())
            return QString();

        return QLatin1String("}\n");
    }

    return QLatin1String("}");
}
