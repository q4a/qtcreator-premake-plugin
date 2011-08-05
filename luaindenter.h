#ifndef LUAINDENTER_H
#define LUAINDENTER_H

#include <texteditor/normalindenter.h>

namespace PremakeProjectManager {
namespace Internal {

class LuaIndenter : public TextEditor::NormalIndenter
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
