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
		SDL_Window* m_window = nullptr;
		SDL_Renderer* m_renderer = nullptr;
		SDL_Texture* m_texture = nullptr;
};
