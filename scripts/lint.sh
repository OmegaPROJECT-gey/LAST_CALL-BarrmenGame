#!/usr/bin/env bash
# Run every LAST CALL linter, exactly as CI does.
# Usage: scripts/lint.sh
set -euo pipefail
cd "$(dirname "$0")/.."

cargo build --manifest-path tools/Cargo.toml

./tools/target/debug/lc-data-lint
./tools/target/debug/lc-dialogue-lint
./tools/target/debug/lc-docs-check

echo "all linters passed"
