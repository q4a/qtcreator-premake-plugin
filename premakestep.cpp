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

#include "premakestep.h"

#include "premakeprojectconstants.h"
#include "premakeproject.h"
#include "premaketarget.h"
#include "ui_premakestep.h"
#include "premakebuildconfiguration.h"
#include "luamanager.h"

#include <extensionsystem/pluginmanager.h>
#include <projectexplorer/buildsteplist.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/projectexplorer.h>
#include <coreplugin/variablemanager.h>
#include <utils/stringutils.h>
#include <utils/qtcassert.h>
#include <utils/qtcprocess.h>

#include <QtGui/QFormLayout>
#include <QtGui/QGroupBox>
#include <QtGui/QCheckBox>
#include <QtGui/QLineEdit>
#include <QtGui/QListWidget>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;
using namespace LuaSupport;

namespace {
const char * const PREMAKE_MS_ID("PremakeProjectManager.PremakeMakeStep");
const char * const PREMAKE_MS_DISPLAY_NAME(QT_TRANSLATE_NOOP("PremakeProjectManager::Internal::PremakeMakeStep",
                                                             "Premake"));

const char * const BUILD_TARGETS_KEY("PremakeProjectManager.PremakeMakeStep.BuildTargets");
const char * const MAKE_ARGUMENTS_KEY("PremakeProjectManager.PremakeMakeStep.MakeArguments");
const char * const MAKE_COMMAND_KEY("PremakeProjectManager.PremakeMakeStep.MakeCommand");
}

PremakeStep::PremakeStep(ProjectExplorer::BuildStepList *parent) :
    BuildStep(parent, QLatin1String(PREMAKE_MS_ID))
{
    ctor();
}

PremakeStep::PremakeStep(ProjectExplorer::BuildStepList *parent, const QString &id) :
    BuildStep(parent, id)
{
    ctor();
}

PremakeStep::PremakeStep(ProjectExplorer::BuildStepList *parent, PremakeStep *bs) :
    BuildStep(parent, bs),
    m_premakeArguments(bs->m_premakeArguments)
{
    ctor();
}

void PremakeStep::ctor()
{
    setDefaultDisplayName(QCoreApplication::translate("PremakeProjectManager::Internal::PremakeMakeStep",
                                                      PREMAKE_MS_DISPLAY_NAME));
}

PremakeStep::~PremakeStep()
{
}

PremakeBuildConfiguration *PremakeStep::premakeBuildConfiguration() const
{
    return static_cast<PremakeBuildConfiguration *>(buildConfiguration());
}

bool PremakeStep::init()
{
    qDebug() << Q_FUNC_INFO << m_premakeArguments;
//    PremakeBuildConfiguration *bc = premakeBuildConfiguration();

//    setEnabled(true);
//    ProjectExplorer::ProcessParameters *pp = processParameters();
//    pp->setMacroExpander(bc->macroExpander());
//    pp->setWorkingDirectory(bc->buildDirectory());
//    pp->setEnvironment(bc->environment());
//    pp->setCommand(makeCommand());
//    pp->setArguments(allArguments());

//    setOutputParser(new ProjectExplorer::GnuMakeParser());
//    if (bc->premakeTarget()->premakeProject()->toolChain())
//        appendOutputParser(bc->premakeTarget()->premakeProject()->toolChain()->outputParser());
//    outputParser()->setWorkingDirectory(pp->effectiveWorkingDirectory());

//    return AbstractProcessStep::init();
    return true;
}

QVariantMap PremakeStep::toMap() const
{
    QVariantMap map(BuildStep::toMap());
    map.insert(QLatin1String(MAKE_ARGUMENTS_KEY), m_premakeArguments);
    return map;
}

bool PremakeStep::fromMap(const QVariantMap &map)
{
    m_buildTargets = map.value(QLatin1String(BUILD_TARGETS_KEY)).toStringList();
    m_premakeArguments = map.value(QLatin1String(MAKE_ARGUMENTS_KEY)).toString();

    return BuildStep::fromMap(map);
}

QString PremakeStep::allArguments() const
{
    QString args = m_premakeArguments;
    if (premakeBuildConfiguration()->shadowBuildEnabled()) {
        QDir projectDir = QFileInfo(premakeBuildConfiguration()
                                    ->projectFileName()).dir();

        if (projectDir.absolutePath() != premakeBuildConfiguration()->buildDirectory()) {
            args += QString(" --to=%1")
                    .arg(projectDir.relativeFilePath(premakeBuildConfiguration()->buildDirectory()));
        }
    }
    args += " gmake";
    return args;
}

void PremakeStep::run(QFutureInterface<bool> &fi)
{
    emit addOutput(QString("premake4 %1").arg(allArguments()), BuildStep::MessageOutput);

    lua_State *L = LuaManager::instance()->initLuaState(
                premakeBuildConfiguration()->projectFileName(),
                "gmake",
                premakeBuildConfiguration()->shadowBuildEnabled(),
                premakeBuildConfiguration()->buildDirectory());
    if(call_premake_main(L) != 0){
        emit addOutput(lua_tostring(L, -1), BuildStep::ErrorMessageOutput);
        fi.reportResult(false);
    } else {
        emit addOutput("Premake exited normally.", BuildStep::MessageOutput);
        fi.reportResult(true);
    }
}

ProjectExplorer::BuildStepConfigWidget *PremakeStep::createConfigWidget()
{
    return new PremakeStepConfigWidget(this);
}

bool PremakeStep::immutable() const
{
    return false;
}


//
// PremakeMakeStepConfigWidget
//

PremakeStepConfigWidget::PremakeStepConfigWidget(PremakeStep *step)
    : m_step(step)
{
    m_ui = new Ui::PremakeStep;
    m_ui->setupUi(this);
    init();

    // TODO update this list also on rescans of the PremakeLists.txt
//    PremakeProject *pro = m_makeStep->premakeBuildConfiguration()->premakeTarget()->premakeProject();
//    foreach (const QString &target, pro->buildTargets()) {
//        QListWidgetItem *item = new QListWidgetItem(target, m_ui->targetsList);
//        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
//        item->setCheckState(Qt::Unchecked);
//    }

    connect(m_ui->makeArgumentsLineEdit, SIGNAL(textEdited(QString)),
            this, SLOT(makeArgumentsLineEditTextEdited()));
    connect(ProjectExplorer::ProjectExplorerPlugin::instance(), SIGNAL(settingsChanged()),
            this, SLOT(updateDetails()));
    connect(step->buildConfiguration(), SIGNAL(buildDirectoryChanged()),
            this, SLOT(updateDetails()));
    connect(step->buildConfiguration(), SIGNAL(shadowBuildChanged()),
            this, SLOT(updateDetails()));
}

QString PremakeStepConfigWidget::displayName() const
{
    return tr("Premake", "PremakeMake step display name.");
}

void PremakeStepConfigWidget::init()
{
//    updateMakeOverrrideLabel();

//    m_ui->makeLineEdit->setText(m_makeStep->m_makeCommand);
    m_ui->makeArgumentsLineEdit->setText(m_step->m_premakeArguments);

    // Disconnect to make the changes to the items
//    disconnect(m_ui->targetsList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));

//    int count = m_ui->targetsList->count();
//    for (int i = 0; i < count; ++i) {
//        QListWidgetItem *item = m_ui->targetsList->item(i);
//        item->setCheckState(m_makeStep->buildsTarget(item->text()) ? Qt::Checked : Qt::Unchecked);
//    }

    updateDetails();
    // and connect again
//    connect(m_ui->targetsList, SIGNAL(itemChanged(QListWidgetItem*)), this, SLOT(itemChanged(QListWidgetItem*)));
}

void PremakeStepConfigWidget::updateDetails()
{
    m_summaryText = QString("<b>Premake:</b> premake4 %1 %2 %3").arg(m_step->allArguments())
            .arg(tr("in")).arg(m_step->buildConfiguration()->buildDirectory());
    emit updateSummary();
}

QString PremakeStepConfigWidget::summaryText() const
{
    return m_summaryText;
}

void PremakeStepConfigWidget::makeArgumentsLineEditTextEdited()
{
    m_step->m_premakeArguments = m_ui->makeArgumentsLineEdit->text();
    updateDetails();
}

//
// PremakeMakeStepFactory
//

PremakeStepFactory::PremakeStepFactory(QObject *parent) :
    ProjectExplorer::IBuildStepFactory(parent)
{
}

PremakeStepFactory::~PremakeStepFactory()
{
}

bool PremakeStepFactory::canCreate(ProjectExplorer::BuildStepList *parent,
                                       const QString &id) const
{
    if (parent->target()->project()->id() != QLatin1String(Constants::PREMAKEPROJECT_ID))
        return false;
    return id == QLatin1String(PREMAKE_MS_ID);
}

ProjectExplorer::BuildStep *PremakeStepFactory::create(ProjectExplorer::BuildStepList *parent,
                                                           const QString &id)
{
    if (!canCreate(parent, id))
        return 0;
    return new PremakeStep(parent);
}

bool PremakeStepFactory::canClone(ProjectExplorer::BuildStepList *parent,
                                      ProjectExplorer::BuildStep *source) const
{
    const QString id(source->id());
    return canCreate(parent, id);
}

ProjectExplorer::BuildStep *PremakeStepFactory::clone(ProjectExplorer::BuildStepList *parent,
                                                          ProjectExplorer::BuildStep *source)
{
    if (!canClone(parent, source))
        return 0;
    PremakeStep *old(qobject_cast<PremakeStep *>(source));
    Q_ASSERT(old);
    return new PremakeStep(parent, old);
}

bool PremakeStepFactory::canRestore(ProjectExplorer::BuildStepList *parent,
                                        const QVariantMap &map) const
{
    QString id(ProjectExplorer::idFromMap(map));
    return canCreate(parent, id);
}

ProjectExplorer::BuildStep *PremakeStepFactory::restore(ProjectExplorer::BuildStepList *parent,
                                                            const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;
    PremakeStep *bs(new PremakeStep(parent));
    if (bs->fromMap(map))
        return bs;
    delete bs;
    return 0;
}

QStringList PremakeStepFactory::availableCreationIds(ProjectExplorer::BuildStepList *parent) const
{
    if (parent->target()->project()->id() != QLatin1String(Constants::PREMAKEPROJECT_ID))
        return QStringList();
    return QStringList() << QLatin1String(PREMAKE_MS_ID);
}

QString PremakeStepFactory::displayNameForId(const QString &id) const
{
    if (id == QLatin1String(PREMAKE_MS_ID))
        return QCoreApplication::translate("PremakeProjectManager::Internal::PremakeMakeStep",
                                           PREMAKE_MS_DISPLAY_NAME);
    return QString();
}
