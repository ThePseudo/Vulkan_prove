#pragma once

// Tell SDL not to mess with main()
#define SDL_MAIN_HANDLED

#include <glm/glm.hpp>

#include <iostream>
#include <cstdlib>

#include "VkGraphics.h"

class App
{
public:
	App();
	~App();
	void run();

private:
	void init();
	void onResize();

	VkGraphics	_graphics;
	SDL_Event	_event;
	bool		_shouldRun;

};