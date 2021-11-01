@echo off

if not exist build mkdir build

rem PROJECT INFO / SOURCE FILES
rem ************************************************************
set PROJECT_NAME=physics_sim
set SOURCES=^
F:\Dev\PhysicsSim\src\%PROJECT_NAME%.cpp

rem BUILD TOOLS
rem ************************************************************
set D3D9_DIR=F:\Dev_Tools\DirectXSDKLegacy


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

set MSVC_SEARCH_DIRS=

rem -I%D3D9_DIR%\Include\

rem ************************************************************
rem LINKER(MSVC) OPTIONS
rem ************************************************************
set MSVC_LIBS= ^
user32.lib ^
shell32.lib ^
opengl32.lib ^
kernel32.lib ^
gdi32.lib ^
dinput8.lib ^
dxguid.lib ^
%D3D9_DIR%\Lib\x64\d3dx11.lib ^
%D3D9_DIR%\Lib\x64\d3d11.lib ^
%D3D9_DIR%\Lib\x64\dxerr.lib ^
winmm.lib ^
ws2_32.lib 


set MSVC_LINK_FLAGS= ^
-nodefaultlib ^
-subsystem:windows ^
-stack:0x100000,0x100000


rem ************************************************************
rem START BUILD
rem ************************************************************
pushd build

set path=%path%;F:\Dev\PhysicsSim\build

rem ==================        COMPILE         ==================
call cl ^
%MSVC_FLAGS% ^
%MSVC_SEARCH_DIRS% ^
%SOURCES% ^
/link ^
%MSVC_LIBS% ^
%MSVC_LINK_FLAGS%

popd

pause
