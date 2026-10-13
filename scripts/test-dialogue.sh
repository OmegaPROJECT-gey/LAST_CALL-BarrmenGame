#!/usr/bin/env bash
# Build and run the dialogue runner core unit tests without Unreal.
# Drives the real Data/Dialogue/act1_harris_meeting.json.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Dialogue/DialogueRunnerCore.cpp \
	tools/tests/dialogue/dialogue_runner_test.cpp \
	-o build/dialogue_runner_test

./build/dialogue_runner_test
