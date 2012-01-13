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

#include "luaeditor.h"
#include "luaconstants.h"
#include "luacompleter.h"
#include "luaindenter.h"

#include <coreplugin/icore.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/fileiconprovider.h>
#include <texteditor/fontsettings.h>
#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>


#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtGui/QAction>
#include <QtGui/QMenu>
#include <QtGui/QTextBlock>

#include <QtCore/QDebug>


using namespace LuaSupport;


////////////////////////////////////////////////////////////////////////////////////////
// ProjectFilesFactory
////////////////////////////////////////////////////////////////////////////////////////

LuaEditorFactory::LuaEditorFactory(QObject *parent)
    : Core::IEditorFactory(parent)
{
    using namespace Core;
    using namespace TextEditor;

    FileIconProvider *iconProvider = FileIconProvider::instance();
    iconProvider->registerIconOverlayForSuffix(QIcon(QLatin1String(Constants::ICON_LUA_FILE)),
                                        QLatin1String("lua"));
    m_mimeTypes << QLatin1String(Constants::LUA_MIMETYPE);

    m_actionHandler =
            new TextEditorActionHandler(Constants::C_LUAEDITOR,
            TextEditorActionHandler::UnCommentSelection |
            TextEditorActionHandler::Format |
            TextEditorActionHandler::UnCollapseAll);

    // Lua editor context menu
    ICore *core = ICore::instance();
    ActionManager *am = core->actionManager();
    ActionContainer *contextMenu = am->createMenu(Constants::M_CONTEXT);
    Command *cmd;
    Context luaEditorContext = Context(Constants::C_LUAEDITOR);

    QAction *jumpToFile = new QAction(tr("Jump to File Under Cursor"), this);
    cmd = am->registerAction(jumpToFile,
        Constants::JUMP_TO_FILE, luaEditorContext);
    cmd->setDefaultKeySequence(QKeySequence(Qt::Key_F2));
    connect(jumpToFile, SIGNAL(triggered()),
            this, SLOT(jumpToFile()));
    contextMenu->addAction(cmd);

    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    contextMenu->addAction(am->registerAction(separator,
                  Id(Constants::SEPARATOR), luaEditorContext));

    cmd = am->command(TextEditor::Constants::AUTO_INDENT_SELECTION);
    contextMenu->addAction(cmd);

    cmd = am->command(TextEditor::Constants::UN_COMMENT_SELECTION);
    contextMenu->addAction(cmd);
}

LuaEditorFactory::~LuaEditorFactory()
{
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

#if IDE_VER >= IDE_VERSION_CHECK(2, 3, 80)
Core::Id LuaEditorFactory::id() const
{
    return Core::Id(Constants::LUA_EDITOR_ID);
}
#else
QString LuaEditorFactory::id() const
{
    return QLatin1String(Constants::LUA_EDITOR_ID);
}
#endif

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

void LuaEditorFactory::jumpToFile()
{
    Core::EditorManager *em = Core::EditorManager::instance();
    LuaEditorWidget *editor = qobject_cast<LuaEditorWidget*>(em->currentEditor()->widget());
    if (editor)
        editor->jumpToFile();
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

#if IDE_VER >= IDE_VERSION_CHECK(2, 4, 80)
Core::Id LuaEditor::id() const
{
    return Core::Id(Constants::LUA_EDITOR_ID);
}
#else
QString LuaEditor::id() const
{
    return QLatin1String(Constants::LUA_EDITOR_ID);
}
#endif

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
    m_commentDefinition.clearCommentStyles();
    m_commentDefinition.setSingleLine(QLatin1String("--"));
    m_commentDefinition.setMultiLineStart(QLatin1String("--[["));
    m_commentDefinition.setMultiLineEnd(QLatin1String("]]"));

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

void LuaEditorWidget::unCommentSelection()
{
    Utils::unCommentSelection(this, m_commentDefinition);
}

void LuaEditorWidget::contextMenuEvent(QContextMenuEvent *e)
{
    QMenu *menu = new QMenu();

    Core::ActionManager *am = Core::ICore::instance()->actionManager();
    Core::ActionContainer *mcontext = am->actionContainer(Constants::M_CONTEXT);
    QMenu *contextMenu = mcontext->menu();

    foreach (QAction *action, contextMenu->actions())
        menu->addAction(action);

    appendStandardContextMenuActions(menu);

    menu->exec(e->globalPos());
    delete menu;
}

LuaEditorWidget::Link LuaEditorWidget::findLinkAt(const QTextCursor &cursor,
                                      bool /* resolveTarget */)
{
    Link link;

    int lineNumber = 0, positionInBlock = 0;
    convertPosition(cursor.position(), &lineNumber, &positionInBlock);

    const QString block = cursor.block().text();

    // check if the current position is commented out
    // FIXME: check for multiline comments!
    const int hashPos = block.indexOf(QLatin1String("--"));
    if (hashPos >= 0 && hashPos < positionInBlock)
        return link;

    // find the beginning of string literal
    QString buffer;
    int beginPos = positionInBlock - 1;
    while (beginPos >= 0) {
        QChar c = block.at(beginPos);
        if (c != QLatin1Char('\"')) {
            buffer.prepend(c);
            beginPos--;
        } else {
            break;
        }
    }

    // find the end of string literal
    int endPos = positionInBlock;
    while (endPos < block.count()) {
        QChar c = block.at(endPos);
        if (c != QLatin1Char('\"')) {
            buffer.append(c);
            endPos++;
        } else {
            break;
        }
    }

    if (buffer.isEmpty())
        return link;

    QDir dir(QFileInfo(file()->fileName()).absolutePath());
    QString fileName = dir.filePath(buffer);
    QFileInfo fi(fileName);
    if (fi.exists()) {
        if (fi.isDir()) {
            QDir subDir(fi.absoluteFilePath());
            QString subProject = subDir.filePath(QLatin1String("premake4.lua"));
            if (QFileInfo(subProject).exists())
                fileName = subProject;
            else
                return link;
        }
        link.fileName = fileName;
        link.begin = cursor.position() - positionInBlock + beginPos + 1;
        link.end = cursor.position() - positionInBlock + endPos;
    }

    // require "fileName"
    // TODO: Support custom module paths
    // TODO: Check if it's really 'require'
    fileName.append(QLatin1String(".lua"));
    fi.setFile(fileName);
    if (fi.exists()) {
        link.fileName = fileName;
        link.begin = cursor.position() - positionInBlock + beginPos + 1;
        link.end = cursor.position() - positionInBlock + endPos;
    }

    return link;
}

void LuaEditorWidget::jumpToFile()
{
    openLink(findLinkAt(textCursor()));
}
