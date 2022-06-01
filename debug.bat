@echo off

set OPTION=%1

set DEBUG_FILE= .\debug\debug.rdbg
set EXE= .\build\physics_sim.exe


if "%OPTION%" equ "-rdbg" (goto :REMEDY)
if "%OPTION%" equ "-rdoc" (goto :RENDERDOC)
goto :REMEDY rem !!!DEFAULT PATH!!!

:REMEDY
call F:\Dev_Tools\RemedyBG\release_0.3.7.1\remedybg.exe -g -q %DEBUG_FILE%
goto eof

:RENDERDOC
pushd build
F:\\Dev_Tools\\RenderDoc\\qrenderdoc.exe physics_sim.exe
popd
goto eof


:eof
pause



