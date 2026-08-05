#!/bin/bash
# =============================================================================
# ROS2 Workspace Build Script
# Toolchain: LLVM 22 (clang / clang++ / lld)  |  Backend: Ninja
#
# Usage:
#   ./build.sh          - Incremental build (CMake export + merge)
#   ./build.sh --clean  - Full clean rebuild (bear + colcon)
# =============================================================================

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

# Terminal colors
GREEN='\033[0;32m'
CYAN='\033[0;36m'
RED='\033[0;31m'
YELLOW='\033[0;33m'
BOLD='\033[1m'
NC='\033[0m'

# -----------------------------------------------------------------------------
# Prepare environment
# -----------------------------------------------------------------------------
export PATH="${LLVM_BIN}:$PATH"
export LD_LIBRARY_PATH="${LLVM_BIN}/../lib:${LD_LIBRARY_PATH}"
export CC CXX AR RANLIB

# Detect workspace root (script location)
SCRIPT_PATH=$(readlink -f "$0")
WS_ROOT=$(cd "$(dirname "$SCRIPT_PATH")" && pwd)
cd "$WS_ROOT" || exit 1

# Source ROS 2 environment
source "/opt/ros/${ROS_DISTRO}/setup.bash"

# Append Qt6 path to CMAKE_PREFIX_PATH (do NOT overwrite)
export CMAKE_PREFIX_PATH="${QT6_PATH}:${CMAKE_PREFIX_PATH}"

# =============================================================================
# Main entry point
# =============================================================================
main() {
    if [[ "${1:-}" == "--clean" ]]; then
        build_full
    else
        build_incremental
    fi
}

# -----------------------------------------------------------------------------
# Function definitions (all at the end, as requested)
# -----------------------------------------------------------------------------

# Helper: print elapsed time (exactly as original user format)
print_elapsed() {
    local start=$1
    local end=$(date +%s)
    local duration=$((end - start))
    echo -e "\n${GREEN}${BOLD}[INFO] Build completed successfully. Elapsed time: ${duration}s${NC}"
}

# Full clean build (bear + colcon)
build_full() {
    local start_time=$(date +%s)
    echo -e "${CYAN}[Full Build] Cleaning build/ install/ log/ compile_commands.json ...${NC}"
    rm -rf build/ install/ log/ compile_commands.json

    echo -e "${CYAN}[Full Build] Starting with bear + colcon (Clang-22, LLD, Ninja)...${NC}"
    bear --output compile_commands.json -- \
        colcon build \
            --symlink-install \
            --event-handlers console_direct+ status+ \
            --cmake-args \
                -G Ninja \
                -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
                -DCMAKE_C_COMPILER="${CC}" \
                -DCMAKE_CXX_COMPILER="${CXX}" \
                -DCMAKE_AR="${AR}" \
                -DCMAKE_RANLIB="${RANLIB}" \
                -DCMAKE_C_COMPILER_AR="${AR}" \
                -DCMAKE_CXX_COMPILER_AR="${AR}" \
                -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
                -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" \
                -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld"

    local status=$?
    if [ $status -ne 0 ]; then
        echo -e "${RED}[Full Build] FAILED.${NC}"
        exit $status
    fi

    # Filter compile_commands.json for clangd (optional)
    if command -v jq &> /dev/null; then

        jq 'map(
            select(
                (.directory | contains("CMakeScratch") | not) and
                (.directory | contains("CompilerId") | not)
            )
        ) | unique_by(.file)' compile_commands.json > compile_commands_clean.json
        mv compile_commands_clean.json compile_commands.json
        echo -e "${GREEN}[Full Build] compile_commands.json filtered and deduplicated.${NC}"
    else
        echo -e "${YELLOW}[Full Build] jq not installed; skipping filter.${NC}"
    fi

    print_elapsed $start_time
}

# Incremental build (CMake export + merge subdir compile_commands)
build_incremental() {
    local start_time=$(date +%s)
    echo -e "${CYAN}[Incremental Build] Starting colcon (CMake export mode, no bear)...${NC}"

    # Remove root compile_commands.json to avoid confusion (subdirs will be merged)
    rm -f compile_commands.json

    # Build with CMake export enabled – each package generates its own compile_commands.json
    colcon build \
        --symlink-install \
        --event-handlers console_direct+ status+ \
        --cmake-args \
            -G Ninja \
            -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
            -DCMAKE_C_COMPILER="${CC}" \
            -DCMAKE_CXX_COMPILER="${CXX}" \
            -DCMAKE_AR="${AR}" \
            -DCMAKE_RANLIB="${RANLIB}" \
            -DCMAKE_C_COMPILER_AR="${AR}" \
            -DCMAKE_CXX_COMPILER_AR="${AR}" \
            -DCMAKE_EXE_LINKER_FLAGS="-fuse-ld=lld" \
            -DCMAKE_SHARED_LINKER_FLAGS="-fuse-ld=lld" \
            -DCMAKE_MODULE_LINKER_FLAGS="-fuse-ld=lld"


    local status=$?
    if [ $status -ne 0 ]; then
        echo -e "${RED}[Incremental Build] FAILED.${NC}"
        exit $status
    fi

   # Merge all package‑level compile_commands.json into root
   if command -v jq &> /dev/null; then
       find build/ -name "compile_commands.json" -print0 | xargs -0 cat | \
           jq -s 'add
               | map(
                   select(
                       (.directory | contains("CMakeScratch") | not) and
                       (.directory | contains("CompilerId") | not)
                   )
               )
               | unique_by(.file)' > compile_commands.json
       echo -e "${GREEN}[Incremental Build] Merged compile_commands.json created.${NC}"
   else
       echo -e "${YELLOW}[Incremental Build] jq not installed; cannot merge compile_commands.json.${NC}"
   fi

    print_elapsed $start_time
}

# Invoke main with all arguments
main "$@"