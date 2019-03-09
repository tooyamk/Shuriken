#include "math/Math.h"
#include "math/Matrix44.h"
#include "math/Vector3.h"

AE_NS_BEGIN

inline void Matrix34::transpose(Matrix44& dst) const {
	Math::transposeMat(m34, dst.m44);
}

inline bool Matrix34::invert() {
	return Math::invertMat(m34, m34);
}

inline bool Matrix34::invert(Matrix34& dst) const {
	return Math::invertMat(m34, dst.m34);
}

inline bool Matrix34::invert(Matrix44& dst) const {
	return Math::invertMat(m34, ((Matrix34&)dst).m34);
}

inline void Matrix34::append(const Matrix34& rhs) {
	append(rhs, *this);
}

inline void Matrix34::append(const Matrix44& rhs) {
	append(rhs, *this);
}

inline void Matrix34::append(const Matrix34& rhs, Matrix34& dst) const {
	Math::appendMat(m34, rhs.m34, dst.m34);
}

inline void Matrix34::append(const Matrix34& rhs, Matrix44& dst) const {
	Math::appendMat(m34, rhs.m34, dst.m44);
}

inline void Matrix34::append(const Matrix44& rhs, Matrix34& dst) const {
	Math::appendMat(m34, rhs.m44, dst.m34);
}

inline void Matrix34::append(const Matrix44& rhs, Matrix44& dst) const {
	Math::appendMat(m34, rhs.m44, dst.m44);
}

inline void Matrix34::appendTranslate(const Vector3& t) {
	m34[0][3] += t.x;
	m34[1][3] += t.y;
	m34[2][3] += t.z;
}

inline void Matrix34::prependTranslate(const Vector3& t) {
	m34[0][3] += t.x * m34[0][0] + t.y * m34[0][1] + t.z * m34[0][2];
	m34[1][3] += t.x * m34[1][0] + t.y * m34[1][1] + t.z * m34[1][2];
	m34[2][3] += t.x * m34[2][0] + t.y * m34[2][1] + t.z * m34[2][2];
}

inline void Matrix34::setPosition(const Vector3& p) {
	m34[0][3] = p.x;
	m34[1][3] = p.y;
	m34[2][3] = p.z;
}

inline void Matrix34::setPosition(const f32* p) {
	m34[0][3] = p[0];
	m34[1][3] = p[1];
	m34[2][3] = p[2];
}

inline void Matrix34::setPosition(const Matrix34& m) {
	auto mm = m.m34;

	m34[0][3] = mm[0][3];
	m34[1][3] = mm[1][3];
	m34[2][3] = mm[2][3];
}

inline void Matrix34::setPosition(const Matrix44& m) {
	auto mm = m.m44;

	m34[0][3] = mm[0][3];
	m34[1][3] = mm[1][3];
	m34[2][3] = mm[2][3];
}

inline void Matrix34::prependScale(const Vector3& t) {
	m34[0][0] *= t.x;
	m34[0][1] *= t.y;
	m34[0][2] *= t.z;

	m34[1][0] *= t.x;
	m34[1][1] *= t.y;
	m34[1][2] *= t.z;

	m34[2][0] *= t.x;
	m34[2][1] *= t.y;
	m34[2][2] *= t.z;
}

AE_NS_END