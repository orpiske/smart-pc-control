cmake_minimum_required(VERSION 2.8.4)
project(smart_pc_control C)

set(CMAKE_C_STANDARD 99)
set(GRU_DIR "/usr" CACHE STRING "GRU directory")
add_definitions(-DPROJECT_NAME="${PROJECT_NAME}")

if (UNIX)
    message(STATUS "Adding Unix related build rules")
    include(CMakeLists.unix.cmake)
endif(UNIX)

set(GRU_DIR "/usr" CACHE STRING "GRU directory")
set(BUILD_WITH_EXTRA_DEBUG OFF CACHE BOOL "Build extra debug information")
set(BUILD_WITH_UNIT_TESTS ON CACHE BOOL "Build unit tests")

include_directories(${GRU_INCLUDE_DIR}
        ${JSON_INCLUDE_DIR}
        ${URIPARSER_INCLUDE_DIR}
        ${ZLIB_INCLUDE_DIR}
        )

link_libraries(${MATH_LIB}
        ${RT_LIB}
        ${GRU_LIB}
        ${URIPARSER_LIB}
        ${UUID_LIB}
        ${ZLIB_LIB}
        )


include_directories(src/ src/common)
add_executable(smart_pc_control src/power-control/main.c src/power-control/main.h src/common/client.c src/common/client.h)