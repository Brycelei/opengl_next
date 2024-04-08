#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 5) in ivec4 aBoneIds;
layout (location = 6) in vec4 aBoneWeights;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoords;

uniform mat4 model;
uniform mat3 model_normals;
uniform mat4 view_projection;



void main() {
 

    vec4 PosLocal = vec4(aPos, 1.0);
    vec4 NormalLocal = vec4(aNormal, 0.0);
    FragPos = vec3(model * PosLocal);
    Normal = model_normals * vec3(NormalLocal);
    TexCoords = aTexCoords;
    gl_Position = view_projection * vec4(FragPos, 1.0);
}