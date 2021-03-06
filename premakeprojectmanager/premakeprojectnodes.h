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

#ifndef PREMAKEPROJECTNODE_H
#define PREMAKEPROJECTNODE_H

#include <projectexplorer/projectnodes.h>
#include <projectexplorer/runconfiguration.h>

#include <QtCore/QStringList>
#include <QtCore/QHash>

namespace PremakeProjectManager {
namespace Internal {

class PremakeProject;

class PremakeProjectNode : public ProjectExplorer::ProjectNode
{
public:
    PremakeProjectNode(PremakeProject *project);
    virtual ~PremakeProjectNode();

    virtual bool hasBuildTargets() const;

    virtual QList<ProjectExplorer::ProjectAction> supportedActions(Node *node) const;

    virtual bool canAddSubProject(const QString &proFilePath) const;

    virtual bool addSubProjects(const QStringList &proFilePaths);
    virtual bool removeSubProjects(const QStringList &proFilePaths);

    virtual bool addFiles(const ProjectExplorer::FileType fileType,
                          const QStringList &filePaths,
                          QStringList *notAdded = 0);

    virtual bool removeFiles(const ProjectExplorer::FileType fileType,
                             const QStringList &filePaths,
                             QStringList *notRemoved = 0);
    virtual bool deleteFiles(const ProjectExplorer::FileType fileType,
                             const QStringList &filePaths);

    virtual bool renameFile(const ProjectExplorer::FileType fileType,
                             const QString &filePath,
                             const QString &newFilePath);

    virtual QList<ProjectExplorer::RunConfiguration *> runConfigurationsFor(Node *node);

    void refresh();

    void setPath(const QString &path);

private:
    FolderNode *findOrCreateFolderByName(const QString &filePath);
    FolderNode *findOrCreateFolderByName(const QStringList &components, int end);

private:
    PremakeProject *m_project;
    QHash<QString, FolderNode *> m_folderByName;
};

} // namespace Internal
} // namespace PremakeProjectManager

#endif // PREMAKEPROJECTNODE_H
