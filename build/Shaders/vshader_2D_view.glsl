#version 140
#extension GL_ARB_compatibility: enable
out vec2 pixel_position;

void main(){
    pixel_position=vec2(gl_Vertex.x,-gl_Vertex.y);
    gl_Position =gl_Vertex;
}
