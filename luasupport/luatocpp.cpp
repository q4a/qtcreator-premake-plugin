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

#include <QStringList>

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
