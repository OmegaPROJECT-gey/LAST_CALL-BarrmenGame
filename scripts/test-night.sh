#!/usr/bin/env bash
# Build and run the night mode core unit tests without Unreal.
# Covers: Data/Night -> flag gating -> death consequences.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Core/ConsequenceLogCore.cpp \
	Source/LastCall/Night/NightCore.cpp \
	tools/tests/night/night_core_test.cpp \
	-o build/night_core_test

./build/night_core_test
