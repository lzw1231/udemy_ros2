#!/bin/bash
# =============================================================================
# ROS2 Workspace Full Build Script
# Toolchain: LLVM 22 (clang / clang++ / llvm-ar / llvm-ranlib / lld)
# Build Backend: Ninja | Compile Database: bear
#
# Function: Full clean rebuild + generate standard compile_commands.json
# Applicable: Local development environment, ROS2 Jazzy
# Feature: Strict LLVM toolchain enforcement, LLD linking, Clang static checking
# =============================================================================

BUILD_START=$(date +%s)

# -----------------------------------------------------------------------------
# Configuration
# -----------------------------------------------------------------------------
ROS_DISTRO="jazzy"
QT6_PATH="/home/lzw/Qt/6.11.1/gcc_64"
LLVM_BIN="/opt/LLVM-22/bin"

# LLVM Toolchain binaries
CC="${LLVM_BIN}/clang"
CXX="${LLVM_BIN}/clang++"
AR="${LLVM_BIN}/llvm-ar"
RANLIB="${LLVM_BIN}/llvm-ranlib"

# Terminal colours
GREEN='\033[0;32m'
CYAN='\033[0;36m'
RED='\033[0;31m'
NC='\033[0m'
YELLOW='\033[0;33m'

# -----------------------------------------------------------------------------
# Prepend LLVM to PATH (ensure clang picks ld.lld)
# -----------------------------------------------------------------------------
export PATH="${LLVM_BIN}:$PATH"
export LD_LIBRARY_PATH="${LLVM_BIN}/../lib:${LD_LIBRARY_PATH}"

# Export compiler/archiver variables
export CC CXX AR RANLIB

# -----------------------------------------------------------------------------
# Workspace root detection
# -----------------------------------------------------------------------------
SCRIPT_PATH=$(readlink -f "$0")
WS_ROOT=$(cd "$(dirname "$SCRIPT_PATH")" && pwd)
cd "${WS_ROOT}" || exit 1

# -----------------------------------------------------------------------------
# Source ROS 2 environment (sets AMENT_PREFIX_PATH, CMAKE_PREFIX_PATH, etc.)
# -----------------------------------------------------------------------------
source "/opt/ros/${ROS_DISTRO}/setup.bash"

# Append Qt6 path to the existing CMAKE_PREFIX_PATH (do NOT overwrite!)
export CMAKE_PREFIX_PATH="${QT6_PATH}:${CMAKE_PREFIX_PATH}"

# -----------------------------------------------------------------------------
# Clean previous build artifacts
# -----------------------------------------------------------------------------
echo -e "${CYAN}Cleaning build/ install/ log/ compile_commands.json ...${NC}"
rm -rf build/ install/ log/ compile_commands.json

# -----------------------------------------------------------------------------
# Build with bear + colcon + Ninja + Clang22 + lld
# -----------------------------------------------------------------------------
echo -e "${CYAN}Building with Clang-22, LLD, Ninja...${NC}"
bear --output compile_commands.json -- \
    colcon build \
        --symlink-install \
        --event-handlers console_direct+ status+ \
        --cmake-args \
            -G Ninja \
            -DCMAKE_C_COMPILER="${CC}" \
            -DCMAKE_CXX_COMPILER="${CXX}" \
            -DCMAKE_AR="${AR}" \
            -DCMAKE_RANLIB="${RANLIB}" \
            -DCMAKE_C_COMPILER_AR="${AR}" \
            -DCMAKE_CXX_COMPILER_AR="${AR}" \
            -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
            -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" \
            -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld"

BUILD_STATUS=$?
BUILD_END=$(date +%s)
DURATION=$((BUILD_END - BUILD_START))

if [ $BUILD_STATUS -ne 0 ]; then
    echo -e "${RED}Build failed after ${DURATION}s.${NC}"
    exit 1
fi

# -----------------------------------------------------------------------------
# Filter compile_commands.json for clangd (optional)
# -----------------------------------------------------------------------------
if command -v jq &> /dev/null; then
    jq 'map(
        select(
            (.directory | contains("CMakeScratch") | not) and
            (.directory | contains("CompilerId") | not)
        )
    ) | unique_by(.file)' compile_commands.json > compile_commands_clean.json
    mv compile_commands_clean.json compile_commands.json
fi

# -----------------------------------------------------------------------------
# Done
# -----------------------------------------------------------------------------
echo -e "\n${GREEN}${BOLD}[INFO] Build completed successfully. Elapsed time: ${DURATION}s${NC}"