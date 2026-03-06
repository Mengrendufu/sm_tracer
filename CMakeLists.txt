#
# Required minimal cmake version
#
cmake_minimum_required(VERSION 3.10)

#
# Indicated compiler
#
# before project()
#
set(CMAKE_C_COMPILER gcc)

#
# c standard
#
set(CMAKE_C_STANDARD 99)

#
# Compiling options
#
# -save-temps=obj: Save the middle files(.i, .s, .o) and put it on the
#                  directory where the target file exists
#
# -MMD: Generate the .d dependency file
#
# add_compile_options(-save-temps=obj -MMD -Wall -Wextra -g)
add_compile_options(-MMD -Wall -Wextra -g)

#
# Generate compile_commands.json ?
#
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

#
# Output directories setting
#
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)  # .exe
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)  # .lib

#
# Name of project
#
project(sm_tracer C)


#
# component qpc ==============================================================
#
set(PATH_QPC
    ./
)
include_directories(
    ${PATH_QPC}/include
)
file(GLOB_RECURSE QPC_SRC
    ${PATH_QPC}/src/qf/*.c
)

#
# component SDL2 =============================================================
#
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON  CACHE BOOL "" FORCE)
add_subdirectory(3rd_party/sdl2)

#
# component libserialport ====================================================
#
set(LIBSERIALPORT_SRC_DIR ${CMAKE_SOURCE_DIR}/3rd_party/libserialport)

# Build libserialport as a static library (it has no CMakeLists.txt)
if(WIN32)
    set(SERIALPORT_PLATFORM_SRC
        ${LIBSERIALPORT_SRC_DIR}/windows.c
    )
    set(SERIALPORT_PLATFORM_LIBS setupapi)
elseif(APPLE)
    set(SERIALPORT_PLATFORM_SRC
        ${LIBSERIALPORT_SRC_DIR}/macosx.c
    )
    set(SERIALPORT_PLATFORM_LIBS "-framework IOKit" "-framework CoreFoundation")
else()
    # Linux — POSIX logic is already in serialport.c, only add linux-specific files
    set(SERIALPORT_PLATFORM_SRC
        ${LIBSERIALPORT_SRC_DIR}/linux.c
        ${LIBSERIALPORT_SRC_DIR}/linux_termios.c
    )
    find_package(PkgConfig QUIET)
    if(PkgConfig_FOUND)
        pkg_check_modules(LIBUDEV libudev)
    endif()
    if(LIBUDEV_FOUND)
        set(SERIALPORT_PLATFORM_LIBS ${LIBUDEV_LIBRARIES})
    else()
        set(SERIALPORT_PLATFORM_LIBS "")
    endif()
endif()

# Generate minimal config.h for libserialport (replaces autoconf output)
set(LIBSP_CONFIG_DIR "${CMAKE_BINARY_DIR}/libserialport_config")
file(MAKE_DIRECTORY "${LIBSP_CONFIG_DIR}")
if(WIN32)
    file(WRITE "${LIBSP_CONFIG_DIR}/config.h" "/* Minimal config.h for libserialport — Windows/CMake build */\n#define PACKAGE_VERSION \"0.1.1\"\n#define SP_PRIV\n")
else()
    file(WRITE "${LIBSP_CONFIG_DIR}/config.h"
        "/* Minimal config.h for libserialport — Unix/CMake build */\n"
        "#define HAVE_SYS_FILE_H 1\n"
        "#define HAVE_FLOCK 1\n"
        "#define HAVE_TERMIOS_H 1\n"
        "#define HAVE_STRUCT_SERIAL_STRUCT 1\n"
        "#define PACKAGE_VERSION \"0.1.1\"\n"
        "#define SP_PRIV\n"
    )
endif()

add_library(serialport STATIC
    ${LIBSERIALPORT_SRC_DIR}/serialport.c
    ${LIBSERIALPORT_SRC_DIR}/timing.c
    ${SERIALPORT_PLATFORM_SRC}
)
target_include_directories(serialport
    PUBLIC  ${LIBSERIALPORT_SRC_DIR}
    PRIVATE ${LIBSP_CONFIG_DIR}
)
# libserialport: use MSBUILD path (skips autoconf config.h include in internal header)
target_compile_definitions(serialport PRIVATE
    LIBSERIALPORT_MSBUILD
)
if(WIN32)
    target_compile_definitions(serialport PRIVATE _WIN32_WINNT=0x0501)
endif()
target_link_libraries(serialport ${SERIALPORT_PLATFORM_LIBS})

#
# component LVGL =============================================================
#
set(PATH_LVGL
    ${CMAKE_SOURCE_DIR}/3rd_party/lvgl
)
include_directories(
    ${PATH_LVGL}
    ${PATH_LVGL}/src
)
file(GLOB_RECURSE LVGL_SRC
    ${PATH_LVGL}/src/*.c
)

#
# component cJSON ============================================================
#
set(DIR_cJSON
    ${CMAKE_SOURCE_DIR}/3rd_party/cJSON)
include_directories(
    ${DIR_cJSON}
)
file(GLOB_RECURSE SRC_cJSON
    ${DIR_cJSON}/cJSON.c
)

#
# component SM_Hsm ===========================================================
#
set(PATH_SM_HSM
    ${CMAKE_SOURCE_DIR}/3rd_party/sm_hsm
)
include_directories(
    ${PATH_SM_HSM}/include
)
file(GLOB_RECURSE SM_HSM_SRC
    ${PATH_SM_HSM}/src/sm_hsm.c
)

#
# component local ============================================================
#
include_directories(
    3rd_party/
    application/
    bsp/
    bsp/lvgl/
    bsp/sdl/
    bsp/serial_port/
    ports/
)
file(GLOB_RECURSE LOCAL_SRC
    application/*.c
    bsp/*.c
    ports/*.c
)

#
# Join all the c files of the project ========================================
#
set(ALL_SOURCES
    ${LOCAL_SRC}
    ${QPC_SRC}
    ${LVGL_SRC}
    ${SRC_cJSON}
    ${SM_HSM_SRC}
)

#
# Executable rules ===========================================================
# WIN32: no console generated.
# add_executable(${PROJECT_NAME} WIN32 ${ALL_SOURCES})
add_executable(${PROJECT_NAME} ${ALL_SOURCES})

#
# linking libraries ==========================================================
#
target_link_libraries(${PROJECT_NAME}
    # SDL_MAIN_HANDLED is defined in bsp_sdl.c, so SDL2main must NOT be linked.
    # mingw32 is still required on Windows as the MinGW runtime entry point.
    $<$<BOOL:${WIN32}>:mingw32>
    SDL2::SDL2-static
    serialport
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} dwmapi)
endif()

#
# Debug infomations ==========================================================
#
message(STATUS
    "Executable will be located in: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
