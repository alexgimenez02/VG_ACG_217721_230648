/*  by Javi Agenjo 2013 UPF  javi.agenjo@gmail.com
	This class encapsulates the game, is in charge of creating the game, getting the user input, process the update and render.
*/

#ifndef APPLICATION_H
#define APPLICATION_H

#include "includes.h"
#include "camera.h"
#include "utils.h"
#include "scenenode.h"
#include "material.h"


class Application
{
public:
	static Application* instance;

	std::vector <SceneNode*> node_list;

	// window
	SDL_Window* window;
	int window_width;
	int window_height;

	// some globals
	long frame;
	float time;
	float elapsed_time;
	float brightness;
	float ray_step;
	float alpha_filter;
	float tf_filter;
	float h_value;
	float light_intensity;
	unsigned int volume_selected;
	unsigned int prev_volume;
	unsigned int method;
	unsigned int tf_selected;
	unsigned int prev_tf_texture;
	int fps;
	bool must_exit;
	bool render_debug;
	bool tf;
	bool vc;
	bool iso;
	bool phong;
	Plane pl;
	bool jitter;
	bool show_light;
	float threshold;

	Vector3 light_position;

	// some vars
	static Camera* camera; //our GLOBAL camera
	bool mouse_locked; //tells if the mouse is locked (not seen)

	// constructor
	Application(int window_width, int window_height, SDL_Window* window);

	// main functions
	void render(void);
	void update(double dt);

	// events
	void onKeyDown(SDL_KeyboardEvent event);
	void onKeyUp(SDL_KeyboardEvent event);
	void onMouseButtonDown(SDL_MouseButtonEvent event);
	void onMouseButtonUp(SDL_MouseButtonEvent event);
	void onMouseWheel(SDL_MouseWheelEvent event);
	void onGamepadButtonDown(SDL_JoyButtonEvent event);
	void onGamepadButtonUp(SDL_JoyButtonEvent event);
	void onResize(int width, int height);
	void onFileChanged(const char* filename);

	void renderInMenu();
};


#endif 