#include "premakebuildsettingswidget.h"

#include "ui_premakeprojectconfigwidget.h"

#include "premakeproject.h"
#include "premaketarget.h"

#include <coreplugin/icore.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/toolchain.h>
#include <projectexplorer/toolchainmanager.h>
#include <qtsupport/qtsupportconstants.h>
#include <qtsupport/qtversionmanager.h>
#include <utils/pathchooser.h>

#include <QtGui/QFormLayout>
#include <QtGui/QMainWindow>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;
using namespace ProjectExplorer;
using namespace QtSupport;

PremakeBuildSettingsWidget::PremakeBuildSettingsWidget(PremakeTarget *target)
    : m_target(target)
    , m_buildConfiguration(0)
    , m_ui(new Ui::PremakeProjectConfigWidget)
{
    m_ui->setupUi(this);

    updateToolChainList();
    connect(m_ui->shadowBuildCheckBox, SIGNAL(toggled(bool)), this, SLOT(shadowBuildToggled(bool)));

    m_ui->shadowBuildDirEdit->setBaseDirectory(m_target->premakeProject()->projectDirectory());
    m_ui->shadowBuildDirEdit->setExpectedKind(Utils::PathChooser::ExistingDirectory);
    m_ui->shadowBuildDirEdit->setPromptDialogTitle(tr("Shadow Build Directory"));
    connect(m_ui->shadowBuildDirEdit, SIGNAL(changed(QString)), this, SLOT(buildDirectoryChanged()));

    m_ui->problemLabel->hide();
    m_ui->importLabel->hide();
    m_ui->warningLabel->hide();

    connect(m_ui->toolChainComboBox, SIGNAL(activated(int)), this, SLOT(toolChainSelected(int)));
    connect(m_ui->qtVersionComboBox, SIGNAL(activated(int)), this, SLOT(qtVersionSelected(int)));

    connect(m_target->premakeProject(), SIGNAL(toolChainChanged(ToolChain*)),
            this, SLOT(toolChainChanged(ToolChain*)));
    connect(ToolChainManager::instance(), SIGNAL(toolChainAdded(ToolChain*)),
            this, SLOT(updateToolChainList()));
    connect(ToolChainManager::instance(), SIGNAL(toolChainRemoved(ToolChain*)),
            this, SLOT(updateToolChainList()));

    connect(m_ui->manageQtVersionPushButtons, SIGNAL(clicked()),
            this, SLOT(manageQtVersions()));

    connect(m_ui->manageToolChainPushButton, SIGNAL(clicked()),
            this, SLOT(manageToolChains()));
}

PremakeBuildSettingsWidget::~PremakeBuildSettingsWidget()
{ }

QString PremakeBuildSettingsWidget::displayName() const
{ return tr("Premake Manager"); }

void PremakeBuildSettingsWidget::init(BuildConfiguration *bc)
{
    m_buildConfiguration = static_cast<PremakeBuildConfiguration *>(bc);
    m_ui->shadowBuildCheckBox->setChecked(m_buildConfiguration->shadowBuildEnabled());
    m_ui->shadowBuildDirEdit->setPath(m_buildConfiguration->rawBuildDirectory());
    m_ui->shadowBuildDirEdit->setEnabled(m_buildConfiguration->shadowBuildEnabled());
}

void PremakeBuildSettingsWidget::buildDirectoryChanged()
{
    m_buildConfiguration->setBuildDirectory(m_ui->shadowBuildDirEdit->rawPath());
}

void PremakeBuildSettingsWidget::toolChainSelected(int index)
{
    ToolChain *tc = static_cast<ToolChain *>(m_ui->toolChainComboBox->itemData(index).value<void *>());
    m_target->activeBuildConfiguration()->setToolChain(tc);
    updateQtVersionList();
    m_target->premakeProject()->refresh(PremakeProject::Everything);
}

void PremakeBuildSettingsWidget::qtVersionSelected(int index)
{
    BaseQtVersion *ver = static_cast<BaseQtVersion *>(m_ui->qtVersionComboBox->itemData(index).value<void *>());
    m_target->activeBuildConfiguration()->setQtVersion(ver);
    m_target->premakeProject()->refresh(PremakeProject::Everything);
}

void PremakeBuildSettingsWidget::toolChainChanged(ProjectExplorer::ToolChain *tc)
{
    for (int i = 0; i < m_ui->toolChainComboBox->count(); ++i) {
        ToolChain * currentTc = static_cast<ToolChain *>(m_ui->toolChainComboBox->itemData(i).value<void *>());
        if (currentTc != tc)
            continue;
        m_ui->toolChainComboBox->setCurrentIndex(i);
        return;
    }
}

void PremakeBuildSettingsWidget::updateToolChainList()
{
    m_ui->toolChainComboBox->clear();

    QList<ToolChain *> tcs = ToolChainManager::instance()->toolChains();
    if (!m_target->activeBuildConfiguration()->toolChain()) {
        m_ui->toolChainComboBox->addItem(tr("<Invalid tool chain>"), qVariantFromValue(static_cast<void *>(0)));
        m_ui->toolChainComboBox->setCurrentIndex(0);
    }
    foreach (ToolChain *tc, tcs) {
        // We don't support MSVC yet
        if (abiIsMsvc(tc->targetAbi()))
            continue;

        m_ui->toolChainComboBox->addItem(tc->displayName(), qVariantFromValue(static_cast<void *>(tc)));
        if (m_target->activeBuildConfiguration()->toolChain()
                && m_target->activeBuildConfiguration()->toolChain()->id() == tc->id())
            m_ui->toolChainComboBox->setCurrentIndex(m_ui->toolChainComboBox->count() - 1);
        if (!m_target->activeBuildConfiguration()->toolChain()) {
            m_target->activeBuildConfiguration()->setToolChain(tc);
        }
    }
    updateQtVersionList();
}

static bool qtVersionCompatibleWithToolchain(const BaseQtVersion *ver, const ToolChain *tc)
{
    if (ver && tc) {
        const Abi &tcAbi = tc->targetAbi();
        foreach (const Abi &abi, ver->qtAbis()) {
            qDebug() << Q_FUNC_INFO << abi.toString() << tcAbi.toString() << abi.isCompatibleWith(tcAbi);
            if (abi.isCompatibleWith(tcAbi))
                return true;
        }
    }
    return false;
}

void PremakeBuildSettingsWidget::updateQtVersionList()
{
    m_ui->qtVersionComboBox->clear();

    ToolChain *tc = m_target->activeBuildConfiguration()->toolChain();
    BaseQtVersion *currentVer = m_target->activeBuildConfiguration()->qtVersion();
    if (currentVer && !qtVersionCompatibleWithToolchain(currentVer, tc)) {
        m_target->activeBuildConfiguration()->setQtVersion(0);
    }

    QtVersionManager *vm = QtVersionManager::instance();
    foreach (BaseQtVersion *ver, vm->validVersions()) {
        if (tc && qtVersionCompatibleWithToolchain(ver, tc)) {
            m_ui->qtVersionComboBox->addItem(ver->displayName(),
                                             qVariantFromValue(static_cast<void *>(ver)));

            if (ver->uniqueId() == m_target->activeBuildConfiguration()->qtVersionId())
                m_ui->qtVersionComboBox->setCurrentIndex(m_ui->qtVersionComboBox->count() - 1);
            if (!m_target->activeBuildConfiguration()->qtVersion())
                m_target->activeBuildConfiguration()->setQtVersion(ver);
        }
    }

    if (!m_target->activeBuildConfiguration()->qtVersion()) {
        m_ui->qtVersionComboBox->addItem(tr("<Auto-detected Qt version>"), qVariantFromValue(static_cast<void *>(0)));
        m_ui->qtVersionComboBox->setCurrentIndex(0);
    }
}


void PremakeBuildSettingsWidget::shadowBuildToggled(bool checked)
{
    m_ui->shadowBuildDirEdit->setEnabled(checked);
    m_buildConfiguration->setShadowBuildEnabled(checked);
}

void PremakeBuildSettingsWidget::manageQtVersions()
{
    Core::ICore *core = Core::ICore::instance();
    core->showOptionsDialog(QLatin1String(ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_CATEGORY),
                            QLatin1String(QtSupport::Constants::QTVERSION_SETTINGS_PAGE_ID));
}

void PremakeBuildSettingsWidget::manageToolChains()
{
    Core::ICore *core = Core::ICore::instance();
    core->showOptionsDialog(QLatin1String(ProjectExplorer::Constants::PROJECTEXPLORER_SETTINGS_CATEGORY),
                            QLatin1String(ProjectExplorer::Constants::TOOLCHAIN_SETTINGS_PAGE_ID));
}
