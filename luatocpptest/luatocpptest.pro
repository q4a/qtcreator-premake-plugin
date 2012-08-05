#-------------------------------------------------
#
# Project created by QtCreator 2012-07-28T17:20:01
#
#-------------------------------------------------

QT       += testlib

QT       -= gui

TARGET = tst_luatocpp
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

include(../premake.pri)

INCLUDEPATH += ..
SOURCES += tst_luatocpp.cpp \
    ../luasupport/luatocpp.cpp

HEADERS += \
    ../luasupport/luatocpp.h
