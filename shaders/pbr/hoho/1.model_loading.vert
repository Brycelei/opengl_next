#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;


void main()
{

    vs_out.FragPos = vec3(model * vec4(aPos, 1.0));   
    vs_out.TexCoords = aTexCoords;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalMatrix * aNormal;
    vec3 T = normalize(normalMatrix * aTangent).xyz;
    vec3 N = normalize(normalMatrix * aNormal).xyz;
    //Gram-Schmidt process
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}