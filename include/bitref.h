#pragma once

#include "ref.h"

// modelled after reference_wrapper (http://en.cppreference.com/w/cpp/utility/functional/reference_wrapper)
template <typename T, int I>
class BitRef : public Ref<bool> {
	public:
		BitRef(T& ref) : ptr{&ref} {}
		BitRef(const BitRef&) = default;
		BitRef& operator=(const BitRef&) = default;
		virtual operator bool () const override {
			return (((*ptr >> I) & 1) != 0);
		}
		// TODO: return type?
		virtual void operator=(bool rhs) override {
			*ptr = (*ptr & (~(1 << I))) | static_cast<T>(rhs << I);
		}
	private:
		T* ptr;
};
