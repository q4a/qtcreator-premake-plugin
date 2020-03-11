#ifndef LUAINDENTER_H
#define LUAINDENTER_H

#include <texteditor/textindenter.h>
#include <texteditor/tabsettingswidget.h>

namespace TextEditor {
class TabSettingsWidget;
}

namespace CppTools {
    class CppCodeStyleSettings;
    class CppCodeStylePreferences;
}

namespace LuaSupport {
namespace Internal {

class LuaIndenter : public TextEditor::TextIndenter
{
public:
    explicit LuaIndenter(QTextDocument *doc);
    ~LuaIndenter();

    bool isElectricCharacter(const QChar &ch) const;

    void indentBlock(QTextDocument *doc,
                             const QTextBlock &block,
                             const QChar &typedChar,
                             TextEditor::TabSettingsWidget *editor);
    void indentBlock(QTextDocument *doc,
                             const QTextBlock &block,
                             const QChar &typedChar,
                             const TextEditor::TabSettings &tabSettings);
};

}
}

#endif // LUAINDENTER_H
