TARGET = qtaskone
TEMPLATE = lib
CONFIG += plugin

QT += opengl
QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/platforms

DEFINES += TASKONE_EGL_MODIFIED \
           USE_MOTION \
           QT_QWS_KBD_LINUXINPUT

HEADERS =   qtaskoneintegration.h \
            ../eglconvenience/qeglconvenience.h \
            ../eglconvenience/qeglplatformcontext.h \
            qtaskonedefs.h \
            qtaskonewindow.h \
            qtaskonewindowsurface.h \
            qtaskonescreen.h \
            qinputdevicescanner.h \
            qlinuxkeyboard.h \
            qlinuxmouse.h \
            qtaskonemotion.h \
            qtaskonecursor.h \
            qtaskonecursordraw.h

SOURCES =   main.cpp \
            qtaskoneintegration.cpp \
            ../eglconvenience/qeglconvenience.cpp \
            ../eglconvenience/qeglplatformcontext.cpp \
            qtaskonewindow.cpp \
            qtaskonewindowsurface.cpp \
            qtaskonescreen.cpp \
            qinputdevicescanner.cpp \
            qlinuxkeyboard.cpp \
            qlinuxmouse.cpp \
            qtaskonemotion.cpp \
            qtaskonecursor.cpp \
            qtaskonecursordraw.cpp

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
