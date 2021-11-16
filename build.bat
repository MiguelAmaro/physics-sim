@echo off

if not exist build mkdir build

set OUTPUT=%1

rem PROJECT INFO / SOURCE FILES
rem ************************************************************
set PROJECT_NAME=physics_sim
set SOURCES=^
F:\Dev\PhysicsSim\src\%PROJECT_NAME%.cpp

rem BUILD TOOLS
rem ************************************************************
set D3D_DIR=F:\Dev_Tools\DirectXSDKLegacy


rem ************************************************************
rem COMPILER(MSVC) OPTIONS
rem ************************************************************
set MSVC_WARNINGS= -wd4700

set MSVC_FLAGS= ^
%MSVC_WARNINGS% ^
-nologo ^
-Zi ^
-DRENDERER_OPENGL ^
-Gm- ^
-GR- ^
-EHa- ^
-Oi ^
-GS- ^
-Gs9999999

set MSVC_SEARCH_DIRS= -I..\
rem -I%D3D_DIR%\Include\

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
pushd build

call cl ^
%MSVC_FLAGS% ^
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

rem del *.pdb > NUL 2> NUL
rem echo WAITING FOR PDB > lock.tmp

call cl ^
%MSVC_FLAGS% ^
%MSVC_SEARCH_DIRS% ^
F:\Dev\PhysicsSim\src\%PROJECT_NAME%_blah.cpp ^
-Fm%PROJECT_NAME%_blah.map ^
-DLL ^
-LD ^
/link ^
-PDB:%PROJECT_NAME%_%random%.pdb ^
-EXPORT:Update

rem del lock.tmp

popd

goto :EOF

pause
