#pragma once

#include "types.h"

// modelled after reference_wrapper (http://en.cppreference.com/w/cpp/utility/functional/reference_wrapper)
template <typename T, int I>
class BitRef {
	public:
		BitRef(T& ref) : m_ptr{ref} {}
		BitRef(const BitRef&) = default;
		BitRef& operator=(const BitRef&) = default;
		BitRef(T&& ref) : m_ptr{ref} {}

		operator bool () const {
			return (((m_ptr >> I) & 1) != 0);
		}
		// TODO: return type?
		void operator=(bool rhs) {
			m_ptr = static_cast<BYTE>((m_ptr & (~(1 << I))) | (rhs << I));
		}
	private:
		T& m_ptr;
};
