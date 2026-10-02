#!/usr/bin/env bash

debug_name=$1

make

mkdir -p ./test-images/arch
mkdir -p ./test-images/rod_traad
mkdir -p ./test-images/dylan_portrait

# arch
./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/arch.jpg" \
    --columns 40 \
    --out-image-path "test-images/arch/${debug_name}_arch.png" \
    > /dev/null 2>&1

echo "Generated: test-images/arch/${debug_name}_arch.png"

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/arch.jpg" \
    --columns 40 \
    --wiggle \
    --out-image-path "test-images/arch/${debug_name}_wiggle_arch.png" \
    > /dev/null 2>&1

echo "Generated: test-images/arch/${debug_name}_wiggle_arch.png"


# rod_traad
./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/rod_traad.png" \
    --columns 80 \
    --out-image-path "test-images/rod_traad/${debug_name}_rod_traad.png" \
    > /dev/null 2>&1

echo "Generated: test-images/rod_traad/${debug_name}_rod_traad.png"

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/rod_traad.png" \
    --columns 80 \
    --wiggle \
    --out-image-path "test-images/rod_traad/${debug_name}_wiggle_rod_traad.png" \
    > /dev/null 2>&1

echo "Generated: test-images/rod_traad/${debug_name}_wiggle_rod_traad.png"


# dylan_portrait
./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --out-image-path "test-images/dylan_portrait/${debug_name}_dylan_portrait.png" \
    > /dev/null 2>&1

echo "Generated: test-images/dylan_portrait/${debug_name}_dylan_portrait.png"

./build/ascii_converter \
    --font-path "resources/fonts/FiraCode-Bold.ttf" \
    --input-path "resources/dylan_portrait.png" \
    --columns 100 \
    --wiggle \
    --out-image-path "test-images/dylan_portrait/${debug_name}_wiggle_dylan_portrait.png" \
    > /dev/null 2>&1

echo "Generated: test-images/dylan_portrait/${debug_name}_wiggle_dylan_portrait.png"
