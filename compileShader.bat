glslc -o "shader.vert.spv"   -fshader-stage=vert "shader/shader.glsl.vert"
glslc -o  "textureShader.frag.spv" -fshader-stage=frag "shader/textureShader.glsl.frag"
glslc -o  "noTextureShader.frag.spv" -fshader-stage=frag "shader/noTextureShader.glsl.frag"

xxd -i "textureShader.frag.spv" > include/textureFragmentShader.h
xxd -i "noTextureShader.frag.spv" > include/noTextureFragmentShader.h
xxd -i "shader.vert.spv" > include/vertexShader.h


::xxd -i triangle.vert.spv > triangle_vert.h
::cloc . --fullpath --not-match-d='(lib[\\/](glm|glfw|stb)|cmake-build-debug|.idea)'
