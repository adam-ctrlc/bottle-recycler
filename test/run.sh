#!/bin/sh
# Builds the sketch on your computer with fake Arduino parts (test/mock) and
# runs the scenarios in test/sim.cpp. Needs a C++ compiler (c++, g++ or clang++).
set -e
cd "$(dirname "$0")/.."
out=$(mktemp -d)
trap 'rm -rf "$out"' EXIT
c++ -std=c++17 -O2 -Wall -Wextra -I test/mock -I . -o "$out/sim" \
  test/sim.cpp -x c++ bottle_recycler.ino display.cpp sensor.cpp servos.cpp
"$out/sim"
