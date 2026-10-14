#!/usr/bin/env bash
# Build and run the skill tree core unit tests without Unreal.
# Drives Data/Skills/skills.json and cross-checks dialogue skill ids.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Core/SkillsCore.cpp \
	Source/LastCall/Dialogue/DialogueRunnerCore.cpp \
	tools/tests/skills/skills_tree_test.cpp \
	-o build/skills_tree_test

./build/skills_tree_test
