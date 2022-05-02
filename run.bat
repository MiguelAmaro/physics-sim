@echo off

pushd build

if exist physics_sim.exe (start physics_sim.exe) else (echo "ERROR: physics_sim.exe" does not exist!!!)

popd
