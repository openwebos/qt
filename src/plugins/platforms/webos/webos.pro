TARGET = qwebos
TEMPLATE = lib
CONFIG += plugin warn_off

QT += opengl core-private gui-private opengl-private
QTDIR_build:DESTDIR = $$QT_BUILD_TREE/plugins/platforms

#include(externalplugin.pri)

INCLUDEPATH += $$QT_SOURCE_TREE/src/3rdparty/harfbuzz/src
DEFINES += QT_NO_FONTCONFIG

CONFIG += link_pkgconfig
PKGCONFIG += glib-2.0 freetype2

SOURCES =   main.cpp \
            qwebosintegration.cpp \
            qweboswindow.cpp \
            qweboswindowsurface.cpp \
            qwebosscreen.cpp \
            ../eglconvenience/qeglconvenience.cpp \
            ../eglconvenience/qeglplatformcontext.cpp \
            qbasicunixfontdatabase.cpp
#            qeglplatformcontext.cpp \
#            qweboswindowevents.cpp \
#            qwebosstyle.cpp

HEADERS =   qwebosintegration.h \
            qweboswindow.h \
            qweboswindowsurface.h \
            qwebosscreen.h \
            qbasicunixfontdatabase.h \
            ../eglconvenience/qeglconvenience.h \
            ../eglconvenience/qeglplatformcontext.h \
#            qeglplatformcontext.h \
#            qweboswindow_p.h \
#            qweboswindowevents.h \
#            qwebosstyle.h

# webOS clipboard
INCLUDEPATH += ../clipboards
SOURCES += ../clipboards/qwebosclipboard.cpp
HEADERS += ../clipboards/qwebosclipboard.h

INCLUDEPATH += ../eglconvenience/
INCLUDEPATH += $$QT_BUILD_TREE/include
INCLUDEPATH += $$QT_BUILD_TREE/include/QtOpenGL
INCLUDEPATH += $$QT_BUILD_TREE/include/QtGui
INCLUDEPATH += $$QT_BUILD_TREE/include/QtCore
SOURCES += $$QT_SOURCE_TREE/src/gui/text/qfontengine_ft.cpp

INCLUDEPATH += $$(STAGING_INCDIR)/napp \
               $$(STAGING_INCDIR)/sysmgr-ipc \
               $$(STAGING_INCDIR)/ime

#LIBS += -lnapp -lnrwindow

QMAKE_CXXFLAGS += -fno-rtti -fno-exceptions

QMAKE_CLEAN += libqwebos.so

target.path += $$[QT_INSTALL_PLUGINS]/platforms
INSTALLS += target
