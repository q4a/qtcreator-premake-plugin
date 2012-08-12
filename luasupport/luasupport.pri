INCLUDEPATH += $$PWD \
    ../premake/src/host/lua-5.1.4/src

HEADERS += \
    $$PWD/luacompleter.h \
    $$PWD/luaeditor.h \
    $$PWD/luaindenter.h \
    $$PWD/luamanager.h \
    $$PWD/luaconstants.h \
    $$PWD/luatocpp.h

SOURCES += \
    $$PWD/luacompleter.cpp \
    $$PWD/luaeditor.cpp \
    $$PWD/luaindenter.cpp \
    $$PWD/luamanager.cpp \
    $$PWD/luatocpp.cpp
