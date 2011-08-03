TARGET = qpalm
TEMPLATE = lib
CONFIG += plugin

QT += opengl core-private gui-private opengl-private
QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/platforms

SOURCES =   main.cpp \
            qeglfsintegration.cpp \
            ../eglconvenience/qeglconvenience.cpp \
            ../eglconvenience/qeglplatformcontext.cpp \
            qeglfswindow.cpp \
            qeglfswindowsurface.cpp \
            qeglfsscreen.cpp \
            hiddtp_qpa.cpp \
            HalInputControl.cpp

HEADERS =   qeglfsintegration.h \
            ../eglconvenience/qeglconvenience.h \
            ../eglconvenience/qeglplatformcontext.h \
            qeglfswindow.h \
            qeglfswindowsurface.h \
            qeglfsscreen.h \
            hidd_qpa.h \
            hiddtp_qpa.h \
            InputControl.h \
            HalInputControl.h

include(../fontdatabases/genericunix/genericunix.pri)

QMAKE_CXXFLAGS += $$QT_CFLAGS_GLIB
LIBS_PRIVATE +=$$QT_LIBS_GLIB
LIBS_PRIVATE += -lhal -lhid
target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
