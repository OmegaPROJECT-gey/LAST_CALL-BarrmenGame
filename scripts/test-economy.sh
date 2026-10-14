#!/usr/bin/env bash
# Build and run the economy core unit tests without Unreal.
# Covers: suppliers -> price drift -> morning ledger rules.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Bar/LiquidCore.cpp \
	Source/LastCall/Economy/EconomyCore.cpp \
	tools/tests/economy/economy_core_test.cpp \
	-o build/economy_core_test

./build/economy_core_test
