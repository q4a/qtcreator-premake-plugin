TEMPLATE = lib
TARGET = PremakeProjectManager

isEmpty(QTC_SOURCE): QTC_SOURCE = ../qt-creator
isEmpty(QTC_BUILD): QTC_BUILD = $$QTC_SOURCE
macx:isEmpty(QTC_INSTALL_ROOT): QTC_INSTALL_ROOT = "$$QTC_BUILD/bin/Qt Creator.app"

exists($$QTC_BUILD/src/app/app_version.h) {
    DEFINES += USE_APP_VERSION
}
!exists($$QTC_BUILD/src/plugins/coreplugin/ide_version.h):!exists($$QTC_BUILD/src/app/app_version.h) {
    error("Please set QTC_BUILD to build directory of Qt Creator")
}


message("Qt Creator source code: QTC_SOURCE=$$QTC_SOURCE")
message("Qt Creator binaries: QTC_BUILD=$$QTC_BUILD")
message("Qt Creator installation root: QTC_INSTALL_ROOT=$$QTC_INSTALL_ROOT")

IDE_BUILD_TREE = $$QTC_BUILD

include($$QTC_SOURCE/src/qtcreatorplugin.pri)
include($$QTC_SOURCE/src/plugins/projectexplorer/projectexplorer.pri)
include($$QTC_SOURCE/src/plugins/cpptools/cpptools.pri)
include($$QTC_SOURCE/src/plugins/texteditor/texteditor.pri)
include(luasupport/luasupport.pri)

INCLUDEPATH += . premake/src/host/lua-5.1.4/src
DEFINES += PREMAKE_LIBRARY NDEBUG
HEADERS += premakeproject.h \
    premakeprojectplugin.h \
    premaketarget.h \
    premakeprojectmanager.h \
    premakeprojectconstants.h \
    premakeprojectnodes.h \
    premakeprojectwizard.h \
    premakemakestep.h \
    premakebuildconfiguration.h \
    makestep.h
SOURCES += premakeproject.cpp \
    premakeprojectplugin.cpp \
    premaketarget.cpp \
    premakeprojectmanager.cpp \
    premakeprojectnodes.cpp \
    premakeprojectwizard.cpp \
    premakemakestep.cpp \
    premakebuildconfiguration.cpp \
    makestep.cpp

SOURCES += \
    scripts.c \
    premake/src/host/premake.c \
    premake/src/host/lua-5.1.4/src/lzio.c \
    premake/src/host/lua-5.1.4/src/lvm.c \
    premake/src/host/lua-5.1.4/src/lundump.c \
    premake/src/host/lua-5.1.4/src/ltm.c \
    premake/src/host/lua-5.1.4/src/ltablib.c \
    premake/src/host/lua-5.1.4/src/ltable.c \
    premake/src/host/lua-5.1.4/src/lstrlib.c \
    premake/src/host/lua-5.1.4/src/lstring.c \
    premake/src/host/lua-5.1.4/src/lstate.c \
    premake/src/host/lua-5.1.4/src/lparser.c \
    premake/src/host/lua-5.1.4/src/loslib.c \
    premake/src/host/lua-5.1.4/src/lopcodes.c \
    premake/src/host/lua-5.1.4/src/lobject.c \
    premake/src/host/lua-5.1.4/src/loadlib.c \
    premake/src/host/lua-5.1.4/src/lmem.c \
    premake/src/host/lua-5.1.4/src/lmathlib.c \
    premake/src/host/lua-5.1.4/src/llex.c \
    premake/src/host/lua-5.1.4/src/liolib.c \
    premake/src/host/lua-5.1.4/src/linit.c \
    premake/src/host/lua-5.1.4/src/lgc.c \
    premake/src/host/lua-5.1.4/src/lfunc.c \
    premake/src/host/lua-5.1.4/src/ldump.c \
    premake/src/host/lua-5.1.4/src/ldo.c \
    premake/src/host/lua-5.1.4/src/ldebug.c \
    premake/src/host/lua-5.1.4/src/ldblib.c \
    premake/src/host/lua-5.1.4/src/lcode.c \
    premake/src/host/lua-5.1.4/src/lbaselib.c \
    premake/src/host/lua-5.1.4/src/lauxlib.c \
    premake/src/host/lua-5.1.4/src/lapi.c \
    premake/src/host/string_endswith.c \
    premake/src/host/path_isabsolute.c \
    premake/src/host/os_uuid.c \
    premake/src/host/os_rmdir.c \
    premake/src/host/os_pathsearch.c \
    premake/src/host/os_mkdir.c \
    premake/src/host/os_match.c \
    premake/src/host/os_isfile.c \
    premake/src/host/os_isdir.c \
    premake/src/host/os_is64bit.c \
    premake/src/host/os_getversion.c \
    premake/src/host/os_getcwd.c \
    premake/src/host/os_copyfile.c \
    premake/src/host/os_chdir.c \
    premake/src/host/os_stat.c
RESOURCES += premakeproject.qrc
FORMS += premakemakestep.ui \
    makestep.ui
OTHER_FILES += premakebridge.lua \
    PremakeProject.mimetypes.xml \
    README

unix:!macx {
    DEFINES += LUA_USE_DLOPEN LUA_USE_POSIX
}

linux*:LIBS += -ldl

macx {
    LIBS += -framework CoreServices
    DEFINES += LUA_USE_MACOSX
}

win32 {
    DEFINES += _CRT_SECURE_NO_WARNINGS
    LIBS += -luser32 -lole32
}

!macx {
    lua_highlight.target = $$QTC_BUILD/share/qtcreator/generic-highlighter/lua.xml
} else {
    lua_highlight.target = $$IDE_DATA_PATH/generic-highlighter/lua.xml
}
!win32 {
    lua_highlight.commands = $$QMAKE_COPY generic-highlighter/lua.xml \"$$lua_highlight.target\"
    QMAKE_EXTRA_TARGETS += lua_highlight
    PRE_TARGETDEPS += $$lua_highlight.target
}

## Installation ##
!isEmpty(QTC_INSTALL_ROOT) {

    install_lua_highlight.files = generic-highlighter/lua.xml

    unix:!macx {
        install_plugin.files = \
            $$DESTDIR/lib$${TARGET}.so \
            $$DESTDIR/$${TARGET}.pluginspec

        exists($$QTC_INSTALL_ROOT/lib/qtcreator) {
            install_plugin.path = $$QTC_INSTALL_ROOT/lib/qtcreator/plugins/Nokia
        } else:exists($$QTC_INSTALL_ROOT/lib64/qtcreator) {
            install_plugin.path = $$QTC_INSTALL_ROOT/lib64/qtcreator/plugins/Nokia
        } else {
            error("Cannot find library path of Qt Creator")
        }
        install_lua_highlight.path = $$QTC_INSTALL_ROOT/share/qtcreator/generic-highlighter
    }

    macx {
        install_plugin.files = \
            $$DESTDIR/lib$${TARGET}.dylib \
            $$DESTDIR/$${TARGET}.pluginspec

        install_plugin.path = "$$QTC_INSTALL_ROOT/Contents/PlugIns/Nokia"
        install_lua_highlight.path = "$$QTC_INSTALL_ROOT/Contents/Resources/generic-highlighter"
    }

#win32:      install_plugin.files = $$DESTDIR/$${TARGET}.dll $$DESTDIR/$${TARGET}.pluginspec # ???

    INSTALLS = \
        install_plugin \
        install_lua_highlight
}















