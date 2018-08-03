#include "App.h"

App _app;

// main
int main() {
	_app.run();
	return 0;
}

// App
App::App()
{
	init();
}

App::~App()
{
}

void App::init()
{
	// This is where most initializtion for a program should be performed
}

void App::onResize()
{
	_graphics.onResize();
}

void App::run()
{
	// Poll for user input.
	_shouldRun = true;
	while (_shouldRun) {

		while (SDL_PollEvent(&_event)) {

			switch (_event.type) {

			case SDL_QUIT:
				_shouldRun = false;
				break;
			case SDL_WINDOWEVENT:
				switch (_event.window.event) {
				case SDL_WINDOWEVENT_RESIZED:
					onResize();
					break;
				}
				break;
			default:
				// Do nothing.
				break;
			}
		}
		SDL_Delay(10);
	}
}