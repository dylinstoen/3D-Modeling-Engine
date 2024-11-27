REM Batch file for auto deploy
REM !!Warning!! Remember to use the command prompt that Qt provides to run the script
REM !!Info!! Install 7-zip if you want to archive the solution directly

set PROJECT_NAME=animator
set BINARY_NAME=AnimatorEditor

REM TA environment customization
set PROJECT_DIR=C:\TA\cse457sp20\projects\animator
set QT_BINARY_DIR=C:\Qt\Qt5.12.2\5.12.2\msvc2017_64\bin
set QT_CREATOR_BIN_DIR=C:\Qt\Qt5.12.2\Tools\QtCreator\bin
set VSVARSALL_PATH="C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\VC\Auxiliary\Build\vcvarsall.bat"

REM Set internal variables
set SOLUTION_DIR=%PROJECT_DIR%\windows
set BUILD_DIR=%PROJECT_DIR%\build

REM Create and goto build folder
mkdir %BUILD_DIR%
cd %BUILD_DIR%

REM Initialise builds
call %VSVARSALL_PATH% amd64

REM Build project
%QT_BINARY_DIR%\qmake.exe %PROJECT_DIR%\%PROJECT_NAME%.pro -spec win32-msvc "CONFIG+=qtquickcompiler" && %QT_CREATOR_BIN_DIR%\jom.exe qmake_all
%QT_CREATOR_BIN_DIR%\jom.exe
%QT_CREATOR_BIN_DIR%\jom.exe clean

REM Create binary folder
mkdir %SOLUTION_DIR%

REM Build release
set WINDEPLOYQT_EXE=%QT_BINARY_DIR%\windeployqt.exe
set RELEASE_DIR=%BUILD_DIR%\Editor\release
copy %RELEASE_DIR%\%BINARY_NAME%.exe %SOLUTION_DIR%\%BINARY_NAME%.exe
xcopy %BUILD_DIR%\Editor\assets %SOLUTION_DIR%\assets /I /E
%WINDEPLOYQT_EXE% %SOLUTION_DIR%\%BINARY_NAME%.exe

REM Remove build dir
cd %PROJECT_DIR%
rmdir %BUILD_DIR% /s /q

REM Zip solution
7z a animator_solution_windows.zip %SOLUTION_DIR%

REM Remove solution dir
rmdir %SOLUTION_DIR% /s /q
