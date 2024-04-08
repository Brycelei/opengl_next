#include <iostream>
#include <shader.h>
#include <string>
#include <assert.h>//断言
#include "wrapper/include/checkError.h"
#include <Application.h>
#include <log.h>
#include <model.h>
#include <camera.h>

const unsigned int SCR_WIDTH = 1200;
const unsigned int SCR_HEIGHT = 800;
GLuint vao;
Shader* shader = nullptr;

void OnResize(int width, int height) {
	//GL_CALL(glViewport(0, 0, width, height));
	glViewport(0, 0, width, height);
	std::cout << "OnResize:" << width<< " " << height << std::endl;

}

void OnKey(int key, int action, int mods) {
	std::cout << key << std::endl;
}

void prepareShader(Shader **pbrShader, Shader **pbr_no_normal_Shader) {
	//shader = new Shader("assets/shaders/vertex.glsl","assets/shaders/fragment.glsl");
	*pbrShader = new Shader("../shaders/pbr/hoho/1.model_loading.vert", "../shaders/pbr/hoho/1.model_loading1.frag");
	//*pbr_no_normal_Shader = new Shader("../shaders/pbr/2.2/2.2.1.pbr_no_normal.vert", "../shaders/pbr/2.2/2.2.1.pbr_no_normal.frag");
}


void render(Shader* pbrShader) {
	//执行opengl画布清理操作
	//GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
	std::string objpath = "../asset/obj/E371/E371.obj";
	
	Model ourModel(objpath);
	Material *material = nullptr;
	pbrShader->use();
	glm::mat4 model = glm::mat4(1.0f);
	model = glm::scale(model, glm::vec3(0.08f, 0.08f, 0.08f));
	model = glm::rotate(model, glm::radians(90.0f), glm::vec3(0.0f, 1.0f, 0.0f));

	pbrShader->setMat4("model", model);
	ourModel.draw(pbrShader, material, false);

}

void resgistScroll_callback(double xoffset, double yoffset, Render* render)
{
	Render* render_instance = Render::get_instance();
	Camera* camera = render_instance->camera_viewport;
	camera->ProcessMouseScroll(static_cast<float>(yoffset));
}


int main() {
	auto app = NESI_NEXT::Application::getInstance();
	NESI_NEXT::Log::init();
	NESI_TRACE("Welcome to NESI_NEXT!");
	if (!app->init(SCR_WIDTH, SCR_HEIGHT)) {
		return -1;
	}

	app->setResizeCallback(OnResize);
	app->setKeyBoardCallback(OnKey);
	app->setScroll_callback(resgistScroll_callback);

	//GL_CALL(glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT));

	//////设置opengl视口以及清理颜色
	//GL_CALL(glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT));
	//GL_CALL(glClearColor(0.2f, 0.3f, 0.3f, 1.0f));
	//GL_CALL(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	//// camera
	//Camera camera(glm::vec3(0.0f, 0.0f, 800.0f));
	//Shader* pbrShader = nullptr;
	//Shader* pbr_no_normal_Shader = nullptr;
	//prepareShader(&pbrShader,&pbr_no_normal_Shader);
	//glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 1000.0f);
	//pbrShader->use();
	//pbrShader->setMat4("projection", projection);
	//pbrShader->setMat4("view", camera.GetViewMatrix());

	NESI_INFO("Start main loop");
	while (app->update()) {

		//render(pbrShader);
		app->getRender()->render_viewport();
	}
	NESI_INFO("Break main loop");
	app->destroy();
	return 0;
}