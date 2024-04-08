#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{    
  // 通过传过来的向量，通过samplerCube这种类型的纹理，就可以采样到颜色值，作为输出
    FragColor = texture(skybox, TexCoords);
}