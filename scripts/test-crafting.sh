#!/usr/bin/env bash
# Build and run the crafting core unit tests without Unreal.
# Covers: Data/Crafting -> attempts/outcomes -> liquid cross-check.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Bar/LiquidCore.cpp \
	Source/LastCall/Crafting/CraftingCore.cpp \
	tools/tests/crafting/crafting_core_test.cpp \
	-o build/crafting_core_test

./build/crafting_core_test
