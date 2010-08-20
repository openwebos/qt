TEMPLATE = lib
TARGET = qgfxegl
CONFIG += qt plugin warn_on
QT += opengl

HEADERS	= \
    eglscreen.h \
    eglwindowsurface.h

SOURCES	= \
        eglscreenplugin.cpp \
        eglscreen.cpp \
        eglwindowsurface.cpp

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/gfxdrivers

target.path = $$[QT_INSTALL_PLUGINS]/gfxdrivers
INSTALLS += target
