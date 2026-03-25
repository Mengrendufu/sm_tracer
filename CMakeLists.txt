#=============================================================================
# sm_tracer root build file
# Purpose: keep project structure in CMakeLists.txt and move compiler/generator
# choices into toolchain.cmake + CMakePresets.json.
#=============================================================================

cmake_minimum_required(VERSION 3.20)

project(sm_tracer LANGUAGES C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

set(PROJECT_ROOT ${CMAKE_SOURCE_DIR})

set(ROOT_QPC ${PROJECT_ROOT})
set(ROOT_LVGL ${PROJECT_ROOT}/3rd_party/lvgl)
set(ROOT_CJSON ${PROJECT_ROOT}/3rd_party/cJSON)
set(ROOT_SM_HSM ${PROJECT_ROOT}/3rd_party/sm_hsm)
set(ROOT_SDL2_LIBS ${PROJECT_ROOT}/3rd_party/libs/SDL2)
set(ROOT_SERIALPORT_LIBS ${PROJECT_ROOT}/3rd_party/libs/libserialport)

file(GLOB_RECURSE SRC_QPC CONFIGURE_DEPENDS
    ${ROOT_QPC}/src/qf/*.c
)

file(GLOB_RECURSE SRC_LVGL CONFIGURE_DEPENDS
    ${ROOT_LVGL}/src/*.c
)

set(SRC_CJSON
    ${ROOT_CJSON}/cJSON.c
)

set(SRC_SM_HSM
    ${ROOT_SM_HSM}/src/sm_hsm.c
)

file(GLOB_RECURSE SRC_LOCAL CONFIGURE_DEPENDS
    ${PROJECT_ROOT}/application/*.c
    ${PROJECT_ROOT}/bsp/*.c
    ${PROJECT_ROOT}/ports/*.c
)

set(SRC_ALL
    ${SRC_LOCAL}
    ${SRC_QPC}
    ${SRC_LVGL}
    ${SRC_CJSON}
    ${SRC_SM_HSM}
)

set(INC_ALL
    ${ROOT_QPC}/include
    ${ROOT_LVGL}
    ${ROOT_LVGL}/src
    ${ROOT_CJSON}
    ${ROOT_SM_HSM}/include
    ${PROJECT_ROOT}/3rd_party
    ${PROJECT_ROOT}/application
    ${PROJECT_ROOT}/bsp
    ${PROJECT_ROOT}/bsp/lvgl
    ${PROJECT_ROOT}/bsp/sdl
    ${PROJECT_ROOT}/bsp/serial_port
    ${PROJECT_ROOT}/ports
)

add_library(SDL2::SDL2 SHARED IMPORTED GLOBAL)
add_library(serialport SHARED IMPORTED GLOBAL)

if(WIN32)
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION "${ROOT_SDL2_LIBS}/windows/SDL2.dll"
        IMPORTED_IMPLIB "${ROOT_SDL2_LIBS}/windows/libSDL2.dll.a"
        INTERFACE_INCLUDE_DIRECTORIES "${ROOT_SDL2_LIBS}/windows/include/SDL2"
    )

    set_target_properties(serialport PROPERTIES
        IMPORTED_LOCATION "${ROOT_SERIALPORT_LIBS}/windows/libserialport-0.dll"
        IMPORTED_IMPLIB "${ROOT_SERIALPORT_LIBS}/windows/libserialport.dll.a"
        INTERFACE_INCLUDE_DIRECTORIES "${ROOT_SERIALPORT_LIBS}/windows/include"
    )
else()
    set_target_properties(SDL2::SDL2 PROPERTIES
        IMPORTED_LOCATION "${ROOT_SDL2_LIBS}/linux/libSDL2.so"
        INTERFACE_INCLUDE_DIRECTORIES "${ROOT_SDL2_LIBS}/linux/include/SDL2"
    )

    set_target_properties(serialport PROPERTIES
        IMPORTED_LOCATION "${ROOT_SERIALPORT_LIBS}/linux/libserialport.so"
        INTERFACE_INCLUDE_DIRECTORIES "${ROOT_SERIALPORT_LIBS}/linux/include"
    )
endif()

if(WIN32)
    add_executable(${PROJECT_NAME} WIN32 ${SRC_ALL})
else()
    add_executable(${PROJECT_NAME} ${SRC_ALL})
endif()

target_include_directories(${PROJECT_NAME} PRIVATE
    ${INC_ALL}
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    SDL2::SDL2
    serialport
)

if(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE dwmapi winmm)

    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ROOT_SDL2_LIBS}/windows/SDL2.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${ROOT_SERIALPORT_LIBS}/windows/libserialport-0.dll"
            $<TARGET_FILE_DIR:${PROJECT_NAME}>
    )
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES
        BUILD_RPATH "${ROOT_SDL2_LIBS}/linux;${ROOT_SERIALPORT_LIBS}/linux"
        INSTALL_RPATH_USE_LINK_PATH TRUE
    )
endif()

add_custom_target(run
    COMMAND $<TARGET_FILE:${PROJECT_NAME}>
    DEPENDS ${PROJECT_NAME}
    USES_TERMINAL
    VERBATIM
)

message(STATUS "Executable output directory: ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
