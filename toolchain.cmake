# =============================================================================
# GCC desktop C toolchain for sm_tracer
# Purpose: keep compiler and common warning flags outside root CMakeLists.txt.
# =============================================================================

set(CMAKE_C_COMPILER gcc)

set(CMAKE_C_FLAGS
    "-MMD -Wall -Wextra -Wpedantic"
    CACHE STRING "Common C flags"
    FORCE
)
