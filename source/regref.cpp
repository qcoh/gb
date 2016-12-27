#include "regref.h"

RegRef::RegRef(BYTE& reg_) : reg{reg_} {}

RegRef::operator BYTE () const {
	return reg;
}

void RegRef::operator=(BYTE rhs) {
	reg = rhs;
}
