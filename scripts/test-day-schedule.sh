#!/usr/bin/env bash
# Build and run the day-schedule (phase clock) unit tests without Unreal.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/DayScheduleCore.cpp \
	tools/tests/day/day_schedule_test.cpp \
	-o build/day_schedule_test

./build/day_schedule_test
