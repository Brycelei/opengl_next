#version 330 core
out vec4 FragColor;

in vec2 TexCoords;
const float PI = 3.14159265359;
uniform sampler2D texture_diffuse;
uniform sampler2D texture_albedo;
uniform sampler2D texture_normal;
uniform sampler2D texture_metalness;
uniform sampler2D texture_roughness;
uniform sampler2D texture_emission;
uniform sampler2D texture_ambient_occlusion;

uniform bool has_texture_diffuse;
uniform bool has_texture_albedo;
uniform bool has_texture_normal;
uniform bool has_texture_metalness;
uniform bool has_texture_roughness;
uniform bool has_texture_emission;
uniform bool has_texture_ambient_occlusion;

uniform int material_format;
uniform vec3 viewPos;
uniform vec3 albedo_model;
uniform float intensity;

//light 
uniform vec3 lightPos;
uniform vec3 lightColors;

in VS_OUT {
    vec3 FragPos;
    vec2 TexCoords;
    vec3 Normal;
} fs_in;


vec3 getNormalFromMap() {
    vec3 tangentNormal = texture(texture_normal, fs_in.TexCoords).xyz * 2.0 - 1.0;

    vec3 Q1  = dFdx(fs_in.FragPos);
    vec3 Q2  = dFdy(fs_in.FragPos);
    vec2 st1 = dFdx(fs_in.TexCoords);
    vec2 st2 = dFdy(fs_in.TexCoords);

    vec3 N   = normalize(fs_in.Normal);
    vec3 T  = normalize(Q1*st2.t - Q2*st1.t);
    vec3 B  = -normalize(cross(N, T));
    mat3 TBN = mat3(T, B, N);

    return normalize(TBN * tangentNormal);
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
	
	vec3 normal = fs_in.Normal;
    vec3 albedo = albedo_model;
    if(has_texture_albedo)
    {
        albedo = pow(texture(texture_albedo, fs_in.FragPos), vec3(2.2));
    }
    if(has_texture_normal)
    {
        normal = getNormalFromMap();
    }
    vec3 N = normal;
    float metallic = 0.1f;
    vec3 viewDir = normalize(viewPos - fs_in.FragPos);
    vec3 V = viewDir'
    vec3 lightDir = normalize(lightPos - fs_in.FragPos);
    vec3 R  =  reflect(-lightDir, normal);

    vec3 F0 = vec3(0.04);
    F0 = mix(F0, albedo, metallic);
    vec3 lo = vec3(0.0);

    vec3 L = normalize(lightPos - fs_in.FragPos);
    vec3 H = normalize(V + L);

    float distance = length(lightPos - fs_in.FragPos);
    float attenuation = 1 / (distance * distance);
    vec3 radiance = lightColors * attenuation * intensity;

    //cook -torrence brdf
    float roughness = 0.3f;
    float NDF = DistributionGGX(N, H, roughness);   
    float G   = GeometrySmith(N, V, L, roughness);    
    vec3 F    = fresnelSchlick(max(dot(H, V), 0.0), F0);  
    vec3 numerator = NDF * G * F;
    float denominator = 4.0 * max(dot(N, V), 0.0) * max(dot(N, L), 0.0) + 0.0001;
    vec3 specular = numerator / denominator;
    specular *= texture(texture_specular1, fs_in.TexCoords);
    vec3 kS = F;
        // for energy conservation, the diffuse and specular light can't
        // be above 1.0 (unless the surface emits light); to preserve this
        // relationship the diffuse component (kD) should equal 1.0 - kS.
    vec3 kD = vec3(1.0) - kS;
        // multiply kD by the inverse metalness such that only non-metals 
        // have diffuse lighting, or a linear blend if partly metal (pure metals
        // have no diffuse light).
    kD *= 1.0 - metallic;

    float NdotL = max(dot(N, L), 0.0);     

        // add to outgoing radiance Lo
    Lo += (kD * albedo / PI + specular) * radiance * NdotL; // note that we already multiplied the BRDF by the Fresnel (kS) so we won't multiply by kS again

    F = fresnelSchlickRoughness(max(dot(N, V), 0.0), F0, roughness);
    
    kS = F;
    kD = 1.0 - kS;
    kD *= 1.0 - metallic;	  
    
    vec3 irradiance = texture(irradianceMap, N).rgb;
    vec3 diffuse      = irradiance * albedo;
    
    // sample both the pre-filter map and the BRDF lut and combine them together as per the Split-Sum approximation to get the IBL specular part.
    const float MAX_REFLECTION_LOD = 4.0;
    vec3 prefilteredColor = textureLod(prefilterMap, R,  roughness * MAX_REFLECTION_LOD).rgb;    
    vec2 brdf  = texture(brdfLUT, vec2(max(dot(N, V), 0.0), roughness)).rg;
    specular = prefilteredColor * (F * brdf.x + brdf.y);

    float ao = 1.0f;
    vec3 ambient = (kD * diffuse + specular) * ao;
    
    vec3 color = ambient + Lo;
   
    // HDR tonemapping
    color = color / (color + vec3(1.0));

     color *= texture(texture_diffuse1, fs_in.TexCoords).rgb;
    // gamma correct
    color = pow(color, vec3(1.0/2.2)); 

    FragColor = vec4(color , 1.0);
    FragColor = vec4(normal , 1.0);

}