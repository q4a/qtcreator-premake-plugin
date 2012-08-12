isEmpty(QTC_SOURCE): QTC_SOURCE = ../../qt-creator
isEmpty(QTC_BUILD): QTC_BUILD = $$QTC_SOURCE
macx:isEmpty(QTC_INSTALL_ROOT): QTC_INSTALL_ROOT = "$$QTC_BUILD/bin/Qt Creator.app"

include($$QTC_SOURCE/qtcreator.pri)

TEMPLATE = app
TARGET = premake
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
QMAKE_LINK = $$QMAKE_LINK_C

IDE_APP_PATH = $$QTC_BUILD/bin
DESTDIR   = $$IDE_APP_PATH

include(../libpremake/libpremake.pri)

SOURCES += \
    main.c

