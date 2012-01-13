#include "luaindenter.h"

#include "luaeditor.h"

#include <cpptools/cppcodeformatter.h>
#include <cpptools/cppcodestylepreferences.h>
#include <cpptools/cppcodestylesettings.h>
#include <texteditor/tabsettings.h>

#include <QtCore/QDebug>

using namespace LuaSupport;

LuaIndenter::LuaIndenter()
{}

LuaIndenter::~LuaIndenter()
{}

bool LuaIndenter::isElectricCharacter(const QChar &ch) const
{
    if (ch == QLatin1Char('{')
            || ch == QLatin1Char('}')
            || ch == QLatin1Char('d') // end
            || ch == QLatin1Char('e') // else
            || ch == QLatin1Char('l') // until
            ) {
        return true;
    }
    return false;
}

void LuaIndenter::indentBlock(QTextDocument *doc, const QTextBlock &block,
                              const QChar &typedChar,
                              TextEditor::BaseTextEditorWidget *editor)
{
    indentBlock(doc, block, typedChar, editor->tabSettings());
}

void LuaIndenter::indentBlock(QTextDocument *doc,
                         const QTextBlock &block,
                         const QChar &typedChar,
                         const TextEditor::TabSettings &ts)
{
    const QString blockText = block.text();//.trimmed();
    if ((typedChar == QLatin1Char('d') && !blockText.endsWith(QLatin1String("end")))
     || (typedChar == QLatin1Char('e') && !blockText.endsWith(QLatin1String("else")))
     || (typedChar == QLatin1Char('l') && !blockText.endsWith(QLatin1String("until")))
    ) {
        return;
    }

    // At beginning: Leave as is.
    if (block == doc->begin())
        return;

    QTextBlock previous = block.previous();
    QString previousText = previous.text();
    // Empty line indicates a start of a new paragraph. Leave as is.
    while (previousText.isEmpty() || previousText.trimmed().isEmpty()) {
        previous = previous.previous();
        if (previous == doc->begin())
            return;
        previousText = previous.text();
    }

    int firstNonSpace = ts.firstNonSpace(previousText);

    // Just use previous line.
    ts.indentLine(block, ts.columnAt(previousText, firstNonSpace));

    // Increase indent after lines that start a block:
    // 'function', 'if', 'for', 'while', 'repeat', 'else', 'elseif', '{'
    int midx = previousText.indexOf(QRegExp(QLatin1String("^\\s*\\b(if|for|while|repeat|else|elseif|do|then)\\b")));
    if (midx == -1) {
        midx = previousText.indexOf(QRegExp(QLatin1String("\\{\\s*$")));
        if (midx == -1) {
            midx = previousText.indexOf(QRegExp(QLatin1String("\\bfunction\\b\\s*")));
        }
    }
    if(midx != -1) {
        // Increase indent if what we found previously is not in a comment and
        // an "end" or "until" is not present on the same line.
        if (!previousText.contains(QRegExp(QLatin1String("\\b(end|until)\\b")))) {
            ts.reindentLine(block, ts.m_indentSize);
        }
    }

    // Decrease indent on end, else (and elseif), until and '}'
    if (blockText.contains(QRegExp(QLatin1String("^\\s*\\b(end|else|until)\\b")))
            || blockText.trimmed() == QLatin1String("}")) {
        ts.reindentLine(block, -ts.m_indentSize);
    }
}

/*
TODO: do something useful

CppTools::CppCodeStyleSettings LuaIndenter::codeStyleSettings() const
{
    if (m_luaCodeStylePreferences)
        return m_luaCodeStylePreferences->currentSettings();
    return CppTools::CppCodeStyleSettings();
}


void LuaIndenter::setCodeStylePreferences(TextEditor::ICodeStylePreferences *preferences)
{
    CppTools::CppCodeStylePreferences *luaCodeStylePreferences
            = qobject_cast<CppTools::CppCodeStylePreferences *>(preferences);

    if (luaCodeStylePreferences)
        m_luaCodeStylePreferences = luaCodeStylePreferences;
}
*/

