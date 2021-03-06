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

#ifndef PREMAKEPROJECT_H
#define PREMAKEPROJECT_H

#include "premakeprojectconstants.h"
#include "premakeprojectmanager.h"
#include "premakeprojectnodes.h"
#include "premaketarget.h"

#include <projectexplorer/project.h>
#include <projectexplorer/projectnodes.h>
#include <projectexplorer/target.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/buildstep.h>
#include <projectexplorer/buildconfiguration.h>

#include <utils/fileutils.h>

#include <QtCore/QFuture>

struct lua_State;

namespace PremakeProjectManager {
namespace Internal {
class PremakeBuildConfiguration;
class PremakeProject;
class PremakeTarget;
class PremakeStep;
class PremakeProjectFile;

struct PremakeBuildTarget
{
    QString executable;
    QString workingDirectory;
    bool isConsole;
};

class PremakeProject : public ProjectExplorer::Project
{
    Q_OBJECT

public:
    explicit PremakeProject(const Utils::FileName &filename);
    virtual ~PremakeProject();

    QString displayName() const;
    QString id() const;
    Core::IDocument *document() const;
    Core::IDocument *file() const { return document(); }
    QObject *projectManager() const;
    PremakeTarget *activeTarget() const;

    QList<ProjectExplorer::Project *> dependsOn();

    //QList<ProjectExplorer::NamedWidget*> subConfigWidgets();

    PremakeProjectNode *rootProjectNode() const;
    // FIXME: Qt5 no FilesMode
    //QStringList files(FilesMode fileMode) const;

    bool addFiles(const QStringList &filePaths);
    bool removeFiles(const QStringList &filePaths);

    enum RefreshOptions {
        Files         = 0x01,
        Configuration = 0x02,
        ProjectName   = 0x04,
        RootPath      = 0x08,
        Everything = Files | Configuration | ProjectName | RootPath
    };

    void refresh(RefreshOptions options);

    QStringList includePaths() const;
    void setIncludePaths(const QStringList &includePaths);

    QByteArray defines() const;
    QStringList allIncludePaths() const;
    QStringList files() const;
    QStringList generated() const;
    QStringList scriptDepends() const;

    QStringList configurations();

    ProjectExplorer::ToolChain *toolChain() const;
    void setToolChain(ProjectExplorer::ToolChain *tc);

    QStringList buildTargetTitles() const;
    bool hasBuildTarget(const QString &title) const;
    PremakeBuildTarget buildTargetForTitle(const QString &title);

    QVariantMap toMap() const;

signals:
    void toolChainChanged(ProjectExplorer::ToolChain *);signals:
    /// emitted after parsing
    void buildTargetsChanged();

protected:
    virtual bool fromMap(const QVariantMap &map);

private:
    void parseProject(RefreshOptions options);
    void parseConfigurations();

    PremakeManager *m_manager;
    QString m_fileName;
    PremakeProjectFile *m_file;
    QString m_projectName;

    QStringList m_files;
    QStringList m_generated;
    QStringList m_scriptDepends;
    QStringList m_includePaths;
    QByteArray m_defines;
    QStringList m_configurations;

    QHash<QString, PremakeBuildTarget> m_buildTargets;

    PremakeProjectNode *m_rootNode;
    ProjectExplorer::ToolChain *m_toolChain;
    QFuture<void> m_codeModelFuture;
};

class PremakeProjectFile : public Core::IDocument
{
    Q_OBJECT

public:
    PremakeProjectFile(PremakeProject *parent, QString fileName);
    virtual ~PremakeProjectFile();

    virtual bool save(QString *errorString, const QString &fileName = QString(), bool autoSave = false);
    virtual QString fileName() const;

    virtual QString defaultPath() const;
    virtual QString suggestedFileName() const;
    virtual QString mimeType() const;

    virtual bool isModified() const;
    virtual bool isReadOnly() const;
    virtual bool isSaveAsAllowed() const;
    virtual void rename(const QString &newName);

    ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const;
    virtual bool reload(QString *errorString, ReloadFlag flag, ChangeType type);

private:
    PremakeProject *m_project;
    QString m_fileName;
};

} // namespace Internal
} // namespace PremakeProjectManager

#endif // PREMAKEPROJECT_H
