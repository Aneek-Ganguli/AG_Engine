glslc "shader/shader.glsl.vert" -o "cmake-build-debug/res/shader/shader.vert.spv"
glslc "shader/shader.glsl.frag" -o "cmake-build-debug/res/shader/shader.frag.spv"
::xxd -i triangle.vert.spv > triangle_vert.h
::cloc . --fullpath --not-match-d='(include[\\/](glm|imgui)|cmake-build-debug)'
