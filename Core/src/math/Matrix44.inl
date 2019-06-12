#include "math/Math.h"
#include "math/Matrix34.h"

namespace aurora {
	inline void Matrix44::transpose() {
		Math::transposeMat(data, this->data);
	}

	inline void Matrix44::transpose(Matrix44& dst) const {
		Math::transposeMat(data, dst.data);
	}

	inline bool Matrix44::invert() {
		return Math::invertMat(data, data);
	}

	inline bool Matrix44::invert(Matrix44& dst) const {
		return Math::invertMat(data, dst.data);
	}

	inline void Matrix44::append(const Matrix34& rhs) {
		append(rhs, *this);
	}

	inline void Matrix44::append(const Matrix44& rhs) {
		append(rhs, *this);
	}

	inline void Matrix44::append(const Matrix34& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void Matrix44::append(const Matrix34& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void Matrix44::append(const Matrix44& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void Matrix44::append(const Matrix44& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}
}