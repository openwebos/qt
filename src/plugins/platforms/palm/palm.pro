TARGET = qpalm
TEMPLATE = lib
CONFIG += plugin $$(WEBOS_CONFIG)

QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/platforms

SOURCES = main.cpp \
          hiddtp_qpa.cpp \
          NyxInputControl.cpp \
          nyxkeyboardhandler.cpp

HEADERS += hidd_qpa.h \
           hiddtp_qpa.h \
           InputControl.h \
           NyxInputControl.h \
           nyxkeyboardhandler.h \
           FlickGesture.h \
           ScreenEdgeFlickGesture.h

webos {
    qemu* {
        include(../fb_base/fb_base.pri)
        SOURCES += ../linuxfb/qlinuxfbintegration.cpp \
                   emulatorfbintegration.cpp
        HEADERS += ../linuxfb/qlinuxfbintegration.h \
                   emulatorfbintegration.h

        LIBS_PRIVATE += -lnyx
    } else {
        QT += opengl
        SOURCES += qeglfsintegration.cpp \
                   ../eglconvenience/qeglconvenience.cpp \
                   ../eglconvenience/qeglplatformcontext.cpp \
                   qeglfswindow.cpp \
                   qeglfswindowsurface.cpp \
                   qeglfsscreen.cpp

        HEADERS += qeglfsintegration.h \
                   ../eglconvenience/qeglconvenience.h \
                   ../eglconvenience/qeglplatformcontext.h \
                   qeglfswindow.h \
                   qeglfswindowsurface.h \
                   qeglfsscreen.h
        DEFINES += TARGET_DEVICE
        LIBS_PRIVATE += -lnyx -lhid -ldl
    }
}


INCLUDEPATH += ../clipboards
SOURCES += ../clipboards/qwebosclipboard.cpp
HEADERS += ../clipboards/qwebosclipboard.h

include(../fontdatabases/genericunix/genericunix.pri)

QMAKE_CXXFLAGS += $$QT_CFLAGS_GLIB
LIBS_PRIVATE +=$$QT_LIBS_GLIB
target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
