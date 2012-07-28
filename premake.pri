INCLUDEPATH += $$PWD/premake/src/host/lua-5.1.4/src
DEFINES += PREMAKE_LIBRARY NDEBUG

unix:!macx {
    DEFINES += LUA_USE_DLOPEN LUA_USE_POSIX
}

linux*:LIBS += -ldl

macx {
    LIBS += -framework CoreServices
    DEFINES += LUA_USE_MACOSX
}

win32 {
    DEFINES -= UNICODE
    DEFINES += _CRT_SECURE_NO_WARNINGS
    LIBS += -luser32 -lole32
}

SOURCES += \
    $$PWD/scripts.c \
    $$PWD/premake/src/host/premake.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lzio.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lvm.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lundump.c \
    $$PWD/premake/src/host/lua-5.1.4/src/ltm.c \
    $$PWD/premake/src/host/lua-5.1.4/src/ltablib.c \
    $$PWD/premake/src/host/lua-5.1.4/src/ltable.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lstrlib.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lstring.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lstate.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lparser.c \
    $$PWD/premake/src/host/lua-5.1.4/src/loslib.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lopcodes.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lobject.c \
    $$PWD/premake/src/host/lua-5.1.4/src/loadlib.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lmem.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lmathlib.c \
    $$PWD/premake/src/host/lua-5.1.4/src/llex.c \
    $$PWD/premake/src/host/lua-5.1.4/src/liolib.c \
    $$PWD/premake/src/host/lua-5.1.4/src/linit.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lgc.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lfunc.c \
    $$PWD/premake/src/host/lua-5.1.4/src/ldump.c \
    $$PWD/premake/src/host/lua-5.1.4/src/ldo.c \
    $$PWD/premake/src/host/lua-5.1.4/src/ldebug.c \
    $$PWD/premake/src/host/lua-5.1.4/src/ldblib.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lcode.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lbaselib.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lauxlib.c \
    $$PWD/premake/src/host/lua-5.1.4/src/lapi.c \
    $$PWD/premake/src/host/string_endswith.c \
    $$PWD/premake/src/host/path_isabsolute.c \
    $$PWD/premake/src/host/os_uuid.c \
    $$PWD/premake/src/host/os_rmdir.c \
    $$PWD/premake/src/host/os_pathsearch.c \
    $$PWD/premake/src/host/os_mkdir.c \
    $$PWD/premake/src/host/os_match.c \
    $$PWD/premake/src/host/os_isfile.c \
    $$PWD/premake/src/host/os_isdir.c \
    $$PWD/premake/src/host/os_is64bit.c \
    $$PWD/premake/src/host/os_getversion.c \
    $$PWD/premake/src/host/os_getcwd.c \
    $$PWD/premake/src/host/os_copyfile.c \
    $$PWD/premake/src/host/os_chdir.c \
    $$PWD/premake/src/host/os_stat.c
