#!/usr/bin/env bash
# Build and run the debt runs core unit tests without Unreal.
# Covers: Data/Debts -> approaches/outcomes -> violence penalties.
set -euo pipefail
cd "$(dirname "$0")/.."

mkdir -p build
g++ -std=c++17 -Wall -Wextra -Werror -O1 \
	-I Source/LastCall -I tools/tests \
	Source/LastCall/Core/JsonCore.cpp \
	Source/LastCall/Core/ConsequenceLogCore.cpp \
	Source/LastCall/Social/SocialCore.cpp \
	Source/LastCall/Night/DebtCore.cpp \
	tools/tests/debts/debt_core_test.cpp \
	-o build/debt_core_test

./build/debt_core_test
