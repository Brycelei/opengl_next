#include<Application.h>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <camera.h>
#include<log.h>
//初始化Application的静态变量
namespace NESI_NEXT {
	Application* Application::mInstance = nullptr;
	std::mutex Application::m_mutex;
	Application* Application::getInstance() {
		//如果mInstance已经实例化了（new出来了），就直接返回
		//否则需要先new出来，再返回
		if (mInstance == nullptr) {
			{
				m_mutex.lock();
				if (nullptr == mInstance)
				{
					mInstance = new Application();
				}
				m_mutex.unlock();
			}
		}
		return mInstance;
	}

	Application::Application() {
		render_instance = nullptr;
		firstMouse = true;

		// timing
		deltaTime = 0.0f;
		lastFrame = 0.0f;

		lastX = mWidth / 2.0f;
		lastY = mHeight / 2.0f;
	}

	Application::~Application() {
	}

	bool Application::init(const int& width, const int& height) {
		mWidth = width;
		mHeight = height;

		//1 初始化GLFW基本环境
		glfwInit();
		//1.1 设置OpenGL主版本号、次版本号
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		//1.2 设置OpenGL启用核心模式（非立即渲染模式）
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		//2 创建窗体对象
		mWindow = glfwCreateWindow(mWidth, mHeight, "NESI_NEXT", NULL, NULL);
		if (mWindow == NULL) {
			return false;
		}

		//**设置当前窗体对象为OpenGL的绘制舞台
		glfwMakeContextCurrent(mWindow);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			//std::cout << "Failed to initialize GLAD" << std::endl;
			NESI_FATAL("Failed to initialize GLAD");
			return false;
		}

		glfwSetFramebufferSizeCallback(mWindow, frameBufferSizeCallback);

		//this就是当前全局唯一的Application对象
		glfwSetWindowUserPointer(mWindow, this);
		//键盘响应
		glfwSetKeyCallback(mWindow, keyCallback);
		glfwSetScrollCallback(mWindow, scroll_callback);
		glfwSetCursorPosCallback(mWindow, mouse_callback);

		render_instance = Render::get_instance();
		render_instance->set_opengl_state();
		render_instance->initialize_shaders();
		render_instance->render_view_initialize(this->getWidth(), this->getHeight());
		render_instance->set_viewportdata();
		render_instance->initialize_object();
		render_instance->set_time_before_rendering_loop();
		return true;
	}

	bool Application::update() {
		if (glfwWindowShouldClose(mWindow)) {
			return false;
		}
		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

		//键盘输入响应
		processInput(mWindow, this);

		//接收并分发窗体消息
		//检查消息队列是否有需要处理的鼠标、键盘等消息
		//如果有的话就将消息批量处理，清空队列
		glfwPollEvents();

		//切换双缓存
		glfwSwapBuffers(mWindow);

		return true;
	}

	void Application::destroy() {
		//退出程序前做相关清理
		render_instance->render_destroy();
		glfwTerminate();
	}

	void Application::frameBufferSizeCallback(GLFWwindow* window, int width, int height) {
		std::cout << "Resize" << std::endl;

		Application* self = (Application*)glfwGetWindowUserPointer(window);
		if (self->mResizeCallback != nullptr) {
			self->mResizeCallback(width, height);
		}

		/*if (Application::getInstance()->mResizeCallback != nullptr) {
			Application::getInstance()->mResizeCallback(width, height);
		}*/
	}

	void Application::keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods) {
		Application* self = (Application*)glfwGetWindowUserPointer(window);
		if (self->mKeyBoardCallback != nullptr) {
			self->mKeyBoardCallback(key, action, mods);
		}
	}
	//静态函数访问非静态成员的4种方法
	/*

	① 在静态函数的形参表里加上实例的地址；
	② 在静态函数中使用全局变量；
	③ 静态成员函数可以访问静态成员，在类是单例类的情况下，可以在创建的时候把this指针赋值给那个静态成员，然后在静态成员函数内部访问this指向的静态成员；
	④ 在静态函数的形参比加上一个void *的内存首地址，然后在内部做转换；
	原文链接：https://blog.csdn.net/yueguangmuyu/article/details/118390764*/

	void Application::processInput(GLFWwindow* window, Application* app)
	{
		app->render_instance = Render::get_instance();
		Camera* camera = app->render_instance->camera_viewport;
		if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
			glfwSetWindowShouldClose(window, true);

		if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
			camera->ProcessKeyboard(FORWARD, app->deltaTime);
		if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
			camera->ProcessKeyboard(BACKWARD, app->deltaTime);
		if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
			camera->ProcessKeyboard(LEFT, app->deltaTime);
		if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
			camera->ProcessKeyboard(RIGHT, app->deltaTime);
	}
	void Application::scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
	{

		Application* self = (Application*)glfwGetWindowUserPointer(window);
		if (self->mScroll_callback != nullptr) {
			self->mScroll_callback(xoffset, yoffset, self->render_instance);
		}

	}

	void Application::mouse_callback(GLFWwindow* window, double xposIn, double yposIn)
	{
		Application* self = (Application*)glfwGetWindowUserPointer(window);

		float xpos = static_cast<float>(xposIn);
		float ypos = static_cast<float>(yposIn);

		if (self->firstMouse)
		{
			self->lastX = xpos;
			self->lastY = ypos;
			self->firstMouse = false;
		}

		float xoffset = xpos - self->lastX;
		float yoffset = self->lastY - ypos; // reversed since y-coordinates go from bottom to top

		self->lastX = xpos;
		self->lastY = ypos;

		self->render_instance = Render::get_instance();
		Camera* camera = self->render_instance->camera_viewport;
		camera->ProcessMouseMovement(xoffset, yoffset);
	}
}