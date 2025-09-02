cmake_minimum_required(VERSION 4.0)
project(AG_Engine)

set(CMAKE_CXX_STANDARD 20)



include_directories("${PROJECT_SOURCE_DIR}/include")
add_executable(AG_Engine
        src/main.cpp
        src/entity.cpp
        src/component.cpp
        src/vertexData.cpp
        src/window.cpp
        src/component.cpp
)

target_link_libraries(AG_Engine PRIVATE "C:/DEV/SDL/SDL3/SDL3 VC/lib/x64/SDL3.lib")
target_link_libraries(AG_Engine PRIVATE "C:/Program Files/Assimp/lib/x64/assimp-vc143-mt.lib")
target_link_libraries(AG_Engine PRIVATE "C:/DEV/SDL/SDL_image/SDL3_image VC/lib/x64/SDL3_image.lib")
#target_include_directories(SDL3_GPU_c PRIVATE "${PROJECT_SOURCE_DIR}/include")
target_include_directories(AG_Engine PRIVATE "C:/DEV/SDL/SDL3/SDL3 VC/include")
target_include_directories(AG_Engine PRIVATE "C:/DEV/SDL/SDL_image/SDL3_image VC/include")
target_include_directories(AG_Engine PRIVATE "C:/Program Files/Assimp/include")