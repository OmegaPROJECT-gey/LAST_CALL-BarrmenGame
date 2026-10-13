#!/usr/bin/env bash
# Build and run the F3 core unit tests (order layer + consequence log) without Unreal.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Customers/OrderCore.cpp \
	Source/LastCall/Core/ConsequenceLogCore.cpp \
	tools/tests/order/order_core_test.cpp \
	-o build/order_core_test

./build/order_core_test
