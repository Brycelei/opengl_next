#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    TexCoords = aPos;
    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;//（x/w 为片段的深度值，将z改为w，w/w= 1.0f，天空盒子的深度为1.0f，保证天空盒子是宇宙的尽头
}  