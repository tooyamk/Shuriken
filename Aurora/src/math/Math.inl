namespace aurora {
	inline void Math::appendQuat(const f32* lhs, const f32* rhs, f32* dst) {
		auto w = lhs[3] * rhs[3] - lhs[0] * rhs[0] - lhs[1] * rhs[1] - lhs[2] * rhs[2];
		auto x = lhs[0] * rhs[3] + lhs[3] * rhs[0] + lhs[2] * rhs[1] - lhs[1] * rhs[2];
		auto y = lhs[1] * rhs[3] + lhs[3] * rhs[1] + lhs[0] * rhs[2] - lhs[2] * rhs[0];
		auto z = lhs[2] * rhs[3] + lhs[3] * rhs[2] + lhs[1] * rhs[0] - lhs[0] * rhs[1];

		dst[0] = x;
		dst[1] = y;
		dst[2] = z;
		dst[3] = w;
	}

	inline void Math::quatRotateVec3(const f32* q, const f32* p, f32* dst) {
		auto w = -p[0] * q[0] - p[1] * q[1] - p[2] * q[2];
		auto x = q[3] * p[0] + q[1] * p[2] - q[2] * p[1];
		auto y = q[3] * p[1] - q[0] * p[2] + q[2] * p[0];
		auto z = q[3] * p[2] + q[0] * p[1] - q[1] * p[0];

		auto dx = -w * q[0] + x * q[3] - y * q[2] + z * q[1];
		auto dy = -w * q[1] + x * q[2] + y * q[3] - z * q[0];
		dst[0] = dx;
		dst[1] = dy;
		dst[2] = -w * q[2] - x * q[1] + y * q[0] + z * q[3];
	}

	inline void Math::matTransformPoint(const f32(&m)[3][4], const f32(&p)[3], f32(&dst)[3]) {
		auto x = p[0] * m[0][0] + p[1] * m[0][1] + p[2] * m[0][2] + m[0][3];
		auto y = p[0] * m[1][0] + p[1] * m[1][1] + p[2] * m[1][2] + m[1][3];
		auto z = p[0] * m[2][0] + p[1] * m[2][1] + p[2] * m[2][2] + m[2][3];

		dst[0] = x;
		dst[1] = y;
		dst[2] = z;
	}

	inline bool Math::isPOT(ui32 n) {
		return n < 1 ? false : !(n & (n - 1));
	}
}