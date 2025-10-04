// chip8-emulator.cpp : Defines the entry point for the application.
//

#include "chip8-emulator.h"
#include <SDL.h>

using namespace std;

int main(int argc, char *argv[])
{
	SDL_Init(SDL_INIT_VIDEO);
	cout << "Hello CMake." << endl;

	SDL_Quit();
	return 0;
}
