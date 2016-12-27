#pragma once

template <typename T>
class Ref {
	public:
		virtual ~Ref() = default;
		virtual operator T () const = 0;
		virtual void operator=(T) = 0;
};
