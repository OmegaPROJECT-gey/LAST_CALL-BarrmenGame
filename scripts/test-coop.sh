#!/usr/bin/env bash
# Build and run the co-op roles core unit tests without Unreal.
# Covers: Data/Coop -> role assignment -> the job split.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Coop/CoopCore.cpp \
	Source/LastCall/Venue/VenueCore.cpp \
	tools/tests/coop/coop_test.cpp \
	-o build/coop_test

./build/coop_test
