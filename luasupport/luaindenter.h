#ifndef LUAINDENTER_H
#define LUAINDENTER_H

#include <texteditor/indenter.h>
#include <texteditor/tabsettings.h>

namespace TextEditor {
class BaseTextEditorWidget;
}

namespace CppTools {
    class CppCodeStyleSettings;
    class CppCodeStylePreferences;
}

namespace LuaSupport {

class LuaIndenter : public TextEditor::Indenter
{
public:
    LuaIndenter();
    ~LuaIndenter();

    bool isElectricCharacter(const QChar &ch) const;

    void indentBlock(QTextDocument *doc,
                             const QTextBlock &block,
                             const QChar &typedChar,
                             TextEditor::BaseTextEditorWidget *editor);
    void indentBlock(QTextDocument *doc,
                             const QTextBlock &block,
                             const QChar &typedChar,
                             const TextEditor::TabSettings &tabSettings);
};

}

#endif // LUAINDENTER_H
