#ifndef PREMAKEBUILDSETTINGSWIDGET_H
#define PREMAKEBUILDSETTINGSWIDGET_H

#include <projectexplorer/buildstep.h>

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
QT_END_NAMESPACE

QT_BEGIN_NAMESPACE
namespace Ui { class PremakeProjectConfigWidget; }
QT_END_NAMESPACE

namespace ProjectExplorer { class ToolChain; }
namespace Utils { class PathChooser; }

namespace PremakeProjectManager {
namespace Internal {

class PremakeBuildConfiguration;
class PremakeTarget;

class PremakeBuildSettingsWidget : public ProjectExplorer::BuildConfigWidget
{
    Q_OBJECT

public:
    PremakeBuildSettingsWidget(PremakeTarget *target);
    virtual ~PremakeBuildSettingsWidget();

    virtual QString displayName() const;

    virtual void init(ProjectExplorer::BuildConfiguration *bc);

private Q_SLOTS:
    void buildDirectoryChanged();
    void toolChainSelected(int index);
    void qtVersionSelected(int index);
    void toolChainChanged(ProjectExplorer::ToolChain *);
    void updateToolChainList();
    void updateQtVersionList();
    void shadowBuildToggled(bool);
    void manageQtVersions();
    void manageToolChains();

private:
    PremakeTarget *m_target;
    PremakeBuildConfiguration *m_buildConfiguration;
    Ui::PremakeProjectConfigWidget *m_ui;
};

} // namespace Internal
} // namespace PremakeProjectManager

#endif // PREMAKEBUILDSETTINGSWIDGET_H
