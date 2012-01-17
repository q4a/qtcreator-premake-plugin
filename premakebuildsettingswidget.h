#ifndef PREMAKEBUILDSETTINGSWIDGET_H
#define PREMAKEBUILDSETTINGSWIDGET_H

#include <projectexplorer/buildstep.h>

QT_BEGIN_NAMESPACE
class QComboBox;
class QCheckBox;
QT_END_NAMESPACE

namespace ProjectExplorer {
class ToolChain;
}

namespace Utils {
class PathChooser;
}

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
    void toolChainChanged(ProjectExplorer::ToolChain *);
    void updateToolChainList();
    void shadowBuildToggled(bool);

private:
    PremakeTarget *m_target;
    Utils::PathChooser *m_pathChooser;
    QComboBox *m_toolChainChooser;
    QCheckBox *m_shadowBuild;
    PremakeBuildConfiguration *m_buildConfiguration;
};

} // namespace Internal
} // namespace PremakeProjectManager

#endif // PREMAKEBUILDSETTINGSWIDGET_H
