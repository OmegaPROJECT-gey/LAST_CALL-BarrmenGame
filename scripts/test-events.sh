#!/usr/bin/env bash
# Build and run the dynamic events core unit tests without Unreal.
# Covers: Data/Events -> the weighted morning pool -> the day's event.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Social/SocialCore.cpp \
	Source/LastCall/Events/EventCore.cpp \
	tools/tests/events/events_test.cpp \
	-o build/events_test

./build/events_test
