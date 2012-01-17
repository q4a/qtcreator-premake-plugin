#include "premakebuildsettingswidget.h"

#include "premakeproject.h"
#include "premaketarget.h"

#include <projectexplorer/toolchain.h>
#include <projectexplorer/toolchainmanager.h>
#include <utils/pathchooser.h>

#include <QtGui/QFormLayout>
#include <QtGui/QMainWindow>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>

using namespace PremakeProjectManager;
using namespace PremakeProjectManager::Internal;
using namespace ProjectExplorer;

PremakeBuildSettingsWidget::PremakeBuildSettingsWidget(PremakeTarget *target)
    : m_target(target), m_toolChainChooser(0), m_buildConfiguration(0)
{
    QFormLayout *fl = new QFormLayout(this);
    fl->setContentsMargins(0, -1, 0, -1);
    fl->setFieldGrowthPolicy(QFormLayout::ExpandingFieldsGrow);

    // Qt Profile

    // tool chain
    m_toolChainChooser = new QComboBox;
    m_toolChainChooser->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    updateToolChainList();
    fl->addRow(tr("Tool chain:"), m_toolChainChooser);

    // Shadow build
    m_shadowBuild = new QCheckBox;
    fl->addRow(tr("Shadow build:"), m_shadowBuild);
    connect(m_shadowBuild, SIGNAL(toggled(bool)), this, SLOT(shadowBuildToggled(bool)));

    // build directory
    m_pathChooser = new Utils::PathChooser(this);
    m_pathChooser->setEnabled(true);
    m_pathChooser->setBaseDirectory(m_target->premakeProject()->projectDirectory());
    fl->addRow(tr("Build directory:"), m_pathChooser);
    connect(m_pathChooser, SIGNAL(changed(QString)), this, SLOT(buildDirectoryChanged()));


    connect(m_toolChainChooser, SIGNAL(activated(int)), this, SLOT(toolChainSelected(int)));
    connect(m_target->premakeProject(), SIGNAL(toolChainChanged(ToolChain*)),
            this, SLOT(toolChainChanged(ToolChain*)));
    connect(ToolChainManager::instance(), SIGNAL(toolChainAdded(ToolChain*)),
            this, SLOT(updateToolChainList()));
    connect(ToolChainManager::instance(), SIGNAL(toolChainRemoved(ToolChain*)),
            this, SLOT(updateToolChainList()));
}

PremakeBuildSettingsWidget::~PremakeBuildSettingsWidget()
{ }

QString PremakeBuildSettingsWidget::displayName() const
{ return tr("Premake Manager"); }

void PremakeBuildSettingsWidget::init(BuildConfiguration *bc)
{
    m_buildConfiguration = static_cast<PremakeBuildConfiguration *>(bc);
    m_shadowBuild->setChecked(m_buildConfiguration->shadowBuildEnabled());
    m_pathChooser->setPath(m_buildConfiguration->rawBuildDirectory());
    m_pathChooser->setEnabled(m_buildConfiguration->shadowBuildEnabled());
}

void PremakeBuildSettingsWidget::buildDirectoryChanged()
{
    m_buildConfiguration->setBuildDirectory(m_pathChooser->rawPath());
}

void PremakeBuildSettingsWidget::toolChainSelected(int index)
{
    using namespace ProjectExplorer;

    ToolChain *tc = static_cast<ToolChain *>(m_toolChainChooser->itemData(index).value<void *>());
    m_target->activeBuildConfiguration()->setToolChain(tc);
    m_target->premakeProject()->refresh(PremakeProject::Everything);
}

void PremakeBuildSettingsWidget::toolChainChanged(ProjectExplorer::ToolChain *tc)
{
    for (int i = 0; i < m_toolChainChooser->count(); ++i) {
        ToolChain * currentTc = static_cast<ToolChain *>(m_toolChainChooser->itemData(i).value<void *>());
        if (currentTc != tc)
            continue;
        m_toolChainChooser->setCurrentIndex(i);
        return;
    }
}

void PremakeBuildSettingsWidget::updateToolChainList()
{
    m_toolChainChooser->clear();

    QList<ToolChain *> tcs = ToolChainManager::instance()->toolChains();
    if (!m_target->activeBuildConfiguration()->toolChain()) {
        m_toolChainChooser->addItem(tr("<Invalid tool chain>"), qVariantFromValue(static_cast<void *>(0)));
        m_toolChainChooser->setCurrentIndex(0);
    }
    foreach (ToolChain *tc, tcs) {
        // We don't support MSVC yet
        if (abiIsMsvc(tc->targetAbi()))
            continue;

        m_toolChainChooser->addItem(tc->displayName(), qVariantFromValue(static_cast<void *>(tc)));
        if (m_target->activeBuildConfiguration()->toolChain()
                && m_target->activeBuildConfiguration()->toolChain()->id() == tc->id())
            m_toolChainChooser->setCurrentIndex(m_toolChainChooser->count() - 1);
        if (!m_target->activeBuildConfiguration()->toolChain())
            m_target->activeBuildConfiguration()->setToolChain(tc);
    }
}


void PremakeBuildSettingsWidget::shadowBuildToggled(bool checked)
{
    m_pathChooser->setEnabled(checked);
    m_buildConfiguration->setShadowBuildEnabled(checked);
}
