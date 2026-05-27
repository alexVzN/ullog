#!/bin/bash

# Suggested alias in ~/.gitconfig:
# ----
# [alias]
#    format = "!f() { cd $(git rev-parse --show-toplevel); ./bin/run-clang-format.sh; }; f"
# ----

if ! command -v clang-format &>/dev/null; then
    echo "Error: Cannot find clang-format. Please ensure that you have added it to the PATH"
    exit 1
fi

REQUIRED_VERSION="19.0.0"
CURRENT_VERSION=$(clang-format --version | head -n 1 | awk '{print $3}')
if [ "$CURRENT_VERSION" \< "$REQUIRED_VERSION" ]; then
    echo "Error: Required clang-format version $REQUIRED_VERSION or higher"
    exit 1
fi

cd $(git rev-parse --show-toplevel)
SRC_FILES=$(git ls-files "*.cpp" "*.c" "*.hpp" "*.h")

# https://stackoverflow.com/a/71575442/4561887
IFS=$'\n' read -r -d '' -a SRC_FILES_ARRAY <<< "$SRC_FILES"

clang-format -i --verbose --style=file "${SRC_FILES_ARRAY[@]}"
