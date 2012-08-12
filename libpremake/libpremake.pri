INCLUDEPATH += \
    $$PWD/../premake/src/host \
    $$PWD/../premake/src/host/lua-5.1.4/src

LIBS += -L$$PWD -lPremake -framework CoreServices
