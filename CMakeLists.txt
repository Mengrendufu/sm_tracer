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
# component SDL2 (prebuilt shared library) ===================================
#
set(SDL2_LIBS_DIR "${CMAKE_SOURCE_DIR}/3rd_party/libs/SDL2")
add_library(SDL2::SDL2 SHARED IMPORTED GLOBAL)
if(WIN32)
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION  "${SDL2_LIBS_DIR}/windows/SDL2.dll"
        IMPORTED_IMPLIB    "${SDL2_LIBS_DIR}/windows/libSDL2.dll.a"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_LIBS_DIR}/windows/include/SDL2"
    )
elseif(APPLE)
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION "${SDL2_LIBS_DIR}/macos/libSDL2.dylib"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_LIBS_DIR}/macos/include/SDL2"
    )
else()  # Linux
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION "${SDL2_LIBS_DIR}/linux/libSDL2.so"
        INTERFACE_INCLUDE_DIRECTORIES "${SDL2_LIBS_DIR}/linux/include/SDL2"
    )
endif()

#
# component libserialport (prebuilt shared library) ==========================
#
set(SERIALPORT_LIBS_DIR "${CMAKE_SOURCE_DIR}/3rd_party/libs/libserialport")
add_library(serialport SHARED IMPORTED GLOBAL)
if(WIN32)
    set_target_properties(serialport PROPERTIES
        IMPORTED_LOCATION  "${SERIALPORT_LIBS_DIR}/windows/libserialport-0.dll"
        IMPORTED_IMPLIB    "${SERIALPORT_LIBS_DIR}/windows/libserialport.dll.a"
        INTERFACE_INCLUDE_DIRECTORIES "${SERIALPORT_LIBS_DIR}/windows/include"
    )
elseif(APPLE)
    set_target_properties(serialport PROPERTIES
        IMPORTED_LOCATION "${SERIALPORT_LIBS_DIR}/macos/libserialport.dylib"
        INTERFACE_INCLUDE_DIRECTORIES "${SERIALPORT_LIBS_DIR}/macos/include"
    )
else()  # Linux
    set_target_properties(serialport PROPERTIES
        IMPORTED_LOCATION "${SERIALPORT_LIBS_DIR}/linux/libserialport.so"
        INTERFACE_INCLUDE_DIRECTORIES "${SERIALPORT_LIBS_DIR}/linux/include"
    )
endif()

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
    SDL2::SDL2
    serialport
)

# Windows: copy DLLs to executable directory at build time
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SDL2_LIBS_DIR}/windows/SDL2.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SERIALPORT_LIBS_DIR}/windows/libserialport-0.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
endif()

# Linux/macOS: embed rpath so the executable finds shared libs at runtime
if(NOT WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH
            "${SDL2_LIBS_DIR}/$<IF:$<BOOL:${APPLE}>,macos,linux>;${SERIALPORT_LIBS_DIR}/$<IF:$<BOOL:${APPLE}>,macos,linux>"
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()

if(WIN32)
    target_link_libraries(${PROJECT_NAME} dwmapi)
endif()

#
# Debug infomations ==========================================================
#
message(STATUS
    "Executable will be located in: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
