#pragma once

// modelled after reference_wrapper (http://en.cppreference.com/w/cpp/utility/functional/reference_wrapper)
template <typename T, int I>
class BitRef {
	public:
		BitRef(T& ref) : ptr{&ref} {}
		BitRef(const BitRef&) = default;
		BitRef& operator=(const BitRef&) = default;
		operator bool () const {
			return (((*ptr >> I) & 1) != 0);
		}
		// TODO: return type?
		void operator=(bool rhs) {
			*ptr = (*ptr & (~(1 << I))) | static_cast<T>(rhs << I);
		}
	private:
		T* ptr;
};
