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
    //Core::ICore::instance()->messageManager()->printToOutputPanePopup(
    //        QCoreApplication::translate("LuaManager", "Premake error: ") + errorMessage);
    const QString fullMessage = QCoreApplication::translate("LuaManager", "Premake error: %1").arg(errorMessage);
    const Core::MessageManager::PrintToOutputPaneFlags flags = Core::MessageManager::ModeSwitch;
    Core::MessageManager::write(fullMessage, flags);
}

lua_State * LuaManager::initLuaState(const QString &fileName,
                                     const QByteArray &action,
                                     const QByteArray &configuration,
                                     bool shadowBuild,
                                     const QString &buildDir,
                                     bool usesQt,
                                     const QString &qmakePath) const
{
    lua_State *L = lua_open();
    luaL_openlibs(L);

    // Initialize Premake
    QByteArray file = QFile::encodeName(QString::fromLatin1("--file=").append(fileName));
    QByteArray to = QFile::encodeName(QString::fromLatin1("--to=").append(buildDir));
    QByteArray qmake = QFile::encodeName(QString::fromLatin1("--qt-qmake=").append(qmakePath));
    const char *argv[5];
    int argc = 0;
    argv[argc++] = "";
    argv[argc++] = file.data();

    if (shadowBuild)
        argv[argc++] = to.data();

    if (usesQt)
        argv[argc++] = qmake.data();

    argv[argc++] = action.data();

    if (!configuration.isEmpty())
        argv[argc++] = configuration.data();

    if(premake_init(L, argc, argv) != 0)
        projectParseError(QString::fromLocal8Bit(lua_tostring(L, -1)));

    const char * lua_bridge_code = m_premakeBridge.constData();

    if (luaL_dostring(L, lua_bridge_code) != 0) {
        projectParseError(QString::fromLocal8Bit(lua_tostring(L, -1)));
    }
    return L;
}
