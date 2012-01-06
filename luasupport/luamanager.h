#ifndef LUAMANAGER_H
#define LUAMANAGER_H

#include <QtCore/QString>
#include <QtCore/QDataStream>

extern "C" {
#include "premake/src/host/premake.h"
}

namespace LuaSupport {

class LuaManager
{
public:
    static LuaManager *instance();

    // return ready to use L
    lua_State *initLuaState(const QString &fileName, const QByteArray &action,
                                  bool shadowBuild, const QString &buildDir) const;

private:
    LuaManager();
    ~LuaManager();
    int writer(lua_State *L, const void *p, size_t size, void *u);

    QByteArray m_premakeBridge;
    QByteArray m_bytecode;
    QDataStream m_bytecodeWriteStream;
};

}

#endif // LUAMANAGER_H
