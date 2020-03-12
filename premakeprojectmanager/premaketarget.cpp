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

#include "premaketarget.h"

#include "premakebuildconfiguration.h"
#include "premakebuildsettingswidget.h"
#include "premakeproject.h"
#include "premakerunconfiguration.h"
#include "makestep.h"

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/customexecutablerunconfiguration.h>
#include <projectexplorer/deployconfiguration.h>
#include <projectexplorer/projectexplorerconstants.h>

#include <QtWidgets/QApplication>
#include <QtWidgets/QStyle>

namespace {
const char * const PREMAKE_DESKTOP_TARGET_DISPLAY_NAME("Desktop");
}

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;

////////////////////////////////////////////////////////////////////////////////////
// PremakeTarget
////////////////////////////////////////////////////////////////////////////////////

// FIXME: Qt5 do I need this file? cmake and cmake2 targets has only header file
/*
PremakeTarget::PremakeTarget(PremakeProject *parent) :
    ProjectExplorer::Target(parent, QLatin1String(PREMAKE_DESKTOP_TARGET_ID)),
    m_buildConfigurationFactory(new PremakeBuildConfigurationFactory(this)),
    m_deployConfigurationFactory(new ProjectExplorer::DeployConfigurationFactory(this))
{
    setDefaultDisplayName(QApplication::translate("PremakeProjectManager::PremakeTarget",
                                                  PREMAKE_DESKTOP_TARGET_DISPLAY_NAME));
    setIcon(qApp->style()->standardIcon(QStyle::SP_ComputerIcon));
    connect(parent, SIGNAL(buildTargetsChanged()), SLOT(updateRunConfigurations()));
}
*/

PremakeTarget::~PremakeTarget()
{
}

ProjectExplorer::NamedWidget *PremakeTarget::createConfigWidget()
{
    return new PremakeBuildSettingsWidget(this);
}

PremakeProject *PremakeTarget::premakeProject() const
{
    return static_cast<PremakeProject *>(project());
}

PremakeBuildConfigurationFactory *PremakeTarget::buildConfigurationFactory() const
{
    return m_buildConfigurationFactory;
}

ProjectExplorer::DeployConfigurationFactory *PremakeTarget::deployConfigurationFactory() const
{
    return m_deployConfigurationFactory;
}

PremakeBuildConfiguration *PremakeTarget::activeBuildConfiguration() const
{
    return static_cast<PremakeBuildConfiguration *>(Target::activeBuildConfiguration());
}

bool PremakeTarget::fromMap(const QVariantMap &map)
{
    if (!Target::fromMap(map))
        return false;

    return true;
}

void PremakeTarget::updateRunConfigurations()
{
    qDebug() << Q_FUNC_INFO;
    // *Update* runconfigurations:
    QMultiMap<QString, PremakeRunConfiguration*> existingRunConfigurations;
    QList<ProjectExplorer::RunConfiguration *> toRemove;
    foreach (ProjectExplorer::RunConfiguration* rc, runConfigurations()) {
        if (PremakeRunConfiguration* premakeRC = qobject_cast<PremakeRunConfiguration *>(rc))
            existingRunConfigurations.insert(premakeRC->title(), premakeRC);
        ProjectExplorer::CustomExecutableRunConfiguration *ceRC =
                qobject_cast<ProjectExplorer::CustomExecutableRunConfiguration *>(rc);
        if (ceRC && !ceRC->isConfigured())
            toRemove << rc;
    }

    foreach (const QString &title, premakeProject()->buildTargetTitles()) {
        qDebug() << Q_FUNC_INFO << title;
        PremakeBuildTarget pt = premakeProject()->buildTargetForTitle(title);
        QList<PremakeRunConfiguration *> list = existingRunConfigurations.values(title);
        if (!list.isEmpty()) {
            // Already exists, so override the settings...
            foreach (PremakeRunConfiguration *rc, list) {
                rc->setExecutable(pt.executable);
                rc->setBaseWorkingDirectory(pt.workingDirectory);
                rc->setEnabled(true);
            }
            existingRunConfigurations.remove(title);
        } else {
            // Does not exist yet
            addRunConfiguration(new PremakeRunConfiguration(this, pt.executable,
                                                            pt.workingDirectory, title));
        }
    }

    QMultiMap<QString, PremakeRunConfiguration *>::const_iterator it =
            existingRunConfigurations.constBegin();
    for ( ; it != existingRunConfigurations.constEnd(); ++it) {
        PremakeRunConfiguration *rc = it.value();
        // The executables for those runconfigurations aren't build by the current buildconfiguration
        // We just set a disable flag and show that in the display name
        rc->setEnabled(false);
        // removeRunConfiguration(rc);
    }

    foreach (ProjectExplorer::RunConfiguration *rc, toRemove)
        removeRunConfiguration(rc);

    if (runConfigurations().isEmpty()) {
        // Oh no, no run configuration,
        // create a custom executable run configuration
        ProjectExplorer::CustomExecutableRunConfiguration *rc = new ProjectExplorer::CustomExecutableRunConfiguration(this);
        addRunConfiguration(rc);
    }
}

////////////////////////////////////////////////////////////////////////////////////
// PremakeTargetFactory
////////////////////////////////////////////////////////////////////////////////////

PremakeTargetFactory::PremakeTargetFactory(QObject *parent) :
    ITargetFactory(parent)
{
//    setDefaultDisplayName(displayNameForId(id()));
//    setIcon(qApp->style()->standardIcon(QStyle::SP_ComputerIcon));
//    connect(parent, SIGNAL(buildTargetsChanged()), SLOT(updateRunConfigurations()));
}

PremakeTargetFactory::~PremakeTargetFactory()
{
}

bool PremakeTargetFactory::supportsTargetId(const QString &id) const
{
    return id == QLatin1String(PREMAKE_DESKTOP_TARGET_ID);
}

QStringList PremakeTargetFactory::supportedTargetIds() const
{
    return QStringList(QLatin1String(PREMAKE_DESKTOP_TARGET_ID));
}

QStringList PremakeTargetFactory::supportedTargetIds(ProjectExplorer::Project *parent) const
{
    if (!qobject_cast<PremakeProject *>(parent))
        return QStringList();
    return QStringList() << QLatin1String(PREMAKE_DESKTOP_TARGET_ID);
}

QString PremakeTargetFactory::displayNameForId(const QString &id) const
{
    if (id == QLatin1String(PREMAKE_DESKTOP_TARGET_ID))
        return QCoreApplication::translate("PremakeProjectManager::PremakeTarget",
                                           PREMAKE_DESKTOP_TARGET_DISPLAY_NAME,
                                           "Premake desktop target display name");
    return QString();
}

bool PremakeTargetFactory::canCreate(ProjectExplorer::Project *parent, const QString &id) const
{
    if (!qobject_cast<PremakeProject *>(parent))
        return false;
    return id == QLatin1String(PREMAKE_DESKTOP_TARGET_ID);
}

PremakeTarget *PremakeTargetFactory::create(ProjectExplorer::Project *parent, const QString &id)
{
    if (!canCreate(parent, id))
        return 0;
    PremakeProject *project = static_cast<PremakeProject *>(parent);
    PremakeTarget *t = new PremakeTarget(project);

    // Set up BuildConfiguration:
    foreach (const QString conf, project->configurations()) {
        PremakeBuildConfiguration *bc = static_cast<PremakeBuildConfiguration *>
            (t->buildConfigurationFactory()->createBuildConfiguration(t, conf, conf.toLocal8Bit()));
        t->addBuildConfiguration(bc);
    }

    t->addDeployConfiguration(t->deployConfigurationFactory()->create(t,
                QLatin1String(ProjectExplorer::Constants::DEFAULT_DEPLOYCONFIGURATION_ID)));

    qDebug() << Q_FUNC_INFO << "here";
    // Query project on executables
    t->updateRunConfigurations();

    return t;
}

bool PremakeTargetFactory::canRestore(ProjectExplorer::Project *parent, const QVariantMap &map) const
{
    return canCreate(parent, ProjectExplorer::idFromMap(map));
}

PremakeTarget *PremakeTargetFactory::restore(ProjectExplorer::Project *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;
    PremakeProject *project = static_cast<PremakeProject *>(parent);
    PremakeTarget *target = new PremakeTarget(project);
    if (target->fromMap(map))
        return target;
    delete target;
    return 0;
}
