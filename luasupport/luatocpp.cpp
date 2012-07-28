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

#include <QDebug>
#include <QStringList>

bool LuaSupport::Internal::getFieldByPath(lua_State *L, const QList<QByteArray> &fields, const QByteArray &objname)
{
    Q_ASSERT(fields.size() > 0);

    // Bring target table on stack
    lua_checkstack(L, fields.size());
    lua_getglobal(L, fields.first().data());
    for (int i = 1; i < fields.size(); ++i) {
        if (!lua_istable(L, -1)) {
            qWarning() << "Cannot access" << objname << ":" << fields.at(i-1) << "is not table";
            lua_pop(L, i);
            return false;
        }
        lua_getfield(L, -1, fields.at(i).data());
    }
    return true;
}

using namespace LuaSupport;

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
        const QString value = QString::fromLocal8Bit(lua_tostring(L, -1));
        lua_pop(L, 1);
        m_to << value;
    }
    return true;
}

QString GetStringList::error() const
{
    return QString();
}

CallLuaFunctionSingleReturnValue::CallLuaFunctionSingleReturnValue(const QList<QByteArray> &args)
    : m_args(args)
{
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

QString CallLuaFunctionSingleReturnValue::result()
{
    return m_result;
}

QString CallLuaFunctionSingleReturnValue::error() const
{
    return m_error;
}


GetStringMap::GetStringMap(StringMap &to)
    : m_to(to)
{
}

bool GetStringMap::call(lua_State *L)
{
    m_to.clear();
    for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
        if (lua_type(L, -2) != LUA_TSTRING)
            continue;

        const QString key = QString::fromLocal8Bit(lua_tostring(L, -2));
        const QString value = QString::fromLocal8Bit(lua_tostring(L, -1));
        m_to[key] = value;
    }
    return true;
}

QString GetStringMap::error() const
{
    return QString();
}


GetStringMapList::GetStringMapList(StringMapList &to)
    : m_to(to)
{
}

bool GetStringMapList::call(lua_State *L)
{
    m_to.clear();
    int n_elements = lua_objlen(L, -1);
    // Lua index starts from 1
    for(int i = 1; i <= n_elements; ++i) {
        lua_pushinteger(L, i);
        lua_gettable(L, -2);

        if (lua_type(L, -1) == LUA_TTABLE) {
            StringMap map;
            for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
                if (lua_type(L, -2) != LUA_TSTRING)
                    continue;

                const QString key = QString::fromLocal8Bit(lua_tostring(L, -2));
                const QString value = QString::fromLocal8Bit(lua_tostring(L, -1));
                map[key] = value;
            }
            m_to << map;
        }
        lua_pop(L, 1);
    }
    return true;
}

QString GetStringMapList::error() const
{
    return QString();
}
