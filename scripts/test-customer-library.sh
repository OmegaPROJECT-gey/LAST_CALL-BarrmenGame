#!/usr/bin/env bash
# Build and run the customer persona core unit tests without Unreal.
# Covers: Data/Customers -> tells/reading -> threat bands -> arrival.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Customers/CustomerCore.cpp \
	tools/tests/customer/customer_core_test.cpp \
	-o build/customer_core_test

./build/customer_core_test
