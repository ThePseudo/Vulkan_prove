#include "Window.h"

Window::Window()
{
	_resolution = Resolution(1280, 720);
}

Window::~Window()
{
	SDL_DestroyWindow(_win);
	_win = nullptr;
	ZeroMemory(&_info, sizeof(_info));
	SDL_Quit();
}

void Window::createWindow()
{

	// Create an SDL window that supports Vulkan and OpenGL rendering.
	if (SDL_Init(SDL_INIT_VIDEO) != 0) {
		std::cout << "Could not initialize SDL." << std::endl;
		exit(1);
	}
	_win = SDL_CreateWindow("Vulkan Window", SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED, _resolution.width, _resolution.height, SDL_WINDOW_VULKAN );
	if (_win == NULL) {
		std::cout << "Could not create SDL window." << std::endl;
		exit(1);
	}

	//creating window info
	SDL_SysWMinfo windowInfo;
	SDL_VERSION(&windowInfo.version);
	if (!SDL_GetWindowWMInfo(_win, &windowInfo)) {
		throw std::system_error(std::error_code(), "SDK window manager info is not available.");
	}
	_info = windowInfo;
}