#include "aurora/math/Math.h"
#include "aurora/math/Matrix34.h"

namespace aurora {
	inline AE_CALL Matrix44::operator Matrix44::Data34& () {
		return (Data34&)data;
	}

	inline AE_CALL Matrix44::operator const Matrix44::Data34& () const {
		return (Data34&)data;
	}

	inline AE_CALL Matrix44::operator Matrix44::Data44& () {
		return data;
	}

	inline AE_CALL Matrix44::operator const Matrix44::Data44& () const {
		return data;
	}

	inline void AE_CALL Matrix44::transpose() {
		Math::transposeMat(data, this->data);
	}

	inline void AE_CALL Matrix44::transpose(Matrix44& dst) const {
		Math::transposeMat(data, dst.data);
	}

	inline bool AE_CALL Matrix44::invert() {
		return Math::invertMat(data, data);
	}

	inline bool AE_CALL Matrix44::invert(Matrix44& dst) const {
		return Math::invertMat(data, dst.data);
	}

	inline void AE_CALL Matrix44::append(const Matrix34& rhs) {
		append(rhs, *this);
	}

	inline void AE_CALL Matrix44::append(const Matrix44& rhs) {
		append(rhs, *this);
	}

	inline void AE_CALL Matrix44::append(const Matrix34& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void AE_CALL Matrix44::append(const Matrix34& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void AE_CALL Matrix44::append(const Matrix44& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void AE_CALL Matrix44::append(const Matrix44& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}
}