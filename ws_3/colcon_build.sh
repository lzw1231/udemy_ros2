#!/bin/bash
# =============================================================================
# Clean LLVM-Based Build Script for ROS 2 Workspace
# Single-user Local Machine, Clang-22 + lld, Ninja + bear
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
YELLOW='\033[0;33m'
BOLD='\033[1m'
NC='\033[0m'

# -----------------------------------------------------------------------------
# Prepend LLVM to PATH (ensure clang picks llvm22 ld.lld)
# -----------------------------------------------------------------------------
export PATH="${LLVM_BIN}:$PATH"
export LD_LIBRARY_PATH="${LLVM_BIN}/../lib:${LD_LIBRARY_PATH}"

# Export compiler/archiver variables
export CC
export CXX
export AR
export RANLIB

# -----------------------------------------------------------------------------
# Workspace detection
# -----------------------------------------------------------------------------
SCRIPT_PATH=$(readlink -f "$0")
WS_ROOT=$(cd "$(dirname "$SCRIPT_PATH")" && pwd)
if [ ! -d "${WS_ROOT}/src" ]; then
    echo -e "${RED}${BOLD}Error: src/ directory not found. Place this script in the workspace root.${NC}"
    exit 1
fi
cd "${WS_ROOT}" || exit 1


# -----------------------------------------------------------------------------
# Source ROS 2 environment
# -----------------------------------------------------------------------------
ROS_SETUP="/opt/ros/${ROS_DISTRO}/setup.bash"
if [ ! -f "$ROS_SETUP" ]; then
    echo -e "${RED}${BOLD}Error: ROS 2 environment not found at ${ROS_SETUP}${NC}"
    exit 1
fi
source "$ROS_SETUP"

export AMENT_PREFIX_PATH="/opt/ros/${ROS_DISTRO}"
export CMAKE_PREFIX_PATH="/opt/ros/${ROS_DISTRO}"
# -----------------------------------------------------------------------------
# Install bear / jq if missing
# -----------------------------------------------------------------------------
if ! command -v bear &> /dev/null; then
    sudo apt update && sudo apt install -y bear > /dev/null 2>&1
fi
if ! command -v jq &> /dev/null; then
    sudo apt update && sudo apt install -y jq > /dev/null 2>&1
fi

# -----------------------------------------------------------------------------
# Clean build artifacts
# -----------------------------------------------------------------------------
echo -e "${CYAN}Cleaning previous build artifacts...${NC}"
rm -rf build/ install/ log/ compile_commands.json

# -----------------------------------------------------------------------------
# Build: bear + colcon + Ninja + Clang22 + lld
# -----------------------------------------------------------------------------
echo -e "${CYAN}Building with bear + LLVM22 (clang / lld / llvm-ar)...${NC}"
bear --output compile_commands.json -- \
    colcon build \
        --symlink-install \
        --event-handlers console_direct+ status+ \
        --cmake-args \
            -G Ninja \
            -DCMAKE_C_COMPILER="${CC}" \
            -DCMAKE_CXX_COMPILER="${CXX}" \
            -DCMAKE_AR="${LLVM_BIN}/llvm-ar" \
            -DCMAKE_RANLIB="${LLVM_BIN}/llvm-ranlib" \
            -DCMAKE_C_COMPILER_AR="${LLVM_BIN}/llvm-ar" \
            -DCMAKE_CXX_COMPILER_AR="${LLVM_BIN}/llvm-ar" \
            -DCMAKE_PREFIX_PATH="${QT6_PATH}" \
            -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
            -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" \
            -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld" \
            -DCMAKE_STATIC_LINKER_FLAGS="-fuse-ld=lld"

BUILD_STATUS=$?
BUILD_END=$(date +%s)
DURATION=$((BUILD_END - BUILD_START))

if [ $BUILD_STATUS -ne 0 ]; then
    echo -e "${RED}${BOLD}Build failed after ${DURATION} seconds.${NC}"
    exit 1
fi

# -----------------------------------------------------------------------------
# Filter out CMakeScratch entries from compile_commands.json
# -----------------------------------------------------------------------------
if command -v jq &> /dev/null; then
    jq 'map(
        select(
            (.directory | contains("CMakeScratch") | not)
            and (.directory | contains("CompilerId") | not)
        )
    )' compile_commands.json > compile_commands_clean.json
    mv compile_commands_clean.json compile_commands.json
else
    echo -e "${YELLOW}Warning: jq unavailable, compile_commands.json unfiltered.${NC}"
fi

# -----------------------------------------------------------------------------
# Finish
# -----------------------------------------------------------------------------
echo -e "\n${GREEN}${BOLD}[BUILD SUCCESS] Workspace compilation finished. Duration: ${DURATION}s${NC}"