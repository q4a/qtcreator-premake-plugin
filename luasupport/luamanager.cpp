#include "luamanager.h"

#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>

#include <QtCore/QCoreApplication>
#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>

using namespace LuaSupport;

LuaManager::LuaManager() : m_bytecodeWriteStream(&m_bytecode, QIODevice::WriteOnly)
{
    // Load bridge code
    QFile premakebridge(QLatin1String(":/premakeproject/premakebridge.lua"));
    premakebridge.open(QIODevice::ReadOnly);
    m_premakeBridge = QTextStream(&premakebridge).readAll().toUtf8();
    premakebridge.close();
}


int LuaManager::writer(lua_State*, const void* p, size_t size, void*)
{
    return m_bytecodeWriteStream.writeRawData((const char *)p, size);
}

LuaManager * LuaManager::instance()
{
    static LuaManager *obj = 0;
    if(!obj)
        obj = new LuaManager;
    return obj;
}

static void projectParseError(const QString &errorMessage)
{
    Core::ICore::instance()->messageManager()->printToOutputPanePopup(
            QCoreApplication::translate("LuaManager", "Premake error: ") + errorMessage);
}

lua_State * LuaManager::initLuaState(const QString &fileName,
                                     const QByteArray &action,
                                     bool shadowBuild,
                                     const QString &buildDir) const
{
    lua_State *L = lua_open();
    luaL_openlibs(L);

    // Initialize Premake
    QByteArray file = QFile::encodeName(QString::fromLatin1("--file=").append(fileName));
    QByteArray to = QFile::encodeName(QString::fromLatin1("--to=").append(buildDir));
    const char *argv[4];
    int argc;
    argv[0] = "";
    argv[1] = file.data();
    if(shadowBuild) {
        argv[2] = to.data();
        argv[3] = action.data();
        argc = 4;
    } else {
        argv[2] = action.data();
        argc = 3;
    }

    if(premake_init(L, argc, argv) != 0) {
        projectParseError(QString::fromLocal8Bit(lua_tostring(L, -1)));
    }

    const char * lua_bridge_code = m_premakeBridge.constData();

    if (luaL_dostring(L, lua_bridge_code) != 0) {
        projectParseError(QString::fromLocal8Bit(lua_tostring(L, -1)));
    }
    return L;
}
