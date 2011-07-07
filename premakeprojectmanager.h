/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2011 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (info@qt.nokia.com)
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

#ifndef PREMAKEPROJECTMANAGER_H
#define PREMAKEPROJECTMANAGER_H

#include <projectexplorer/iprojectmanager.h>
#include <coreplugin/icontext.h>
#include <QtCore/QDataStream>

struct lua_State;

namespace PremakeProjectManager {
namespace Internal {

class PremakeProject;

class PremakeManager : public ProjectExplorer::IProjectManager
{
    Q_OBJECT

public:
    PremakeManager();
    virtual ~PremakeManager();

    virtual Core::Context projectContext() const;
    virtual Core::Context projectLanguage() const;

    virtual QString mimeType() const;
    virtual ProjectExplorer::Project *openProject(const QString &fileName);

    void notifyChanged(const QString &fileName);

    void registerProject(PremakeProject *project);
    void unregisterProject(PremakeProject *project);

    // return ready to use L
    lua_State *createPremakeLuaState(const QString &fileName) const;


private:
    int writer(lua_State *L, const void *p, size_t size, void *u);

    Core::Context m_projectContext;
    Core::Context m_projectLanguage;
    QList<PremakeProject *> m_projects;
    QByteArray m_premakeBridge;
    QByteArray m_bytecode;
    QDataStream m_bytecodeWriteStream;
};

} // namespace Internal
} // namespace PremakeProjectManager

#endif // PREMAKEPROJECTMANAGER_H
