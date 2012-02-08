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

#include "luatocpp.h"

extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <QDebug>
#include <QStringList>

using namespace LuaSupport;

Callback::~Callback()
{
}


bool LuaSupport::luaRecursiveAccessor(lua_State *L, const QByteArray &objname, Callback &callback)
{
    qDebug() << Q_FUNC_INFO << "enter stack pos" << lua_gettop(L);
    const QList<QByteArray> fields = objname.split('.');
    Q_ASSERT(fields.size() > 0);

    // Bring target table on stack
    lua_checkstack(L, fields.size());
    lua_getglobal(L, fields.first().data());
    for (int i = 1; i < fields.size(); ++i) {
        if (lua_isnil(L, -1)) {
            qWarning() << "Cannot access" << objname << ":" << fields.at(i-1) << "is nil";
            lua_pop(L, i);
            return false;
        }
        lua_getfield(L, -1, fields.at(i).data());
    }

    // Perform needed actions
    qDebug() << Q_FUNC_INFO << "callback enter stack pos" << lua_gettop(L);
    bool result = callback.call(L);
    qDebug() << Q_FUNC_INFO << "callback exit stack pos" << lua_gettop(L);

    // Restore stack state
    lua_pop(L, fields.size());
    qDebug() << Q_FUNC_INFO << "exit stack pos" << lua_gettop(L);
    return result;
}


GetStringList::GetStringList(QStringList &to)
    : m_to(to)
{
}

bool GetStringList::call(lua_State *L)
{
    int n_elements = lua_objlen(L, -1);
    m_to.clear();
    // Lua index starts from 1
    for(int i = 1; i <= n_elements; ++i) {
        lua_pushinteger(L, i);
        lua_gettable(L, -2);
        const QString value = QString::fromLocal8Bit(lua_tolstring(L, -1, 0));
        lua_pop(L, 1);
        m_to << value;
    }
    return true;
}

QString GetStringList::error() const
{
    return QString();
}

bool CallLuaFunctionSingleReturnValue::call(lua_State *L)
{
    m_result.clear();

    foreach (const QByteArray &arg, m_args)
        lua_pushstring(L, arg);

    if(lua_pcall(L, m_args.size(), 1, 0) != 0) {
        m_error = QString::fromLocal8Bit(lua_tostring(L, -1));
        return false;
    } else {
        m_result = QString::fromLocal8Bit(lua_tostring(L, -1));
        return true;
    }
}

void CallLuaFunctionSingleReturnValue::setArgs(const QList<QByteArray> &args)
{
    m_args = args;
}

QString CallLuaFunctionSingleReturnValue::result()
{
    return m_result;
}

QString CallLuaFunctionSingleReturnValue::error() const
{
    return m_error;
}
