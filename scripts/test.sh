#!/bin/sh
# Host-only diagnostic of shared application code; not an Android product build.
set -eu
cd "$(dirname "$0")/.."
temporary=$(mktemp -d)
trap 'rm -rf "$temporary"' EXIT HUP INT TERM
"${CC:-cc}" -std=c11 -O2 -DNDEBUG -Wall -Wextra -Werror -pedantic \
    -Isrc src/flower.c src/flower_mesh.c tests/test_flower.c -lm -o "$temporary/test-flower"
"$temporary/test-flower"
