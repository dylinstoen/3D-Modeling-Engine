#!/bin/bash

# TA environment customization
QT_BIN_DIR=~/Qt/5.14.1/clang_64/bin
BUILD_DIR=../build/release

BINARY_NAME=AnimatorEditor
ARCHIVE_NAME=animator_solution_mac

echo "create solution dir"
SOLUTION_DIR=mac
mkdir $SOLUTION_DIR

echo "copy application"
cp -r $BUILD_DIR/Editor/$BINARY_NAME.app $SOLUTION_DIR

echo "copy assets"
cp -r Editor/assets $SOLUTION_DIR

echo "deploy application"
$QT_BIN_DIR/macdeployqt $SOLUTION_DIR/$BINARY_NAME.app

echo "zip the solution"
zip -qr $ARCHIVE_NAME.zip ./mac

echo "remove temp dirs"
rm -rf mac
