glslc "shader/shader.glsl.vert" -o "shader.vert.spv"
glslc "shader/shape.glsl.frag" -o "shape.frag.spv"
glslc "shader/texture.glsl.frag" -o "texture.frag.spv"

xxd -i "texture.frag.spv" > include/shaderTexture.h
xxd -i "shape.frag.spv" > include/shaderShape.h
xxd -i "shader.vert.spv" > include/shaderVert.h
::xxd -i triangle.vert.spv > triangle_vert.h
::cloc . --fullpath --not-match-d='(include[\\/](glm|imgui)|cmake-build-debug|.idea)'
