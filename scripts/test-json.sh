#!/usr/bin/env bash
# Build and run the minimal JSON reader unit tests without Unreal.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	tools/tests/json/json_core_test.cpp \
	-o build/json_core_test

./build/json_core_test
