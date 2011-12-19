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

#include "premakebuildconfiguration.h"

#include "premakemakestep.h"
#include "makestep.h"
#include "premakeproject.h"
#include "premaketarget.h"

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

#include <QtGui/QInputDialog>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;
using ProjectExplorer::BuildConfiguration;

namespace {
const char * const PREMAKE_BC_ID("PremakeProjectManager.PremakeBuildConfiguration");

const char * const BUILD_DIRECTORY_KEY("PremakeProjectManager.PremakeBuildConfiguration.BuildDirectory");
const char * const SHADOW_BUILD_KEY("PremakeProjectManager.PremakeBuildConfiguration.ShadowBuild");
}

PremakeBuildConfiguration::PremakeBuildConfiguration(PremakeTarget *parent)
    : BuildConfiguration(parent, QLatin1String(PREMAKE_BC_ID))
    , m_fileName(parent->premakeProject()->file()->fileName())
{
}

PremakeBuildConfiguration::PremakeBuildConfiguration(PremakeTarget *parent, const QString &id)
    : BuildConfiguration(parent, id)
{
}

PremakeBuildConfiguration::PremakeBuildConfiguration(PremakeTarget *parent, PremakeBuildConfiguration *source) :
    BuildConfiguration(parent, source),
    m_buildDirectory(source->m_buildDirectory)
{
    cloneSteps(source);
}

PremakeBuildConfiguration::~PremakeBuildConfiguration()
{
}

QVariantMap PremakeBuildConfiguration::toMap() const
{
    QVariantMap map(BuildConfiguration::toMap());
    map.insert(QLatin1String(BUILD_DIRECTORY_KEY), m_buildDirectory);
    map.insert(QLatin1String(SHADOW_BUILD_KEY), m_shadowBuildEnabled);
    return map;
}

bool PremakeBuildConfiguration::fromMap(const QVariantMap &map)
{
    m_buildDirectory = map.value(QLatin1String(BUILD_DIRECTORY_KEY), target()->project()->projectDirectory()).toString();
    m_shadowBuildEnabled = map.value(QLatin1String(SHADOW_BUILD_KEY), target()->project()->projectDirectory()).toBool();

    return BuildConfiguration::fromMap(map);
}

QString PremakeBuildConfiguration::buildDirectory() const
{
    // Convert to absolute path when necessary
    const QDir projectDir(target()->project()->projectDirectory());
    return projectDir.absoluteFilePath(m_buildDirectory);
}

/**
 * Returns the build directory unmodified, instead of making it absolute like
 * buildDirectory() does.
 */
QString PremakeBuildConfiguration::rawBuildDirectory() const
{
    return m_buildDirectory;
}

void PremakeBuildConfiguration::setBuildDirectory(const QString &buildDirectory)
{
    if (m_buildDirectory == buildDirectory)
        return;
    m_buildDirectory = buildDirectory;
    emit buildDirectoryChanged();
}

PremakeTarget *PremakeBuildConfiguration::premakeTarget() const
{
    return static_cast<PremakeTarget *>(target());
}

ProjectExplorer::IOutputParser *PremakeBuildConfiguration::createOutputParser() const
{
    ProjectExplorer::ToolChain *tc = premakeTarget()->premakeProject()->toolChain();
    if (tc)
        return tc->outputParser();
    return 0;
}


/*!
  \class PremakeBuildConfigurationFactory
*/

PremakeBuildConfigurationFactory::PremakeBuildConfigurationFactory(QObject *parent) :
    ProjectExplorer::IBuildConfigurationFactory(parent)
{
}

PremakeBuildConfigurationFactory::~PremakeBuildConfigurationFactory()
{
}

QStringList PremakeBuildConfigurationFactory::availableCreationIds(ProjectExplorer::Target *parent) const
{
    if (!qobject_cast<PremakeTarget *>(parent))
        return QStringList();
    return QStringList() << QLatin1String(PREMAKE_BC_ID);
}

QString PremakeBuildConfigurationFactory::displayNameForId(const QString &id) const
{
    if (id == QLatin1String(PREMAKE_BC_ID))
        return tr("Build");
    return QString();
}

bool PremakeBuildConfigurationFactory::canCreate(ProjectExplorer::Target *parent, const QString &id) const
{
    if (!qobject_cast<PremakeTarget *>(parent))
        return false;
    if (id == QLatin1String(PREMAKE_BC_ID))
        return true;
    return false;
}

BuildConfiguration *PremakeBuildConfigurationFactory::create(ProjectExplorer::Target *parent, const QString &id)
{
    if (!canCreate(parent, id))
        return 0;
    PremakeTarget *target(static_cast<PremakeTarget *>(parent));

    //TODO asking for name is duplicated everywhere, but maybe more
    // wizards will show up, that incorporate choosing the name
    bool ok;
    QString buildConfigurationName = QInputDialog::getText(0,
                          tr("New Configuration"),
                          tr("New configuration name:"),
                          QLineEdit::Normal,
                          QString(),
                          &ok);
    if (!ok || buildConfigurationName.isEmpty())
        return false;
    PremakeBuildConfiguration *bc = new PremakeBuildConfiguration(target);
    bc->setDisplayName(buildConfigurationName);

    ProjectExplorer::BuildStepList *buildSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_BUILD);
    ProjectExplorer::BuildStepList *cleanSteps = bc->stepList(ProjectExplorer::Constants::BUILDSTEPS_CLEAN);
    Q_ASSERT(buildSteps);
    Q_ASSERT(cleanSteps);
    PremakeMakeStep *premakeStep = new PremakeMakeStep(buildSteps);
    buildSteps->insertStep(0, premakeStep);
    premakeStep->setBuildTarget("all", /* on = */ true);

    MakeStep *makeStep = new MakeStep(buildSteps);
    buildSteps->insertStep(1, makeStep);

    MakeStep* cleanStep = new MakeStep(cleanSteps);
    cleanStep->setClean(true);
    cleanStep->setUserArguments("clean");
    cleanSteps->insertStep(0, cleanStep);

    target->addBuildConfiguration(bc); // also makes the name unique...
    return bc;
}

bool PremakeBuildConfigurationFactory::canClone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source) const
{
    return canCreate(parent, source->id());
}

BuildConfiguration *PremakeBuildConfigurationFactory::clone(ProjectExplorer::Target *parent, BuildConfiguration *source)
{
    if (!canClone(parent, source))
        return 0;
    PremakeTarget *target(static_cast<PremakeTarget *>(parent));
    return new PremakeBuildConfiguration(target, qobject_cast<PremakeBuildConfiguration *>(source));
}

bool PremakeBuildConfigurationFactory::canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    QString id(ProjectExplorer::idFromMap(map));
    return canCreate(parent, id);
}

BuildConfiguration *PremakeBuildConfigurationFactory::restore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;
    PremakeTarget *target(static_cast<PremakeTarget *>(parent));
    PremakeBuildConfiguration *bc(new PremakeBuildConfiguration(target));
    if (bc->fromMap(map))
        return bc;
    delete bc;
    return 0;
}

BuildConfiguration::BuildType PremakeBuildConfiguration::buildType() const
{
    return Unknown;
}

QString PremakeBuildConfiguration::projectFileName() const
{
    return m_fileName;
}

#if IDE_VER >= IDE_VERSION_CHECK(2,2,80)
QtSupport::BaseQtVersion * PremakeBuildConfiguration::qtVersion() const
{
    return 0;
}

void PremakeBuildConfiguration::setQtVersion(QtSupport::BaseQtVersion *)
{
}
#endif

QString PremakeBuildConfiguration::makeCommand() const
{
    return QString("make");
}

QString PremakeBuildConfiguration::defaultMakeTarget() const
{
    return QString();
}

QString PremakeBuildConfiguration::makefile() const
{
    return QString("Makefile");
}

bool PremakeBuildConfiguration::shadowBuildEnabled() const
{
    return m_shadowBuildEnabled;
}

void PremakeBuildConfiguration::setShadowBuildEnabled(bool enabled)
{
    m_shadowBuildEnabled = enabled;
    emit shadowBuildChanged();
}

