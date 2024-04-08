#pragma once 

/*
*�������Ŀ�꣺
*��
*����		1	�����ࣨȫ��Ψһʵ����
*����		2	��Ա���� + ��Ա����
*				2.1 ��Ա����-init����ʼ����
*				2.2 ��Ա����-update��ÿһִ֡�У�
*				2.3 ��Ա����-destroy����βִ�У�
*����		3	��Ӧ�ص�����(Resize)
*				3.1 ����һ������ָ��ResizeCallback
*				3.2 ����һ��ResizeCallback���͵ĳ�Ա����
*				3.3 ����һ��SetResizeCallback�ĺ��� �����ô���仯��Ӧ�ص�����
*				3.4 ����һ��static�ľ�̬������������Ӧglfw����仯
*				3.5 ����̬�������õ�glfw�ļ���Resize��������
*				3.6 * ѧ��ʹ��glfw��UserPointer
*����		4	��Ӧ������Ϣ����(KeyBoard)
*				3.1 ����һ��static�ľ�̬������������Ӧglfw�ļ����¼�
*				3.2 ����̬�������õ�glfw�ļ���KeyCallback��������
*				3.3 ����һ������ָ��KeyBoardCallback
*				3.4 ����һ��KeyBoardCallback���͵ĳ�Ա����
*				3.5 ����һ��SetKeyBoardCallback�ĺ��� �����ü�����Ӧ�ص�����
*				3.6 * ѧ��ʹ��glfw��UserPointer
*����������������������������������������������������������������������������������������������������
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

		//���ڷ���ʵ���ľ�̬����
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
		//C++���ں���ָ��
		static void frameBufferSizeCallback(GLFWwindow* window, int width, int height);
		static void keyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void processInput(GLFWwindow* window, Application* app);
		static void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
		static void mouse_callback(GLFWwindow* window, double xposIn, double yposIn);

	private:
		//ȫ��Ψһ�ľ�̬����ʵ��
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
		
		// ��ֹ�ⲿ����
		Application();
		// ��ֹ�ⲿ����
		Application(const Application& Application) = delete;
		// ��ֹ�ⲿ��ֵ
		const Application& operator=(const Application& Application) = delete;
	};
}
