#pragma once 
#include <iostream>
#include<mutex>
#include<render.h>
#include<user_interface.h>
class GLFWwindow;

using ResizeCallback = void(*)(int width, int height);
using KeyBoardCallback = void(*)(int key, int action, int mods);
using ScrollCallback = void (*)(double a, double b, NESI_NEXT::Render* render);

namespace NESI_NEXT {

	class UserInterface;
	class Application {
	public:
		~Application();

		//用于访问实例的静态函数
		static Application* getInstance();
		static std::mutex m_mutex;

		bool init(const int& width = 800, const int& height = 600);

		bool update();

		void destroy();

		//void run();

		uint32_t getWidth()const { return mWidth; }
		uint32_t getHeight()const { return mHeight; }
		GLFWwindow* getWindow() const{ return mWindow; }
		const char* getGLversion() const { return glsl_version; }
		Render* getRender() { return this->render_instance; }
		void setResizeCallback(ResizeCallback callback) { 
			mResizeCallback = callback;
		}
		void setKeyBoardCallback(KeyBoardCallback callback) { mKeyBoardCallback = callback; }
		void setScroll_callback(ScrollCallback callback){
			mScroll_callback = callback;
		}
		//void do_movement(GLFWwindow* window, Application* app);
		void run();

		// timing
		float deltaTime;
		float lastFrame;
		float frames_per_second;

		// config info
		char* glInfos;
		char* hardwareInfos;

		ImGuiIO io;
	private:
		//C++类内函数指针
		static void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void processInput(GLFWwindow* window, Application* app);
		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);


	private:
		//全局唯一的静态变量实例
		static Application* mInstance;
		Render* render_instance;
		UserInterface* user_interface;

		uint32_t mWidth{ 0 };
		uint32_t mHeight{ 0 };
		GLFWwindow* mWindow{ nullptr };

		ResizeCallback mResizeCallback{ nullptr };
		KeyBoardCallback mKeyBoardCallback{ nullptr };
		ScrollCallback mScroll_callback{ nullptr };


		bool firstMouse;
		
		float lastX;
		float lastY;
		const char * glsl_version;

		
		// 禁止外部构造
		Application();
		// 禁止外部拷贝
		Application(const Application& Application) = delete;
		// 禁止外部赋值
		const Application& operator=(const Application& Application) = delete;
	};
}
