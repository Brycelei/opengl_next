#include "gameobject.h"
#include "render.h"

#include<assert.h>
#include <model.h>
GameObject::GameObject(const std::string& name, const std::string& model_name) {
    this->name = name;
    this->model_name = model_name;
    type = TypeBaseModel;
    position = glm::vec3(0.0f);
    rotation = glm::angleAxis(glm::radians(0.0f), glm::vec3(1.0f, 0.0f, 0.0f));
    scale = glm::vec3(1.0f);
    albedo = glm::vec3(1.0f);
    metalness = 0.1f;
    roughness = 0.1f;
    emission = glm::vec3(0.0f);
    animation_id = -1;
    is_selected = false;
    render_only_ambient = false;
    render_one_color = false;
    material = nullptr;
    //id_color = color_generator->generate_color();

    set_model_matrices_standard();
}

GameObject::~GameObject() {

}

void GameObject::set_model_matrices_standard()
{
    model = glm::mat4(1.0f);
    model = glm::scale(model, scale);
    model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));
    /*model = glm::translate(model, position);
    model *= glm::mat4_cast(rotation);
    

    model_inv = glm::inverse(model);

    model_normals = glm::mat3(glm::transpose(model_inv));
    */
}

void GameObject::draw(Shader* shader, bool disable_depth_test) {
    Render* render = Render::get_instance();
    shader->setMat4("model", model);

    shader->setVec3("albedo_model", albedo);
    shader->setFloat("metalness_model", metalness);
    shader->setFloat("roughness_model", roughness);
    shader->setVec3("emission_model", emission);
    render->loaded_models[model_name]->draw(shader, material, is_selected, disable_depth_test, render_only_ambient, render_one_color);


    //shader->setMat4("model", model);
    //shader->setMat3("model_normals", model_normals);

    //glUniform3ui(glGetUniformLocation(shader->ID, "id_color_game_object"), id_color.r, id_color.g, id_color.b);

    //if (model_name != "") {
    //    if (this->animation_id != -1) { // There is an animation specified for the model of this game object
    //        auto current_time = std::chrono::system_clock::now();
    //        std::chrono::duration<float> elapsed_seconds = current_time - render->time_before_rendering;
    //        render->loaded_models[model_name]->update_bone_transformations(elapsed_seconds.count(), this->animation_id);
    //        shader->setInt("is_animated", true);
    //        assert(render->loaded_models[model_name]->bones.size() <= MAX_NUMBER_BONES);
    //        for (int i = 0; i < render->loaded_models[model_name]->bones.size(); i++) {
    //            glm::mat4 glm_matrix;
    //            std::memcpy(glm::value_ptr(glm_matrix), &(render->loaded_models[model_name]->bones[i].final_transformation), sizeof(aiMatrix4x4));
    //            glm_matrix = glm::transpose(glm_matrix);
    //            shader->setMat4("bone_transforms[" + std::to_string(i) + "]", glm_matrix);
    //        }
    //    }
    //    else {
    //        shader->setInt("is_animated", false);
    //    }
    //    render->loaded_models[model_name]->draw(shader, material, is_selected, disable_depth_test, render_only_ambient, render_one_color);
    //}
}

Skybox::Skybox(const std::string& name) : GameObject(name, "") {
    cubemap_name = "";
}