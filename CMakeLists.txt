
cmake_minimum_required(VERSION 3.12)

project(i106tools
    VERSION 0.1.2
    LANGUAGES C CXX)

# Build flags
if (MSVC)
    add_compile_options("-D_MSC_VER")
elseif(NOT APPLE)
    add_compile_options("-D_FILE_OFFSET_BITS=64 -D_LARGEFILE64_SOURCE")
endif()
set(CMAKE_C_FLAGS "-ggdb -fpack-struct=1 -fPIC -std=c99 -Wall")

# Library
include(DownloadProject.cmake)
download_project(
    PROJ                libirig106
    GIT_REPOSITORY      https://github.com/atac/libirig106.git
    GIT_TAG             cmake
)
add_subdirectory(${libirig106_SOURCE_DIR} ${libirig106_BINARY_DIR})
include_directories("${libirig106_SOURCE_DIR}/include")

# Individual tools
add_executable(i106stat src/i106stat.c)
target_link_libraries(i106stat libirig106)
