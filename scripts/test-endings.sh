#!/usr/bin/env bash
# Build and run the endings core unit tests without Unreal.
# Covers: Data/Endings -> selection rules -> standing x information x tier.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Core/ConsequenceLogCore.cpp \
	Source/LastCall/Social/SocialCore.cpp \
	Source/LastCall/Story/EndingsCore.cpp \
	tools/tests/story/endings_test.cpp \
	-o build/endings_test

./build/endings_test
