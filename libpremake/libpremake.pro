TEMPLATE = lib
CONFIG += static
CONFIG -= dll
TARGET = Premake

PREMAKE_DIR = ../premake

INCLUDEPATH += $$PREMAKE_DIR/src/host/lua-5.1.4/src
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
    scripts.c \
    $$PREMAKE_DIR/src/host/premake.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lzio.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lvm.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lundump.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/ltm.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/ltablib.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/ltable.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lstrlib.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lstring.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lstate.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lparser.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/loslib.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lopcodes.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lobject.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/loadlib.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lmem.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lmathlib.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/llex.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/liolib.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/linit.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lgc.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lfunc.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/ldump.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/ldo.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/ldebug.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/ldblib.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lcode.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lbaselib.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lauxlib.c \
    $$PREMAKE_DIR/src/host/lua-5.1.4/src/lapi.c \
    $$PREMAKE_DIR/src/host/string_endswith.c \
    $$PREMAKE_DIR/src/host/path_isabsolute.c \
    $$PREMAKE_DIR/src/host/os_uuid.c \
    $$PREMAKE_DIR/src/host/os_rmdir.c \
    $$PREMAKE_DIR/src/host/os_pathsearch.c \
    $$PREMAKE_DIR/src/host/os_mkdir.c \
    $$PREMAKE_DIR/src/host/os_match.c \
    $$PREMAKE_DIR/src/host/os_isfile.c \
    $$PREMAKE_DIR/src/host/os_isdir.c \
    $$PREMAKE_DIR/src/host/os_is64bit.c \
    $$PREMAKE_DIR/src/host/os_getversion.c \
    $$PREMAKE_DIR/src/host/os_getcwd.c \
    $$PREMAKE_DIR/src/host/os_copyfile.c \
    $$PREMAKE_DIR/src/host/os_chdir.c \
    $$PREMAKE_DIR/src/host/os_stat.c
