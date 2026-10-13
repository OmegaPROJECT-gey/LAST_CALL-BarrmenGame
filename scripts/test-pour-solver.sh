#!/usr/bin/env bash
# Build and run the pour-solver unit tests without Unreal.
# The same tests become UE Automation tests once the engine is installed
# (ARCHITECTURE.md, section 10).
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall \
	Source/LastCall/Bar/PourSolverCore.cpp \
	tools/tests/pour/pour_solver_test.cpp \
	-o build/pour_solver_test

./build/pour_solver_test
