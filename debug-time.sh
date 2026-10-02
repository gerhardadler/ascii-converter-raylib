#!/usr/bin/env bash

echo "Building (with release)"
make release >/dev/null

debug_name=$1

mkdir -p ./test-images/timed

echo "# With wiggle"
./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --out-image-path "test-images/timed/${debug_name}_timed.png" \
    --wiggle | grep '^Calculation time'

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --out-image-path "test-images/timed/${debug_name}_timed.png" \
    --wiggle | grep '^Calculation time'

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --out-image-path "test-images/timed/${debug_name}_timed.png" \
    --wiggle | grep '^Calculation time'

echo "# Without wiggle"

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --out-image-path "test-images/timed/${debug_name}_timed.png" \
    | grep '^Calculation time'

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --out-image-path "test-images/timed/${debug_name}_timed.png" \
    | grep '^Calculation time'

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --out-image-path "test-images/timed/${debug_name}_timed.png" \
    | grep '^Calculation time'

echo "# Without wiggle without perpixeldifference"

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --out-image-path "test-images/timed/${debug_name}_timed.png" \
    --per-pixel-weight 0 \
    | grep '^Calculation time'

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --out-image-path "test-images/timed/${debug_name}_timed.png" \
    --per-pixel-weight 0 \
    | grep '^Calculation time'

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --out-image-path "test-images/timed/${debug_name}_timed.png" \
    --per-pixel-weight 0 \
    | grep '^Calculation time'
