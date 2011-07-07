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

#include "premaketarget.h"

#include "premakebuildconfiguration.h"
#include "premakeproject.h"
#include "premakemakestep.h"

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/customexecutablerunconfiguration.h>
#include <projectexplorer/deployconfiguration.h>
#include <projectexplorer/projectexplorerconstants.h>

#include <QtGui/QApplication>
#include <QtGui/QStyle>

namespace {
const char * const PREMAKE_DESKTOP_TARGET_DISPLAY_NAME("Desktop");
}

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;

////////////////////////////////////////////////////////////////////////////////////
// PremakeTarget
////////////////////////////////////////////////////////////////////////////////////

PremakeTarget::PremakeTarget(PremakeProject *parent) :
    ProjectExplorer::Target(parent, QLatin1String(PREMAKE_DESKTOP_TARGET_ID)),
    m_buildConfigurationFactory(new PremakeBuildConfigurationFactory(this)),
    m_deployConfigurationFactory(new ProjectExplorer::DeployConfigurationFactory(this))
{
    setDefaultDisplayName(QApplication::translate("PremakeProjectManager::PremakeTarget",
                                                  PREMAKE_DESKTOP_TARGET_DISPLAY_NAME));
    setIcon(qApp->style()->standardIcon(QStyle::SP_ComputerIcon));
}

PremakeTarget::~PremakeTarget()
{
}

ProjectExplorer::BuildConfigWidget *PremakeTarget::createConfigWidget()
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

////////////////////////////////////////////////////////////////////////////////////
// PremakeTargetFactory
////////////////////////////////////////////////////////////////////////////////////

PremakeTargetFactory::PremakeTargetFactory(QObject *parent) :
    ITargetFactory(parent)
{
}

PremakeTargetFactory::~PremakeTargetFactory()
{
}

bool PremakeTargetFactory::supportsTargetId(const QString &id) const
{
    return id == QLatin1String(PREMAKE_DESKTOP_TARGET_ID);
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
    PremakeProject *genericproject = static_cast<PremakeProject *>(parent);
    PremakeTarget *t = new PremakeTarget(genericproject);

    // Set up BuildConfiguration:
    PremakeBuildConfiguration *bc = new PremakeBuildConfiguration(t);
    bc->setDisplayName("all");

    ProjectExplorer::BuildStepList *buildSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    PremakeMakeStep *makeStep = new PremakeMakeStep(buildSteps);
    buildSteps->insertStep(0, makeStep);

    makeStep->setBuildTarget("all", /* on = */ true);

    bc->setBuildDirectory(genericproject->projectDirectory());

    t->addBuildConfiguration(bc);

    t->addDeployConfiguration(t->deployConfigurationFactory()->create(t, ProjectExplorer::Constants::DEFAULT_DEPLOYCONFIGURATION_ID));

    // Add a runconfiguration. The CustomExecutableRC one will query the user
    // for its settings, so it is a good choice here.
    t->addRunConfiguration(new ProjectExplorer::CustomExecutableRunConfiguration(t));

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
    PremakeProject *genericproject = static_cast<PremakeProject *>(parent);
    PremakeTarget *target = new PremakeTarget(genericproject);
    if (target->fromMap(map))
        return target;
    delete target;
    return 0;
}
