#-------------------------------------------------
#
# Project created by QtCreator 2016-08-12T01:00:49
#
#-------------------------------------------------

# Make sure we're using an up-to-date version of Qt
lessThan( QT_MAJOR_VERSION, 5 ): error( "Qt version 5.7+ required" )
lessThan( QT_MINOR_VERSION, 7 ): error( "Qt version 5.7+ required" )

# Specifices the QT modules used by the project
QT = core gui widgets printsupport

# What the output of the build process will be (app, lib, plugin, etc.)
TEMPLATE = app

# Executable name
TARGET = AnimatorEditor

# Application version number
VERSION = 0.0.1

# Directory in which the executable will be placed
#DESTDIR = bin
OBJECTS_DIR = tmp

# General project configuration options
CONFIG -= flat
CONFIG += c++14 precompile_header force_debug_info

# List of filesnames of header files used when building the project
HEADERS += \
    src/scenewindow.h \
    src/mainwindow.h \
    src/widgets/qcomponent.h \
    src/widgets/checkbox.h \
    src/widgets/assetpicker.h \
    src/widgets/vec3edit.h \
    src/widgets/doubleedit.h \
    src/qtwidgets.h \
    src/widgets/combobox.h \
    src/widgets/slider.h \
    src/widgets/assetpickerdialog.h \
    src/widgets/filepicker.h \
    src/widgets/colorpicker.h \
    src/widgets/intedit.h \
    src/widgets/inspectablewidget.h \
    src/widgets/lineedit.h \
    src/widgets/menutoolbutton.h \
    src/widgets/QCustomPlot/qcustomplot.h \
    src/qglinclude.h \
    src/gizmomanager.h \
    src/actionmanager.h \
    src/assets/qassetwidgetitem.h \
    src/hierarchy/qobjectwidgetitem.h \
    src/hierarchy/hierarchyview.h \
    src/assets/assetbrowser.h \
    src/widgets/propertiesdialog.h \
    src/animation/qanimtablewidgetitem.h \
    src/inspector/inspectableitem.h \
    src/inspector/inspector.h \
    src/animation/animationwidget.h \
    src/animation/curve.h \
    src/animation/controlpoint.h \
    src/animation/curvesplot.h \
    src/filterdialog.h \
    src/renderview.h \
    src/renderwindow.h \
    src/curveeditorwindow.h \
    src/widgets/curveeditorcanvas.h

# List of source code files to be used when building the project
SOURCES += \
    src/scenewindow.cpp \
    src/mainwindow.cpp \
    src/main.cpp \
    src/widgets/qcomponent.cpp \
    src/widgets/checkbox.cpp \
    src/widgets/assetpicker.cpp \
    src/widgets/vec3edit.cpp \
    src/widgets/doubleedit.cpp \
    src/widgets/combobox.cpp \
    src/widgets/slider.cpp \
    src/widgets/assetpickerdialog.cpp \
    src/widgets/filepicker.cpp \
    src/widgets/colorpicker.cpp \
    src/widgets/intedit.cpp \
    src/widgets/lineedit.cpp \
    src/widgets/menutoolbutton.cpp \
    src/widgets/QCustomPlot/qcustomplot.cpp \
    src/gizmomanager.cpp \
    src/actionmanager.cpp \
    src/assets/qassetwidgetitem.cpp \
    src/hierarchy/qobjectwidgetitem.cpp \
    src/hierarchy/hierarchyview.cpp \
    src/assets/assetbrowser.cpp \
    src/animation/qanimtablewidgetitem.cpp \
    src/animation/controlpoint.cpp \
    src/animation/curve.cpp \
    src/animation/curvesplot.cpp \
    src/animation/animationwidget.cpp \
    src/widgets/propertiesdialog.cpp \
    src/inspector/inspector.cpp \
    src/filterdialog.cpp \
    src/renderview.cpp \
    src/renderwindow.cpp \
    src/curveeditorwindow.cpp \
    src/widgets/curveeditorcanvas.cpp

RESOURCES += \
    resources.qrc \
    fatcowtheme.qrc

include(../Libraries/Qt-Color-Widgets/color_widgets.pri)

# List of UI files to be processed by user interface coimpiler
FORMS += \
    src/forms/mainwindow.ui \
    src/forms/propertiesdialog.ui \
    src/forms/animationwidget.ui \
    src/forms/filterdialog.ui \
    src/forms/curveeditorwindow.ui

# Compiler preprocessor macros
DEFINES += APP_VERSION=\\\"$$VERSION\\\"

# Specifies the include directories which should be searched when compiling the project
INCLUDEPATH += \
    "$$_PRO_FILE_PWD_/src" \
    "$$_PRO_FILE_PWD_/../Engine/src" \
    "$$_PRO_FILE_PWD_/../Libraries" \
    "$$_PRO_FILE_PWD_/../Libraries/Qt-Color-Widgets/include" \
    "$$_PRO_FILE_PWD_/../Libraries/signals"

#Copy assets to the build directory
win32 {
    # xcopy on Windows needs forward slashes to be converted to backslashes
    SRCDIR_WIN = $$PWD/assets
    DESTDIR_WIN = $$OUT_PWD/assets
    SRCDIR_WIN ~= s,/,\\,g
    DESTDIR_WIN ~= s,/,\\,g
    copyassets.commands = $(COPY_DIR) $$system_quote($${SRCDIR_WIN}) $$system_quote($${DESTDIR_WIN})
}
linux {
    copyassets.commands = $(COPY_DIR) $$system_quote($$PWD/assets) $$system_quote($$OUT_PWD)
}
macx {
    copyassets.commands = $(COPY_DIR) $$system_quote($$PWD/assets) $$system_quote($$OUT_PWD/AnimatorEditor.app/Contents/MacOS/)
}
first.depends = $(first) copyassets
export(first.depends)
export(copyassets.commands)
QMAKE_EXTRA_TARGETS += first copyassets

# Depend on AnimatorEngine Library
win32:CONFIG(release, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Engine/bin -lAnimatorEngine
else:win32:CONFIG(debug, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Engine/bin -lAnimatorEngined
else:unix: LIBS += -L$$_PRO_FILE_PWD_/../Engine/bin -lEngine

INCLUDEPATH += $$_PRO_FILE_PWD_/../Engine
DEPENDPATH += $$_PRO_FILE_PWD_/../Engine
INCLUDEPATH += $$_PRO_FILE_PWD_/../Engine/include
DEPENDPATH += $$_PRO_FILE_PWD_/../Engine/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Engine/bin/libAnimatorEngine.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Engine/bin/libAnimatorEngined.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Engine/bin/AnimatorEngine.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Engine/bin/AnimatorEngined.lib
else:unix: PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Engine/bin/libEngine.a

# Depend on SOIL Library
win32:CONFIG(release, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Libraries/soil/bin -lSOIL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Libraries/soil/bin -lSOILd
else:unix: LIBS += -L$$_PRO_FILE_PWD_/../Libraries/soil/bin -lsoil

INCLUDEPATH += $$_PRO_FILE_PWD_/../Libraries/soil/include
DEPENDPATH += $$_PRO_FILE_PWD_/../Libraries/soil/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/soil/bin/libSOIL.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/soil/bin/libSOILd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/soil/bin/SOIL.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/soil/bin/SOILd.lib
else:unix: PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/soil/bin/libsoil.a

# Depend on assimp Library
win32:CONFIG(release, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Libraries/assimp/bin -lassimp
else:win32:CONFIG(debug, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Libraries/assimp/bin -lassimpd
else:unix: LIBS += -L$$_PRO_FILE_PWD_/../Libraries/assimp/bin -lassimp

INCLUDEPATH += $$_PRO_FILE_PWD_/../Libraries/assimp/include
DEPENDPATH += $$_PRO_FILE_PWD_/../Libraries/assimp/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/assimp/bin/libassimp.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/assimp/bin/libassimpd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/assimp/bin/assimp.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/assimp/bin/assimpd.lib
else:unix: PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/assimp/bin/libassimp.a

# Depend on GLEW Library
win32:CONFIG(release, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/bin -lGLEW
else:win32:CONFIG(debug, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/bin -lGLEWd
else:linux: LIBS += -L$$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/bin -lglew-2

INCLUDEPATH += $$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/include
DEPENDPATH += $$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/bin/libGLEW.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/bin/libGLEWd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/bin/GLEW.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/bin/GLEWd.lib
else:linux: PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/bin/libglew-2.a

# Depend on yaml-cpp Library
win32:CONFIG(release, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Libraries/yaml-cpp/bin -llibyaml-cppmd
else:win32:CONFIG(debug, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Libraries/yaml-cpp/bin -llibyaml-cppmdd
else:unix: LIBS += -L$$_PRO_FILE_PWD_/../Libraries/yaml-cpp/bin -lyaml-cpp

INCLUDEPATH += $$_PRO_FILE_PWD_/../Libraries/yaml-cpp/include
DEPENDPATH += $$_PRO_FILE_PWD_/../Libraries/yaml-cpp/include

win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/yaml-cpp/bin/liblibyaml-cppmd.a
else:win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/yaml-cpp/bin/liblibyaml-cppmdd.a
else:win32:!win32-g++:CONFIG(release, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/yaml-cpp/bin/libyaml-cppmd.lib
else:win32:!win32-g++:CONFIG(debug, debug|release): PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/yaml-cpp/bin/libyaml-cppmdd.lib
else:unix: PRE_TARGETDEPS += $$_PRO_FILE_PWD_/../Libraries/yaml-cpp/bin/libyaml-cpp.a

# Depend on OpenGL
win32:LIBS += -lopengl32
linux:LIBS += -lGL
macx:LIBS += -framework OpenGL -framework CoreFoundation -framework GLUT
