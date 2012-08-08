#ifndef LUACOMPLETER_H
#define LUACOMPLETER_H

#include <texteditor/autocompleter.h>

namespace LuaSupport {

class LuaCompleter : public TextEditor::AutoCompleter
{
public:
    LuaCompleter();
    virtual ~LuaCompleter();

    virtual bool contextAllowsAutoParentheses(const QTextCursor &cursor,
                                              const QString &textToInsert = QString()) const;
    virtual bool contextAllowsElectricCharacters(const QTextCursor &cursor) const;
    virtual bool isInComment(const QTextCursor &cursor) const;
    virtual QString insertMatchingBrace(const QTextCursor &cursor,
                                        const QString &text,
                                        QChar la,
                                        int *skippedChars) const;
//    virtual QString insertParagraphSeparator(const QTextCursor &cursor) const;
};

}

#endif // LUACOMPLETER_H
