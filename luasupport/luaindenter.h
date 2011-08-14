#ifndef LUAINDENTER_H
#define LUAINDENTER_H

#include <texteditor/indenter.h>

namespace LuaSupport {

class LuaIndenter : public TextEditor::Indenter
{
public:
    LuaIndenter();
    virtual ~LuaIndenter();

    virtual bool isElectricCharacter(const QChar &ch) const;

    virtual void indentBlock(QTextDocument *doc,
                             const QTextBlock &block,
                             const QChar &typedChar,
                             TextEditor::BaseTextEditorWidget *editor);

};

}

#endif // LUAINDENTER_H
