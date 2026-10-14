#!/usr/bin/env bash
# Build and run the raid director core unit tests without Unreal.
# Covers: evidence + partner capacity + distraction score -> outcomes.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Bar/RaidCore.cpp \
	tools/tests/raid/raid_core_test.cpp \
	-o build/raid_core_test

./build/raid_core_test
