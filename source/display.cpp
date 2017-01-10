#include <stdexcept>
#include "display.h"

Display::Display() : window{nullptr}, renderer{nullptr}, texture{nullptr} {
	if ((window = SDL_CreateWindow("gb", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160, 144, 0)) == nullptr) {
		throw std::runtime_error{SDL_GetError()};
	}
	if ((renderer = SDL_CreateRenderer(window, -1, 0)) == nullptr) {
		SDL_DestroyWindow(window);
		throw std::runtime_error{SDL_GetError()};
	}
	if ((texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 160, 140)) == nullptr) {
		SDL_DestroyRenderer(renderer);
		SDL_DestroyWindow(window);
		throw std::runtime_error{SDL_GetError()};
	}		
}

Display::~Display() {
	if (texture != nullptr) {
		SDL_DestroyTexture(texture);
	}
	if (renderer != nullptr) {
		SDL_DestroyRenderer(renderer);
	}
	if (window != nullptr) {
		SDL_DestroyWindow(window);
	}
}

void Display::render(PixelArray& pixel) {
	SDL_RenderClear(renderer);
	SDL_UpdateTexture(texture, nullptr, pixel.data(), 160 * 4);
	SDL_RenderCopy(renderer, texture, nullptr, nullptr);
	SDL_RenderPresent(renderer);
}
