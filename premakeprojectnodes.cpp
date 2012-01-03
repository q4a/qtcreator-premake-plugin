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

#include "premakeprojectnodes.h"
#include "premakeproject.h"
#include "premakeprojectconstants.h"

#include <coreplugin/fileiconprovider.h>
#include <coreplugin/ifile.h>
#include <projectexplorer/projectexplorer.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QFileInfo>
#include <QtGui/QStyle>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;

struct PremakeNodeStaticData
{
    QIcon projectIcon;
};

static void clearPremakeNodeStaticData();

// TODO: Add file type icons
Q_GLOBAL_STATIC_WITH_INITIALIZER(PremakeNodeStaticData, qt4NodeStaticData, {
    // Overlay the SP_DirIcon with the custom icons
    const QSize desiredSize = QSize(16, 16);

    // Project icon
    const QIcon projectBaseIcon(Constants::ICON_PREMAKEPROJECT);
    const QPixmap projectPixmap = Core::FileIconProvider::overlayIcon(QStyle::SP_DirIcon,
                                                                  projectBaseIcon,
                                                                  desiredSize);
    x->projectIcon.addPixmap(projectPixmap);

    qAddPostRoutine(clearPremakeNodeStaticData);
})

static void clearPremakeNodeStaticData()
{
    qt4NodeStaticData()->projectIcon = QIcon();
}

PremakeProjectNode::PremakeProjectNode(PremakeProject *project)
    : ProjectExplorer::ProjectNode(project->file()->fileName()),
      m_project(project)
{
    setDisplayName(m_project->displayName());
    setIcon(qt4NodeStaticData()->projectIcon);
}

PremakeProjectNode::~PremakeProjectNode()
{ }

void PremakeProjectNode::refresh()
{
    using namespace ProjectExplorer;

    // remove the existing nodes.
    removeFileNodes(fileNodes(), this);
    removeFolderNodes(subFolderNodes(), this);

    //ProjectExplorerPlugin::instance()->setCurrentNode(0); // ### remove me


    QList<FileNode *> projectFileNodes;

    projectFileNodes << new FileNode(m_project->file()->fileName(),
                                     ProjectFileType,
                                     /* generated = */ false);

//    foreach(const QString &filename, m_project->scriptDepends()) {
//        projectFileNodes << new FileNode(filename,
//                                         ProjectFileType,
//                                         /* generated = */ false);
//    }

    addFileNodes(QList<FileNode *>()
                 << projectFileNodes,
                 this);

    QStringList files = m_project->files();
    files << m_project->scriptDepends();
    files.removeAll(m_project->file()->fileName());

    QStringList filePaths;
    QHash<QString, QStringList> filesInPath;
    const QString base = QFileInfo(path()).absolutePath();
    const QDir baseDir(base);

    foreach (const QString &absoluteFileName, files) {
        QFileInfo fileInfo(absoluteFileName);
        const QString absoluteFilePath = fileInfo.path();
        QString relativeFilePath;

        if (absoluteFilePath.startsWith(base)) {
            relativeFilePath = absoluteFilePath.mid(base.length() + 1);
        } else {
            // `file' is not part of the project.
            relativeFilePath = baseDir.relativeFilePath(absoluteFilePath);
        }

        if (! filePaths.contains(relativeFilePath))
            filePaths.append(relativeFilePath);

        filesInPath[relativeFilePath].append(absoluteFileName);
    }

    foreach (const QString &filePath, filePaths) {
        FolderNode *folder = findOrCreateFolderByName(filePath);

        QList<FileNode *> fileNodes;
        foreach (const QString &file, filesInPath.value(filePath)) {
            FileType fileType = SourceType; // ### FIXME
            const bool generated = m_project->generated().contains(baseDir.relativeFilePath(file));
            FileNode *fileNode = new FileNode(file, fileType, generated);
            fileNodes.append(fileNode);
        }

        addFileNodes(fileNodes, folder);
    }

    m_folderByName.clear();
}

ProjectExplorer::FolderNode *PremakeProjectNode::findOrCreateFolderByName(const QStringList &components, int end)
{
    if (! end)
        return 0;

    QString folderName;
    for (int i = 0; i < end; ++i) {
        folderName.append(components.at(i));
        folderName += QLatin1Char('/'); // ### FIXME
    }

    const QString component = components.at(end - 1);

    if (component.isEmpty())
        return this;

    else if (FolderNode *folder = m_folderByName.value(folderName))
        return folder;

    const QString baseDir = QFileInfo(path()).path();
    FolderNode *folder = new FolderNode(baseDir + QLatin1Char('/') + folderName);
    folder->setDisplayName(component);
    m_folderByName.insert(folderName, folder);

    FolderNode *parent = findOrCreateFolderByName(components, end - 1);
    if (! parent)
        parent = this;
    addFolderNodes(QList<FolderNode*>() << folder, parent);

    return folder;
}

ProjectExplorer::FolderNode *PremakeProjectNode::findOrCreateFolderByName(const QString &filePath)
{
    QStringList components = filePath.split(QLatin1Char('/'));
    return findOrCreateFolderByName(components, components.length());
}

bool PremakeProjectNode::hasBuildTargets() const
{
    return true;
}

QList<ProjectExplorer::ProjectNode::ProjectAction> PremakeProjectNode::supportedActions(Node *node) const
{
    Q_UNUSED(node);
    return QList<ProjectAction>(); // Don't support yet
//        << AddNewFile
//        << AddExistingFile
//        << RemoveFile;
}

bool PremakeProjectNode::canAddSubProject(const QString &proFilePath) const
{
    Q_UNUSED(proFilePath)
    return false;
}

bool PremakeProjectNode::addSubProjects(const QStringList &proFilePaths)
{
    Q_UNUSED(proFilePaths)
    return false;
}

bool PremakeProjectNode::removeSubProjects(const QStringList &proFilePaths)
{
    Q_UNUSED(proFilePaths)
    return false;
}

bool PremakeProjectNode::addFiles(const ProjectExplorer::FileType fileType,
                                  const QStringList &filePaths, QStringList *notAdded)
{
    Q_UNUSED(fileType)
    Q_UNUSED(notAdded)

    return false; //m_project->addFiles(filePaths);
}

bool PremakeProjectNode::removeFiles(const ProjectExplorer::FileType fileType,
                                     const QStringList &filePaths, QStringList *notRemoved)
{
    Q_UNUSED(fileType)
    Q_UNUSED(notRemoved)

    return false; //m_project->removeFiles(filePaths);
}

bool PremakeProjectNode::deleteFiles(const ProjectExplorer::FileType fileType,
                                     const QStringList &filePaths)
{
    Q_UNUSED(fileType)
    Q_UNUSED(filePaths)
    return false;
}

bool PremakeProjectNode::renameFile(const ProjectExplorer::FileType fileType,
                                    const QString &filePath, const QString &newFilePath)
{
    Q_UNUSED(fileType)
    Q_UNUSED(filePath)
    Q_UNUSED(newFilePath)
    return false;
}

QList<ProjectExplorer::RunConfiguration *> PremakeProjectNode::runConfigurationsFor(Node *node)
{
    Q_UNUSED(node)
    return QList<ProjectExplorer::RunConfiguration *>();
}

void PremakeProjectManager::Internal::PremakeProjectNode::setPath(const QString &path)
{
    if(!path.isEmpty())
        Node::setPath(path);
}
