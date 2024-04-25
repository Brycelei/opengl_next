#version 330 core
out vec4 FragColor;

// IBL
uniform samplerCube irradianceMap;
uniform samplerCube prefilterMap;
uniform sampler2D brdfLUT;

uniform sampler2D texture_albedo;
uniform sampler2D texture_normal;
uniform sampler2D texture_metalness;
uniform sampler2D texture_roughness;
uniform sampler2D texture_emission;
uniform sampler2D texture_ambient_occlusion;
uniform sampler2D texture_specular;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_ambient;


uniform int has_texture_albedo;
uniform int has_texture_normal;
uniform int has_texture_metalness;
uniform int has_texture_roughness;
uniform int has_texture_emission;
uniform int has_texture_ambient_occlusion;
uniform int has_texture_specular;
uniform int has_texture_diffuse;
uniform int has_texture_ambient;


uniform float intensity;
uniform vec3 albedo_model;
uniform float metalness_model;
uniform float roughness_model;
uniform vec3 emission_model;
uniform vec3 emission_strength;

uniform vec3 lightPos;
uniform vec3 viewPos;
uniform vec3 lightColors;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
    mat3 TBN;
} fs_in;


const float PI = 3.14159265359;

// Easy trick to get tangent-normals to world-space to keep PBR code simplified.
// Don't worry if you don't get what's going on; you generally want to do normal 
// mapping the usual way for performance anways; I do plan make a note of this 
// technique somewhere later in the normal mapping tutorial.
vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(texture_normal, fs_in.TexCoords).xyz * 2.0 - 1.0;
    //tangentNormal.b = 1.0f;
//    vec3 Q1  = dFdx(fs_in.FragPos);
//    vec3 Q2  = dFdy(fs_in.FragPos);
//    vec2 st1 = dFdx(fs_in.TexCoords);
//    vec2 st2 = dFdy(fs_in.TexCoords);
//
//    vec3 N   = normalize(fs_in.Normal);
//    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
//    vec3 B  = -normalize(cross(N, T));
//    mat3 TBN = mat3(T, B, N);
    
    return normalize(fs_in.TBN * tangentNormal);
}

float DistributionGGX(vec3 N, vec3 H, float roughness) {
    float a = roughness*roughness;
    float a2 = a*a;
    float NdotH = max(dot(N, H), 0.0);
    float NdotH2 = NdotH*NdotH;

    float nom   = a2;
    float denom = (NdotH2 * (a2 - 1.0) + 1.0);
    denom = PI * denom * denom;

    return nom / denom;
}

float GeometrySchlickGGX(float NdotV, float roughness) {
    float r = (roughness + 1.0);
    float k = (r*r) / 8.0;

    float nom   = NdotV;
    float denom = NdotV * (1.0 - k) + k;

    return nom / denom;
}

float GeometrySmith(vec3 N, vec3 V, vec3 L, float roughness) {
    float NdotV = max(dot(N, V), 0.0);
    float NdotL = max(dot(N, L), 0.0);
    float ggx2 = GeometrySchlickGGX(NdotV, roughness);
    float ggx1 = GeometrySchlickGGX(NdotL, roughness);

    return ggx1 * ggx2;
}

vec3 fresnelSchlick(float cosTheta, vec3 F0) {
    return F0 + (1.0 - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}

vec3 fresnelSchlickRoughness(float cosTheta, vec3 F0, float roughness) {
    return F0 + (max(vec3(1.0 - roughness), F0) - F0) * pow(clamp(1.0 - cosTheta, 0.0, 1.0), 5.0);
}


void main()
{       
    vec3 noramal = fs_in.Normal;
    if(has_texture_normal == 1)
    {   
        noramal = getNormalFromMap();
    }
      // material properties
    vec3 albedo = albedo_model;
    if (has_texture_albedo == 1) {
         albedo = pow(texture(texture_albedo, fs_in.TexCoords).rgb, vec3(2.2));
    }

    float metallic = metalness_model;
    if (has_texture_metalness == 1) {
           metallic = texture(texture_metalness, fs_in.TexCoords).r;
    }

    float roughness = roughness_model;
    if (has_texture_roughness == 1) 
    {
        roughness = texture(texture_roughness, fs_in.TexCoords).r;
    }

    vec3 emission = emission_model;
    if (has_texture_emission == 1) 
    {
        emission = pow(texture(texture_emission, fs_in.TexCoords).rgb, vec3(2.2));
        emission *= emission_strength;
    }

    float ambient_occlusion = 1.0;
    if (has_texture_ambient_occlusion == 1) 
    {
        ambient_occlusion = texture(texture_ambient_occlusion, fs_in.TexCoords).r;
     }
    
    vec3 ambient_color  = vec3(1.0f);
    if(has_texture_ambient == 1)
    {
        ambient_color = texture(texture_ambient, fs_in.TexCoords).rgb;
    }

    vec3 N = noramal;
    // input light data
    vec3 V = normalize(viewPos - fs_in.FragPos).rgb;
    vec3 R = reflect(-V, N);

    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)  
    vec3 F0 = vec3(0.4);
    F0 = mix(F0, albedo, metallic);

    //reflectance equation
    vec3 Lo = vec3(0.0f);

    //points light
    vec3 L = normalize(lightPos - fs_in.FragPos);
    vec3 H = normalize(V + L);

    float dis = length(lightPos - fs_in.FragPos);
    float attenuation = 1.0 / (dis * dis);
    vec3 radiance = lightColors * attenuation * intensity;

    //cook-torrence brdf 
    float NDF = DistributionGGX(N, H, roughness);
    float G = GeometrySmith(N, V, L, roughness);
    vec3 F = fresnelSchlick(max(dot(H, V), 0.0), F0);
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001; // + 0.0001 to prevent divide by zero

    vec3 specular = numerator / denominator;
    if(has_texture_specular == 1)
    {
        specular *= texture(texture_specular, fs_in.TexCoords).rgb;
    }

    vec3 kS = F;
    vec3 kD = vec3(1.0) - kS;
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);
    // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL;
   
    F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    kS = F;
    kD = 1.0 - kS;
    kD *= 1.0 - metallic;	 

    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse = irradiance * albedo;

    if(has_texture_diffuse == 1)
    {
        diffuse *= texture(texture_diffuse, fs_in.TexCoords).rgb;
    }

     // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    specular = prefilteredColor * (F * brdf.x + brdf.y);

    vec3 ambient = (kD * diffuse + specular ) * ambient_occlusion * ambient_color;
    
    vec3 color1 = ambient + Lo + emission;
    // HDR tonemapping
    color1 = color1 / (color1 + vec3(1.0));
    color1 = pow(color1, vec3(1.0/2.2)); 
    FragColor = vec4(color1 , 1.0);
    //FragColor = vec4(N , 1.0);

} 