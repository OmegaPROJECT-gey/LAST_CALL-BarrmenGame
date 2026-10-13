#!/usr/bin/env bash
# Build and run the recipe library core unit tests without Unreal.
# Covers: Data/Recipes/*.json -> RecipeRecord -> pour solver combo.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Bar/PourSolverCore.cpp \
	Source/LastCall/Customers/OrderCore.cpp \
	Source/LastCall/Bar/RecipeLibraryCore.cpp \
	tools/tests/recipes/recipe_library_test.cpp \
	-o build/recipe_library_test

./build/recipe_library_test
