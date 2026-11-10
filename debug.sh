#!/bin/bash

set -e
cmake -S . -B Build/Debug -D CMAKE_EXPORT_COMPILE_COMMANDS=ON -D CMAKE_BUILD_TYPE=Debug
cmake --build Build/Debug --config Debug -j
ln -sf Build/Debug/compile_commands.json .
./Build/Debug/Run/MyProject
