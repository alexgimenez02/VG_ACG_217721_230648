#include "application.h"
#include "utils.h"
#include "mesh.h"
#include "texture.h"
#include "volume.h"
#include "fbo.h"
#include "shader.h"
#include "input.h"
#include "animation.h"
#include "extra/hdre.h"
#include "extra/imgui/imgui.h"
#include "extra/imgui/imgui_impl_sdl.h"
#include "extra/imgui/imgui_impl_opengl3.h"

#include <cmath>

bool render_wireframe = false;
Camera* Application::camera = nullptr;
Application* Application::instance = NULL;
bool jitter = false;
Application::Application(int window_width, int window_height, SDL_Window* window)
{
	this->window_width = window_width;
	this->window_height = window_height;
	this->window = window;
	instance = this;
	must_exit = false;
	render_debug = true;

	fps = 0;
	frame = 0;
	volume_selected = 0;
	method = 1;
	prev_volume = 0;
	time = 0.0f;
	elapsed_time = 0.0f;
	mouse_locked = false;

	// OpenGL flags
	glEnable( GL_CULL_FACE ); //render both sides of every triangle
	glEnable( GL_DEPTH_TEST ); //check the occlusions using the Z buffer

	// Create camera
	camera = new Camera();
	camera->lookAt(Vector3(5.f, 5.f, 5.f), Vector3(0.f, 0.0f, 0.f), Vector3(0.f, 1.f, 0.f));
	camera->setPerspective(45.f, window_width/(float)window_height, 0.1f, 10000.f); //set the projection, we want to be perspective

	brightness = 1.0f;
	ray_step = 0.001f;
	alpha_filter = 0.01f;
	{

		//Added
		// Construct volume node
		SceneNode* volNode = new SceneNode("Volume node");
		
		// Create the volume
		Volume* volume = new Volume();
		volNode->mesh = new Mesh();
		volNode->mesh->createCube(); //Create the mesh
		volume->loadPVM("data/volumes/CT-Abdomen.pvm"); //Add the PVM/PNG/VL	

		//Create texture
		Texture* tex = new Texture();
		tex->create3DFromVolume(volume, GL_CLAMP_TO_EDGE); //Create texture from volume
		
		//Create material
		StandardMaterial* stdMat = new StandardMaterial();
		stdMat->shader = Shader::Get("data/shaders/basic.vs", "data/shaders/volshader.fs"); //Add volume shader to the material
		stdMat->texture = tex; //Add texture
		stdMat->noise_texture = Texture::Get("data/images/blueNoise.PNG");
		//Brightness and ray_step setup
		{
			stdMat->brightness = brightness; 
			stdMat->ray_step = ray_step;
			stdMat->alpha_filter = alpha_filter;
			stdMat->jitter = jitter;
			stdMat->method = method;
			stdMat->jitterMethodb = true;
		}
		//Add material to the volume node
		volNode->material = stdMat;
		//Scale the model matrix so it fits the whole volume
		volNode->model.setScale(1, (volume->height * volume->heightSpacing) / (volume->width * volume->widthSpacing), (volume->depth * volume->depthSpacing) / (volume->width * volume->widthSpacing));
		node_list.push_back(volNode); //Add the node to the list of nodes
	}
	//hide the cursor
	SDL_ShowCursor(!mouse_locked); //hide or show the mouse
}

// what to do when the image has to be draw
void Application::render(void)
{
	// set the clear color (the background color)
	glClearColor(0.1, 0.1, 0.1, 1.0);

	// Clear the window and the depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	// set the camera as default
	camera->enable();

	// set flags
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	for (size_t i = 0; i < node_list.size(); i++) {
		node_list[i]->render(camera);

		if(render_wireframe)
			node_list[i]->renderWireframe(camera);
	}

	//Draw the floor grid
	if(render_debug)
		drawGrid();
}

void Application::update(double seconds_elapsed)
{
	float speed = seconds_elapsed * 10; //the speed is defined by the seconds_elapsed so it goes constant
	float orbit_speed = seconds_elapsed * 1;

	// example
	float angle = seconds_elapsed * 10.f * DEG2RAD;
	/*for (int i = 0; i < root.size(); i++) {
		root[i]->model.rotate(angle, Vector3(0,1,0));
	}*/

	// mouse input to rotate the cam
	if ((Input::mouse_state & SDL_BUTTON_LEFT && !ImGui::IsAnyWindowHovered()
		&& !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())) //is left button pressed?
	{
		camera->orbit(-Input::mouse_delta.x * orbit_speed, Input::mouse_delta.y * orbit_speed);
	}

	// async input to move the camera around
	if (Input::isKeyPressed(SDL_SCANCODE_LSHIFT)) speed *= 10; //move faster with left shift
	if (Input::isKeyPressed(SDL_SCANCODE_W) || Input::isKeyPressed(SDL_SCANCODE_UP)) camera->move(Vector3(0.0f, 0.0f, 1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_S) || Input::isKeyPressed(SDL_SCANCODE_DOWN)) camera->move(Vector3(0.0f, 0.0f, -1.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_A) || Input::isKeyPressed(SDL_SCANCODE_LEFT)) camera->move(Vector3(1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_D) || Input::isKeyPressed(SDL_SCANCODE_RIGHT)) camera->move(Vector3(-1.0f, 0.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_SPACE)) camera->moveGlobal(Vector3(0.0f, -1.0f, 0.0f) * speed);
	if (Input::isKeyPressed(SDL_SCANCODE_LCTRL)) camera->moveGlobal(Vector3(0.0f, 1.0f, 0.0f) * speed);

	// to navigate with the mouse fixed in the middle
	if (mouse_locked)
		Input::centerMouse();

	for (auto& node : node_list) {
		if(node->material->brightness != brightness) node->material->brightness = brightness;
		if(node->material->ray_step != ray_step) node->material->ray_step = ray_step;
		if(node->material->alpha_filter != alpha_filter) node->material->alpha_filter = alpha_filter;
		if (volume_selected != prev_volume) 
			node->swapVolume(volume_selected);
		if (node->material->method != method) {
			node->material->jitterMethodb = !node->material->jitterMethodb;
			node->material->method = method;
		}
		if (node->material->jitter != jitter) node->material->jitter = jitter;
	}
	prev_volume = volume_selected;
}

// Keyboard event handler (sync input)
void Application::onKeyDown(SDL_KeyboardEvent event)
{
	switch (event.keysym.sym)
	{
	case SDLK_ESCAPE: must_exit = true; break; //ESC key, kill the app
	case SDLK_F1: render_debug = !render_debug; break;
	case SDLK_F2: render_wireframe = !render_wireframe; break;
	case SDLK_F5: Shader::ReloadAll(); break;
	}
}

void Application::onKeyUp(SDL_KeyboardEvent event)
{
}

void Application::onGamepadButtonDown(SDL_JoyButtonEvent event)
{

}

void Application::onGamepadButtonUp(SDL_JoyButtonEvent event)
{

}

void Application::onMouseButtonDown(SDL_MouseButtonEvent event)
{
	if (event.button == SDL_BUTTON_MIDDLE) //middle mouse
	{
		mouse_locked = !mouse_locked;
		SDL_ShowCursor(!mouse_locked);
	}
}

void Application::onMouseButtonUp(SDL_MouseButtonEvent event)
{
}

void Application::onMouseWheel(SDL_MouseWheelEvent event)
{
	ImGuiIO& io = ImGui::GetIO();
	switch (event.type)
	{
	case SDL_MOUSEWHEEL:
	{
		if (event.x > 0) io.MouseWheelH += 1;
		if (event.x < 0) io.MouseWheelH -= 1;
		if (event.y > 0) io.MouseWheel += 1;
		if (event.y < 0) io.MouseWheel -= 1;
	}
	}

	if (!ImGui::IsAnyWindowHovered() && event.y)
		camera->changeDistance(event.y * 0.5);
}

void Application::onResize(int width, int height)
{
	std::cout << "window resized: " << width << "," << height << std::endl;
	glViewport(0, 0, width, height);
	camera->aspect = width / (float)height;
	window_width = width;
	window_height = height;
}

void Application::onFileChanged(const char* filename)
{
	Shader::ReloadAll();
}

void Application::renderInMenu() {

	if (ImGui::TreeNode("Scene")) {
		//
		ImGui::TreePop();
	}

	if (ImGui::TreeNode("Camera")) {
		camera->renderInMenu();
		ImGui::TreePop();
	}

	//Scene graph
	if (ImGui::TreeNode("Entities"))
	{
		unsigned int count = 0;
		std::stringstream ss;
		for (auto& node : node_list)
		{
			ss << count;
			if (ImGui::TreeNode(node->name.c_str()))
			{
				node->renderInMenu();
				ImGui::TreePop();
			}
			++count;
			ss.str("");
		}
		ImGui::TreePop();
	}

	//ImGUI Sliders and selectors
	//Added
	{ //Part 1
		ImGui::SliderFloat("Brightness", &brightness, 0.0f, 15.0f);
		ImGui::SliderFloat("Step vector", &ray_step, 0.001f, 1.00f);
		ImGui::SliderFloat("Alpha filter", &alpha_filter, 0.01f, 0.1f);

		bool changed = false;
		changed |= ImGui::Combo("Volume", (int*)&volume_selected, "CT-ABDOMEN\0DAISY\0ORANGE\0BONSAI\0FOOT\0TEAPOT");
	}
	if(ImGui::TreeNode("Jittering")) { //Part 2

		ImGui::Checkbox("Activate", (bool*) & jitter);
		ImGui::Combo("Method", (int*)&method, "Blue Noise Texture\0Pseudorandom-looking\0");
		ImGui::TreePop();
	}

		ImGui::Checkbox("Render debug", &render_debug);
		ImGui::Checkbox("Wireframe", &render_wireframe);
		if (ImGui::Button("Print fs shader")) {
			for each (auto& node in node_list)
			{
				cout << node->material->shader->getPixelShaderName() << endl;
			}
		}

}
