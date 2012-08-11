#ifndef LUACOMPLETER_H
#define LUACOMPLETER_H

#include <texteditor/autocompleter.h>

namespace LuaSupport {

class LuaCompleter : public TextEditor::AutoCompleter
{
public:
    LuaCompleter();
    ~LuaCompleter();

    bool contextAllowsAutoParentheses(const QTextCursor &cursor,
                                              const QString &textToInsert = QString()) const;
    bool contextAllowsElectricCharacters(const QTextCursor &cursor) const;
    bool isInComment(const QTextCursor &cursor) const;
    QString insertMatchingBrace(const QTextCursor &cursor,
                                        const QString &text,
                                        QChar la,
                                        int *skippedChars) const;
    QString insertParagraphSeparator(const QTextCursor &cursor) const;
};

}

#endif // LUACOMPLETER_H
