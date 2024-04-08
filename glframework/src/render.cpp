#include "render.h"
#include "cubemap.h"
#include"shader.h"
#include"model.h"
#include <pbr.hpp>
#include <gameobject.h>
#include"camera.h"
#include<log.h>
Render* Render::instance = nullptr;
std::mutex Render::render_mutex;

Render::Render()
{
	pbr_shader = nullptr;
	skybox_shader = nullptr;
	equirectangularToCubemapShader = nullptr;
	irradianceShader = nullptr;
	prefilterShader = nullptr;
	brdfShader = nullptr;
	hdr_to_ldr_shader = nullptr;
	cubemap = nullptr;

	// PBR parameters
	ENVIRONMENT_MAP_WIDTH = 2048;
	ENVIRONMENT_MAP_HEIGHT = 2048;
	IRRADIANCE_MAP_WIDTH = 128;
	IRRADIANCE_MAP_HEIGHT = 128;
	PREFILTER_MAP_WIDTH = 512;
	PREFILTER_MAP_HEIGHT = 512;
	BRDF_LUT_MAP_WIDTH = 2048;
	BRDF_LUT_MAP_HEIGHT = 2048;

    camera_viewport = new Camera((glm::vec3(0.0f, 0.0f, 800.0f)));
    near_camera_viewport = 0.1f;
    far_camera_viewport = 1000.0f;

    cubemap_texture_mipmap_level = 0.0f;

	// set up projection and view matrices for capturing data onto the 6 cubemap face directions
	captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	captureViews = {
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

    loaded_materials["Default"] = nullptr;
	cubemap_texture_type = EnvironmentMap;
}
Render::~Render()
{

}

Render* Render::get_instance()
{
	if (instance == nullptr)
	{
		std::lock_guard<std::mutex> lock(render_mutex);
		if (instance == nullptr)
		{
			instance = new Render();
		}
	}
	return instance;
}

void Render::initialize_shaders()
{
	//pbr_shader = new Shader("../shaders/hoho/vertices_3d_model.vert", "../shaders/hoho/PBR.frag");
    pbr_shader = new Shader("../shaders/pbr/hoho/1.model_loading.vert", "../shaders/pbr/hoho/1.model_loading1.frag");
	equirectangularToCubemapShader = new Shader("../shaders/hoho/cubemap.vert", "../shaders/hoho/equirectangular_to_cubemap.frag");
	irradianceShader = new Shader("../shaders/hoho/cubemap.vert", "../shaders/hoho/irradiance_convolution.frag");
	prefilterShader = new Shader("../shaders/hoho/cubemap.vert", "../shaders/hoho/prefilter.frag");
	brdfShader = new Shader("../shaders/hoho/brdf.vert", "../shaders/hoho/brdf.frag");
	skybox_shader = new Shader("../shaders/hoho/skybox.vert", "../shaders/hoho/skybox.frag");
	hdr_to_ldr_shader = new Shader("../shaders/hoho/vertices_quad.vert", "../shaders/hoho/hdr_to_ldr.frag");
    
    bloom_downsample_shader = new Shader("../shaders/hoho/vertices_quad.vert", "../shaders/hoho/bloom_downsample.frag");
    bloom_upsample_shader = new Shader("../shaders/hoho/vertices_quad.vert", "../shaders/hoho/bloom_upsample.frag");
}

void Render::set_opengl_state()
{
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);

	glEnable(GL_STENCIL_TEST);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	// PBR: for sampling in lower mip levels in the pre-filter map
	glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
}

void Render::load_cubemap(const std::string& cubemap_name, const std::vector<std::string>& cubemap_paths, bool is_hdri) {
	auto begin_timer = std::chrono::high_resolution_clock::now();
	unsigned int cubemap_texture;
	if (!is_hdri) {
		cubemap->add_cubemap_texture(cubemap_name, cubemap_paths, is_hdri);
		cubemap_texture = cubemap->umap_name_to_cubemap_data[cubemap_name].environment_texture;
	}
	else {
		cubemap_texture = create_environment_map_from_equirectangular_map(cubemap_paths[0], captureFBO,
			ENVIRONMENT_MAP_WIDTH, ENVIRONMENT_MAP_HEIGHT, captureProjection, captureViews,
			equirectangularToCubemapShader);
		cubemap->add_cubemap_texture(cubemap_name, cubemap_texture, is_hdri);
	}
	cubemap->umap_name_to_cubemap_data[cubemap_name].irradiance_texture = create_irradiance_map_from_environment_map(captureFBO, cubemap_texture, ENVIRONMENT_MAP_WIDTH, IRRADIANCE_MAP_WIDTH, IRRADIANCE_MAP_HEIGHT, captureProjection, captureViews, irradianceShader);
	cubemap->umap_name_to_cubemap_data[cubemap_name].prefilter_texture = create_prefilter_map_from_environment_map(captureFBO, cubemap_texture, ENVIRONMENT_MAP_WIDTH, PREFILTER_MAP_WIDTH, PREFILTER_MAP_HEIGHT, captureProjection, captureViews, prefilterShader);
	auto end_timer = std::chrono::high_resolution_clock::now();
	double elapsed_time_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end_timer - begin_timer).count();
    NESI_INFO("CREATING PBR DATA IN {0} seconds", elapsed_time_seconds);
}

void Render::set_viewportdata()
{
	captureFBO = create_framebuffer_pbr();

	// BRDF LUT texture
	brdfLUTTexture = create_brdf_lut_texture(captureFBO, BRDF_LUT_MAP_WIDTH, BRDF_LUT_MAP_HEIGHT, brdfShader);

	cubemap = new Cubemap();

	std::vector<std::string> hdrivec; 
	hdrivec.emplace_back("../asset/hdri/GCanyon_C_YumaPoint_3k.hdr");
    
	load_cubemap("hdri", hdrivec, true);
    
 


	screen_quad = new Quad();

	Model* E371 = new Model("E371", "../asset/obj/E371/E371.obj", false, false);
	add_model_to_loaded_data(E371);
}

void Render::set_time_before_rendering_loop() {
	this->time_before_rendering = std::chrono::system_clock::now();
}

void Render::add_model_to_loaded_data(Model* model) {
	loaded_models[model->name] = model;
	for (auto it = model->loaded_materials.begin(); it != model->loaded_materials.end(); it++) {
		Material* material = it->second;
		loaded_materials[material->name] = material;
	}
	for (auto it = model->loaded_textures.begin(); it != model->loaded_textures.end(); it++) {
		Texture* texture = it->second;
		loaded_textures[texture->get_name()] = texture;
	}
}

void Render::initialize_object()
{
	GameObject* E371 = new GameObject("E371", "E371");
	E371->position = glm::vec3(0.0f, 0.0f, 0.0f);
	E371->scale = glm::vec3(0.08f);
	//E371->animation_id = 0;
	E371->set_model_matrices_standard();
	game_objects[E371->name] = E371;
	//id_color_to_game_object[E371->id_color] = E371;

    Skybox* skybox = new Skybox("skybox");
    skybox->type = TypeSkybox;
    skybox->cubemap_name = "hdri";
    skybox->set_model_matrices_standard();
    game_objects[skybox->name] = skybox;
    //id_color_to_game_object[skybox->id_color] = skybox;

}

void Render::render_viewport()
{
    int texture_viewport_width = weight;
    int texture_viewport_height = height;
    glViewport(0, 0, texture_viewport_width, texture_viewport_height);

    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

    // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    unsigned int attachments0[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
    glDrawBuffers(6, attachments0);

    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

    // view/projection transformations
    projection = glm::perspective(glm::radians(camera_viewport->Zoom), (float)texture_viewport_width / (float)texture_viewport_height, near_camera_viewport, far_camera_viewport);
    view = camera_viewport->GetViewMatrix();
    view_projection = projection * view;

    Shader* lighting_shader = pbr_shader;
    //Shader* lighting_shader = phong_shader;

    // bind pre-computed IBL data
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->umap_name_to_cubemap_data[((Skybox*)game_objects["skybox"])->cubemap_name].irradiance_texture);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->umap_name_to_cubemap_data[((Skybox*)game_objects["skybox"])->cubemap_name].prefilter_texture);
    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

    lighting_shader->use();

    lighting_shader->setMat4("view", view);
    lighting_shader->setMat4("projection", projection);
    lighting_shader->setVec3("viewPos", camera_viewport->Position);
    lighting_shader->setVec3("lightPos", glm::vec3(0.0f, 0.0f, 10.0f));



    lighting_shader->setInt("irradianceMap", 0);
    lighting_shader->setInt("prefilterMap", 1);
    lighting_shader->setInt("brdfLUT", 2);

    /*
    lighting_shader->setMat4("view_projection", view_projection);

    lighting_shader->setVec3("viewPos", camera_viewport->Position);*/

    /*lighting_shader->setInt("num_point_lights", point_lights.size());
    lighting_shader->setInt("num_directional_lights", directional_lights.size());
    lighting_shader->setInt("num_spot_lights", spot_lights.size());

    lighting_shader->setFloat("emission_strength", emission_strength);

    int idx_point_light = 0;
    for (auto it = point_lights.begin(); it != point_lights.end(); it++, idx_point_light++) {
        lighting_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].position", it->second->position);
        lighting_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].ambient", it->second->ambient);
        lighting_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].light_color", it->second->albedo);
        lighting_shader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].intensity", it->second->intensity);
        lighting_shader->setVec3("pointLights[" + std::to_string(idx_point_light) + "].specular", it->second->specular);
        lighting_shader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].constant", it->second->constant);
        lighting_shader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].linear", it->second->linear);
        lighting_shader->setFloat("pointLights[" + std::to_string(idx_point_light) + "].quadratic", it->second->quadratic);
    }

    int idx_directional_light = 0;
    for (auto it = directional_lights.begin(); it != directional_lights.end(); it++, idx_directional_light++) {
        lighting_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].direction", it->second->direction);
        lighting_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].ambient", it->second->ambient);
        lighting_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].light_color", it->second->albedo);
        lighting_shader->setFloat("directionalLights[" + std::to_string(idx_directional_light) + "].intensity", it->second->intensity);
        lighting_shader->setVec3("directionalLights[" + std::to_string(idx_directional_light) + "].specular", it->second->specular);
    }

    int idx_spot_light = 0;
    for (auto it = spot_lights.begin(); it != spot_lights.end(); it++, idx_spot_light++) {
        lighting_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].position", it->second->position);
        lighting_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].direction", it->second->direction);
        lighting_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].ambient", it->second->ambient);
        lighting_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].light_color", it->second->albedo);
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].intensity", it->second->intensity);
        lighting_shader->setVec3("spotLights[" + std::to_string(idx_spot_light) + "].specular", it->second->specular);
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].constant", it->second->constant);
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].linear", it->second->linear);
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].quadratic", it->second->quadratic);
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].inner_cut_off", it->second->get_inner_cut_off());
        lighting_shader->setFloat("spotLights[" + std::to_string(idx_spot_light) + "].outer_cut_off", it->second->get_outer_cut_off());
    }*/

    // Render the game objects with the selected lighting shading and also render the unique Color IDs of each game object
    // (later for the selection technique: Color Picking)
    unsigned int attachments1[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT5 };
    glDrawBuffers(4, attachments1);
    lighting_shader->setInt("is_transform3d", 0);
    for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
        GameObject* game_object = it->second;
        if (game_object->type != TypeSkybox) {
            if (game_object->type == TypeBaseModel) {
                lighting_shader->setFloat("intensity", 1.0);
                game_object->draw(lighting_shader, false);
            }
            //else { // It is a light
            //    lighting_shader->setFloat("intensity", ((Light*)game_object)->intensity);
            //}
            //game_object->draw(lighting_shader, false);
        }
    }

    // Draw skybox
    unsigned int attachments_skybox[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT5 };
    glDrawBuffers(2, attachments_skybox);
    view_skybox = glm::mat4(glm::mat3(view)); // remove translation from the view matrix so it doesn't affect the skybox
    view_projection_skybox = projection * view_skybox;
    skybox_shader->use();
    skybox_shader->setInt("is_hdri", cubemap->umap_name_to_cubemap_data[((Skybox*)game_objects["skybox"])->cubemap_name].is_hdri);
    //skybox_shader->setFloat("exposure", exposure);
    skybox_shader->setMat4("view_projection", view_projection_skybox);
    skybox_shader->setFloat("mipmap_level", cubemap_texture_mipmap_level);
    cubemap->draw(skybox_shader, ((Skybox*)game_objects["skybox"])->cubemap_name, cubemap_texture_type);

    // Apply bloom to the rendered HDR bright color texture
    //if (bloom_activated) {
    //    unsigned int attachments_bloom[1] = { GL_COLOR_ATTACHMENT0 };
    //    glDrawBuffers(1, attachments_bloom);
    //    glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
    //    bloom_downsampling(bloom_downsample_shader, textureHDRBrightColorbuffer, bloom_textures, texture_viewport_width, texture_viewport_height);
    //    bloom_upsampling(bloom_upsample_shader, bloom_textures, bloom_filter_radius);
    //    glViewport(0, 0, texture_viewport_width, texture_viewport_height);
    //    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    //}

    // Render only the last selected object (if it exists) to later outline the shape of this object
    //unsigned int attachments3[1] = { GL_COLOR_ATTACHMENT3 };
    //glDrawBuffers(1, attachments3);
    //selection_shader->use();
    //selection_shader->setMat4("view_projection", view_projection);
    //if (last_selected_object != nullptr && last_selected_object->type != TypeSkybox) {
    //    last_selected_object->draw(selection_shader, true);
    //}

    //// Convert HDR color texture to LDR
    //unsigned int attachments4[1] = { GL_COLOR_ATTACHMENT4 }; // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
    //glDrawBuffers(1, attachments4);
    //hdr_to_ldr_shader->use();
    //hdr_to_ldr_shader->setFloat("exposure", exposure);
    //hdr_to_ldr_shader->setInt("hdr_texture", 0);
    //hdr_to_ldr_shader->setInt("bloom_activated", bloom_activated);
    //hdr_to_ldr_shader->setFloat("bloomStrength", bloom_strength);
    //glActiveTexture(GL_TEXTURE0);
    //glBindTexture(GL_TEXTURE_2D, textureHDRColorbuffer);
    //hdr_to_ldr_shader->setInt("bloom_texture", 1);
    //glActiveTexture(GL_TEXTURE1);
    //glBindTexture(GL_TEXTURE_2D, bloom_textures[0].texture_id);
    //screen_quad->draw(hdr_to_ldr_shader, false);


    // Clear the depth buffer so the Transform3D is drawn over everything
    glClear(GL_DEPTH_BUFFER_BIT);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void Render::render_view_initialize(int w, int h)
{
    weight = w;
    height = h;
}


void Render::render_destroy()
{
    delete pbr_shader;
    delete skybox_shader;
    delete equirectangularToCubemapShader;
    delete irradianceShader;
    delete prefilterShader;
    delete brdfShader;
    delete hdr_to_ldr_shader;
    delete cubemap;


    pbr_shader = nullptr;
    skybox_shader = nullptr;
    equirectangularToCubemapShader = nullptr;
    irradianceShader = nullptr;
    prefilterShader = nullptr;
    brdfShader = nullptr;
    hdr_to_ldr_shader = nullptr;
    cubemap = nullptr;

    for (auto it = loaded_models.begin(); it != loaded_models.end(); it++) {
        delete it->second;
        it->second = nullptr;
    }
    for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
        delete it->second;
        it->second = nullptr;
    }

}