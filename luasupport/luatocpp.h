/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2012 Konstantin Tokarev <annulen@yandex.ru>
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

#ifndef LUATOCPP_H
#define LUATOCPP_H

extern "C" {
#include "lua.h"
}

#include <QList>
#include <QString>

struct lua_State;
class QByteArray;
class QString;
class QStringList;

namespace LuaSupport {

namespace Internal {
bool getFieldByPath(lua_State *L, const QList<QByteArray> &fields, const QByteArray &objname);
}

template <typename Callback>
bool luaRecursiveAccessor(lua_State *L, const QByteArray &objname, Callback &callback)
{
    const QList<QByteArray> fields = objname.split('.');
    if (!Internal::getFieldByPath(L, fields, objname))
        return false; // getFieldByPath cleans stack on failure

    // Perform callback action
    bool result = callback.call(L);
    // Restore stack state
    lua_pop(L, fields.size());

    return result;
}

class GetStringList
{
public:
    GetStringList(QStringList &to);
    bool call(lua_State *L);
    QString error() const;

private:
    QStringList &m_to;
};

class CallLuaFunctionSingleReturnValue
{
public:
    CallLuaFunctionSingleReturnValue(const QList<QByteArray> &args);
    bool call(lua_State *L);
    QString result();
    QString error() const;

private:
    QList<QByteArray> m_args;
    QString m_result;
    QString m_error;
};

}

#endif // LUATOCPP_H
