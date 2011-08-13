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

#include "premakeprojectplugin.h"
#include "premakeprojectmanager.h"
#include "premakeprojectwizard.h"
#include "premakeprojectconstants.h"
#include "luaeditor.h"
#include "premakemakestep.h"
#include "makestep.h"
#include "premaketarget.h"

#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>

#include <texteditor/texteditoractionhandler.h>
#include <texteditor/texteditorconstants.h>

#include <QtCore/QtPlugin>
#include <QtCore/QDebug>
#include <QtGui/QAction>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;

PremakeProjectPlugin::PremakeProjectPlugin()
    : m_luaEditorFactory(0)
{ }

PremakeProjectPlugin::~PremakeProjectPlugin()
{
    removeObject(m_luaEditorFactory);
    delete m_luaEditorFactory;
}

bool PremakeProjectPlugin::initialize(const QStringList &, QString *errorMessage)
{
    using namespace Core;
    using namespace TextEditor;

    ICore *core = ICore::instance();
    Core::MimeDatabase *mimeDB = core->mimeDatabase();

    const QLatin1String mimetypesXml(":/premakeproject/PremakeProject.mimetypes.xml");

    if (! mimeDB->addMimeTypes(mimetypesXml, errorMessage))
        return false;

    PremakeManager *manager = new PremakeManager;

    TextEditorActionHandler *actionHandler =
            new TextEditorActionHandler(Constants::C_LUAEDITOR,
            TextEditorActionHandler::UnCommentSelection |
            TextEditorActionHandler::Format |
            TextEditorActionHandler::UnCollapseAll);

    m_luaEditorFactory = new LuaEditorFactory(manager, actionHandler);
    addObject(m_luaEditorFactory);

    addAutoReleasedObject(manager);
    addAutoReleasedObject(new MakeStepFactory);
    addAutoReleasedObject(new PremakeMakeStepFactory);
    addAutoReleasedObject(new PremakeProjectWizard);
    addAutoReleasedObject(new PremakeTargetFactory);


    // Lua editor context menu
    ActionManager *am = core->actionManager();
    ActionContainer *contextMenu = am->createMenu(PremakeProjectManager::Constants::M_CONTEXT);
    Command *cmd;
    Context luaEditorContext = Core::Context(PremakeProjectManager::Constants::C_LUAEDITOR);

    QAction *separator = new QAction(this);
    separator->setSeparator(true);
    contextMenu->addAction(am->registerAction(separator,
                  Id(Constants::SEPARATOR), luaEditorContext));

    cmd = am->command(TextEditor::Constants::AUTO_INDENT_SELECTION);
    contextMenu->addAction(cmd);

    cmd = am->command(TextEditor::Constants::UN_COMMENT_SELECTION);
    contextMenu->addAction(cmd);

    return true;
}

void PremakeProjectPlugin::extensionsInitialized()
{ }

Q_EXPORT_PLUGIN(PremakeProjectPlugin)
