cmake_minimum_required(VERSION 3.16)
project(VulkanProjectV2)

set(CMAKE_CXX_STANDARD 20)

set(CMAKE_WINDOWS_EXPORT_ALL_SYMBOLS ON)

if(MSVC)
    add_compile_options(/Zi) # Full debugging information
    add_link_options(/DEBUG) # Generate .pdb files for linker
endif()

find_package(Vulkan REQUIRED)

add_subdirectory(lib/glfw EXCLUDE_FROM_ALL)

add_library(VulkanProjectV2 STATIC
        src/Window.cpp
        src/Logger.cpp
        src/PhyscialDevice.cpp
        src/Queue.cpp
        src/Device.cpp
        src/Surface.cpp
        src/Swapchain.cpp
)

target_include_directories(VulkanProjectV2
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/lib/glfw/include>
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/lib
)

target_link_libraries(VulkanProjectV2
        PUBLIC
        Vulkan::Vulkan
        glfw
)

add_executable(VulkanApp src/main.cpp)
target_link_libraries(VulkanApp PUBLIC VulkanProjectV2)