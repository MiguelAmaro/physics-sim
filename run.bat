@echo off

pushd build

if exist win32_main.exe (start win32_main.exe) else (echo "ERROR: win32_main.exe" does not exist!!!)

popd
