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

#ifndef PREMAKESTEP_H
#define PREMAKESTEP_H

#include <projectexplorer/buildstep.h>

QT_BEGIN_NAMESPACE
class QListWidgetItem;

namespace Ui {
class PremakeStep;
}
QT_END_NAMESPACE

namespace PremakeProjectManager {
namespace Internal {

class PremakeBuildConfiguration;
class PremakeStepConfigWidget;
class PremakeStepFactory;

class PremakeStep : public ProjectExplorer::BuildStep
{
    Q_OBJECT
    friend class PremakeStepConfigWidget; // TODO remove again?
    friend class PremakeStepFactory;

public:
    PremakeStep(ProjectExplorer::BuildStepList *parent);
    virtual ~PremakeStep();

    PremakeBuildConfiguration *premakeBuildConfiguration() const;

    virtual bool init();

    virtual void run(QFutureInterface<bool> &fi);

    virtual ProjectExplorer::BuildStepConfigWidget *createConfigWidget();
    virtual bool immutable() const;
    QString allArguments() const;

    QVariantMap toMap() const;

protected:
    PremakeStep(ProjectExplorer::BuildStepList *parent, PremakeStep *bs);
    PremakeStep(ProjectExplorer::BuildStepList *parent, const QString &id);
    virtual bool fromMap(const QVariantMap &map);

private:
    void ctor();

    QStringList m_buildTargets;
    QString m_premakeArguments;
};

class PremakeStepConfigWidget : public ProjectExplorer::BuildStepConfigWidget
{
    Q_OBJECT
public:
    PremakeStepConfigWidget(PremakeStep *step);
    virtual QString displayName() const;
    void init();
    virtual QString summaryText() const;
private slots:
    void makeArgumentsLineEditTextEdited();
    void updateDetails();
private:
    Ui::PremakeStep *m_ui;
    PremakeStep *m_step;
    QString m_summaryText;
};

class PremakeStepFactory : public ProjectExplorer::IBuildStepFactory
{
    Q_OBJECT

public:
    explicit PremakeStepFactory(QObject *parent = 0);
    virtual ~PremakeStepFactory();

    virtual bool canCreate(ProjectExplorer::BuildStepList *parent,
                           const QString &id) const;
    virtual ProjectExplorer::BuildStep *create(ProjectExplorer::BuildStepList *parent,
                                               const QString &id);
    virtual bool canClone(ProjectExplorer::BuildStepList *parent,
                          ProjectExplorer::BuildStep *source) const;
    virtual ProjectExplorer::BuildStep *clone(ProjectExplorer::BuildStepList *parent,
                                              ProjectExplorer::BuildStep *source);
    virtual bool canRestore(ProjectExplorer::BuildStepList *parent,
                            const QVariantMap &map) const;
    virtual ProjectExplorer::BuildStep *restore(ProjectExplorer::BuildStepList *parent,
                                                const QVariantMap &map);

    virtual QStringList availableCreationIds(ProjectExplorer::BuildStepList *bc) const;
    virtual QString displayNameForId(const QString &id) const;
};

} // namespace Internal
} // namespace PremakeProjectManager

#endif // PREMAKESTEP_H
