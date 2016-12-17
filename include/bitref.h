#pragma once

#include <functional>

// modelled after reference_wrapper (http://en.cppreference.com/w/cpp/utility/functional/reference_wrapper)
template <typename T, int I>
class BitRef {
	public:
		BitRef(T& ref) noexcept : ptr{std::addressof(ref)} {}
		BitRef(T&&) = delete;
		BitRef(const BitRef&) noexcept = default;
		BitRef& operator=(const BitRef&) noexcept = default;
		operator bool () const noexcept {
			return (((*ptr >> I) & 1) != 0);
		}
		// TODO: return type?
		void operator=(bool rhs) noexcept {
			*ptr = (*ptr & (~(1 << I))) | static_cast<T>(rhs << I);
		}
	private:
		T* ptr;
};
