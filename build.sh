#!/usr/bin/env bash

set -exu

#cmd.exe /c "cmake -S . -B build"
#cmd.exe /c "cmake --build build --config Release"

cmake -S . -B build
cmake --build build --config Release

