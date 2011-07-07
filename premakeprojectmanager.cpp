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

#include "premakeprojectmanager.h"
#include "premakeprojectconstants.h"
#include "premakeproject.h"

#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/session.h>

#include <QtDebug>

extern "C" {
#include "premake/src/host/premake.h"
}

using namespace PremakeProjectManager::Internal;

int PremakeManager::writer(lua_State*, const void* p, size_t size, void*)
{
    return m_bytecodeWriteStream.writeRawData((const char *)p, size);
}

PremakeManager::PremakeManager() : m_bytecodeWriteStream(&m_bytecode, QIODevice::WriteOnly)
{
    m_projectContext  = Core::Context(PremakeProjectManager::Constants::PROJECTCONTEXT);
    m_projectLanguage = Core::Context(ProjectExplorer::Constants::LANG_CXX);

    // Load bridge code
    QFile premakebridge(":/premakeproject/premakebridge.lua");
    premakebridge.open(QIODevice::ReadOnly);
    m_premakeBridge = QTextStream(&premakebridge).readAll().toUtf8();
    premakebridge.close();
}

PremakeManager::~PremakeManager()
{ }

Core::Context PremakeManager::projectContext() const
{ return m_projectContext; }

Core::Context PremakeManager::projectLanguage() const
{ return m_projectLanguage; }

QString PremakeManager::mimeType() const
{ return QLatin1String(Constants::PREMAKEMIMETYPE); }

ProjectExplorer::Project *PremakeManager::openProject(const QString &fileName)
{
    if (!QFileInfo(fileName).isFile())
        return 0;

    ProjectExplorer::ProjectExplorerPlugin *projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
    foreach (ProjectExplorer::Project *pi, projectExplorer->session()->projects()) {
        if (fileName == pi->file()->fileName()) {
            Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
            messageManager->printToOutputPanePopup(tr("Failed opening project '%1': Project already open")
                                                   .arg(QDir::toNativeSeparators(fileName)));
            return 0;
        }
    }

    PremakeProject *project = new PremakeProject(this, fileName);
    qWarning() << Q_FUNC_INFO << "here";
    return project;
}

void PremakeManager::registerProject(PremakeProject *project)
{ m_projects.append(project); }

void PremakeManager::unregisterProject(PremakeProject *project)
{ m_projects.removeAll(project); }

void PremakeManager::notifyChanged(const QString &fileName)
{
    foreach (PremakeProject *project, m_projects) {
//        if (fileName == project->filesFileName()) {
//            project->refresh(PremakeProject::Files);
//        }
//        else if (fileName == project->includesFileName() ||
//                 fileName == project->configFileName()) {
//            project->refresh(PremakeProject::Configuration);
//        }
    }
}

static void projectParseError(const QString &errorMessage)
{
    Core::ICore::instance()->messageManager()->printToOutputPanePopup("Premake error: " + errorMessage);
}

lua_State * PremakeManager::createPremakeLuaState(const QString &fileName) const
{
    lua_State *L = lua_open();
    luaL_openlibs(L);

    // Initialize Premake
    QByteArray file = QFile::encodeName(QString("--file=").append(fileName));
    const char *argv[3];
    argv[0] = "";
    argv[1] = file.data();
    argv[2] = "_qtcreator";

    if(premake_init(L, 3, argv) != 0){
        projectParseError(lua_tostring(L, -1));
    }

    const char * lua_bridge_code = m_premakeBridge.constData();

    if (luaL_dostring(L, lua_bridge_code) != 0) {
        projectParseError(lua_tostring(L, -1));
    }
    return L;
}
