#!/bin/bash

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

echo "Starting clang-format check"

cd $(git rev-parse --show-toplevel)
SRC_FILES=$(git ls-files "*.cpp" "*.c" "*.h" "*.hpp")

# https://stackoverflow.com/a/71575442/4561887
IFS=$'\n' read -r -d '' -a SRC_FILES_ARRAY <<< "$SRC_FILES"

UNFORMATTED_FILES=()

for file in "${SRC_FILES_ARRAY[@]}"; do
    echo "Check $file:"
    DIFF=$(diff -u <(dos2unix < "$file" 2>/dev/null) <(clang-format "$file" | dos2unix 2>/dev/null))
    if [ $? -ne 0 ]; then
        UNFORMATTED_FILES+=("$file")
        echo "Unformatted $file:"
        echo "$DIFF"
    fi
done

if [ ${#UNFORMATTED_FILES[@]} -ne 0 ]; then
    echo "Error: The following files are not properly formatted:"
    for file in "${UNFORMATTED_FILES[@]}"; do
        echo "$file"
    done
    exit 1
fi

echo "All files are properly formatted."
