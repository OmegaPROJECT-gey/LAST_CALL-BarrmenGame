#!/usr/bin/env bash
# Build and run the balance (Data/Balance/core.json) unit tests without Unreal.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/BalanceCore.cpp \
	Source/LastCall/Core/JsonCore.cpp \
	tools/tests/balance/balance_core_test.cpp \
	-o build/balance_core_test

./build/balance_core_test
