#!/usr/bin/env bash
# Build and run the venue progression core unit tests without Unreal.
# Covers: Data/Venue -> the growth ladder -> plan to climb one rung.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Story/EndingsCore.cpp \
	Source/LastCall/Venue/VenueCore.cpp \
	tools/tests/venue/venue_test.cpp \
	-o build/venue_test

./build/venue_test
