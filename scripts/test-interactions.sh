#!/usr/bin/env bash
# Build and run the destructibility core unit tests without Unreal.
# Covers: Data/Interactions -> roles/rules -> durability -> best weapon.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Bar/InteractionCore.cpp \
	tools/tests/interactions/interaction_core_test.cpp \
	-o build/interaction_core_test

./build/interaction_core_test
