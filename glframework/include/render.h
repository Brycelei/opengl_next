#pragma once
#include <mutex>
#include <vector>
#include <unordered_map>
#include <map>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>
#include <chrono>
#include <base_model.h>
#include"gameobject.h"

class Camera;
class Shader;
class Model;
class Cubemap;
class Texture;
class Material;
class Light;
class Quad;
class PointLight;

enum CubemapTextureType;

class Render
{
public:
	static Render* get_instance();
	//void initialize();
    ~Render();
	Render(Render& ren) = delete;
	const Render& operator=(const Render&) = delete;

    Shader* pbr_shader;
    Shader* skybox_shader;
    Shader* equirectangularToCubemapShader;
    Shader* irradianceShader;
    Shader* prefilterShader;
    Shader* brdfShader;
    Shader* hdr_to_ldr_shader;

    Cubemap* cubemap;
    CubemapTextureType cubemap_texture_type;
    
    std::chrono::time_point<std::chrono::system_clock> time_before_rendering;

    unsigned int framebuffer, textureHDRColorbuffer, texture_id_colors, texture_selected_color_buffer, textureLDRColorbuffer;


    glm::mat4 view, projection;
    glm::mat4 view_projection;
    //glm::mat4 view_projection_inv;
    glm::mat4 view_skybox, view_projection_skybox;
    Camera* camera_viewport;
    Quad* screen_quad;
    float near_camera_viewport, far_camera_viewport;
    float cubemap_texture_mipmap_level;
    //viewport_size
    int weight;
    int height;

    unsigned int bloom_fbo;
    float bloom_filter_radius;
    float bloom_strength;
    bool bloom_activated;
    Shader* bloom_downsample_shader;
    Shader* bloom_upsample_shader;


    // PBR parameters
    int ENVIRONMENT_MAP_WIDTH;
    int ENVIRONMENT_MAP_HEIGHT;
    int IRRADIANCE_MAP_WIDTH;
    int IRRADIANCE_MAP_HEIGHT;
    int PREFILTER_MAP_WIDTH;
    int PREFILTER_MAP_HEIGHT;
    int BRDF_LUT_MAP_WIDTH;
    int BRDF_LUT_MAP_HEIGHT;

    unsigned int captureFBO;
    glm::mat4 captureProjection;
    std::vector<glm::mat4> captureViews;

    unsigned int brdfLUTTexture;

    void initialize_shaders();
    void initialize_object();
    void set_opengl_state();
    void set_viewportdata();
    void load_cubemap(const std::string& cubemap_name, const std::vector<std::string>& cube_map_paths, bool is_hdri);
    void set_time_before_rendering_loop();
    void render_viewport();
    void render_view_initialize(int w, int h);
    void render_destroy();

    void add_model_to_loaded_data(Model* model);
    std::map<std::string, BaseModel*> loaded_models;
    std::map<std::string, Texture*> loaded_textures;
    std::map<std::string, Material*> loaded_materials;

    std::map<std::string, PointLight*> point_lights;
    std::map<std::string, GameObject*> game_objects;

private:
    Render();
    static Render* instance;
    static std::mutex render_mutex;
   
};

