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

#include "premakebuildconfiguration.h"

#include "premakestep.h"
#include "makestep.h"
#include "premakeproject.h"
#include "premaketarget.h"

#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

#include <QtWidgets/QInputDialog>

using namespace ProjectExplorer;
using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;
using ProjectExplorer::BuildConfiguration;
using namespace QtSupport;

namespace {
const char * const BUILD_DIRECTORY_KEY("PremakeProjectManager.PremakeBuildConfiguration.BuildDirectory");
const char * const SHADOW_BUILD_KEY("PremakeProjectManager.PremakeBuildConfiguration.ShadowBuild");
const char * const QT_VERSION_ID_KEY("PremakeProjectManager.PremakeBuildConfiguration.QtVersionId");
const char * const INTERNAL_CONFIGURATION_KEY("PremakeProjectManager.PremakeBuildConfiguration.InternalConfiguration");
}

PremakeBuildConfiguration::PremakeBuildConfiguration(Target *target, Core::Id id)
    : BuildConfiguration(target, id)
{
}

PremakeBuildConfiguration::~PremakeBuildConfiguration()
{
}

QVariantMap PremakeBuildConfiguration::toMap() const
{
    QVariantMap map(BuildConfiguration::toMap());
    map.insert(QLatin1String(BUILD_DIRECTORY_KEY), m_buildDirectory);
    map.insert(QLatin1String(SHADOW_BUILD_KEY), m_shadowBuildEnabled);
    map.insert(QLatin1String(QT_VERSION_ID_KEY), m_qtVersion ? m_qtVersion->uniqueId() : 0);
    map.insert(QLatin1String(INTERNAL_CONFIGURATION_KEY), m_internalConfiguration);
    return map;
}

bool PremakeBuildConfiguration::fromMap(const QVariantMap &map)
{
    if (!BuildConfiguration::fromMap(map))
        return false;

    // Default value ???
    m_internalConfiguration = map.value(QLatin1String(INTERNAL_CONFIGURATION_KEY), QByteArray()).toByteArray();

    m_buildDirectory = map.value(QLatin1String(BUILD_DIRECTORY_KEY),
        target()->project()->projectDirectory().appendString(QLatin1String("/build")).toString());
    m_shadowBuildEnabled = map.value(QLatin1String(SHADOW_BUILD_KEY),
                                     true).toBool();
    const int qtVersionId = map.value(QLatin1String(QT_VERSION_ID_KEY)).toInt();
    m_qtVersion = QtSupport::QtVersionManager::instance()->version(qtVersionId);

//    Should qtVersion support PremakeProjectManager.Target.DesktopTarget?
//    if (m_qtVersion && !m_qtVersion->supportsTargetId(target()->id()))
//        m_qtVersion = 0;

//    if (!toolChain()) {
//        QList<ProjectExplorer::ToolChain *> list = ProjectExplorer::ToolChainManager::instance()->toolChains();
//#ifdef Q_OS_WIN
//            QString toolChainId = ProjectExplorer::Constants::MINGW_TOOLCHAIN_ID;
//#else
//            QString toolChainId = ProjectExplorer::Constants::GCC_TOOLCHAIN_ID;
//#endif
//            foreach (ProjectExplorer::ToolChain *tc, list) {
//                if (tc->id().startsWith(toolChainId)) {
//                    setToolChain(tc);
//                    break;
//                }
//            }
//    }

    return true;
}

QString PremakeBuildConfiguration::buildDirectory() const
{
    const QString projectDir = target()->project()->projectDirectory();
    if (m_shadowBuildEnabled)
        // Convert to absolute path when necessary
        return QDir(projectDir).absoluteFilePath(m_buildDirectory);
    else
        return projectDir;
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
    emit environmentChanged();
    premakeTarget()->premakeProject()->refresh(PremakeProject::Everything);
}

PremakeTarget *PremakeBuildConfiguration::premakeTarget() const
{
    return static_cast<PremakeTarget *>(target());
}

QByteArray PremakeBuildConfiguration::internalConfigurationName() const
{
    return m_internalConfiguration;
}

void PremakeBuildConfiguration::setInternalConfigurationName(const QByteArray &internalConf)
{
    const QByteArray old = m_internalConfiguration;
    m_internalConfiguration = internalConf;

    if (!old.isEmpty() && old != internalConf) {
        m_shortConfiguration.clear();
        // emit ?
        premakeTarget()->premakeProject()->refresh(PremakeProject::Everything);
    }
}

QByteArray PremakeBuildConfiguration::shortConfigurationName() const
{
    return m_shortConfiguration;
}

void PremakeBuildConfiguration::setShortConfigurationName(const QByteArray &shortName)
{
    m_shortConfiguration = shortName;
}

ProjectExplorer::IOutputParser *PremakeBuildConfiguration::createOutputParser() const
{
    ProjectExplorer::ToolChain *tc = toolChain();
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
    return QStringList() << QLatin1String(Constants::PREMAKE_BC_ID);
}

QString PremakeBuildConfigurationFactory::displayNameForId(const QString &id) const
{
    if (id == QLatin1String(Constants::PREMAKE_BC_ID))
        return tr("Build");
    return QString();
}

bool PremakeBuildConfigurationFactory::canCreate(ProjectExplorer::Target *parent, const QString &id) const
{
    if (!qobject_cast<PremakeTarget *>(parent))
        return false;
    if (id == QLatin1String(Constants::PREMAKE_BC_ID))
        return true;
    return false;
}

BuildConfiguration *PremakeBuildConfigurationFactory::create(ProjectExplorer::Target *parent, const QString &id)
{
    if (!canCreate(parent, id))
        return 0;

//    //TODO asking for name is duplicated everywhere, but maybe more
//    // wizards will show up, that incorporate choosing the name
//    bool ok;
//    QString buildConfigurationName = QInputDialog::getText(0,
//                          tr("New Configuration"),
//                          tr("New configuration name:"),
//                          QLineEdit::Normal,
//                          QString(),
//                          &ok);
//    if (!ok || buildConfigurationName.isEmpty())
//        return false;

//    PremakeBuildConfiguration *bc = createBuildConfiguration(static_cast<PremakeTarget *>(parent),
//                                                             buildConfigurationName);
//    parent->addBuildConfiguration(bc); // also makes the name unique...
//    return bc;
    return 0;
}

PremakeBuildConfiguration *PremakeBuildConfigurationFactory::createBuildConfiguration(PremakeTarget *parent,
                                                                                      const QString &name,
                                                                                      const QByteArray &premakeConfigurationName)
{
    PremakeBuildConfiguration *bc = new PremakeBuildConfiguration(parent);
    bc->setInternalConfigurationName(premakeConfigurationName);
    bc->setDisplayName(name);

    ProjectExplorer::BuildStepList *buildSteps = bc->stepList(QLatin1String(ProjectExplorer::Constants::BUILDSTEPS_BUILD));
    ProjectExplorer::BuildStepList *cleanSteps = bc->stepList(QLatin1String(ProjectExplorer::Constants::BUILDSTEPS_CLEAN));
    Q_ASSERT(buildSteps);
    Q_ASSERT(cleanSteps);
    PremakeStep *premakeStep = new PremakeStep(buildSteps);
    buildSteps->insertStep(0, premakeStep);

    MakeStep *makeStep = new MakeStep(buildSteps);
    buildSteps->insertStep(1, makeStep);

    MakeStep* cleanStep = new MakeStep(cleanSteps);
    cleanStep->setClean(true);
    cleanStep->setUserArguments(QLatin1String("clean"));
    cleanSteps->insertStep(0, cleanStep);

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

int PremakeBuildConfiguration::qtVersionId() const
{
    return m_qtVersion ? m_qtVersion->uniqueId() : -1;
}

BaseQtVersion * PremakeBuildConfiguration::qtVersion() const
{
/*    if (!m_qtVersion) {
        QtVersionManager *vm = QtVersionManager::instance();
        foreach (BaseQtVersion *ver, vm->validVersions()) {
            foreach (Abi abi, ver->qtAbis()) {
                if (abiIsMsvc(abi)) {
                    qDebug() << "Skipping" << abi.toString();
                } else {
                    m_qtVersion = ver;
                    return m_qtVersion;
                }
            }
        }
    }*/
    return m_qtVersion;
}

void PremakeBuildConfiguration::setQtVersion(QtSupport::BaseQtVersion *ver)
{
    m_qtVersion = ver;
}

QString PremakeBuildConfiguration::makeCommand() const
{
    ToolChain *tc = toolChain();
    return tc ? tc->makeCommand() : QLatin1String("make");
}

QString PremakeBuildConfiguration::defaultMakeTarget() const
{
    return QString();
}

QString PremakeBuildConfiguration::makefile() const
{
    return QLatin1String("Makefile");
}

bool PremakeBuildConfiguration::shadowBuildEnabled() const
{
    return m_shadowBuildEnabled;
}

void PremakeBuildConfiguration::setShadowBuildEnabled(bool enabled)
{
    m_shadowBuildEnabled = enabled;
    emit environmentChanged();
    emit shadowBuildChanged();
    premakeTarget()->premakeProject()->refresh(PremakeProject::Everything);
}

Utils::Environment PremakeBuildConfiguration::baseEnvironment() const
{
    Utils::Environment env = BuildConfiguration::baseEnvironment();
    if (qtVersion())
        qtVersion()->addToEnvironment(env);

    ToolChain *tc = toolChain();
    if (tc)
        tc->addToEnvironment(env);
    return env;
}

bool abiIsMsvc(ProjectExplorer::Abi abi)
{
    if ((abi.osFlavor() == Abi::WindowsMsvc2005Flavor)
      || (abi.osFlavor() == Abi::WindowsMsvc2008Flavor)
      || (abi.osFlavor() == Abi::WindowsMsvc2010Flavor))
    {
        return true;
    }
    return false;
}

