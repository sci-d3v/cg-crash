project(cg_tests)

find_package(GTest REQUIRED)
include(GoogleTest)

add_executable(cg_tests
    unit/common/detailed_exception.cpp
    unit/common/io_utils.cpp
    unit/common/shader.cpp
    unit/common/shader_program.cpp
    unit/common/texture.cpp
)

target_link_libraries(cg_tests PRIVATE
    GTest::gtest
    GTest::gtest_main
    common
    m
)

if(UNIX AND NOT APPLE)
    message(STATUS "Linux platform")
    target_link_libraries(cg_tests PRIVATE glfw GLEW SOIL GL)
elseif(WIN32)
    message(STATUS "Windows platform")
    # SOIL is a static lib and references GL entry points, so it has to
    # come before opengl32.
    target_link_libraries(cg_tests PRIVATE glfw3 glew32 SOIL glu32 opengl32)
else()
    message(SEND_ERROR "The project supports only Linux and Windows platforms")
endif()

target_include_directories(cg_tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/fixtures
    ..  # adding practicum/ directory to the include path
)

gtest_discover_tests(cg_tests)
