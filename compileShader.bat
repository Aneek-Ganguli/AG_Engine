glslc -o "shader.vert.spv"   -fshader-stage=vert "shader/shader.glsl.vert"
glslc -o  "shader.frag.spv" -fshader-stage=frag "shader/shader.glsl.frag"

xxd -i "shader.frag.spv" > include/fragmentShader.h
xxd -i "shader.vert.spv" > include/vertexShader.h
::xxd -i triangle.vert.spv > triangle_vert.h
::cloc . --fullpath --not-match-d='(lib[\\/](glm|glfw)|cmake-build-debug|.idea)'
