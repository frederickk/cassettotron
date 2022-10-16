#!/usr/bin/env bash

PLATFORM=${1:-"pro-micro"}
# MODE=${2}

echo $PLATFORM

echo "🧹 Cleaning build"
pio run -t clean

# echo "🧰 Compiling $PLATFORM"
# pio run -e $PLATFORM -v

echo "⚡️ Flashing $PLATFORM"
pio run -e $PLATFORM -t upload -v

echo "👌 Done"
