#!/usr/bin/env bash
# Build and run the investigations core unit tests without Unreal.
# Covers: Data/Intel -> tokens -> learning -> granted flags.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Night/IntelCore.cpp \
	tools/tests/intel/intel_core_test.cpp \
	-o build/intel_core_test

./build/intel_core_test
