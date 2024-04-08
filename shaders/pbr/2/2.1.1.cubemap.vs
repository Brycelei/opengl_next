#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 WorldPos;

uniform mat4 projection;
uniform mat4 view;

void main()
{
    WorldPos = aPos;
    gl_Position =  projection * view * vec4(WorldPos, 1.0);
}
//将 equirectangular 图像转换为立方体贴图，我们需要渲染一个（单位）立方体并将 equirectangular 映射从内部投影到立方体的所有面上，并为立方体的每个侧面拍摄 6 张图像作为立方体贴图面。 这个立方体的顶点着色器只是简单地渲染立方体并将其本地位置作为 3D 样本向量传递给fragment shader片段着色器
             
//原文链接：https://blog.csdn.net/u013617851/article/details/122460724