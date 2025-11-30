cmake_minimum_required(VERSION 3.31)
project(voxel LANGUAGES CXX)
set(CMAKE_EXPORT_COMPILE_COMMANDS 1)

include(FetchContent)
FetchContent_Declare(SDL3
  GIT_REPOSITORY https://github.com/libsdl-org/SDL
  GIT_TAG release-3.2.26
)
FetchContent_MakeAvailable(SDL3)

FetchContent_Declare(SDL3_image
  GIT_REPOSITORY https://github.com/libsdl-org/SDL_image
  GIT_TAG release-3.2.4
)
FetchContent_MakeAvailable(SDL3_image)

FetchContent_Declare(glew
  GIT_REPOSITORY https://github.com/Perlmint/glew-cmake
  GIT_TAG glew-cmake-2.2.0
)
FetchContent_MakeAvailable(glew)

FetchContent_Declare(imgui
  GIT_REPOSITORY https://github.com/ocornut/imgui
  GIT_TAG v1.92.5
)
FetchContent_MakeAvailable(imgui)

FetchContent_Declare(glm
  GIT_REPOSITORY https://github.com/g-truc/glm
  GIT_TAG 1.0.2
)
FetchContent_MakeAvailable(glm)

FetchContent_Declare(FastNoiseLite
  GIT_REPOSITORY https://github.com/Auburn/FastNoiseLite
  GIT_TAG v1.1.1
)
FetchContent_MakeAvailable(FastNoiseLite)

set(IMGUI_SOURCES
  ${imgui_SOURCE_DIR}/imgui.h
  ${imgui_SOURCE_DIR}/imgui.cpp
  ${imgui_SOURCE_DIR}/imgui_demo.cpp
  ${imgui_SOURCE_DIR}/imgui_draw.cpp
  ${imgui_SOURCE_DIR}/imgui_widgets.cpp
  ${imgui_SOURCE_DIR}/imgui_tables.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_sdl3.cpp
  ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

file(GLOB_RECURSE SOURCES
  ${PROJECT_SOURCE_DIR}/src/*.cpp
  ${PROJECT_SOURCE_DIR}/src/*.h
)

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

add_executable(${PROJECT_NAME}
  ${SOURCES}
  ${IMGUI_SOURCES}
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${PROJECT_SOURCE_DIR}/src
  ${imgui_SOURCE_DIR}
  ${fastnoiselite_SOURCE_DIR}/Cpp
)

target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wpedantic -g)
target_link_libraries(${PROJECT_NAME} SDL3::SDL3 SDL3_image::SDL3_image
                      libglew_static glm)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 23)

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
  COMMAND ${CMAKE_COMMAND}
  -E create_symlink
  ${CMAKE_SOURCE_DIR}/assets
  ${CMAKE_BINARY_DIR}/assets)
