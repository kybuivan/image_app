cmake_minimum_required(VERSION 3.10)

set(CMAKE_CXX_STANDARD 11)

project(image_app
    VERSION 0.1.0
)

list(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${PROJECT_SOURCE_DIR}/cmake/external)

include(FetchContent)
include(rttr)
include(glfw)
include(glad2)
include(nfd)
include(imgui)
include(imnodes)
include(nlohmann_json)
include(entt)
#opengl

add_executable(${PROJECT_NAME} main.cpp)
add_dependencies(${PROJECT_NAME} glfw glad)
target_link_libraries(${PROJECT_NAME} PRIVATE imgui::imgui nfd glad rttr_core_lib nlohmann_json::nlohmann_json EnTT::EnTT imnodes)

#add_subdirectory(src)