build_pass:CONFIG(debug,   release|debug): TARGET=SOILd
build_pass:CONFIG(release, release|debug): TARGET=SOIL

TEMPLATE = lib
CONFIG += staticlib c++14
QMAKE_CXXFLAGS += -m64
LIBS += -framework OpenGL -framework CoreFoundation -framework GLUT

DESTDIR = $$_PRO_FILE_PWD_/bin
OBJECTS_DIR = tmp

SOURCES += src/image_DXT.c \
           src/image_helper.c \
           src/SOIL.c \
           src/stb_image_aug.c

HEADERS += src/image_DXT.h \
           src/image_helper.h \
           src/SOIL.h \
           src/stb_image_aug.h \
           src/stbi_DDS_aug.h \
           src/stbi_DDS_aug_c.h
