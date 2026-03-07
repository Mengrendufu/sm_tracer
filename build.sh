#!/usr/bin/env bash
set -e
BUILD_DIR="${1:-build}"
CMAKE_GENERATOR="${CMAKE_GENERATOR:-Ninja}"
echo "[sm_tracer] Configuring with CMake (generator: ${CMAKE_GENERATOR})..."
cmake -S . -B "${BUILD_DIR}" -G "${CMAKE_GENERATOR}"
echo "[sm_tracer] Building..."
cmake --build "${BUILD_DIR}" --parallel
echo "[sm_tracer] Build succeeded. Binary: ${BUILD_DIR}/sm_tracer"
