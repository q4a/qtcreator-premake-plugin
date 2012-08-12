#include "premake.h"

int main(int argc, const char** argv)
{
    lua_State* L;
    int z = OKAY;

    /* prepare Lua for use */
    L = lua_open();
    luaL_openlibs(L);

    /* Load built-in scripts and initialize global variables */
    z = premake_init(L, argc, argv);

    if (z == OKAY)  z = call_premake_main(L);
    /* Clean up and turn off the lights */
    lua_close(L);
    return z;
}
