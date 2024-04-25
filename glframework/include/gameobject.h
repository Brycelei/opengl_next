#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include<string>
#include"shader.h"
#include"mesh.h"

namespace NESI_NEXT {
    const int MAX_NUMBER_BONES = 200;


    enum GameObjectType {
        TypeBaseModel,
        TypeLight,
        TypeDirectionalLight,
        TypeSpotLight,
        TypeSkybox
    };

    class GameObject {
    public:
        std::string name;
        std::string model_name;
        GameObjectType type;
        glm::vec3 position;
        glm::vec3 scale;
        glm::quat rotation;
        glm::mat4 model;
        glm::mat4 model_inv;
        glm::mat3 model_normals;
        glm::vec3 albedo;
        float metalness;
        float roughness;
        glm::vec3 emission;
        glm::u8vec3 id_color;
        bool is_selected;
        bool render_only_ambient;
        bool render_one_color;
        int animation_id;
        Material* material;

        //static ColorGenerator* color_generator;

        GameObject(const std::string& name, const std::string& model_name);
        ~GameObject();
        void set_select_state(bool is_game_obj_selected);
        void set_model_matrices_standard();
        void draw(Shader* shader, bool disable_depth_test);
        //bool intersected_ray(const glm::vec3& ray_dir, const glm::vec3& camera_position, float& t);
        //void set_select_state(bool is_game_obj_selected);
    };

    class Skybox : public GameObject {
    public:
        std::string cubemap_name;

        Skybox(const std::string& name);
    };

}