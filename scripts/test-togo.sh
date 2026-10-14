#!/usr/bin/env bash
# Build and run the orders to go core unit tests without Unreal.
# Covers: queue -> tip bands -> pressure -> the window shutting.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Night/ToGoCore.cpp \
	tools/tests/togo/togo_core_test.cpp \
	-o build/togo_core_test

./build/togo_core_test
