glslc "shader/shader.glsl.vert" -o "shader.vert.spv"
glslc "shader/shader.glsl.frag" -o "shader.frag.spv"

xxd -i "shader.frag.spv" > include/fragmentShader.h
xxd -i "shader.vert.spv" > include/vertexShader.h
::xxd -i triangle.vert.spv > triangle_vert.h
::cloc . --fullpath --not-match-d='(lib[\\/](glm|glfw)|cmake-build-debug|.idea)'
