set(CURRENT_PROJECT cg_imgui_transform_inspector)
project(${CURRENT_PROJECT})

find_package(OpenGL REQUIRED)
find_package(glfw3 REQUIRED)
find_package(glm REQUIRED)

include(FetchContent)

FetchContent_Declare(
    imgui
    URL https://github.com/ocornut/imgui/archive/refs/tags/v1.90.4.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(imgui)

FetchContent_Declare(
    stb
    URL https://github.com/nothings/stb/archive/af1a5bc352164740c1cc1354942b1c6b72eacb8a.zip
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)

FetchContent_MakeAvailable(stb)

set(IMGUI_SOURCES
    ${imgui_SOURCE_DIR}/imgui.cpp
    ${imgui_SOURCE_DIR}/imgui_demo.cpp
    ${imgui_SOURCE_DIR}/imgui_draw.cpp
    ${imgui_SOURCE_DIR}/imgui_tables.cpp
    ${imgui_SOURCE_DIR}/imgui_widgets.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
    ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

file(GLOB_RECURSE SRCS ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp)
add_executable(${CURRENT_PROJECT} ${SRCS} ${IMGUI_SOURCES})

target_include_directories(${CURRENT_PROJECT} PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/Practicum
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${stb_SOURCE_DIR}
)

target_link_libraries(${CURRENT_PROJECT} glm::glm)
if(UNIX AND NOT APPLE)
    message(STATUS "Linux platform")
    target_link_libraries(${CURRENT_PROJECT} glfw OpenGL::GL)
elseif(WIN32)
    message(STATUS "Windows platform")
    target_link_libraries(${CURRENT_PROJECT} glu32 opengl32 glfw3)
else()
    message(SEND_ERROR "The project supports only Linux and Windows platforms")
endif()
