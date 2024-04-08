#include<Application.h>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include <camera.h>
#include<log.h>
//��ʼ��Application�ľ�̬����
namespace NESI_NEXT {
	Application* Application::mInstance = nullptr;
	std::mutex Application::m_mutex;
	Application* Application::getInstance() {
		//���mInstance�Ѿ�ʵ�����ˣ�new�����ˣ�����ֱ�ӷ���
		//������Ҫ��new�������ٷ���
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

		//1 ��ʼ��GLFW��������
		glfwInit();
		//1.1 ����OpenGL���汾�š��ΰ汾��
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		//1.2 ����OpenGL���ú���ģʽ����������Ⱦģʽ��
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

		//2 �����������
		mWindow = glfwCreateWindow(mWidth, mHeight, "NESI_NEXT", NULL, NULL);
		if (mWindow == NULL) {
			return false;
		}

		//**���õ�ǰ�������ΪOpenGL�Ļ�����̨
		glfwMakeContextCurrent(mWindow);

		if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
			//std::cout << "Failed to initialize GLAD" << std::endl;
			NESI_FATAL("Failed to initialize GLAD");
			return false;
		}

		glfwSetFramebufferSizeCallback(mWindow, frameBufferSizeCallback);

		//this���ǵ�ǰȫ��Ψһ��Application����
		glfwSetWindowUserPointer(mWindow, this);
		//������Ӧ
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

		//����������Ӧ
		processInput(mWindow, this);

		//���ղ��ַ�������Ϣ
		//�����Ϣ�����Ƿ�����Ҫ�������ꡢ���̵���Ϣ
		//����еĻ��ͽ���Ϣ����������ն���
		glfwPollEvents();

		//�л�˫����
		glfwSwapBuffers(mWindow);

		return true;
	}

	void Application::destroy() {
		//�˳�����ǰ���������
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
	//��̬�������ʷǾ�̬��Ա��4�ַ���
	/*

	�� �ھ�̬�������βα������ʵ���ĵ�ַ��
	�� �ھ�̬������ʹ��ȫ�ֱ�����
	�� ��̬��Ա�������Է��ʾ�̬��Ա�������ǵ����������£������ڴ�����ʱ���thisָ�븳ֵ���Ǹ���̬��Ա��Ȼ���ھ�̬��Ա�����ڲ�����thisָ��ľ�̬��Ա��
	�� �ھ�̬�������βαȼ���һ��void *���ڴ��׵�ַ��Ȼ�����ڲ���ת����
	ԭ�����ӣ�https://blog.csdn.net/yueguangmuyu/article/details/118390764*/

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