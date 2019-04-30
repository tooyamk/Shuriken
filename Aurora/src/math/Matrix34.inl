#include "math/Math.h"
#include "math/Matrix44.h"

namespace aurora {
	inline void Matrix34::transpose(Matrix44& dst) const {
		Math::transposeMat(data, dst.data);
	}

	inline bool Matrix34::invert() {
		return Math::invertMat(data, data);
	}

	inline bool Matrix34::invert(Matrix34& dst) const {
		return Math::invertMat(data, dst.data);
	}

	inline bool Matrix34::invert(Matrix44& dst) const {
		return Math::invertMat(data, ((Matrix34&)dst).data);
	}

	inline void Matrix34::append(const Matrix34& rhs) {
		append(rhs, *this);
	}

	inline void Matrix34::append(const Matrix44& rhs) {
		append(rhs, *this);
	}

	inline void Matrix34::append(const Matrix34& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void Matrix34::append(const Matrix34& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void Matrix34::append(const Matrix44& rhs, Matrix34& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void Matrix34::append(const Matrix44& rhs, Matrix44& dst) const {
		Math::appendMat(data, rhs.data, dst.data);
	}

	inline void Matrix34::appendTranslate(const f32(&t)[3]) {
		data[0][3] += t[0];
		data[1][3] += t[1];
		data[2][3] += t[2];
	}

	inline void Matrix34::prependTranslate(const f32(&t)[3]) {
		auto x = t[0], y = t[1], z = t[2];
		data[0][3] += x * data[0][0] + y * data[0][1] + z * data[0][2];
		data[1][3] += x * data[1][0] + y * data[1][1] + z * data[1][2];
		data[2][3] += x * data[2][0] + y * data[2][1] + z * data[2][2];
	}

	inline void Matrix34::setPosition(const f32(&p)[3]) {
		setPosition(p[0], p[1], p[2]);
	}

	inline void Matrix34::setPosition(const Matrix34& m) {
		auto mm = m.data;
		setPosition(mm[0][3], mm[1][3], mm[2][3]);
	}

	inline void Matrix34::setPosition(const Matrix44& m) {
		auto mm = m.data;
		setPosition(mm[0][3], mm[1][3], mm[2][3]);
	}

	inline void Matrix34::setPosition(f32 x, f32 y, f32 z) {
		data[0][3] = x;
		data[1][3] = y;
		data[2][3] = z;
	}

	inline void Matrix34::prependScale(const f32(&s)[3]) {
		auto x = s[0], y = s[1], z = s[2];

		data[0][0] *= x;
		data[0][1] *= y;
		data[0][2] *= z;

		data[1][0] *= x;
		data[1][1] *= y;
		data[1][2] *= z;

		data[2][0] *= x;
		data[2][1] *= y;
		data[2][2] *= z;
	}
}