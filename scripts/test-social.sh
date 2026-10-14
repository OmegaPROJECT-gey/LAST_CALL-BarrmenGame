#!/usr/bin/env bash
# Build and run the social consequences core unit tests without Unreal.
# Covers: Data/Factions -> reaction tables -> the standing ledger.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Core/ConsequenceLogCore.cpp \
	Source/LastCall/Social/SocialCore.cpp \
	tools/tests/social/social_core_test.cpp \
	-o build/social_core_test

./build/social_core_test
