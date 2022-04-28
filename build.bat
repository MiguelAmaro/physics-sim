@echo off

if not exist build mkdir build
set OUTPUT=%1

rem PROJECT INFO / SOURCE FILES
rem ************************************************************
set PROJECT_DIR=%cd%
set PROJECT_NAME=physics_sim
set SOURCES=^
%PROJECT_DIR%\src\%PROJECT_NAME%.cpp 

rem BUILD TOOLS
rem ************************************************************
set D3D_DIR=F:\Dev_Tools\DirectXSDKLegacy


rem COMPILER(MSVC) OPTIONS
rem ************************************************************
set MSVC_WARNINGS= -wd4700

set MSVC_FLAGS= ^
%MSVC_WARNINGS% ^
-nologo ^
-Wall ^
-wd4820 ^
-wd4668 ^
-wd4577 ^
-wd5045 ^
-wd4505 ^
-wd4365 ^
-wd4305 ^
-wd4201 ^
-wd4100 ^
-wd4191 ^
-wd4061 ^
-Zi ^
-DRENDERER_OPENGL ^
-Gm- ^
-GR- ^
-EHa- ^
-Od ^
-GS- ^
-Gs9999999

set MSVC_SEARCH_DIRS=^
-I%PROJECT_DIR%\ ^
-I%PROJECT_DIR%\thirdparty\freetype2\include

rem ************************************************************
rem LINKER(MSVC) OPTIONS
rem ************************************************************
set MSVC_LIBS= ^
user32.lib ^
shell32.lib ^
opengl32.lib ^
kernel32.lib ^
gdi32.lib ^
d3d11.lib ^
d3dcompiler.lib ^
dinput8.lib ^
dxguid.lib ^
winmm.lib ^
ws2_32.lib


rem %D3D_DIR%\Lib\x64\d3d11.lib ^
rem %D3D_DIR%\Lib\x64\dxerr.lib ^
rem %D3D_DIR%\Lib\x64\ ^
rem %D3D_DIR%\Lib\x64\d3dx11.lib <- Utility Library is deprecated.

set MSVC_LINK_FLAGS= ^
-nodefaultlib ^
-subsystem:windows ^
-stack:0x100000,0x100000


rem ************************************************************
rem START BUILD
rem ************************************************************

set path=%path%;F:\Dev\PhysicsSim\build


if "%OUTPUT%" equ "-exe" (goto :COMPILE_EXE)
if "%OUTPUT%" equ "-dll" (goto :COMPILE_DLL)
goto :COMPILE_EXE rem !!!DEFAULT PATH!!!


:COMPILE_EXE
rem  EXE
echo ============================================================

taskkill /f /im physics_sim.exe
pushd build

cl %MSVC_FLAGS% ^
%MSVC_SEARCH_DIRS% ^
%SOURCES% ^
/link ^
%MSVC_LIBS% ^
%MSVC_LINK_FLAGS%

popd

goto :EOF

:COMPILE_DLL
rem  DLL
echo ============================================================
pushd build

del *.pdb > NUL 2> NUL
rem echo WAITING FOR PDB > lock.tmp

cl ^
%MSVC_FLAGS% ^
%MSVC_SEARCH_DIRS% ^
F:\Dev\PhysicsSim\src\%PROJECT_NAME%_blah.cpp ^
-Fm%PROJECT_NAME%_blah.map ^
-DLL ^
-LD ^
/link ^
-PDB:%PROJECT_NAME%_%random%.pdb ^
%PROJECT_DIR%"\thirdparty\freetype2\release dll\win64\freetype.lib" ^
-EXPORT:Update

rem del lock.tmp

popd

goto :EOF

pause
