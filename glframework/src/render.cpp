#include "render.h"
#include "render.h"
#include "cubemap.h"
#include"shader.h"
#include"model.h"
#include <pbr.hpp>
#include <gameobject.h>
#include"camera.h"
#include<log.h>
#include <filesystem>
namespace NESI_NEXT {
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
        selection_shader = nullptr;
        outline_shader = nullptr;
        cubemap = nullptr;
        light = nullptr;

        // PBR parameters
        ENVIRONMENT_MAP_WIDTH = 512;
        ENVIRONMENT_MAP_HEIGHT = 512;
        IRRADIANCE_MAP_WIDTH = 32;
        IRRADIANCE_MAP_HEIGHT = 32;
        PREFILTER_MAP_WIDTH = 256;
        PREFILTER_MAP_HEIGHT = 256;
        BRDF_LUT_MAP_WIDTH = 512;
        BRDF_LUT_MAP_HEIGHT = 512;

        camera_viewport = new Camera((glm::vec3(0.0f, 0.0f, 800.0f)));
        outline_color = glm::vec3(255.0f / 255.0f, 195.0f / 255.0f, 7.0f / 255.0f);
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
        last_selected_object = nullptr;
        cubemap_texture_type = EnvironmentMap;

        emission_strength = 8.0f;
        bloom_strength = 0.04f;
        exposure = 1.0f;
        bloom_activated = false;
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
        //outline shader
        selection_shader = new Shader("../shaders/pbr/hoho/1.model_loading.vert", "../shaders/hoho/paint_selected.frag");
        outline_shader = new Shader("../shaders/hoho/vertices_quad.vert", "../shaders/hoho/edge_outlining.frag");

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
        cubemap->umap_name_to_cubemap_data[cubemap_name].irradiance_texture = create_irradiance_map_from_environment_map(captureFBO, captureRBO, cubemap_texture, ENVIRONMENT_MAP_WIDTH, IRRADIANCE_MAP_WIDTH, IRRADIANCE_MAP_HEIGHT, captureProjection, captureViews, irradianceShader);
        cubemap->umap_name_to_cubemap_data[cubemap_name].prefilter_texture = create_prefilter_map_from_environment_map(captureFBO, captureRBO, cubemap_texture, ENVIRONMENT_MAP_WIDTH, PREFILTER_MAP_WIDTH, PREFILTER_MAP_HEIGHT, captureProjection, captureViews, prefilterShader);
        auto end_timer = std::chrono::high_resolution_clock::now();
        double elapsed_time_seconds = std::chrono::duration_cast<std::chrono::duration<double>>(end_timer - begin_timer).count();
        NESI_INFO("CREATING cube DATA IN {0} seconds", elapsed_time_seconds);
    }

    void Render::set_viewportdata()
    {
        captureFBO = create_framebuffer_pbr_FBO();
        captureRBO = create_framebuffer_pbr_RBO(ENVIRONMENT_MAP_WIDTH, ENVIRONMENT_MAP_WIDTH);

        
        // BRDF LUT texture
        brdfLUTTexture = create_brdf_lut_texture(captureFBO, captureRBO, BRDF_LUT_MAP_WIDTH, BRDF_LUT_MAP_HEIGHT, brdfShader);

        cubemap = new Cubemap();

        for (const auto& entry : std::filesystem::directory_iterator("../asset/hdri"))
        {
            if (entry.is_regular_file()) {
                std::string path = entry.path().string();
                std::replace(path.begin(), path.end(), '\\', '/');
                load_cubemap(entry.path().stem().string(), {path}, true);
            }
        }
       
        screen_quad = new Quad();

        //for (const auto& objentry : std::filesystem::directory_iterator("../asset/obj"))
        //{
        //    for (const auto& name : std::filesystem::directory_iterator(objentry.path().string()))
        //    {
        //        std::string file_extension = std::filesystem::path(name).extension().string();
        //        if (file_extension == "") break;
        //        file_extension = file_extension.substr(1);
        //        if (file_extension == "obj")
        //        {
        //            std::string path = name.path().string();
        //            std::replace(path.begin(), path.end(), '\\', '/');
        //            Model* E371 = new Model(name.path().stem().string(), path, false, false);
        //            add_model_to_loaded_data(E371);
        //        }
        //    }
        //}
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
        Skybox* skybox = new Skybox("skybox");
        skybox->type = TypeSkybox;
        skybox->cubemap_name = "GCanyon_C_YumaPoint_3k";
        skybox->set_model_matrices_standard();
        game_objects[skybox->name] = skybox;

        SetLight(LightType::Point);
        //game_objects["light"] = light;

    }

    void Render::render_viewport()
    {
        int texture_viewport_width = this->width;
        int texture_viewport_height = this->height;
        glViewport(0, 0, texture_viewport_width, texture_viewport_height);

        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        unsigned int attachments0[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
        glDrawBuffers(6, attachments0);

        //glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

        // view/projection transformations
        projection = glm::perspective(glm::radians(camera_viewport->Zoom), (float)texture_viewport_width / (float)texture_viewport_height, near_camera_viewport, far_camera_viewport);
        view = camera_viewport->GetViewMatrix();
        view_projection = projection * view;

        Shader* lighting_shader = pbr_shader;

        // bind pre-computed IBL data
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->umap_name_to_cubemap_data[((Skybox*)game_objects["skybox"])->cubemap_name].irradiance_texture);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubemap->umap_name_to_cubemap_data[((Skybox*)game_objects["skybox"])->cubemap_name].prefilter_texture);
        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, brdfLUTTexture);

        lighting_shader->use();
        //model view projection
        lighting_shader->setMat4("view", view);
        lighting_shader->setMat4("projection", projection);
        lighting_shader->setVec3("viewPos", camera_viewport->Position);
        //light
        lighting_shader->setVec3("lightPos", glm::vec3(0.0f, 0.0f, 10.0f));
        lighting_shader->setVec3("lightColors", light->GetColor());

        lighting_shader->setInt("irradianceMap", 0);
        lighting_shader->setInt("prefilterMap", 1);
        lighting_shader->setInt("brdfLUT", 2);


        // Render the game objects with the selected lighting shading and also render the unique Color IDs of each game object
        // (later for the selection technique: Color Picking)
        unsigned int attachments1[4] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT5 };
        glDrawBuffers(4, attachments1);
        //lighting_shader->setInt("is_transform3d", 0);
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

       //  Draw skybox
        unsigned int attachments_skybox[2] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT5 };
        glDrawBuffers(2, attachments_skybox);
        view_skybox = glm::mat4(glm::mat3(view)); // remove translation from the view matrix so it doesn't affect the skybox
        view_projection_skybox = projection * view_skybox;
        skybox_shader->use();
        skybox_shader->setInt("is_hdri", cubemap->umap_name_to_cubemap_data[((Skybox*)game_objects["skybox"])->cubemap_name].is_hdri);
        skybox_shader->setFloat("exposure", exposure);
        skybox_shader->setMat4("view_projection", view_projection_skybox);
        skybox_shader->setFloat("mipmap_level", cubemap_texture_mipmap_level);
        cubemap->draw(skybox_shader, ((Skybox*)game_objects["skybox"])->cubemap_name, cubemap_texture_type);

        //Apply bloom to the rendered HDR bright color texture
      /*  if (bloom_activated) {
            unsigned int attachments_bloom[1] = { GL_COLOR_ATTACHMENT0 };
            glDrawBuffers(1, attachments_bloom);
            glBindFramebuffer(GL_FRAMEBUFFER, bloom_fbo);
            bloom_downsampling(bloom_downsample_shader, textureHDRBrightColorbuffer, bloom_textures, texture_viewport_width, texture_viewport_height);
            bloom_upsampling(bloom_upsample_shader, bloom_textures, bloom_filter_radius);
            glViewport(0, 0, texture_viewport_width, texture_viewport_height);
            glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
        }*/

        // Render only the last selected object (if it exists) to later outline the shape of this object
        unsigned int attachments3[1] = { GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(1, attachments3);
        selection_shader->use();
        selection_shader->setMat4("view", view);
        selection_shader->setMat4("projection", projection);
        //selection_shader->setMat4("view_projection", view_projection);
        if (last_selected_object != nullptr && last_selected_object->type != TypeSkybox) {
            last_selected_object->draw(selection_shader, true);
        }

        // Draw border outlining the selected object (if it exists one)
      /*  outline_shader->use();
        outline_shader->setVec2("pixel_size", glm::vec2(1.0f / width, 1.0f / height));
        outline_shader->setVec3("outline_color", outline_color);
        outline_shader->setInt("screen_texture", 0);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture_selected_color_buffer);
        screen_quad->draw(outline_shader, true);*/

        // Convert HDR color texture to LDR
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
        // glClear(GL_DEPTH_BUFFER_BIT);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Render::render_view_initialize(int w, int h)
    {
        width = w;
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
        delete bloom_downsample_shader;
        delete bloom_upsample_shader;
        //delete light;

        pbr_shader = nullptr;
        skybox_shader = nullptr;
        equirectangularToCubemapShader = nullptr;
        irradianceShader = nullptr;
        prefilterShader = nullptr;
        brdfShader = nullptr;
        hdr_to_ldr_shader = nullptr;
        cubemap = nullptr;
        bloom_downsample_shader = nullptr;
        bloom_upsample_shader = nullptr;
        //light = nullptr;

        for (auto it = loaded_models.begin(); it != loaded_models.end(); it++) {
            delete it->second;
            it->second = nullptr;
        }
        for (auto it = game_objects.begin(); it != game_objects.end(); it++) {
            delete it->second;
            it->second = nullptr;
        }

    }

    void Render::run()
    {
        render_viewport();
    }

    void Render::SetLight(LightType type)
    {
        light = new NESI_NEXT::Light(glm::vec3(1.0, 1.0, 1.0), 1.0);
        switch (type)
        {
        case NESI_NEXT::LightType::Directional:
            break;
        case NESI_NEXT::LightType::Point:
        {
            PointLight* plight = static_cast<PointLight*>(light);
            if (plight == nullptr)
            {
                NESI_ERROR("dynamic_cast error");
            }
            plight->SetPosition(glm::vec3(0.0f, 0.0f, 10.0f));

        }
            break;
        case NESI_NEXT::LightType::Spot:
            break;
        case NESI_NEXT::LightType::Area:
            break;
        default:
            break;
        }
    }

    void Render::setup_framebuffer_and_textures()
    {
        int texture_viewport_width = this->width;
        int texture_viewport_height = this->height;

        int color_texture_width = texture_viewport_width;
        int color_texture_height = texture_viewport_height;
        int depth_stencil_rbo_width = texture_viewport_width;
        int depth_stencil_rbo_height = texture_viewport_height;

        // Create and bind Framebuffer
        glGenFramebuffers(1, &framebuffer);
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);

        // tell OpenGL which color attachments we'll use (of this framebuffer) for rendering 
        unsigned int attachments0[6] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2, GL_COLOR_ATTACHMENT3, GL_COLOR_ATTACHMENT4, GL_COLOR_ATTACHMENT5 };
        glDrawBuffers(6, attachments0);

        // Create and set LDR Color Texture for main rendering
        glGenTextures(1, &textureLDRColorbuffer);
        glBindTexture(GL_TEXTURE_2D, textureLDRColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, color_texture_width, color_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Attach the Texture to the currently bound Framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureLDRColorbuffer, 0);

        // Create and set Color Texture for rendering the id colors
        glGenTextures(1, &texture_id_colors);
        glBindTexture(GL_TEXTURE_2D, texture_id_colors);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, color_texture_width, color_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Attach the Texture to the currently bound Framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, texture_id_colors, 0);

        // Create and set Color Texture for rendering of selected objects
        glGenTextures(1, &texture_selected_color_buffer);
        glBindTexture(GL_TEXTURE_2D, texture_selected_color_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, color_texture_width, color_texture_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glBindTexture(GL_TEXTURE_2D, 0);

        // Attach the Texture to the currently bound Framebuffer object
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, texture_selected_color_buffer, 0);

        // Create and set Depth and Stencil Renderbuffer
        glGenRenderbuffers(1, &rboDepthStencil);
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, depth_stencil_rbo_width, depth_stencil_rbo_height);
        glBindRenderbuffer(GL_RENDERBUFFER, 0);

        // Attach the Renderbuffer to the currently bound Gramebuffer object
        glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rboDepthStencil);

        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
           std::cout << "ERROR::FRAMEBUFFER:: Frame-buffer is not complete!" << std::endl;

        // UPDATE BLOOM TEXTURES SIZES
        //bloom_fbo = initialize_bloom(6, bloom_textures, texture_viewport_width, texture_viewport_height);

        // Bind to the default Frame-buffer
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    void Render::resize_textures()
    {
        int texture_viewport_width = this->width;
        int texture_viewport_height = this->height;

        // Resize main rendering textures
        glBindTexture(GL_TEXTURE_2D, textureHDRColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, texture_viewport_width, texture_viewport_height, 0, GL_RGBA, GL_FLOAT, nullptr);

        glBindTexture(GL_TEXTURE_2D, texture_id_colors);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_viewport_width, texture_viewport_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, texture_selected_color_buffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_viewport_width, texture_viewport_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

        glBindTexture(GL_TEXTURE_2D, textureLDRColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texture_viewport_width, texture_viewport_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);

       /* glBindTexture(GL_TEXTURE_2D, textureHDRBrightColorbuffer);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, texture_viewport_width, texture_viewport_height, 0, GL_RGBA, GL_FLOAT, nullptr);*/

        // Resized depth and stencil buffer
        glBindRenderbuffer(GL_RENDERBUFFER, rboDepthStencil);
        glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, texture_viewport_width, texture_viewport_height);

        //// Resize bloom textures
        //glm::vec2 texture_size(texture_viewport_width, texture_viewport_height);
        //for (int i = 0; i < bloom_textures.size(); i++) {
        //    texture_size /= 2.0f;
        //    bloom_textures[i].size = texture_size;

        //    glBindTexture(GL_TEXTURE_2D, bloom_textures[i].texture_id);
        //    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, texture_size.x, texture_size.y, 0, GL_RGBA, GL_FLOAT, nullptr);
        //}
    }

    void Render::clean_viewport_framebuffer() {
        // Clean main rendering
        glDeleteFramebuffers(1, &framebuffer);
        glDeleteTextures(1, &textureHDRColorbuffer);
        glDeleteTextures(1, &texture_id_colors);
        glDeleteTextures(1, &texture_selected_color_buffer);
        glDeleteTextures(1, &textureLDRColorbuffer);
        glDeleteTextures(1, &textureHDRBrightColorbuffer);
        glDeleteTextures(1, &texture_id_colors_transform3d);
        glDeleteRenderbuffers(1, &rboDepthStencil);

        // Clean PBR
        glDeleteFramebuffers(1, &captureFBO);
        glDeleteRenderbuffers(1, &captureRBO);
        glDeleteTextures(1, &brdfLUTTexture);

        // Clean bloom
        glDeleteFramebuffers(1, &bloom_fbo);
        for (int i = 0; i < bloom_textures.size(); i++) {
            glDeleteTextures(1, &(bloom_textures[i].texture_id));
        }
    }
    void Render::OnAddmodel2Scene()
    {
        std::map<std::string, BaseModel*>::iterator iter = loaded_models.begin();
        while (iter != loaded_models.end())
        {
            GameObject* E371 = new GameObject(iter->first, iter->first);
            E371->position = glm::vec3(0.0f, 0.0f, 0.0f);
            E371->scale = glm::vec3(0.3f);
            //E371->animation_id = 0;
            E371->set_model_matrices_standard();
            E371->is_selected = true;
            game_objects[E371->name] = E371;
            id_color_to_game_object[E371->id_color] = E371;
            
            ++iter;
            
            //
            last_selected_object = E371;
        }
    }
}
