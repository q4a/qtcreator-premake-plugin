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

#ifndef PREMAKEPROJECTPLUGIN_H
#define PREMAKEPROJECTPLUGIN_H

#include <extensionsystem/iplugin.h>

#include <QtCore/QObject>

namespace LuaSupport {
class LuaEditorFactory;
}

namespace PremakeProjectManager {

namespace Internal {

class PremakeProjectPlugin: public ExtensionSystem::IPlugin
{
    Q_OBJECT

public:
    PremakeProjectPlugin();
    ~PremakeProjectPlugin();

    virtual bool initialize(const QStringList &arguments, QString *errorString);
    virtual void extensionsInitialized();

private:
    LuaSupport::LuaEditorFactory *m_luaEditorFactory;
};

} // namespace Internal
} // namespace PremakeProject

#endif // PREMAKEPROJECTPLUGIN_H
