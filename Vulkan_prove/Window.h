#pragma once

#include <SDL2/SDL.h>
#include <SDL2/SDL_syswm.h>
#include <iostream>

#include "Resolution.h"

class Window
{
public:
	Window();
	~Window();
	void createWindow();
	inline SDL_Window* window() { return _win; }
	inline SDL_SysWMinfo info() { return _info; }
	inline Resolution resolution() { return _resolution; }
private:
	SDL_Window		*_win;
	SDL_SysWMinfo	_info;
	Resolution		_resolution;
};

