#include <iostream>
#include <shader.h>
#include <string>
#include <assert.h>//╤оят
//#include "wrapper/include/checkError.h"
#include <Application.h>
#include <log.h>
#include <model.h>
#include <camera.h>
/// for optimus feature enablement, below code must be provided.
extern "C"
{
	_declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

const unsigned int SCR_WIDTH = 1920;
const unsigned int SCR_HEIGHT = 1080;

void OnResize(int width, int height) {
	//GL_CALL(glViewport(0, 0, width, height));
	glViewport(0, 0, width, height);
	std::cout << "OnResize:" << width<< " " << height << std::endl;
}

void OnKey(int key, int action, int mods) {
	std::cout << key << std::endl;
}

void resgistScroll_callback(double xoffset, double yoffset, NESI_NEXT::Render* render)
{
	NESI_NEXT::Render* render_instance = NESI_NEXT::Render::get_instance();
	NESI_NEXT::Camera* camera = render_instance->camera_viewport;
	camera->ProcessMouseScroll(static_cast<float>(yoffset));
}

int main() {
	auto app = NESI_NEXT::Application::getInstance();
	NESI_NEXT::Log::init();
	NESI_TRACE("Welcome to NESI_NEXT!");
	if (!app->init(SCR_WIDTH, SCR_HEIGHT)) {
		NESI_ERROR("APP initialize failed!");
		return -1;
	}
	//register callback
	app->setResizeCallback(OnResize);
	app->setKeyBoardCallback(OnKey);
	app->setScroll_callback(resgistScroll_callback);

	NESI_INFO("Start main loop");
	while (app->update()) {
		app->run();
	}
	NESI_INFO("Break main loop");
	app->destroy();
	return 0;
}