#include "math/Math.h"
#include "math/Matrix34.h"

namespace aurora {
	inline void Matrix44::transpose() {
		Math::transposeMat(m44, this->m44);
	}

	inline void Matrix44::transpose(Matrix44& dst) const {
		Math::transposeMat(m44, dst.m44);
	}

	inline bool Matrix44::invert() {
		return Math::invertMat(m44, m44);
	}

	inline bool Matrix44::invert(Matrix44& dst) const {
		return Math::invertMat(m44, dst.m44);
	}

	inline void Matrix44::append(const Matrix34& rhs) {
		append(rhs, *this);
	}

	inline void Matrix44::append(const Matrix44& rhs) {
		append(rhs, *this);
	}

	inline void Matrix44::append(const Matrix34& rhs, Matrix34& dst) const {
		Math::appendMat(m44, rhs.m34, dst.m34);
	}

	inline void Matrix44::append(const Matrix34& rhs, Matrix44& dst) const {
		Math::appendMat(m44, rhs.m34, dst.m44);
	}

	inline void Matrix44::append(const Matrix44& rhs, Matrix34& dst) const {
		Math::appendMat(m44, rhs.m44, dst.m34);
	}

	inline void Matrix44::append(const Matrix44& rhs, Matrix44& dst) const {
		Math::appendMat(m44, rhs.m44, dst.m44);
	}
}