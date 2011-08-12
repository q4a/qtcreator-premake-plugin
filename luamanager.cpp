#include "luamanager.h"

#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>

#include <QtCore/QFile>
#include <QtCore/QTextStream>
#include <QtCore/QDebug>

using namespace PremakeProjectManager;

LuaManager::LuaManager() : m_bytecodeWriteStream(&m_bytecode, QIODevice::WriteOnly)
{
    // Load bridge code
    QFile premakebridge(":/premakeproject/premakebridge.lua");
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
    Core::ICore::instance()->messageManager()->printToOutputPanePopup("Premake error: " + errorMessage);
}

lua_State * LuaManager::luaStateForParsing(const QString &fileName) const
{
    lua_State *L = lua_open();
    luaL_openlibs(L);

    // Initialize Premake
    QByteArray file = QFile::encodeName(QString("--file=").append(fileName));
    const char *argv[3];
    argv[0] = "";
    argv[1] = file.data();
    argv[2] = "_qtcreator";

    if(premake_init(L, 3, argv) != 0){
        projectParseError(lua_tostring(L, -1));
    }

    const char * lua_bridge_code = m_premakeBridge.constData();

    if (luaL_dostring(L, lua_bridge_code) != 0) {
        projectParseError(lua_tostring(L, -1));
    }
    return L;
}

lua_State * LuaManager::luaStateForGenerating(const QString &fileName,
                                                  bool shadowBuild,
                                                  const QString &buildDir) const
{
    qDebug() << Q_FUNC_INFO << fileName << buildDir;
    lua_State *L = lua_open();
    luaL_openlibs(L);

    // Initialize Premake
    QByteArray file = QFile::encodeName(QString("--file=").append(fileName));
    QByteArray to = QFile::encodeName(QString("--to=").append(buildDir));
    const char *argv[4];
    int argc;
    argv[0] = "";
    argv[1] = file.data();
    if(shadowBuild) {
        argv[2] = to.data();
        argv[3] = "_qtcreator_generate";
        argc = 4;
    } else {
        argv[2] = "_qtcreator_generate";
        argc = 3;
    }

    if(premake_init(L, argc, argv) != 0){
        projectParseError(lua_tostring(L, -1));
    }

    const char * lua_bridge_code = m_premakeBridge.constData();

    if (luaL_dostring(L, lua_bridge_code) != 0) {
        projectParseError(lua_tostring(L, -1));
    }
    return L;
}
