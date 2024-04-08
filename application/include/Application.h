#pragma once 

/*
*│　拆分目标：
*│
*│　		1	单例类（全局唯一实例）
*│　		2	成员变量 + 成员函数
*				2.1 成员函数-init（初始化）
*				2.2 成员函数-update（每一帧执行）
*				2.3 成员函数-destroy（结尾执行）
*│　		3	响应回调函数(Resize)
*				3.1 声明一个函数指针ResizeCallback
*				3.2 声明一个ResizeCallback类型的成员变量
*				3.3 声明一个SetResizeCallback的函数 ，设置窗体变化响应回调函数
*				3.4 声明一个static的静态函数，用于响应glfw窗体变化
*				3.5 将静态函数设置到glfw的监听Resize监听当中
*				3.6 * 学会使用glfw的UserPointer
*│　		4	响应键盘消息函数(KeyBoard)
*				3.1 声明一个static的静态函数，用于响应glfw的键盘事件
*				3.2 将静态函数设置到glfw的监听KeyCallback监听当中
*				3.3 声明一个函数指针KeyBoardCallback
*				3.4 声明一个KeyBoardCallback类型的成员变量
*				3.5 声明一个SetKeyBoardCallback的函数 ，设置键盘响应回调函数
*				3.6 * 学会使用glfw的UserPointer
*└────────────────────────────────────────────────┘
*/
#include <iostream>
#include<mutex>
#include<render.h>
class GLFWwindow;


using ResizeCallback = void(*)(int width, int height);
using KeyBoardCallback = void(*)(int key, int action, int mods);
using ScrollCallback = void (*)(double a, double b, Render* render);

namespace NESI_NEXT {
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
		Render* getRender() { return this->render_instance; }
		void setResizeCallback(ResizeCallback callback) { 
			mResizeCallback = callback;
			
		}
		void setKeyBoardCallback(KeyBoardCallback callback) { mKeyBoardCallback = callback; }
		void setScroll_callback(ScrollCallback callback){
			mScroll_callback = callback;
		}

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

		uint32_t mWidth{ 0 };
		uint32_t mHeight{ 0 };
		GLFWwindow* mWindow{ nullptr };

		ResizeCallback mResizeCallback{ nullptr };
		KeyBoardCallback mKeyBoardCallback{ nullptr };
		ScrollCallback mScroll_callback{ nullptr };


		bool firstMouse;
		// timing
		float deltaTime;
		float lastFrame;
		float lastX;
		float lastY;
		
		// 禁止外部构造
		Application();
		// 禁止外部拷贝
		Application(const Application& Application) = delete;
		// 禁止外部赋值
		const Application& operator=(const Application& Application) = delete;
	};
}
