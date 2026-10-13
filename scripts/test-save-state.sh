#!/usr/bin/env bash
# Build and run the save-state core unit tests without Unreal.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/DayScheduleCore.cpp \
	Source/LastCall/Core/SaveStateCore.cpp \
	tools/tests/save/save_state_test.cpp \
	-o build/save_state_test

./build/save_state_test
