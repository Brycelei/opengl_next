#include "user_interface.h"

#include <Application.h>
#include "gameobject.h"
#include "model.h"
#include "log.h"
#include "cubemap.h"

#include <imgui/imgui.h>
#include <imgui/backends/imgui_impl_glfw.h>
#include <imgui/backends/imgui_impl_opengl3.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <mutex>
#include <iostream>
#include <cstring>
#include "../../glframework/src/gameobject.cpp"
#include "../../glframework/src/cubemap.cpp"
#include <GLResources.h>

namespace NESI_NEXT {
	UserInterface* UserInterface::instance = nullptr;
	std::mutex UserInterface::user_interface_mutex;

	void UserInterface::glfw_error_callback(int error, const char* description)
	{
		fprintf(stderr, "GLFW Error %d: %s\n", error, description);
	}

	UserInterface::UserInterface() {
		app = nullptr;
		//input = nullptr;
		render = nullptr;
		window_viewport_width = -1;
		window_viewport_height = -1;
		texture_viewport_width = -1;
		texture_viewport_height = -1;
		texture_viewport_reduce_width_px = 50;
		texture_viewport_reduce_height_px = 50;
		first_time_viewport_fbo = true;
		passed_time_seconds = 0.0f;
		frames_per_second_ui = 0.0f;
		rendered_texture = 0;
		displayed_rendering = DisplayedColors;
		passed_time_resize = 0.0f;
		was_resized = false;
	}

	UserInterface::~UserInterface() {
	}

	UserInterface* UserInterface::get_instance()
	{
		std::lock_guard<std::mutex> lock(user_interface_mutex);
		if (instance == nullptr) {
			instance = new UserInterface();
		}
		return instance;
	}

	void UserInterface::initialize() {
		app = Application::getInstance();
		//input = Input::get_instance();
		render = Render::get_instance();
	}

	void UserInterface::set_ui_style() {
		ImGuiIO& io = ImGui::GetIO();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		io.Fonts->AddFontFromFileTTF("../asset/fonts/Ubuntu-Regular.ttf", 15.0f);

		ImVec4 dark_purple = ImVec4(70.0f / 255.0f * 0.65f, 36.0f / 255.0f * 0.65f, 90.0f / 255.0f * 0.65f, 1.0f);
		ImVec4 very_dark_purple = (glm::vec4)dark_purple * 0.2f; very_dark_purple.w = 1.0f;
		ImVec4 purple = (glm::vec4)dark_purple * 2.0f; purple.w = 1.0f;
		ImVec4 light_purple = (glm::vec4)dark_purple * 3.0f; light_purple.w = 1.0f;
		ImVec4 white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
		ImVec4 dark_white = (glm::vec4)white * 0.8f; dark_white.w = 1.0f;
		ImVec4 light_cyan = ImVec4(16.0f / 255.0f * 0.8f, 186.0f / 255.0f * 0.8f, 233.0f / 255.0f * 0.8f, 1.0f);
		ImVec4 cyan = (glm::vec4)light_cyan * 0.8f; cyan.w = 1.0f;
		ImVec4 dark_cyan = (glm::vec4)light_cyan * 0.5f; dark_cyan.w = 0.5f;
		ImVec4 light_yellow = ImVec4(200.0f / 255.0f, 165.0f / 255.0f, 30.0f / 255.0f, 1.0f);
		ImVec4 yellow = (glm::vec4)light_yellow * 0.8f; yellow.w = 1.0f;
		ImVec4 dark_yellow = (glm::vec4)light_yellow * 0.6f; dark_yellow.w = 1.0f;
		ImVec4 red = ImVec4(233.0f / 255.0f, 16.0f / 255.0f, 96.0f / 255.0f, 1.0f);

		ImVec4* colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_WindowBg] = very_dark_purple;

		colors[ImGuiCol_TitleBg] = yellow;
		colors[ImGuiCol_TitleBgActive] = light_yellow;
		colors[ImGuiCol_TitleBgCollapsed] = dark_yellow;

		colors[ImGuiCol_Header] = dark_purple;
		colors[ImGuiCol_HeaderHovered] = purple;
		colors[ImGuiCol_HeaderActive] = light_purple;

		colors[ImGuiCol_Button] = dark_purple;
		colors[ImGuiCol_ButtonHovered] = purple;
		colors[ImGuiCol_ButtonActive] = light_purple;

		colors[ImGuiCol_FrameBg] = dark_purple;
		colors[ImGuiCol_FrameBgHovered] = purple;
		colors[ImGuiCol_FrameBgActive] = light_purple;

		colors[ImGuiCol_CheckMark] = light_cyan;

		colors[ImGuiCol_SliderGrab] = cyan;
		colors[ImGuiCol_SliderGrabActive] = light_cyan;

		colors[ImGuiCol_Tab] = dark_cyan;
		colors[ImGuiCol_TabHovered] = cyan;
		colors[ImGuiCol_TabActive] = light_cyan;
		colors[ImGuiCol_TabUnfocused] = dark_cyan;
		colors[ImGuiCol_TabUnfocusedActive] = light_cyan;
	}

	void UserInterface::loadImages()
	{
		namespace fs = std::filesystem;
		for (auto& p : fs::directory_iterator("../asset/icon"))
		{
			if (p.path().extension() == ".png"  || p.path().extension() == ".jpg")
			{
				const std::string filename = p.path().filename().string();
				Filetype filetype;
				if (p.path().string().find("folder") != std::string::npos)
				{
					filetype = Folder;

				}
				else {
					filetype = File;
				}
				presetImages.emplace_back(GLResources::LoadPresetImage(p.path().string(), 3), filename, filetype);

			}
		}
	}

	void UserInterface::setup_imgui() {
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO();
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		set_ui_style();
		loadImages();
		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForOpenGL(app->getWindow(), true);
		ImGui_ImplOpenGL3_Init(app->getGLversion());
	}

	void UserInterface::update_fps_ui() {
		passed_time_seconds += app->deltaTime;
		if (passed_time_seconds >= 1.0f) {
			frames_per_second_ui = app->frames_per_second;
			passed_time_seconds = 0.0f;
		}
	}

	void UserInterface::check_if_viewport_window_resized() {
		int new_window_viewport_width = ImGui::GetWindowWidth();
		int new_window_viewport_height = ImGui::GetWindowHeight();
		if (new_window_viewport_width != window_viewport_width || new_window_viewport_height != window_viewport_height) {
			window_viewport_width = new_window_viewport_width;
			window_viewport_height = new_window_viewport_height;
			texture_viewport_width = new_window_viewport_width - texture_viewport_reduce_width_px;
			texture_viewport_height = new_window_viewport_height - texture_viewport_reduce_height_px;
			render->width = texture_viewport_width;
			render->height = texture_viewport_height;
			if (first_time_viewport_fbo) {
				render->setup_framebuffer_and_textures();
				first_time_viewport_fbo = false;
			}
			else {
				render->resize_textures();
			}
		}
	}

	void UserInterface::update_displayed_texture() {
		if (displayed_rendering == DisplayedColors) {
			rendered_texture = render->textureLDRColorbuffer;
		}
		else if (displayed_rendering == DisplayedIdColors) {
			rendered_texture = render->texture_id_colors;
		}
		else if (displayed_rendering == DisplayedIdColorsTransform3d) {
			rendered_texture = render->texture_id_colors_transform3d;
		}
		else if (displayed_rendering == DisplayedSelectedColors) {
			rendered_texture = render->texture_selected_color_buffer;
		}
		else if (displayed_rendering == DisplayedBRDFLut) {
			rendered_texture = render->brdfLUTTexture;
		}
		else if (displayed_rendering == DisplayedHDRBrightColors) {
			rendered_texture = render->textureHDRBrightColorbuffer;
		}
		else if (displayed_rendering == DisplayedBloom) {
			rendered_texture = render->bloom_textures[0].texture_id;
		}
		else {
			rendered_texture = render->textureLDRColorbuffer;
		}
	}

	void UserInterface::render_app() {
		update_fps_ui();
		GetApplicationInfo();
		static bool opt_fullscreen = true;
		static bool opt_padding = false;
		static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

		// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
		// because it would be confusing to have two docking targets within each others.
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
		if (opt_fullscreen)
		{
			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
		}
		else
		{
			dockspace_flags &= ~ImGuiDockNodeFlags_PassthruCentralNode;
		}

		// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
		// and handle the pass-thru hole, so we ask Begin() to not render a background.
		if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
			window_flags |= ImGuiWindowFlags_NoBackground;

		// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
		// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
		// all active windows docked into it will lose their parent and become undocked.
		// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
		// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
		if (!opt_padding)
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("NESI Engine", nullptr, window_flags);
		if (!opt_padding)
			ImGui::PopStyleVar();

		if (opt_fullscreen)
			ImGui::PopStyleVar(2);

		// Submit the DockSpace
		ImGuiIO& io = ImGui::GetIO();
		if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
		{
			ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
			ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
		}

		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("Options"))
			{
				// Disabling fullscreen would allow the window to be moved to the front of other windows,
				// which we can't undo at the moment without finer window depth/z control.
				ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen);
				ImGui::MenuItem("Padding", NULL, &opt_padding);
				ImGui::Separator();

				if (ImGui::MenuItem("Flag: NoSplit", "", (dockspace_flags & ImGuiDockNodeFlags_NoSplit) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoSplit; }
				if (ImGui::MenuItem("Flag: NoResize", "", (dockspace_flags & ImGuiDockNodeFlags_NoResize) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoResize; }
				if (ImGui::MenuItem("Flag: NoDockingInCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_NoDockingInCentralNode) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_NoDockingInCentralNode; }
				if (ImGui::MenuItem("Flag: AutoHideTabBar", "", (dockspace_flags & ImGuiDockNodeFlags_AutoHideTabBar) != 0)) { dockspace_flags ^= ImGuiDockNodeFlags_AutoHideTabBar; }
				if (ImGui::MenuItem("Flag: PassthruCentralNode", "", (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode) != 0, opt_fullscreen)) { dockspace_flags ^= ImGuiDockNodeFlags_PassthruCentralNode; }
				ImGui::Separator();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Files"))
			{
				if (ImGui::MenuItem("Import Model"))
				{
					showImportModelPanel = true;
				}
				if (ImGui::MenuItem("Import hdri"))
				{
					showImportHdriPanel = true;
				}
				ImGui::EndMenu();
			}

			ImGui::EndMenuBar();
			
			
		}
		ImportModelPanel();
		FileBrowser(file_path);

		/////////////////////////////////////////////// VIEWPORT WINDOW ///////////////////////////////////////////////
		ImGui::Begin("Viewport");
		viewport_window_pos = ImGui::GetWindowPos();

		check_if_viewport_window_resized();

		// ---------------------------------------------- MAIN render FUNCTION ----------------------------------------------
		render->render_viewport();
		// ---------------------------------------------------------------------------------------------------------------------

		ImVec2 pos1 = ImGui::GetCursorScreenPos();
		ImVec2 pos2(pos1.x + texture_viewport_width, pos1.y + texture_viewport_height);

		viewport_texture_pos = ImGui::GetCursorPos();
		ImGui::GetWindowDrawList()->AddImage((void*)rendered_texture, pos1, pos2, ImVec2(0, 1), ImVec2(1, 0));

		//input->process_viewport_input();

		ImGui::End();

		///////////////////////////////////// WORLD SETTINGS WINDOW /////////////////////////////////////
		ImGui::Begin("World Settings");
		if (ImGui::BeginTable("WorldSettingsTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
			// Row 1: Displayed render
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Displayed render");

			ImGui::TableSetColumnIndex(1);
			ImGui::PushItemWidth(-1);
			if (ImGui::BeginCombo("##Displayedrender", displayed_rendering_to_string(displayed_rendering).c_str()))
			{
				for (int i = DisplayedColors; i < DisplayedLast; i++) {
					const bool is_selected = (displayed_rendering == i);

					if (ImGui::Selectable(displayed_rendering_to_string((DisplayedRendering)i).c_str(), is_selected)) {
						displayed_rendering = (DisplayedRendering)i;
						update_displayed_texture();
					}
					if (is_selected) {
						ImGui::SetItemDefaultFocus();
					}
				}
				ImGui::EndCombo();
			}
			// Row 2: Exposure
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Exposure");

			ImGui::TableSetColumnIndex(1);
			ImGui::DragFloat("##Exposure", &(render->exposure), 0.05f, 0.0f, std::numeric_limits<float>::max());

			// Row 3: Activate Bloom
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Activate bloom");

			ImGui::TableSetColumnIndex(1);
			ImGui::Checkbox("##ActivateBloom", &(render->bloom_activated));

			// Row: Bloom Strength
			if (render->bloom_activated) {
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Bloom strength");

				ImGui::TableSetColumnIndex(1);
				ImGui::DragFloat("##BloomStrength", &(render->bloom_strength), 0.001f, 0.0f, std::numeric_limits<float>::max());
			}

			// Row 4: Emission Strength
			ImGui::TableNextRow();

			ImGui::TableSetColumnIndex(0);
			ImGui::Text("Emission strength");

			ImGui::TableSetColumnIndex(1);
			ImGui::DragFloat("##EmissionStrength", &(render->emission_strength), 0.2f, 0.0f, std::numeric_limits<float>::max());

			ImGui::PopItemWidth();

			ImGui::EndTable();
		}
		ImGui::End();
		////////////////////////////////////// Application Info //////////////////////////////////////
		ImGui::Begin("Application Info");
		ImGui::Text("OpenGL Version : %s", app->glInfos);
		ImGui::Text("Hardware Informations : %s", app->hardwareInfos);
		ImGui::Text(std::string("FPS: " + std::to_string(frames_per_second_ui)).c_str());
		ImGui::End();

		////////////////////////////////////// DETAILS WINDOW //////////////////////////////////////
		ImGui::Begin("Details");
		show_game_object_ui(render->last_selected_object);
		ImGui::End();

		/////////////////////////////////// OUTLINER WINDOWS ///////////////////////////////////
		ImGui::Begin("Outliner");

		if (ImGui::BeginTable("OutlinerTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg))
		{
			ImGui::TableSetupColumn("Game Object Name");
			ImGui::TableSetupColumn("Type");
			ImGui::TableHeadersRow();

			static int selected_row = 0;

			int row = 1;
			for (auto it = render->game_objects.begin(); it != render->game_objects.end(); it++) {
				if (render->last_selected_object == it->second) {
					selected_row = row;
				}
				row++;
			}

			for (auto it = render->game_objects.begin(); it != render->game_objects.end(); it++)
			{
				std::string game_object_type = game_object_type_to_string(it->second->type);

				ImGui::TableNextRow();

				bool is_row_selected = false;
				if (selected_row == ImGui::TableGetRowIndex()) {
					is_row_selected = true;
				}

				int new_selected_row = 0;
				ImGui::TableSetColumnIndex(0);
				std::string id0 = "row" + std::to_string(ImGui::TableGetRowIndex()) + "col" + std::to_string(ImGui::TableGetColumnIndex());
				if (ImGui::Selectable(std::string(it->first + "##" + id0).c_str(), is_row_selected, ImGuiSelectableFlags_SpanAllColumns)) {
					new_selected_row = ImGui::TableGetRowIndex();
				}

				ImGui::TableSetColumnIndex(1);
				std::string id1 = "row" + std::to_string(ImGui::TableGetRowIndex()) + "col" + std::to_string(ImGui::TableGetColumnIndex());
				if (ImGui::Selectable(std::string(game_object_type + "##" + id1).c_str(), is_row_selected, ImGuiSelectableFlags_SpanAllColumns)) {
					new_selected_row = ImGui::TableGetRowIndex();
				}

				if (new_selected_row != 0 && new_selected_row != selected_row) {
					selected_row = new_selected_row;
					if (render->last_selected_object != nullptr) {
						render->last_selected_object->set_select_state(false);
					}
					render->last_selected_object = it->second;
					it->second->set_select_state(true);
				}
			}

			ImGui::EndTable();
		}

		ImGui::End();

		ImGui::End();
	}

	void UserInterface::show_game_object_ui(GameObject* game_object) {
		if (game_object != nullptr) {
			// Name of the game object
			if (ImGui::BeginTable("GameObjectNameTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
				//Row 1: Game Object name
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Game Object name");

				ImGui::TableSetColumnIndex(1);
				ImGui::PushItemWidth(-1);
				char buffer[100];
				strcpy_s(buffer, game_object->name.c_str());
				ImGui::InputText("##GameObjectName", buffer, sizeof(buffer));
				if (ImGui::IsItemEdited()) {
					render->game_objects.erase(game_object->name);
					game_object->name = buffer;
					render->game_objects[game_object->name] = game_object;
				}

				ImGui::PopItemWidth();

				ImGui::EndTable();
			}

			if (game_object->type != TypeSkybox) {
				// Transformations: Position, Rotation and Scale
				bool update_model_matrices = false;
				if (ImGui::CollapsingHeader("Transformations", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("TransformationsTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
						// Row 1: Position
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Position");

						ImGui::TableSetColumnIndex(1);
						ImGui::PushItemWidth(-1);
						ImGui::DragFloat3("##Position", &(game_object->position.x), 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
						if (ImGui::IsItemEdited()) {
							update_model_matrices = true;
						}

						// Row 2: Rotation
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Rotation");

						ImGui::TableSetColumnIndex(1);
						glm::vec3 euler_rotation = glm::degrees(glm::eulerAngles(game_object->rotation));
						ImGui::DragFloat3("##Rotation", &(euler_rotation.x), 0.5f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
						if (euler_rotation.y >= 90.0f) {
							euler_rotation.y = 89.999;
						}
						else if (euler_rotation.y <= -90.0f) {
							euler_rotation.y = -89.999;
						}
						if (ImGui::IsItemEdited()) {
							game_object->rotation = glm::quat(glm::radians(euler_rotation));
							update_model_matrices = true;
						}

						// Row 3: Scale
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Scale");

						ImGui::TableSetColumnIndex(1);
						ImGui::DragFloat3("##Scale", &(game_object->scale.x), 0.01f, std::numeric_limits<float>::lowest(), std::numeric_limits<float>::max());
						if (ImGui::IsItemEdited()) {
							update_model_matrices = true;
						}
						if (update_model_matrices) {
							game_object->set_model_matrices_standard();
						}

						ImGui::PopItemWidth();

						ImGui::EndTable();
					}
				}

				// Material
				if (ImGui::CollapsingHeader("Material", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("MaterialTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Material Type");
						ImGui::TableSetColumnIndex(1);
						ImGui::PushItemWidth(-1);
						std::string material_preview_value;
						if (game_object->material == nullptr) {
							material_preview_value = "Default";
						}
						else {
							material_preview_value = game_object->material->name;
						}
						if (ImGui::BeginCombo("##Material type", material_preview_value.c_str()))
						{
							for (auto it = render->loaded_materials.begin(); it != render->loaded_materials.end(); it++) {
								const bool is_selected = (game_object->material == it->second);

								if (ImGui::Selectable(it->first.c_str(), is_selected)) {
									game_object->material = it->second;
								}

								// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
								if (is_selected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndCombo();
						}

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Albedo");
						ImGui::TableSetColumnIndex(1);
						ImGui::ColorEdit3("##Albedo", &(game_object->albedo.x));

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Metalness");
						ImGui::TableSetColumnIndex(1);
						ImGui::SliderFloat("##Metalness", &game_object->metalness, 0.0f, 1.0f);

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Roughness");
						ImGui::TableSetColumnIndex(1);
						ImGui::SliderFloat("##Roughness", &game_object->roughness, 0.0f, 1.0f);

						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Emission");
						ImGui::TableSetColumnIndex(1);
						ImGui::ColorEdit3("##Emission", &(game_object->emission.x));

						ImGui::PopItemWidth();

						ImGui::EndTable();
					}
				}

				// Model information
				if (ImGui::CollapsingHeader("Model information", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("ModelInfoTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
						ImGui::TableNextRow();
						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Model name");
						ImGui::TableSetColumnIndex(1);
						ImGui::PushItemWidth(-1);

						if (ImGui::BeginCombo("##Model", game_object->model_name.c_str()))
						{
							for (auto it = render->loaded_models.begin(); it != render->loaded_models.end(); it++) {
								const bool is_selected = (game_object->model_name == it->first);

								if (ImGui::Selectable(it->first.c_str(), is_selected)) {
									game_object->model_name = it->first;
									game_object->animation_id = -1;
								}

								// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
								if (is_selected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndCombo();
						}

						Model* model = dynamic_cast<Model*>(render->loaded_models[game_object->model_name]);
						if (model) {
							std::string file_format;
							if (model->format == FileFormat::glTF) {
								file_format = "glTF";
							}
							else if (model->format == FileFormat::FBX) {
								file_format = "FBX";
							}
							else {
								file_format = "Default";
							}
							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("File format");
							ImGui::TableSetColumnIndex(1);
							ImGui::Text(file_format.c_str());

							std::string no_animation("No Animation");
							std::string animation_preview_value;
							if (game_object->animation_id == -1) {
								animation_preview_value = no_animation;
							}
							else {
								animation_preview_value = model->animations[game_object->animation_id].name;
							}

							ImGui::TableNextRow();
							ImGui::TableSetColumnIndex(0);
							ImGui::Text("Animation type");
							ImGui::TableSetColumnIndex(1);
							if (ImGui::BeginCombo("##AnimationType", animation_preview_value.c_str())) {
								const bool is_selected = (game_object->animation_id == -1);
								if (ImGui::Selectable(no_animation.c_str(), is_selected)) {
									game_object->animation_id = -1;
								}
								if (is_selected) {
									ImGui::SetItemDefaultFocus();
								}
								for (int i = 0; i < model->animations.size(); i++) {
									const bool is_selected = (game_object->animation_id == i);

									if (ImGui::Selectable(model->animations[i].name.c_str(), is_selected)) {
										game_object->animation_id = i;
									}

									// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
									if (is_selected) {
										ImGui::SetItemDefaultFocus();
									}
								}
								ImGui::EndCombo();
							}
						}
						ImGui::PopItemWidth();

						ImGui::EndTable();
					}
				}
			}
			else { // game_object->type == TypeSkybox
				if (ImGui::CollapsingHeader("Skybox Information", ImGuiTreeNodeFlags_DefaultOpen)) {
					if (ImGui::BeginTable("SkyboxTable", 2, ImGuiTableFlags_Resizable | ImGuiTableFlags_RowBg)) {
						//Row: Skybox name
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Skybox name");

						ImGui::TableSetColumnIndex(1);
						ImGui::PushItemWidth(-1);
						if (ImGui::BeginCombo("##SkyboxName", ((Skybox*)game_object)->cubemap_name.c_str()))
						{
							for (auto it = render->cubemap->umap_name_to_cubemap_data.begin(); it != render->cubemap->umap_name_to_cubemap_data.end(); it++) {
								const bool is_selected = (((Skybox*)game_object)->cubemap_name == it->first);

								if (ImGui::Selectable(it->first.c_str(), is_selected)) {
									((Skybox*)game_object)->cubemap_name = it->first;
								}
								if (is_selected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndCombo();
						}

						//Row: Is skybox HDRI
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Is HDRI");

						ImGui::TableSetColumnIndex(1);
						ImGui::Checkbox("##IsHDRI", &(render->cubemap->umap_name_to_cubemap_data[((Skybox*)game_object)->cubemap_name].is_hdri));

						// Row: Cubemap texture type
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Type");

						ImGui::TableSetColumnIndex(1);
						if (ImGui::BeginCombo("##CubemapTextureType", cubemap_texture_type_to_string(render->cubemap_texture_type).c_str()))
						{
							for (int i = EnvironmentMap; i <= PrefilterMap; i++) {
								const bool is_selected = (render->cubemap_texture_type == i);

								if (ImGui::Selectable(cubemap_texture_type_to_string((CubemapTextureType)i).c_str(), is_selected)) {
									render->cubemap_texture_type = (CubemapTextureType)i;
								}
								if (is_selected) {
									ImGui::SetItemDefaultFocus();
								}
							}
							ImGui::EndCombo();
						}

						// Row: Cubemap texture mipmap level
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::Text("Mipmap level");

						ImGui::TableSetColumnIndex(1);
						ImGui::SliderFloat("##CubemapTextureMipmapLevel", &(render->cubemap_texture_mipmap_level), 0.0f, 4.0f);

						ImGui::PopItemWidth();

						ImGui::EndTable();
					}
				}
			}
		}
	}

	void UserInterface::clean_imgui() {
		ImGui_ImplOpenGL3_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void UserInterface::GetApplicationInfo()
	{
		app->glInfos = (char*)glGetString(GL_VERSION);
		app->hardwareInfos = (char*)glGetString(GL_RENDERER);
	}

	void UserInterface::ImportModelPanel()
	{
		if (!showImportModelPanel)
		{
			return;
		}
		int width = ImGui::GetWindowWidth() / 3;
		int height = ImGui::GetWindowHeight() / 8;
		ImGui::SetNextWindowPos(ImVec2((ImGui::GetWindowWidth() - width) / 2, (ImGui::GetWindowHeight() - height) / 2), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Appearing);
		{
			ImGui::Begin("Import Model");
			static char model_path[128];
			strcpy_s(model_path, import_model_path.string().c_str());
			static std::string info = "";
			ImGui::InputText("Model Path", model_path, IM_ARRAYSIZE(model_path));
			ImGui::SameLine();
			if (ImGui::Button("..."))
			{
				import_model_path = FileSystem::GetContentPath();
				file_path = &import_model_path;
				showFileBrowser = true;
			}
			if (ImGui::Button("Cancel"))
			{
				showImportModelPanel = false;
				strcpy_s(model_path, import_model_path.string().c_str());
			}
			ImGui::SameLine();
			if (ImGui::Button("Confirm"))
			{
				std::string path_s = model_path;
				std::replace(path_s.begin(), path_s.end(), '\\', '/');
				int i = path_s.find_last_of('/');
				int j = path_s.find_last_of('.');
				std::string objname;
				if (i != std::string::npos || j != std::string::npos)
				{
					objname = path_s.substr(i + 1, j - i - 1);
				}
				Model* new_model = new Model(objname, path_s, false, false);
				render->add_model_to_loaded_data(new_model);
				render->OnAddmodel2Scene();
				if (new_model->meshes.size() == 0)
				{
					info = "Load failed";
				}
				else
				{
					showImportModelPanel = false;
					strcpy_s(model_path, import_model_path.string().c_str());
				}
			}
			ImGui::Text(info.c_str());
			ImGui::End();
		}
	}

	/*********************
* File Browser
**********************/
	namespace fs = std::filesystem;
	void UserInterface::FileBrowser(std::filesystem::path* _path)
	{
		if (!showFileBrowser)
		{
			return;
		}
		int width = ImGui::GetWindowWidth() / 3;
		int height = ImGui::GetWindowHeight() / 5;
	
		ImGui::SetNextWindowPos(ImVec2((ImGui::GetWindowWidth() - width) / 2, (ImGui::GetWindowHeight() - height) / 2), ImGuiCond_Appearing);
		ImGui::SetNextWindowSize(ImVec2(width, height), ImGuiCond_Appearing);

		ImGui::Begin("File Browser");
		ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 5.0f);

		if (ImGui::Button(".."))
		{
			*_path = _path->parent_path();
		}
		ImGui::SameLine();
		std::vector<fs::path> roots = FileSystem::GetRootPaths();
		int root_num = roots.size();
		static int root_idx = 0;
		if (ImGui::BeginCombo("root paths", _path->string().c_str()))
		{
			for (int n = 0; n < root_num; n++)
			{
				const bool is_selected = (root_idx == n);
				if (ImGui::Selectable(roots[n].string().c_str(), is_selected))
				{
					root_idx = n;
					*_path = fs::path(roots[n].string().c_str());
				}

				// Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
				if (is_selected)
					ImGui::SetItemDefaultFocus();
			}
			ImGui::EndCombo();
		}
		std::vector<fs::directory_entry> files;
		for (auto const& dir_entry : fs::directory_iterator{ *_path })
		{
			files.push_back(dir_entry);
		}
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_HorizontalScrollbar;
		ImGui::BeginChild("FileList", ImVec2(width - 50, height - 100), ImGuiChildFlags_Border, window_flags);
		static int selected_file = -1;
		for (int i = 0; i < files.size(); i++)
		{
			std::string filename = files[i].path().filename().string();
			float width = 16;
			float height = 16;
			ImVec2 uv_min = ImVec2(0.0f, 0.0f);                        // Top-left
			ImVec2 uv_max = ImVec2(1.0f, 1.0f);                        // Lower-right
			ImVec4 tint_col = ImGui::GetStyleColorVec4(ImGuiCol_Text); // No tint
			if (fs::is_directory(files[i]))
			{
				for (auto preset : presetImages)
				{
					if (preset.filetype == Folder)
					{
						ImGui::Image((void*)preset.texture, ImVec2(width, height), uv_min, uv_max, tint_col);
					}
				}
			}
			else
			{
				for (auto preset : presetImages)
				{
					if (preset.filetype == File)
					{
						ImGui::Image((void*)preset.texture, ImVec2(width, height), uv_min, uv_max, tint_col);
					}
				}
			}
			ImGui::SameLine();
			if (ImGui::Selectable(filename.c_str(), selected_file == i))
			{
				if (selected_file == i)
				{
					if (fs::is_directory(files[selected_file]))
					{
						*_path = files[selected_file].path();
					}
					else
					{
						*_path = files[selected_file].path();
						showFileBrowser = false;
					}
					selected_file = -1;
				}
				else
				{
					selected_file = i;
				}
			}
		}
		ImGui::EndChild();
		if (ImGui::Button("Cancel"))
		{
			showFileBrowser = false;
		}
		ImGui::SameLine();
		if (ImGui::Button("Confirm"))
		{
			if (selected_file != -1)
			{
				if (fs::is_directory(files[selected_file]))
				{
					*_path = files[selected_file].path();
				}
				else
				{
					*_path = files[selected_file].path();
					showFileBrowser = false;
				}
				selected_file = -1;
			}
			else
			{
				showFileBrowser = false;
			}
		}
		ImGui::PopStyleVar();
		ImGui::End();
	}
	/////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	std::string displayed_rendering_to_string(DisplayedRendering displayed_render) {
		if (displayed_render == DisplayedColors) {
			return "DisplayedColors";
		}
		else if (displayed_render == DisplayedIdColors) {
			return "DisplayedIdColors";
		}
		else if (displayed_render == DisplayedIdColorsTransform3d) {
			return "DisplayedIdColorsTransform3d";
		}
		else if (displayed_render == DisplayedSelectedColors) {
			return "DisplayedSelectedColors";
		}
		else if (displayed_render == DisplayedBRDFLut) {
			return "DisplayedBRDFLut";
		}
		else if (displayed_render == DisplayedHDRBrightColors) {
			return "DisplayedHDRBrightColors";
		}
		else if (displayed_render == DisplayedBloom) {
			return "DisplayedBloom";
		}
		else {
			return "DisplayedColors";
		}
	}
}