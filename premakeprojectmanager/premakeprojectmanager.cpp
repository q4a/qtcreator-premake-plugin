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

#include "premakeprojectmanager.h"
#include "premakeprojectconstants.h"
#include "premakeproject.h"

#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/session.h>

#include <QtDebug>

using namespace PremakeProjectManager::Internal;

PremakeManager::PremakeManager()
{
    m_projectContext  = Core::Context(PremakeProjectManager::Constants::PROJECTCONTEXT);
    m_projectLanguage = Core::Context(ProjectExplorer::Constants::CXX_LANGUAGE_ID);
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
    return openProject(fileName, 0);
}

ProjectExplorer::Project *PremakeManager::openProject(const QString &fileName, QString *errorString)
{
    Q_UNUSED(errorString)
    if (!QFileInfo(fileName).isFile())
        return 0;

    //ProjectExplorer::ProjectExplorerPlugin *projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
    //foreach (ProjectExplorer::Project *pi, projectExplorer->session()->projects()) {
    foreach (ProjectExplorer::Project *pi, ProjectExplorer::SessionManager::projects()) {
#if IDE_VER >= IDE_VERSION_CHECK(2, 4, 80)
        //const QString existingProjectFileName = pi->document()->fileName();
        const QString existingProjectFileName = pi->document()->displayName();
#else
        const QString existingProjectFileName = pi->file()->fileName();
#endif
        if (fileName == existingProjectFileName) {
            //Core::MessageManager *messageManager = Core::ICore::instance()->messageManager();
            //messageManager->printToOutputPanePopup(tr("Failed opening project '%1': Project already open")
            //                                       .arg(QDir::toNativeSeparators(fileName)));
            const QString fullMessage = QCoreApplication::translate("PremakeProject", "Failed opening project '%1': Project already open").arg(QDir::toNativeSeparators(fileName));
            const Core::MessageManager::PrintToOutputPaneFlags flags = Core::MessageManager::ModeSwitch;
            Core::MessageManager::write(fullMessage, flags);

            return 0;
        }
    }

    PremakeProject *project = new PremakeProject(Utils::FileName::fromString(fileName));
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
