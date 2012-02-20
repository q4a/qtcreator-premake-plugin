/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Konstantin Tokarev <annulen@yandex.ru>
**
** Partially based on code of Qt Creator by Nokia Corporation
**
**
** GNU Lesser General Public License Usage
**
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this file.
** Please review the following information to ensure the GNU Lesser General
** Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** Other Usage
**
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
** If you have questions regarding the use of this file, please contact
** Nokia at qt-info@nokia.com.
**
**************************************************************************/

#ifndef LUAEDITOR_H
#define LUAEDITOR_H

#include "luaconstants.h"

#include <texteditor/plaintexteditor.h>
#include <texteditor/basetextdocument.h>

#include <coreplugin/editormanager/ieditorfactory.h>

namespace TextEditor {
class TextEditorActionHandler;
}

namespace LuaSupport {

class LuaEditor;
class LuaEditorWidget;
class LuaDocument;
class LuaEditorFactory;

class LuaEditorFactory: public Core::IEditorFactory
{
    Q_OBJECT

public:
    LuaEditorFactory(QObject *parent);
    virtual ~LuaEditorFactory();

    virtual Core::IEditor *createEditor(QWidget *parent);

    virtual QStringList mimeTypes() const;

#if IDE_VER >= IDE_VERSION_CHECK(2, 3, 80)
    virtual Core::Id id() const;
#else
    virtual QString id() const;
#endif

    virtual QString displayName() const;
    virtual Core::IDocument *open(const QString &fileName);

public slots:
    void jumpToFile();

private:
    TextEditor::TextEditorActionHandler *m_actionHandler;
    QStringList m_mimeTypes;
};

class LuaEditor : public TextEditor::PlainTextEditor
{
    Q_OBJECT

public:
    LuaEditor(LuaEditorWidget *editorWidget);
    virtual ~LuaEditor();
    virtual Core::Context context() const;
#if IDE_VER >= IDE_VERSION_CHECK(2, 4, 80)
    virtual Core::Id id() const;
#else
    virtual QString id() const;
#endif

    virtual bool duplicateSupported() const;
    virtual Core::IEditor *duplicate(QWidget *parent);

    virtual bool isTemporary() const { return false; }

private:
    Core::Context m_context;
};

class LuaEditorWidget : public TextEditor::PlainTextEditorWidget
{
    Q_OBJECT

public:
    LuaEditorWidget(QWidget *parent, LuaEditorFactory *factory,
                       TextEditor::TextEditorActionHandler *handler);
    virtual ~LuaEditorWidget();

    LuaEditorFactory *factory() const;
    TextEditor::TextEditorActionHandler *actionHandler() const;

    void unCommentSelection();

    void jumpToFile();

    Link findLinkAt(const QTextCursor &cursor,
                                     bool resolveTarget = true);
protected:
    TextEditor::BaseTextEditor *createEditor();
    void contextMenuEvent(QContextMenuEvent *);

private:
    LuaEditorFactory *m_factory;
    TextEditor::TextEditorActionHandler *m_actionHandler;
    Utils::CommentDefinition m_commentDefinition;
};

} // namespace LuaSupport

#endif // LUAEDITOR_H
