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
include($$QTC_SOURCE/src/plugins/qtsupport/qtsupport.pri)
include(luasupport/luasupport.pri)
include(premake.pri)

INCLUDEPATH += .
DEFINES += QT_NO_CAST_FROM_ASCII
HEADERS += premakeproject.h \
    premakeprojectplugin.h \
    premaketarget.h \
    premakeprojectmanager.h \
    premakeprojectconstants.h \
    premakeprojectnodes.h \
    premakeprojectwizard.h \
    premakestep.h \
    premakebuildconfiguration.h \
    premakebuildsettingswidget.h \
    makestep.h \
    premakerunconfiguration.h
SOURCES += premakeproject.cpp \
    premakeprojectplugin.cpp \
    premaketarget.cpp \
    premakeprojectmanager.cpp \
    premakeprojectnodes.cpp \
    premakeprojectwizard.cpp \
    premakestep.cpp \
    premakebuildconfiguration.cpp \
    premakebuildsettingswidget.cpp \
    makestep.cpp \
    premakerunconfiguration.cpp

RESOURCES += premakeproject.qrc
FORMS += premakestep.ui \
    makestep.ui \
    premakeprojectconfigwidget.ui
OTHER_FILES += premakebridge.lua \
    PremakeProject.mimetypes.xml \
    README

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















