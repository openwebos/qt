TARGET = qgoodlife
TEMPLATE = lib
CONFIG += plugin

QT += opengl
QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/platforms

DEFINES += GOODLIFE_EGL_MODIFIED \
           QT_QWS_KBD_LINUXINPUT

HEADERS =   qeglfsintegration.h \
            ../eglconvenience/qeglconvenience.h \
            ../eglconvenience/qeglplatformcontext.h \
            qeglfswindow.h \
            qeglfswindowsurface.h \
            qeglfsscreen.h \
            qinputdevicescanner.h \
            qlinuxkeyboard.h \
            qlinuxmouse.h \
            qgoodlifecursor.h \
            qgoodlifecursordraw.h

SOURCES =   main.cpp \
            qeglfsintegration.cpp \
            ../eglconvenience/qeglconvenience.cpp \
            ../eglconvenience/qeglplatformcontext.cpp \
            qeglfswindow.cpp \
            qeglfswindowsurface.cpp \
            qeglfsscreen.cpp \
            qinputdevicescanner.cpp \
            qlinuxkeyboard.cpp \
            qlinuxmouse.cpp \
            qgoodlifecursor.cpp \
            qgoodlifecursordraw.cpp

HEADERS += $$QT_SOURCE_TREE/src/gui/embedded/qkbd_qws.h \
           $$QT_SOURCE_TREE/src/gui/embedded/qkbd_qws_p.h

SOURCES += $$QT_SOURCE_TREE/src/gui/embedded/qkbd_qws.cpp


INCLUDEPATH  += ${INC_OPENSRC}/kadaptor \
                ${INC_OPENSRC}/kdriver

QMAKE_LIBDIR += ${LIB_OPENSRC}

LIBS         += -lkadaptor

include(../fontdatabases/genericunix/genericunix.pri)

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
