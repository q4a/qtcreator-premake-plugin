#ifndef LUAINDENTER_H
#define LUAINDENTER_H

#include <texteditor/indenter.h>

namespace PremakeProjectManager {
namespace Internal {

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

} // namespace Internal
} // namespace PremakeProjectManager

#endif // LUAINDENTER_H
