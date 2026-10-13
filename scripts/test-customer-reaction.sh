#!/usr/bin/env bash
# Build and run the customer-reaction unit tests without Unreal.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Bar/PourSolverCore.cpp \
	Source/LastCall/Customers/CustomerReactionCore.cpp \
	tools/tests/customer/customer_reaction_test.cpp \
	-o build/customer_reaction_test

./build/customer_reaction_test
