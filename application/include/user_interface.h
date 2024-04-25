#pragma once

#include "imgui_extension.h"
#include<imgui/imgui.h>
#include <mutex>
#include <Application.h>
#include<file_system.h>
namespace NESI_NEXT {
	class Application;
	//class Input;
	class Render;
	class GameObject;

	enum DisplayedRendering {
		DisplayedColors,
		DisplayedIdColors,
		DisplayedIdColorsTransform3d,
		DisplayedSelectedColors,
		DisplayedBRDFLut,
		DisplayedHDRBrightColors,
		DisplayedBloom,
		DisplayedLast
	};

	enum Filetype
	{
		Folder,
		File
	};

	std::string displayed_rendering_to_string(DisplayedRendering displayed_rendering);

	class PresetData
	{
		friend class UserInterface;
	private:
		unsigned int texture;
		std::string name;
		Filetype filetype;
	public:
		PresetData(unsigned int _texture, std::string _name, Filetype _filetype)
			: texture(_texture), name(_name), filetype(_filetype) {};
	};

	class UserInterface {
	public:
		static UserInterface* get_instance();
		static void glfw_error_callback(int error, const char* description);

		UserInterface(UserInterface& other) = delete;
		void operator=(const UserInterface&) = delete;

		void initialize();
		void setup_imgui();
		void update_fps_ui();
		void check_if_viewport_window_resized();
		void show_game_object_ui(GameObject* game_object);
		void update_displayed_texture();
		void render_app();
		void clean_imgui();
		void GetApplicationInfo();

		void ImportModelPanel();
		void FileBrowser(std::filesystem::path* _path);

		int window_viewport_width, window_viewport_height;
		int texture_viewport_width, texture_viewport_height;;
		int texture_viewport_reduce_width_px, texture_viewport_reduce_height_px;
		unsigned int rendered_texture;
		DisplayedRendering displayed_rendering;
		bool first_time_viewport_fbo;
		ImVec2 viewport_window_pos;
		ImVec2 viewport_texture_pos;
		float passed_time_resize;
		bool was_resized;

		

	private:
		UserInterface();
		~UserInterface();

		void set_ui_style();
		void loadImages();
		float passed_time_seconds;
		float frames_per_second_ui;
		std::filesystem::path* file_path;
		std::filesystem::path import_model_path = FileSystem::GetContentPath();
		std::filesystem::path import_hdri_path = FileSystem::GetContentPath();
		bool showFileBrowser = false;
		bool showImportModelPanel = false;
		bool showImportHdriPanel = false;
		std::vector<PresetData> presetImages;

		Application*  app;
		//Input* input;
		Render* render;

		static UserInterface* instance;
		static std::mutex user_interface_mutex;
	};
}