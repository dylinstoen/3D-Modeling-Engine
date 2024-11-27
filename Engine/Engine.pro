build_pass:CONFIG(debug,   release|debug): TARGET=AnimatorEngined
build_pass:CONFIG(release, release|debug): TARGET=AnimatorEngine

TEMPLATE = lib
CONFIG += staticlib c++14 precompile_header force_debug_info
CONFIG -= flat

DESTDIR = $$_PRO_FILE_PWD_/bin
OBJECTS_DIR = tmp

PRECOMPILED_HEADER += \
#                      src/components.h \
#                      src/properties.h \
                      src/resources.h \
                      src/glinclude.h \
                      src/vectors.h \

HEADERS +=  src/components.h \
    src/enum.h \
    src/exceptions.h \
    src/fileio.h \
    src/forward.h \
    src/properties.h \
    src/resources.h \
    src/shadervars.h \
    src/util.h \
    src/debug/log.h \
    src/opengl/glerror.h \
    src/properties/booleanproperty.h \
    src/properties/choiceproperty.h \
    src/properties/colorproperty.h \
    src/properties/doubleproperty.h \
    src/properties/fileproperty.h \
    src/properties/intproperty.h \
    src/properties/matrixproperty.h \
    src/properties/property.h \
    src/properties/resourceproperty.h \
    src/properties/vec3property.h \
    src/resource/asset.h \
    src/resource/assetmanager.h \
    src/resource/importers.h \
    src/resource/material.h \
    src/resource/mesh.h \
    src/resource/shaderprogram.h \
    src/resource/shapes.h \
    src/resource/texture.h \
    src/scene/renderer.h \
    src/scene/scene.h \
    src/scene/scenecamera.h \
    src/scene/sceneobject.h \
    src/scene/trackball.h \
    src/scene/translator.h \
    src/scene/components/camera.h \
    src/scene/components/component.h \
    src/scene/components/cone.h \
    src/scene/components/cylinder.h \
    src/scene/components/geometry.h \
    src/scene/components/light.h \
    src/scene/components/plane.h \
    src/scene/components/sphere.h \
    src/scene/components/surfaceofrevolution.h \
    src/scene/components/transform.h \
    src/opengl/glresourcemanager.h \
    src/opengl/glrenderer.h \
    src/opengl/glshaderprogram.h \
    src/opengl/gltexture2d.h \
    src/resource/shaderfactory.h \
    src/opengl/glmesh.h \
    src/opengl/glslshaderfactory.h \
    src/resource/cacheable.h \
    src/fpscounter.h \
    src/scene/components/particlesystem.h \
    src/animation/curveevaluator.h \
    src/animation/linearcurveevaluator.h \
    src/animation/curvesampler.h \
    src/glinclude.h \
    src/vectors.h \
    src/animation/beziercurveevaluator.h \
    src/animation/catmullromcurveevaluator.h \
    src/animation/bsplinecurveevaluator.h \
    src/animation/keyframe.h \
    src/scene/scenemanager.h \
    src/glextinclude.h \
    src/scene/scaler.h \
    src/scene/rotator.h \
    src/scene/components/ring.h \
    src/meshprocessing.h \
    src/scene/components/spherecollider.h \
    src/scene/components/planecollider.h \
    src/opengl/glcubemap.h \
    src/scene/components/environmentmap.h \
    src/resource/cubemap.h \
    src/opengl/glrenderablecubemap.h \
    src/opengl/glrenderabletexture.h \
    src/opengl/gltexturebase.h \
    src/yamlextensions.h \
    src/animator.h \
    src/scene/boundingbox.h \
    src/trace/ray.h \
    #src/trace/tracematerial.h \
    src/scene/components/trianglemesh.h \
    src/trace/tracelight.h \
    src/trace/tracescene.h \
    src/trace/bsptree.h \
    src/trace/raytracer.h \
    src/scene/components/triangleface.h \
    src/trace/randomsampler.h \
    src/trace/tracesceneobject.h \
    src/serializable.h \
    src/properties/propertygroup.h \
    src/singleton.h \
    src/scene/components/robotarmprop.h \
    src/scene/components/customprop.h \
    src/scene/components/connectedparticlesystem.h \
    src/scene/components/cylindercollider.h

SOURCES +=  src/debug/debug.cpp \
    src/debug/log.cpp \
    src/opengl/glerror.cpp \
    src/properties/booleanproperty.cpp \
    src/properties/choiceproperty.cpp \
    src/properties/colorproperty.cpp \
    src/properties/doubleproperty.cpp \
    src/properties/fileproperty.cpp \
    src/properties/intproperty.cpp \
    src/properties/matrixproperty.cpp \
    src/properties/property.cpp \
    src/properties/resourceproperty.cpp \
    src/properties/vec3property.cpp \
    src/resource/asset.cpp \
    src/resource/assetmanager.cpp \
    src/resource/importers.cpp \
    src/resource/material.cpp \
    src/resource/mesh.cpp \
    src/resource/texture.cpp \
    src/scene/scene.cpp \
    src/scene/scenecamera.cpp \
    src/scene/sceneobject.cpp \
    src/scene/trackball.cpp \
    src/scene/translator.cpp \
    src/scene/components/camera.cpp \
    src/scene/components/component.cpp \
    src/scene/components/cone.cpp \
    src/scene/components/cylinder.cpp \
    src/scene/components/light.cpp \
    src/scene/components/plane.cpp \
    src/scene/components/sphere.cpp \
    src/scene/components/surfaceofrevolution.cpp \
    src/scene/components/transform.cpp \
    src/opengl/glresourcemanager.cpp \
    src/opengl/glrenderer.cpp \
    src/opengl/glshaderprogram.cpp \
    src/opengl/glmesh.cpp \
    src/opengl/gltexture2d.cpp \
    src/scene/components/particlesystem.cpp \
    src/animation/linearcurveevaluator.cpp \
    src/animation/curvesampler.cpp \
    src/opengl/glslshaderfactory.cpp \
    src/animation/beziercurveevaluator.cpp \
    src/animation/catmullromcurveevaluator.cpp \
    src/animation/bsplinecurveevaluator.cpp \
    src/scene/scenemanager.cpp \
    src/scene/scaler.cpp \
    src/scene/rotator.cpp \
    src/scene/components/ring.cpp \
    src/meshprocessing.cpp \
    src/scene/components/spherecollider.cpp \
    src/scene/components/planecollider.cpp \
    src/opengl/glcubemap.cpp \
    src/scene/components/environmentmap.cpp \
    src/resource/cubemap.cpp \
    src/opengl/glrenderablecubemap.cpp \
    src/opengl/glrenderabletexture.cpp \
    src/opengl/gltexturebase.cpp \
    src/yamlextensions.cpp \
    src/scene/boundingbox.cpp \
    src/trace/ray.cpp \
    #src/trace/tracematerial.cpp
    src/scene/components/trianglemesh.cpp \
    src/trace/tracelight.cpp \
    src/trace/tracescene.cpp \
    src/trace/raytracer.cpp \
    src/scene/components/triangleface.cpp \
    src/trace/randomsampler.cpp \
    src/trace/tracesceneobject.cpp \
    src/serializable.cpp \
    src/properties/propertygroup.cpp \
    src/scene/components/robotarmprop.cpp \
    src/scene/components/customprop.cpp \
    src/scene/components/connectedparticlesystem.cpp \
    src/scene/components/cylindercollider.cpp

INCLUDEPATH += \
    "$$_PRO_FILE_PWD_/src" \
    "$$_PRO_FILE_PWD_/../Libraries" \

# Depend on OpenGL
win32:LIBS += -lopengl32
linux:LIBS += -lGL
macx:LIBS += -framework OpenGL -framework CoreFoundation -framework GLUT

# Depend on SOIL Library
win32:CONFIG(release, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Libraries/soil/bin -lSOIL
else:win32:CONFIG(debug, debug|release): LIBS += -L$$_PRO_FILE_PWD_/../Libraries/soil/bin -lSOILd
else:unix: LIBS += -L$$_PRO_FILE_PWD_/../Libraries/soil/bin -lSOIL

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
else:linux: LIBS += -L$$_PRO_FILE_PWD_/../Libraries/glew-2.0.0/bin -lGLEW

INCLUDEPATH += $$PWD/../Libraries/glew-2.0.0/include
DEPENDPATH += $$PWD/../Libraries/glew-2.0.0/include

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
