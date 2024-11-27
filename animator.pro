TEMPLATE = subdirs

# Sub-project names
SUBDIRS = \
    sub_glew \
    sub_soil \
    sub_assimp \
    sub_yaml \
    sub_engine \
    sub_editor

sub_glew.subdir = Libraries/glew-2.0.0
sub_soil.subdir = Libraries/soil
sub_assimp.subdir = Libraries/assimp
sub_yaml.subdir = Libraries/yaml-cpp
sub_engine.subdir = Engine
sub_editor.subdir = Editor
sub_engine.depends = sub_glew sub_soil sub_yaml sub_assimp
sub_editor.depends = sub_engine sub_glew sub_soil sub_yaml sub_assimp
