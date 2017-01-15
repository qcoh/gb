#pragma once

#include <array>
#include <cstdint>
#include <SDL2/SDL.h>

class Display {
	public:
		using PixelArray = std::array<uint32_t, 160 * 144>;
		Display();
		void render(PixelArray&);
		~Display();
	private:
		SDL_Window* window = nullptr;
		SDL_Renderer* renderer = nullptr;
		SDL_Texture* texture = nullptr;
};
