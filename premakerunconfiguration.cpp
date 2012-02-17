/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Konstantin Tokarev <annulen@yandex.ru>
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

#include "premakerunconfiguration.h"

#include "premakeproject.h"
#include "premaketarget.h"

#include <utils/pathchooser.h>
#include <utils/detailswidget.h>
#include <utils/debuggerlanguagechooser.h>
#include <utils/qtcprocess.h>
#include <utils/stringutils.h>

#include <QtCore/QDir>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;

const char PREMAKE_RC_ID[] = "PremakeProjectManager.PremakeRunConfiguration";
const char PREMAKE_RC_PREFIX[] = "PremakeProjectManager.PremakeRunConfiguration.";

QString buildTargetFromId(const QString &id)
{
    if (!id.startsWith(QLatin1String(PREMAKE_RC_PREFIX)))
        return QString();
    return id.mid(QString::fromLatin1(PREMAKE_RC_PREFIX).length());
}

QString idFromBuildTarget(const QString &target)
{
    return QString::fromLatin1(PREMAKE_RC_PREFIX) + target;
}


PremakeRunConfiguration::PremakeRunConfiguration(PremakeTarget *parent, const QByteArray &projectName)
    : LocalApplicationRunConfiguration(parent, QLatin1String(PREMAKE_RC_ID))
{
}

PremakeRunConfiguration::~PremakeRunConfiguration()
{
}

PremakeTarget *PremakeRunConfiguration::premakeTarget() const
{
    return static_cast<PremakeTarget *>(target());
}

QWidget *PremakeRunConfiguration::createConfigurationWidget()
{
    return new QWidget();
}

QString PremakeRunConfiguration::executable() const
{
    return m_buildTarget;
}

ProjectExplorer::LocalApplicationRunConfiguration::RunMode PremakeRunConfiguration::runMode() const
{
    return m_runMode;
}

QString PremakeRunConfiguration::workingDirectory() const
{
    return QDir::cleanPath(environment().expandVariables(
                Utils::expandMacros(baseWorkingDirectory(), macroExpander())));
}

QString PremakeRunConfiguration::baseWorkingDirectory() const
{
    if (!m_userWorkingDirectory.isEmpty())
        return m_userWorkingDirectory;
    return m_workingDirectory;
}

QString PremakeRunConfiguration::commandLineArguments() const
{
    return m_commandLineArguments;
}

Utils::Environment PremakeRunConfiguration::environment() const
{
    Utils::Environment env = baseEnvironment();
    env.modify(userEnvironmentChanges());
    return env;
}

QString PremakeRunConfiguration::dumperLibrary() const
{
    QtSupport::BaseQtVersion *version = premakeTarget()->activeBuildConfiguration()->qtVersion();
    if (version)
        return version->gdbDebuggingHelperLibrary();
    return QString();
}

QStringList PremakeRunConfiguration::dumperLibraryLocations() const
{
    QtSupport::BaseQtVersion *version = premakeTarget()->activeBuildConfiguration()->qtVersion();
    if (version)
        return version->debuggingHelperLibraryLocations();
    return QStringList();
}

bool PremakeRunConfiguration::fromMap(const QVariantMap &map)
{
    return true;
}

QString PremakeRunConfiguration::defaultDisplayName() const
{
    if (m_title.isEmpty())
        return tr("Run Premake target");
    return m_title + (m_enabled ? QLatin1String("") : tr(" (disabled)"));
}

PremakeRunConfiguration::PremakeRunConfiguration(PremakeTarget *parent, PremakeRunConfiguration *source)
    : ProjectExplorer::LocalApplicationRunConfiguration(parent, source)
{
}


Utils::Environment PremakeRunConfiguration::baseEnvironment() const
{
    Utils::Environment env;
    if (m_baseEnvironmentBase == PremakeRunConfiguration::CleanEnvironmentBase) {
        // Nothing
    } else  if (m_baseEnvironmentBase == PremakeRunConfiguration::SystemEnvironmentBase) {
        env = Utils::Environment::systemEnvironment();
    } else  if (m_baseEnvironmentBase == PremakeRunConfiguration::BuildEnvironmentBase) {
        env = activeBuildConfiguration()->environment();
    }
    return env;
}

QString PremakeRunConfiguration::baseEnvironmentText() const
{
    if (m_baseEnvironmentBase == PremakeRunConfiguration::CleanEnvironmentBase) {
        return tr("Clean Environment");
    } else  if (m_baseEnvironmentBase == PremakeRunConfiguration::SystemEnvironmentBase) {
        return tr("System Environment");
    } else  if (m_baseEnvironmentBase == PremakeRunConfiguration::BuildEnvironmentBase) {
        return tr("Build Environment");
    }
    return QString();
}


void PremakeRunConfiguration::setBaseEnvironmentBase(PremakeRunConfiguration::BaseEnvironmentBase env)
{
    if (m_baseEnvironmentBase == env)
        return;
    m_baseEnvironmentBase = env;
//    emit baseEnvironmentChanged();
}

PremakeRunConfiguration::BaseEnvironmentBase PremakeRunConfiguration::baseEnvironmentBase() const
{
    return m_baseEnvironmentBase;
}


QList<Utils::EnvironmentItem> PremakeRunConfiguration::userEnvironmentChanges() const
{
    return m_userEnvironmentChanges;
}


// Factory
PremakeRunConfigurationFactory::PremakeRunConfigurationFactory(QObject *parent) :
    ProjectExplorer::IRunConfigurationFactory(parent)
{
    qDebug() << Q_FUNC_INFO;
}

PremakeRunConfigurationFactory::~PremakeRunConfigurationFactory()
{
}

// used to show the list of possible additons to a project, returns a list of ids
QStringList PremakeRunConfigurationFactory::availableCreationIds(ProjectExplorer::Target *parent) const
{
    qDebug() << Q_FUNC_INFO;
    PremakeTarget *t = qobject_cast<PremakeTarget *>(parent);
    if (!t)
        return QStringList();
    QStringList allIds;
    foreach (const QString &buildTarget, t->premakeProject()->buildTargetTitles())
        allIds << idFromBuildTarget(buildTarget);
    return allIds;
}

// used to translate the ids to names to display to the user
QString PremakeRunConfigurationFactory::displayNameForId(const QString &id) const
{
    return buildTargetFromId(id);
}

bool PremakeRunConfigurationFactory::canCreate(ProjectExplorer::Target *parent, const QString &id) const
{
    PremakeTarget *t = qobject_cast<PremakeTarget *>(parent);
    if (!t)
        return false;
    return t->premakeProject()->hasBuildTarget(buildTargetFromId(id));
}

ProjectExplorer::RunConfiguration *PremakeRunConfigurationFactory::create(ProjectExplorer::Target *parent, const QString &id)
{
    if (!canCreate(parent, id))
        return 0;
    PremakeTarget *t(static_cast<PremakeTarget *>(parent));

    const QString title(buildTargetFromId(id));
    return new PremakeRunConfiguration(t, "");
//    const PremakeBuildTarget &ct = t->premakeProject()->buildTargetForTitle(title);
//    return new PremakeRunConfiguration(t, ct.executable, ct.workingDirectory, ct.title);
}

bool PremakeRunConfigurationFactory::canClone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *source) const
{
    if (!qobject_cast<PremakeTarget *>(parent))
        return false;
    return source->id() == QLatin1String(PREMAKE_RC_ID);
}

ProjectExplorer::RunConfiguration *PremakeRunConfigurationFactory::clone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration * source)
{
    if (!canClone(parent, source))
        return 0;
    PremakeTarget *t(static_cast<PremakeTarget *>(parent));
    PremakeRunConfiguration *prc(static_cast<PremakeRunConfiguration *>(source));
    return new PremakeRunConfiguration(t, prc);
}

bool PremakeRunConfigurationFactory::canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const
{
    if (!qobject_cast<PremakeTarget *>(parent))
        return false;
    QString id(ProjectExplorer::idFromMap(map));
    return id.startsWith(QLatin1String(PREMAKE_RC_ID));
}

ProjectExplorer::RunConfiguration *PremakeRunConfigurationFactory::restore(ProjectExplorer::Target *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;
    PremakeTarget *t(static_cast<PremakeTarget *>(parent));
    PremakeRunConfiguration *rc(new PremakeRunConfiguration(t, ""));
    if (rc->fromMap(map))
        return rc;
    delete rc;
    return 0;
}
