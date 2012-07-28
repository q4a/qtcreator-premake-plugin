#include "luasupport/luatocpp.h"

extern "C" {
#include "premake/src/host/premake.h"
}

#include <QString>
#include <QtTest>
#include <QStringList>

using namespace LuaSupport;

class LuaToCppTest : public QObject
{
    Q_OBJECT

private Q_SLOTS:
    void init()
    {
        L = lua_open();
        luaL_openlibs(L);
    }

    void cleanup()
    {
        lua_close(L);
    }

    void callLuaFunction()
    {
        const char *test_code = "a = {}; a.b = {}; a.b.c = function(x, y) return 'Success!' .. x .. y end";
        QCOMPARE(luaL_dostring(L, test_code), 0);

        const QList<QByteArray> args = QList<QByteArray>() << "arg1" << "arg2";
        CallLuaFunctionSingleReturnValue callback;
        callback.setArgs(args);
        QVERIFY(!luaRecursiveAccessor(L, "a.b", callback));
        QCOMPARE(lua_gettop(L), 0);

        QVERIFY(luaRecursiveAccessor(L, "a.b.c", callback));
        QCOMPARE(callback.result(), QLatin1String("Success!arg1arg2"));
        QCOMPARE(lua_gettop(L), 0);
    }

    void tableToStringList()
    {
        const char *test_code = "a = {}; a.b = {}; a.b.c = { 'abcd', 'efgh', 'ijkl', 'mnop' }";
        QCOMPARE(luaL_dostring(L, test_code), 0);

        QStringList to;
        GetStringList callback(to);
        QVERIFY(luaRecursiveAccessor(L, "a.b", callback));
        QCOMPARE(to, QStringList());
        QCOMPARE(lua_gettop(L), 0);

        QVERIFY(luaRecursiveAccessor(L, "a.b.c", callback));
        QCOMPARE(to, QStringList() << "abcd" << "efgh" << "ijkl" << "mnop");
        QCOMPARE(lua_gettop(L), 0);
    }

private:
    lua_State *L;
};

QTEST_APPLESS_MAIN(LuaToCppTest)

#include "tst_luatocpp.moc"
