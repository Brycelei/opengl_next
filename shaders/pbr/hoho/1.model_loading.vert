#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoords;
layout (location = 3) in vec3 aTangent;
layout (location = 4) in vec3 aBitangent;
layout (location = 5) in ivec4 aBoneIds;
layout (location = 6) in vec4 aBoneWeights;

out VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
} vs_out;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

const int MAX_NUMBER_BONES = 200;
uniform mat4 bone_transforms[MAX_NUMBER_BONES];
uniform int is_animated;

void main()
{

    mat4 BoneTransform = bone_transforms[aBoneIds[0]] * aBoneWeights[0];
    BoneTransform += bone_transforms[aBoneIds[1]] * aBoneWeights[1];
    BoneTransform += bone_transforms[aBoneIds[2]] * aBoneWeights[2];
    BoneTransform += bone_transforms[aBoneIds[3]] * aBoneWeights[3];

    vec4 PosLocal = vec4(aPos, 1.0);
    vec4 NormalLocal = vec4(aNormal, 0.0);
    if (is_animated == 1) {
        PosLocal = BoneTransform * PosLocal;
        NormalLocal = BoneTransform * NormalLocal;
    }

    //vs_out.FragPos = vec3(model * vec4(aPos, 1.0));  
    vs_out.FragPos = vec3(model * PosLocal);
    vs_out.TexCoords = aTexCoords;
    mat3 normalMatrix = transpose(inverse(mat3(model)));
    vs_out.Normal = normalMatrix * vec3(NormalLocal);
    
    vec3 T = normalize(normalMatrix * aTangent).xyz;
    vec3 N = normalize(normalMatrix * aNormal).xyz;
    //Gram-Schmidt process
    T = normalize(T - dot(T, N) * N);
    vec3 B = cross(N, T);
    vs_out.TBN = mat3(T, B, N);
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}