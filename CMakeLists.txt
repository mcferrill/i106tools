
cmake_minimum_required(VERSION 3.12)

project(i106tools
    VERSION 0.1.2
    LANGUAGES C CXX)

# Build flags
if (MSVC)
    add_compile_options("-D_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH")
endif()

# Library
include(DownloadProject.cmake)
download_project(
    PROJ                irig106
    GIT_REPOSITORY      https://github.com/atac/libirig106.git
    GIT_TAG             master 
    UPDATE_DISCONNECTED 1
    QUIET
)
add_subdirectory(${irig106_SOURCE_DIR} ${irig106_BINARY_DIR})
include_directories("${irig106_SOURCE_DIR}/include")

# Individual tools
add_executable(i106stat src/i106stat.cpp)
target_link_libraries(i106stat irig106)

add_executable(demo src/demo.c)
target_link_libraries(demo irig106)
