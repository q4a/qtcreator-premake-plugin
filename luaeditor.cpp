/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (info@qt.nokia.com)
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

#include "luaeditor.h"
#include "premakeprojectmanager.h"
#include "premakeprojectconstants.h"
#include "luacompleter.h"
#include "luaindenter.h"

#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/fileiconprovider.h>
#include <texteditor/fontsettings.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorsettings.h>

#include <QDebug>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;


////////////////////////////////////////////////////////////////////////////////////////
// ProjectFilesFactory
////////////////////////////////////////////////////////////////////////////////////////

LuaEditorFactory::LuaEditorFactory(PremakeManager *manager,
                                         TextEditor::TextEditorActionHandler *handler)
    : Core::IEditorFactory(manager),
      m_manager(manager),
      m_actionHandler(handler)
{
    Core::FileIconProvider *iconProvider = Core::FileIconProvider::instance();
    iconProvider->registerIconOverlayForSuffix(QIcon(QLatin1String(Constants::ICON_LUA_FILE)),
                                        QLatin1String("lua"));
    m_mimeTypes << QLatin1String(Constants::LUA_MIMETYPE)
                << QLatin1String(Constants::PREMAKEMIMETYPE);
}

LuaEditorFactory::~LuaEditorFactory()
{
}

PremakeManager *LuaEditorFactory::manager() const
{
    return m_manager;
}

Core::IEditor *LuaEditorFactory::createEditor(QWidget *parent)
{
    LuaEditorWidget *ed = new LuaEditorWidget(parent, this, m_actionHandler);
    TextEditor::TextEditorSettings::instance()->initializeEditor(ed);
    return ed->editor();
}

QStringList LuaEditorFactory::mimeTypes() const
{
    return m_mimeTypes;
}

QString LuaEditorFactory::id() const
{
    return QLatin1String(Constants::LUA_EDITOR_ID);
}

QString LuaEditorFactory::displayName() const
{
    return tr(Constants::LUA_EDITOR_DISPLAY_NAME);
}

Core::IFile *LuaEditorFactory::open(const QString &fileName)
{
    Core::EditorManager *editorManager = Core::EditorManager::instance();

    if (Core::IEditor *editor = editorManager->openEditor(fileName, id()))
        return editor->file();

    return 0;
}

////////////////////////////////////////////////////////////////////////////////////////
// ProjectFilesEditable
////////////////////////////////////////////////////////////////////////////////////////

LuaEditor::LuaEditor(LuaEditorWidget *editor)
  : TextEditor::PlainTextEditor(editor),
    m_context(Constants::C_LUAEDITOR)
{ }

LuaEditor::~LuaEditor()
{ }

Core::Context LuaEditor::context() const
{
    return m_context;
}

QString LuaEditor::id() const
{
    return QLatin1String(Constants::LUA_EDITOR_ID);
}

bool LuaEditor::duplicateSupported() const
{
    return true;
}

Core::IEditor *LuaEditor::duplicate(QWidget *parent)
{
    LuaEditorWidget *parentEditor = qobject_cast<LuaEditorWidget *>(editorWidget());
    LuaEditorWidget *editor = new LuaEditorWidget(parent,
                                                        parentEditor->factory(),
                                                        parentEditor->actionHandler());
    TextEditor::TextEditorSettings::instance()->initializeEditor(editor);
    return editor->editor();
}

////////////////////////////////////////////////////////////////////////////////////////
// ProjectFilesEditor
////////////////////////////////////////////////////////////////////////////////////////

LuaEditorWidget::LuaEditorWidget(QWidget *parent, LuaEditorFactory *factory,
                                       TextEditor::TextEditorActionHandler *handler)
    : TextEditor::PlainTextEditorWidget(parent),
      m_factory(factory),
      m_actionHandler(handler)
{
    setMimeType(QLatin1String(Constants::LUA_MIMETYPE));
    setDisplayName(QLatin1String(Constants::LUA_EDITOR_DISPLAY_NAME));
    setIndenter(new LuaIndenter);
    setAutoCompleter(new LuaCompleter);

    handler->setupActions(this);
}

LuaEditorWidget::~LuaEditorWidget()
{ }

LuaEditorFactory *LuaEditorWidget::factory() const
{
    return m_factory;
}

TextEditor::TextEditorActionHandler *LuaEditorWidget::actionHandler() const
{
    return m_actionHandler;
}

TextEditor::BaseTextEditor *LuaEditorWidget::createEditor()
{
    return new LuaEditor(this);
}