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

#ifndef PREMAKERUNCONFIGURATION_H
#define PREMAKERUNCONFIGURATION_H

#include "premaketarget.h"

#include <projectexplorer/applicationrunconfiguration.h>
#include <utils/environment.h>

namespace PremakeProjectManager {

namespace Internal {

class PremakeRunConfiguration : public ProjectExplorer::LocalApplicationRunConfiguration
{
    friend class PremakeRunConfigurationFactory;
public:
    PremakeRunConfiguration(PremakeTarget *parent, const QByteArray &projectName);
    ~PremakeRunConfiguration();

    PremakeTarget *premakeTarget() const;

    QWidget *createConfigurationWidget();

    QString executable() const;
    RunMode runMode() const;
    QString workingDirectory() const;
    QString commandLineArguments() const;
    Utils::Environment environment() const;
    QString dumperLibrary() const;
    QStringList dumperLibraryLocations() const;

protected:
    virtual bool fromMap(const QVariantMap &map);
    QString defaultDisplayName() const;

private:
    PremakeRunConfiguration(PremakeTarget *parent, PremakeRunConfiguration *source);
    void setUserWorkingDirectory(const QString &workingDirectory);
    QString baseWorkingDirectory() const;
    void ctor();

    enum BaseEnvironmentBase { CleanEnvironmentBase = 0,
                               SystemEnvironmentBase = 1,
                               BuildEnvironmentBase = 2};
    void setBaseEnvironmentBase(BaseEnvironmentBase env);
    BaseEnvironmentBase baseEnvironmentBase() const;
    Utils::Environment baseEnvironment() const;
    QString baseEnvironmentText() const;

    void setUserEnvironmentChanges(const QList<Utils::EnvironmentItem> &diff);
    QList<Utils::EnvironmentItem> userEnvironmentChanges() const;

    RunMode m_runMode;
    QByteArray m_projectName;
    QString m_buildTarget;
    QString m_workingDirectory;
    QString m_userWorkingDirectory;
    QString m_commandLineArguments;
    QString m_title;
    QString m_arguments;
    QList<Utils::EnvironmentItem> m_userEnvironmentChanges;
    BaseEnvironmentBase m_baseEnvironmentBase;
    bool m_enabled;
};

class PremakeRunConfigurationFactory : public ProjectExplorer::IRunConfigurationFactory
{
    Q_OBJECT

public:
    explicit PremakeRunConfigurationFactory(QObject *parent = 0);
    ~PremakeRunConfigurationFactory();

    bool canCreate(ProjectExplorer::Target *parent, const QString &id) const;
    ProjectExplorer::RunConfiguration *create(ProjectExplorer::Target *parent, const QString &id);
    bool canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const;
    ProjectExplorer::RunConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map);
    bool canClone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *product) const;
    ProjectExplorer::RunConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::RunConfiguration *product);

    QStringList availableCreationIds(ProjectExplorer::Target *parent) const;
    QString displayNameForId(const QString &id) const;
};

} // namespace Internal
} // namespace PremakeProjectManager

#endif // PREMAKERUNCONFIGURATION_H
