TARGET = qminimal
include(../../qpluginbase.pri)

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/platforms

SOURCES =   main.cpp \
            qminimalintegration.cpp \
            qminimalwindowsurface.cpp
HEADERS =   qminimalintegration.h \
            qminimalwindowsurface.h

include(../fontdatabases/genericunix/genericunix.pri)

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
