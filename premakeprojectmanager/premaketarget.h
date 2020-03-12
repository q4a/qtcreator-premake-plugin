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

#ifndef PREMAKETARGET_H
#define PREMAKETARGET_H

#include <projectexplorer/target.h>

#include "premakebuildconfiguration.h"

#include <QtCore/QStringList>
#include <QtCore/QVariantMap>

namespace ProjectExplorer {
class IBuildConfigurationFactory;
} // namespace ProjectExplorer

namespace PremakeProjectManager {

namespace Internal {

const char * const PREMAKE_DESKTOP_TARGET_ID = "PremakeProjectManager.Target.DesktopTarget";

class PremakeProject;
class PremakeRunConfiguration;
class PremakeBuildConfigurationFactory;
//class PremakeTargetFactory;

class PremakeTarget : public ProjectExplorer::Target
{
    Q_OBJECT
//    friend class PremakeTargetFactory;

public:
    //explicit PremakeTarget(PremakeProject *parent);
    ~PremakeTarget();

    ProjectExplorer::NamedWidget *createConfigWidget();

    PremakeProject *premakeProject() const;

    PremakeBuildConfigurationFactory *buildConfigurationFactory() const;
    ProjectExplorer::DeployConfigurationFactory *deployConfigurationFactory() const;
    PremakeBuildConfiguration *activeBuildConfiguration() const;

protected:
    bool fromMap(const QVariantMap &map);

private slots:
    void updateRunConfigurations();

private:
    PremakeBuildConfigurationFactory *m_buildConfigurationFactory;
    ProjectExplorer::DeployConfigurationFactory *m_deployConfigurationFactory;
};

// FIXME: Qt5 do I need this file? cmake and cmake2 targets has only header file
/*
class PremakeTargetFactory : public ProjectExplorer::Target
{
    Q_OBJECT

public:
    explicit PremakeTargetFactory(QObject *parent = 0);
    ~PremakeTargetFactory();

    bool supportsTargetId(const QString &id) const;
    QStringList supportedTargetIds() const;

    QStringList supportedTargetIds(ProjectExplorer::Project *parent) const;
    QString displayNameForId(const QString &id) const;

    bool canCreate(ProjectExplorer::Project *parent, const QString &id) const;
    PremakeTarget *create(ProjectExplorer::Project *parent, const QString &id);
    bool canRestore(ProjectExplorer::Project *parent, const QVariantMap &map) const;
    PremakeTarget *restore(ProjectExplorer::Project *parent, const QVariantMap &map);
};
*/

} // namespace Internal

} // namespace PremakeProjectManager

#endif // PREMAKETARGET_H
