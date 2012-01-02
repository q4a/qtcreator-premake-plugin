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

#ifndef PREMAKEBUILDCONFIGURATION_H
#define PREMAKEBUILDCONFIGURATION_H

#include "premakeprojectconstants.h"
#include <projectexplorer/buildconfiguration.h>
#include <qtsupport/qtversionmanager.h>

namespace PremakeProjectManager {
namespace Internal {

class PremakeTarget;
class PremakeBuildConfigurationFactory;

class PremakeBuildConfiguration : public ProjectExplorer::BuildConfiguration
{
    Q_OBJECT
    friend class PremakeBuildConfigurationFactory;

public:
    explicit PremakeBuildConfiguration(PremakeTarget *parent);
    virtual ~PremakeBuildConfiguration();

    PremakeTarget *premakeTarget() const;

    virtual QString buildDirectory() const;

    QString rawBuildDirectory() const;
    void setBuildDirectory(const QString &buildDirectory);

    QVariantMap toMap() const;

    ProjectExplorer::IOutputParser *createOutputParser() const;

    BuildType buildType() const;

    QString projectFileName() const;

    // returns the qtVersion
    QtSupport::BaseQtVersion *qtVersion() const;
    void setQtVersion(QtSupport::BaseQtVersion *);

    QString makeCommand() const;
    QString defaultMakeTarget() const;
    QString makefile() const;

    bool shadowBuildEnabled() const;
    void setShadowBuildEnabled(bool enabled);

signals:
    void shadowBuildChanged();

protected:
    PremakeBuildConfiguration(PremakeTarget *parent, PremakeBuildConfiguration *source);
    PremakeBuildConfiguration(PremakeTarget *parent, const QString &id);
    virtual bool fromMap(const QVariantMap &map);

private:
    QString m_buildDirectory;
    QString m_fileName;
    bool m_shadowBuildEnabled;
};

class PremakeBuildConfigurationFactory : public ProjectExplorer::IBuildConfigurationFactory
{
    Q_OBJECT

public:
    explicit PremakeBuildConfigurationFactory(QObject *parent = 0);
    virtual ~PremakeBuildConfigurationFactory();

    QStringList availableCreationIds(ProjectExplorer::Target *parent) const;
    QString displayNameForId(const QString &id) const;

    bool canCreate(ProjectExplorer::Target *parent, const QString &id) const;
    ProjectExplorer::BuildConfiguration *create(ProjectExplorer::Target *parent, const QString &id);
    bool canClone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source) const;
    ProjectExplorer::BuildConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source);
    bool canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const;
    ProjectExplorer::BuildConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map);
    PremakeBuildConfiguration *createBuildConfiguration(PremakeTarget *parent, const QString &name);
};

} // namespace Internal
} // namespace PremakeProjectManager


// Helper function
bool abiIsMsvc(ProjectExplorer::Abi abi);

#endif // PREMAKEBUILDCONFIGURATION_H
