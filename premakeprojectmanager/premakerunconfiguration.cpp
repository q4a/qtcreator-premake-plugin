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

#include <coreplugin/coreconstants.h>
#include <projectexplorer/environmentwidget.h>
#include <utils/pathchooser.h>
#include <utils/detailswidget.h>
#include <utils/qtcprocess.h>
#include <utils/stringutils.h>

#include <QtCore/QDir>
#include <QFormLayout>
#include <QLineEdit>
#include <QToolButton>
#include <QComboBox>
#include <QCheckBox>
#include <QLabel>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;

const char PREMAKE_RC_ID[] = "PremakeProjectManager.PremakeRunConfiguration";
const char PREMAKE_RC_PREFIX[] = "PremakeProjectManager.PremakeRunConfiguration.";

const char USER_WORKING_DIRECTORY_KEY[] = "PremakeProjectManager.PremakeRunConfiguration.UserWorkingDirectory";
const char USE_TERMINAL_KEY[] = "PremakeProjectManager.PremakeRunConfiguration.UseTerminal";
const char TITLE_KEY[] = "PremakeProjectManager.PremakeRunConfiguation.Title";
const char ARGUMENTS_KEY[] = "PremakeProjectManager.PremakeRunConfiguration.Arguments";
const char USER_ENVIRONMENT_CHANGES_KEY[] = "PremakeProjectManager.PremakeRunConfiguration.UserEnvironmentChanges";
const char BASE_ENVIRONMENT_BASE_KEY[] = "PremakeProjectManager.BaseEnvironmentBase";

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

PremakeRunConfiguration::PremakeRunConfiguration(PremakeTarget *parent, const QString &target,
                                                 const QString &workingDirectory, const QString &title)
    : LocalApplicationRunConfiguration(parent, QLatin1String(PREMAKE_RC_ID))
    , m_runMode(Gui)
    , m_buildTarget(target)
    , m_workingDirectory(workingDirectory)
    , m_title(title)
    , m_baseEnvironmentBase(BuildEnvironmentBase)
    , m_enabled(true)
{
    setDefaultDisplayName(defaultDisplayName());
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
    return new PremakeRunConfigurationWidget(this);
}

QString PremakeRunConfiguration::executable() const
{
    return m_buildTarget;
}

ProjectExplorer::LocalApplicationRunConfiguration::RunMode PremakeRunConfiguration::runMode() const
{
    return m_runMode;
}

void PremakeRunConfiguration::setRunMode(ProjectExplorer::LocalApplicationRunConfiguration::RunMode runMode)
{
    m_runMode = runMode;
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

void PremakeRunConfiguration::setCommandLineArguments(const QString &args)
{
    m_commandLineArguments = args;
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

bool PremakeRunConfiguration::isEnabled() const
{
    return m_enabled;
}

QString PremakeRunConfiguration::disabledReason() const
{
    if (!m_enabled)
        return tr("The executable is not built by the current build configuration");
    return QString();
}

QVariantMap PremakeRunConfiguration::toMap() const
{
    QVariantMap map(ProjectExplorer::LocalApplicationRunConfiguration::toMap());

    map.insert(QLatin1String(USER_WORKING_DIRECTORY_KEY), m_userWorkingDirectory);
    map.insert(QLatin1String(USE_TERMINAL_KEY), m_runMode == Console);
    map.insert(QLatin1String(TITLE_KEY), m_title);
    map.insert(QLatin1String(ARGUMENTS_KEY), m_arguments);
    map.insert(QLatin1String(USER_ENVIRONMENT_CHANGES_KEY), Utils::EnvironmentItem::toStringList(m_userEnvironmentChanges));
    map.insert(QLatin1String(BASE_ENVIRONMENT_BASE_KEY), m_baseEnvironmentBase);

    return map;
}

QString PremakeRunConfiguration::title() const
{
    return m_title;
}

void PremakeRunConfiguration::setExecutable(const QString &executable)
{
    m_buildTarget = executable;
}

void PremakeRunConfiguration::setBaseWorkingDirectory(const QString &wd)
{
    const QString oldWorkingDirectory = workingDirectory();

    m_workingDirectory = wd;

    const QString &newWorkingDirectory = workingDirectory();
    if (oldWorkingDirectory != newWorkingDirectory)
        emit baseWorkingDirectoryChanged(newWorkingDirectory);
}

void PremakeRunConfiguration::setEnabled(bool b)
{
    if (m_enabled == b)
        return;
    m_enabled = b;
    emit isEnabledChanged(isEnabled());
    setDefaultDisplayName(defaultDisplayName());
}

bool PremakeRunConfiguration::fromMap(const QVariantMap &map)
{
    m_userWorkingDirectory = map.value(QLatin1String(USER_WORKING_DIRECTORY_KEY)).toString();
    m_runMode = map.value(QLatin1String(USE_TERMINAL_KEY)).toBool() ? Console : Gui;
    m_title = map.value(QLatin1String(TITLE_KEY)).toString();
    m_arguments = map.value(QLatin1String(ARGUMENTS_KEY)).toString();
    m_userEnvironmentChanges = Utils::EnvironmentItem::fromStringList(map.value(QLatin1String(USER_ENVIRONMENT_CHANGES_KEY)).toStringList());
    m_baseEnvironmentBase = static_cast<BaseEnvironmentBase>(map.value(QLatin1String(BASE_ENVIRONMENT_BASE_KEY), static_cast<int>(BuildEnvironmentBase)).toInt());

    return RunConfiguration::fromMap(map);
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

void PremakeRunConfiguration::setUserWorkingDirectory(const QString &wd)
{
    const QString oldWorkingDirectory = workingDirectory();

    m_userWorkingDirectory = wd;

    const QString &newWorkingDirectory = workingDirectory();
    if (oldWorkingDirectory != newWorkingDirectory)
        emit baseWorkingDirectoryChanged(newWorkingDirectory);
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

void PremakeRunConfiguration::setUserEnvironmentChanges(const QList<Utils::EnvironmentItem> &diff)
{
    if (m_userEnvironmentChanges != diff) {
        m_userEnvironmentChanges = diff;
        emit userEnvironmentChangesChanged(diff);
    }
}


void PremakeRunConfiguration::setBaseEnvironmentBase(PremakeRunConfiguration::BaseEnvironmentBase env)
{
    if (m_baseEnvironmentBase == env)
        return;
    m_baseEnvironmentBase = env;
    emit baseEnvironmentChanged();
}

PremakeRunConfiguration::BaseEnvironmentBase PremakeRunConfiguration::baseEnvironmentBase() const
{
    return m_baseEnvironmentBase;
}


QList<Utils::EnvironmentItem> PremakeRunConfiguration::userEnvironmentChanges() const
{
    return m_userEnvironmentChanges;
}


// Configuration widget
PremakeRunConfigurationWidget::PremakeRunConfigurationWidget(PremakeRunConfiguration *premakeRunConfiguration, QWidget *parent)
    : QWidget(parent), m_ignoreChange(false), m_premakeRunConfiguration(premakeRunConfiguration)
{
    QFormLayout *fl = new QFormLayout();
    fl->setMargin(0);
    fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);
    QLineEdit *argumentsLineEdit = new QLineEdit();
    argumentsLineEdit->setText(premakeRunConfiguration->commandLineArguments());
    connect(argumentsLineEdit, SIGNAL(textChanged(QString)),
            this, SLOT(setArguments(QString)));
    fl->addRow(tr("Arguments:"), argumentsLineEdit);

    m_workingDirectoryEdit = new Utils::PathChooser();
    m_workingDirectoryEdit->setExpectedKind(Utils::PathChooser::Directory);
    m_workingDirectoryEdit->setBaseDirectory(m_premakeRunConfiguration->target()->project()->projectDirectory());
    m_workingDirectoryEdit->setPath(m_premakeRunConfiguration->baseWorkingDirectory());
    m_workingDirectoryEdit->setPromptDialogTitle(tr("Select Working Directory"));

    QToolButton *resetButton = new QToolButton();
    resetButton->setToolTip(tr("Reset to default"));
    resetButton->setIcon(QIcon(QLatin1String(Core::Constants::ICON_RESET)));

    QHBoxLayout *boxlayout = new QHBoxLayout();
    boxlayout->addWidget(m_workingDirectoryEdit);
    boxlayout->addWidget(resetButton);

    fl->addRow(tr("Working directory:"), boxlayout);

    QCheckBox *runInTerminal = new QCheckBox;
    fl->addRow(tr("Run in Terminal"), runInTerminal);

    m_detailsContainer = new Utils::DetailsWidget(this);
    m_detailsContainer->setState(Utils::DetailsWidget::NoSummary);

    QWidget *m_details = new QWidget(m_detailsContainer);
    m_detailsContainer->setWidget(m_details);
    m_details->setLayout(fl);

    QVBoxLayout *vbx = new QVBoxLayout(this);
    vbx->setMargin(0);;
    vbx->addWidget(m_detailsContainer);

    QLabel *environmentLabel = new QLabel(this);
    environmentLabel->setText(tr("Run Environment"));
    QFont f = environmentLabel->font();
    f.setBold(true);
    f.setPointSizeF(f.pointSizeF() *1.2);
    environmentLabel->setFont(f);
    vbx->addWidget(environmentLabel);

    QWidget *baseEnvironmentWidget = new QWidget;
    QHBoxLayout *baseEnvironmentLayout = new QHBoxLayout(baseEnvironmentWidget);
    baseEnvironmentLayout->setMargin(0);
    QLabel *label = new QLabel(tr("Base environment for this runconfiguration:"), this);
    baseEnvironmentLayout->addWidget(label);
    m_baseEnvironmentComboBox = new QComboBox(this);
    m_baseEnvironmentComboBox->addItems(QStringList()
                                        << tr("Clean Environment")
                                        << tr("System Environment")
                                        << tr("Build Environment"));
    m_baseEnvironmentComboBox->setCurrentIndex(m_premakeRunConfiguration->baseEnvironmentBase());
    connect(m_baseEnvironmentComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(baseEnvironmentComboBoxChanged(int)));
    baseEnvironmentLayout->addWidget(m_baseEnvironmentComboBox);
    baseEnvironmentLayout->addStretch(10);

    m_environmentWidget = new ProjectExplorer::EnvironmentWidget(this, baseEnvironmentWidget);
    m_environmentWidget->setBaseEnvironment(m_premakeRunConfiguration->baseEnvironment());
    m_environmentWidget->setBaseEnvironmentText(m_premakeRunConfiguration->baseEnvironmentText());
    m_environmentWidget->setUserChanges(m_premakeRunConfiguration->userEnvironmentChanges());

    vbx->addWidget(m_environmentWidget);

    connect(m_workingDirectoryEdit, SIGNAL(changed(QString)),
            this, SLOT(setWorkingDirectory()));

    connect(resetButton, SIGNAL(clicked()),
            this, SLOT(resetWorkingDirectory()));

    connect(runInTerminal, SIGNAL(toggled(bool)),
            this, SLOT(runInTerminalToggled(bool)));

    connect(m_environmentWidget, SIGNAL(userChangesChanged()),
            this, SLOT(userChangesChanged()));

    connect(m_premakeRunConfiguration, SIGNAL(baseWorkingDirectoryChanged(QString)),
            this, SLOT(workingDirectoryChanged(QString)));
    connect(m_premakeRunConfiguration, SIGNAL(baseEnvironmentChanged()),
            this, SLOT(baseEnvironmentChanged()));
    connect(m_premakeRunConfiguration, SIGNAL(userEnvironmentChangesChanged(QList<Utils::EnvironmentItem>)),
            this, SLOT(userEnvironmentChangesChanged()));

    setEnabled(m_premakeRunConfiguration->isEnabled());
}

void PremakeRunConfigurationWidget::setWorkingDirectory()
{
    if (m_ignoreChange)
        return;
    m_ignoreChange = true;
    m_premakeRunConfiguration->setUserWorkingDirectory(m_workingDirectoryEdit->rawPath());
    m_ignoreChange = false;
}

void PremakeRunConfigurationWidget::workingDirectoryChanged(const QString &workingDirectory)
{
    if (!m_ignoreChange)
        m_workingDirectoryEdit->setPath(workingDirectory);
}

void PremakeRunConfigurationWidget::resetWorkingDirectory()
{
    // This emits a signal connected to workingDirectoryChanged()
    // that sets the m_workingDirectoryEdit
    m_premakeRunConfiguration->setUserWorkingDirectory(QString());
}

void PremakeRunConfigurationWidget::runInTerminalToggled(bool toggled)
{
    m_premakeRunConfiguration->setRunMode(toggled ? ProjectExplorer::LocalApplicationRunConfiguration::Console
                                                : ProjectExplorer::LocalApplicationRunConfiguration::Gui);
}

void PremakeRunConfigurationWidget::userChangesChanged()
{
    m_premakeRunConfiguration->setUserEnvironmentChanges(m_environmentWidget->userChanges());
}

void PremakeRunConfigurationWidget::baseEnvironmentComboBoxChanged(int index)
{
    m_ignoreChange = true;
    m_premakeRunConfiguration->setBaseEnvironmentBase(PremakeRunConfiguration::BaseEnvironmentBase(index));

    m_environmentWidget->setBaseEnvironment(m_premakeRunConfiguration->baseEnvironment());
    m_environmentWidget->setBaseEnvironmentText(m_premakeRunConfiguration->baseEnvironmentText());
    m_ignoreChange = false;
}

void PremakeRunConfigurationWidget::baseEnvironmentChanged()
{
    if (m_ignoreChange)
        return;

    m_baseEnvironmentComboBox->setCurrentIndex(m_premakeRunConfiguration->baseEnvironmentBase());
    m_environmentWidget->setBaseEnvironment(m_premakeRunConfiguration->baseEnvironment());
    m_environmentWidget->setBaseEnvironmentText(m_premakeRunConfiguration->baseEnvironmentText());
}

void PremakeRunConfigurationWidget::userEnvironmentChangesChanged()
{
    m_environmentWidget->setUserChanges(m_premakeRunConfiguration->userEnvironmentChanges());
}

void PremakeRunConfigurationWidget::setArguments(const QString &args)
{
    m_premakeRunConfiguration->setCommandLineArguments(args);
}


// Factory
PremakeRunConfigurationFactory::PremakeRunConfigurationFactory(QObject *parent) :
    ProjectExplorer::IRunConfigurationFactory(parent)
{
}

PremakeRunConfigurationFactory::~PremakeRunConfigurationFactory()
{
}

// used to show the list of possible additons to a project, returns a list of ids
QStringList PremakeRunConfigurationFactory::availableCreationIds(ProjectExplorer::Target *parent) const
{
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
    const PremakeBuildTarget &ct = t->premakeProject()->buildTargetForTitle(title);
    return new PremakeRunConfiguration(t, ct.executable, ct.workingDirectory, title);
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
    PremakeRunConfiguration *rc(new PremakeRunConfiguration(t, QString(), QString(), QString()));
    if (rc->fromMap(map))
        return rc;
    delete rc;
    return 0;
}
