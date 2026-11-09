cmake_minimum_required(VERSION 3.16)
project(VulkanProjectV2)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(MSVC)
    add_compile_options(/Zi) # Full debugging information
    add_link_options(/DEBUG) # Generate .pdb files for linker
endif()

set(GLM_BUILD_TESTS OFF CACHE INTERNAL "")

find_package(Vulkan REQUIRED)


add_subdirectory(lib/glfw EXCLUDE_FROM_ALL)
add_subdirectory(lib/glm EXCLUDE_FROM_ALL)

add_library(IMGUI STATIC
    lib/imgui/imgui.cpp
    lib/imgui/imgui_draw.cpp
    lib/imgui/imgui_tables.cpp
    lib/imgui/imgui_widgets.cpp
    lib/imgui/backends/imgui_impl_glfw.cpp
    lib/imgui/backends/imgui_impl_vulkan.cpp
    lib/imgui/imgui_demo.cpp
)

include_directories(
    lib/glfw/include
    lib/glm
    lib/imgui
    lib/imgui/backends
)
target_link_libraries(IMGUI PRIVATE glfw Vulkan::Vulkan)

add_library(VulkanProjectV2 STATIC
        src/core/Window.cpp
        src/core/Logger.cpp
        src/core/PhysicalDevice.cpp
        src/core/Queue.cpp
        src/core/Device.cpp
        src/core/Surface.cpp
        src/core/Swapchain.cpp
        src/core/Fence.cpp
        src/core/CommandPool.cpp
        src/core/CommandBuffer.cpp
        src/Buffer.cpp
        src/GraphicsPipeline.cpp
        src/VertexBuffer.cpp
        src/IndexBuffer.cpp
        src/DescriptorSet.cpp
        src/UniformBuffer.cpp
        src/Texture.cpp
        src/DepthBuffer.cpp
        src/Application.cpp
        src/Entity.cpp
        src/BindlessTextures.cpp
        src/Shader.cpp
)

target_include_directories(VulkanProjectV2
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/include>
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lib/glm/glm
        PUBLIC
        "C:/Program Files/Assimp/include"
)

target_link_libraries(VulkanProjectV2
        PUBLIC
        Vulkan::Vulkan
        glfw
        glm::glm
        IMGUI
        "C:/Program Files/Assimp/lib/x64/assimp-vc143-mt.lib"
)


option(BUILD_TESTING "Build the test suite executable" OFF)

if(BUILD_TESTING)
    add_executable(VulkanApp ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
    target_link_libraries(VulkanApp PRIVATE VulkanProjectV2)
endif()

option(BUILD_EDITOR "Build the editor executable" OFF)
if(BUILD_EDITOR)
    add_executable(VulkanEditor ${CMAKE_CURRENT_SOURCE_DIR}/src/editor/main.cpp)
    target_link_libraries(VulkanEditor PRIVATE VulkanProjectV2)
endif()