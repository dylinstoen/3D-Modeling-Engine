# See: http://glew.sourceforge.net/install.html

build_pass:CONFIG(debug,   release|debug): TARGET=GLEWd
build_pass:CONFIG(release, release|debug): TARGET=GLEW

TEMPLATE = lib
CONFIG += staticlib

DEFINES += GLEW_STATIC
DESTDIR = $$_PRO_FILE_PWD_/bin
OBJECTS_DIR = tmp

INCLUDEPATH = include

HEADERS += include/GL/eglew.h \
           include/GL/glew.h \
           include/GL/glxew.h \
           include/GL/wglew.h

SOURCES += src/glew.c
