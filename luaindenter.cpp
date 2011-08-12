#include "luaindenter.h"

#include "luaeditor.h"
#include <texteditor/tabsettings.h>

using namespace PremakeProjectManager;

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

void LuaIndenter::indentBlock(QTextDocument *doc, const QTextBlock &block, const QChar &typedChar, TextEditor::BaseTextEditorWidget *editor)
{
    const QString blockText = block.text();//.trimmed();
    if ((typedChar == 'd' && !blockText.endsWith("end"))
     || (typedChar == 'e' && !blockText.endsWith("else"))
     || (typedChar == 'l' && !blockText.endsWith("until"))
    ) {
        return;
    }


    const TextEditor::TabSettings &ts = editor->tabSettings();

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
    int midx = previousText.indexOf(QRegExp("^\\s*\\b(if|for|while|repeat|else|elseif|do|then)\\b"));
    if (midx == -1) {
        midx = previousText.indexOf(QRegExp("\\{\\s*$"));
        if (midx == -1) {
            midx = previousText.indexOf(QRegExp("\\bfunction\\b\\s*"));
        }
    }
    if(midx != -1) {
        // Increase indent if what we found previously is not in a comment and
        // an "end" or "until" is not present on the same line.
        if (!previousText.contains(QRegExp("\\b(end|until)\\b"))) {
            ts.reindentLine(block, ts.m_indentSize);
        }
    }

    // Decrease indent on end, else (and elseif), until and '}'
    if (blockText.contains(QRegExp("^\\s*\\b(end|else|until)\\b"))
            || blockText.trimmed() == "}") {
        ts.reindentLine(block, -ts.m_indentSize);
    }
}
