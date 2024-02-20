@echo off
cls

if not exist build\debug mkdir build\debug
cd build\debug
cmake -S ..\..\ -B . -G"MinGW Makefiles" -D CMAKE_EXPORT_COMPILE_COMMANDS=ON -D CMAKE_BUILD_TYPE=Debug
C:\Users\gerku\Documents\mingw64\bin\mingw32-make.exe && C:\Users\gerku\Documents\mingw64\bin\mingw32-make.exe Shaders
if not exist ..\compile_commands.json move compile_commands.json ..\
MyProject.exe
cd ..\..\

