#include "luasupport/luatocpp.h"

extern "C" {
#include "lauxlib.h"
#include "lualib.h"
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
        CallLuaFunctionSingleReturnValue callback(args);
        QVERIFY(!luaRecursiveAccessor(L, "a.b", callback));
        QCOMPARE(lua_gettop(L), 0);

        QVERIFY(luaRecursiveAccessor(L, "a.b.c", callback));
        QCOMPARE(callback.result(), QLatin1String("Success!arg1arg2"));
        QCOMPARE(lua_gettop(L), 0);

        QVERIFY(!luaRecursiveAccessor(L, "a.b.c.d", callback));
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

    void tableToStringMap()
    {
        const char *test_code = "a = {}; a.b = {}; a.b.c = { 'abcd', 'efgh', 'ijkl', 'mnop' };"
                "a.b.d = { a = 'b', c = 'd' }";
        QCOMPARE(luaL_dostring(L, test_code), 0);

        StringMap to;
        GetStringMap callback(to);
        QVERIFY(luaRecursiveAccessor(L, "a.b", callback));
        StringMap m1;
        m1["c"] = QString();
        m1["d"] = QString();
        QCOMPARE(to, m1);
        QCOMPARE(lua_gettop(L), 0);

        QVERIFY(luaRecursiveAccessor(L, "a.b.c", callback));
        QCOMPARE(to, StringMap());
        QCOMPARE(lua_gettop(L), 0);

        QVERIFY(luaRecursiveAccessor(L, "a.b.d", callback));
        StringMap m2;
        m2["a"] = QString("b");
        m2["c"] = QString("d");
        QCOMPARE(to, m2);
        QCOMPARE(lua_gettop(L), 0);
    }



    void tableToStringMapList()
    {
        const char *test_code = "a = {}; a.b = {}; a.b.c = { 'abcd', 'efgh', 'ijkl', 'mnop' };"
                "a.b.d = { { a = 'b', c = 'd' }, { x = 'i', y = 'j' } }";
        QCOMPARE(luaL_dostring(L, test_code), 0);

        StringMapList to;
        GetStringMapList callback(to);
        QVERIFY(luaRecursiveAccessor(L, "a.b", callback));
        QCOMPARE(to, StringMapList());
        QCOMPARE(lua_gettop(L), 0);

        QVERIFY(luaRecursiveAccessor(L, "a.b.c", callback));
        QCOMPARE(to, StringMapList());
        QCOMPARE(lua_gettop(L), 0);

        QVERIFY(luaRecursiveAccessor(L, "a.b.d", callback));
        StringMap m1;
        m1["a"] = QString("b");
        m1["c"] = QString("d");
        StringMap m2;
        m2["x"] = QString("i");
        m2["y"] = QString("j");
        QCOMPARE(to, StringMapList() << m1 << m2);
        QCOMPARE(lua_gettop(L), 0);
    }

private:
    lua_State *L;
};

QTEST_APPLESS_MAIN(LuaToCppTest)

#include "tst_luatocpp.moc"
