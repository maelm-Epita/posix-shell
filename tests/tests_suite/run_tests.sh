#!/bin/sh

BASE_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
VENV="$BASE_DIR/env"

export OUTPUT_FILE="${OUTPUT_FILE:-$BASE_DIR/out}"
export BIN_PATH="${BIN_PATH:-$BASE_DIR/../../src/42sh}"
export TEST_TIMEOUT="${TEST_TIMEOUT:-1.0}"

# create venv
if [ ! -d "$VENV" ]; then
    python3 -m venv "$VENV"
fi

# deps
"$VENV/bin/pip" install -q -r "$BASE_DIR/requirements.txt"

# choose tests
TEST_DIRS="$BASE_DIR/step1 $BASE_DIR/step2 $BASE_DIR/step3 $BASE_DIR/step4 $BASE_DIR/coverage-fix"

"$VENV/bin/python" -m pytest -v $TEST_DIRS

exit 0
