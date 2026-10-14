#!/usr/bin/env bash
# Build and run the liquid core unit tests without Unreal.
# Covers: Data/Liquids/*.json -> layering/foam/spills -> solver viscosity.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Bar/PourSolverCore.cpp \
	Source/LastCall/Bar/LiquidCore.cpp \
	tools/tests/liquids/liquid_core_test.cpp \
	-o build/liquid_core_test

./build/liquid_core_test
