cmake_minimum_required(VERSION 3.20)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Outside the build tree, so wiping build/ does not redownload FetchContent deps.
# Must be set before include(FetchContent) in any subdirectory. Does not override
# a value already stored in an existing CMake cache.
set(FETCHCONTENT_BASE_DIR "${CMAKE_SOURCE_DIR}/third_party"
    CACHE PATH "FetchContent cache outside the build tree")

# Add subdirectories in dependency order
add_subdirectory(Practicum)
# add_subdirectory(Lectures)
