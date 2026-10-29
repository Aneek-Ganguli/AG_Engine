#version 450


void main(){
    vec3 position;
    if(gl_VertexIndex == 0){
        position = vec3(-1,-1,0);
    }else if(gl_VertexIndex == 1){
        position = vec3(3,-1,0);
    }else{
        position = vec3(-1,3,0);
    }
    gl_Position = vec4(position,1);
}