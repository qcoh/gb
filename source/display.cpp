#include <stdexcept>
#include "display.h"

Display::Display() {
	if ((m_window = SDL_CreateWindow("gb", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, 160, 144, 0)) == nullptr) {
		throw std::runtime_error{SDL_GetError()};
	}
	if ((m_renderer = SDL_CreateRenderer(m_window, -1, 0)) == nullptr) {
		SDL_DestroyWindow(m_window);
		throw std::runtime_error{SDL_GetError()};
	}
	if ((m_texture = SDL_CreateTexture(m_renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STATIC, 160, 140)) == nullptr) {
		SDL_DestroyRenderer(m_renderer);
		SDL_DestroyWindow(m_window);
		throw std::runtime_error{SDL_GetError()};
	}		
}

Display::~Display() {
	if (m_texture != nullptr) {
		SDL_DestroyTexture(m_texture);
	}
	if (m_renderer != nullptr) {
		SDL_DestroyRenderer(m_renderer);
	}
	if (m_window != nullptr) {
		SDL_DestroyWindow(m_window);
	}
}

void Display::render(PixelArray& pixel) {
	SDL_RenderClear(m_renderer);
	SDL_UpdateTexture(m_texture, nullptr, pixel.data(), 160 * 4);
	SDL_RenderCopy(m_renderer, m_texture, nullptr, nullptr);
	SDL_RenderPresent(m_renderer);
}
