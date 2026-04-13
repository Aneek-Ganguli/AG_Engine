cmake_minimum_required(VERSION 3.16)
project(VulkanProject)

set(CMAKE_CXX_STANDARD 20)

find_package(Vulkan REQUIRED)

add_subdirectory(lib/glfw)
SET(GLM_TEST_ENABLE OFF CACHE BOOL "GLM Build unit tests")
add_subdirectory(lib/glm EXCLUDE_FROM_ALL)

include_directories(include)
include_directories(lib)


add_executable(VulkanProject
        src/main.cpp
        src/window.cpp
        src/logger.cpp
        src/vertexInfo.cpp
        src/entity.cpp
        src/texture.cpp
)
file(COPY ${CMAKE_SOURCE_DIR}/res DESTINATION ${CMAKE_BINARY_DIR})
target_compile_definitions(VulkanProject PRIVATE DEBUG=1)

target_link_libraries(${PROJECT_NAME} glfw ${GLFW_LIBRARIES} Vulkan::Vulkan)
