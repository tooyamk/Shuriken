#include "Math.h"

namespace aurora {
	void Math::slerp(const float32_t(&from)[4], const float32_t(&to)[4], float32_t t, float32_t(&dst)[4]) {
		auto x = to[0], y = to[1], z = to[2], w = to[3];
		auto cos = from[0] * x + from[1] * y + from[2] * z + from[3] * w;
		if (cos < 0.f) {//shortest path
			x = -x;
			y = -y;
			z = -z;
			w = -w;
			cos = -cos;
		}

		float32_t k0, k1;
		if (cos > .9999f) {
			k0 = 1 - t;
			k1 = t;
		} else {
			auto a = std::acos(cos);
			auto s = std::sin(a);
			auto ta = t * a;
			k0 = std::sin(a - ta) / s;
			k1 = std::sin(ta) / s;
		}

		x = from[0] * k0 + x * k1;
		y = from[1] * k0 + y * k1;
		z = from[2] * k0 + z * k1;

		dst[0] = x;
		dst[1] = y;
		dst[2] = z;
		dst[3] = from[3] * k0 + w * k1;
	}

	void Math::transposeMat(const float32_t(&m)[3][4], float32_t(&dst)[4][4]) {
		float32_t d[4][4];

		for (uint8_t c = 0; c < 4; ++c) {
			for (uint8_t r = 0; r < 3; ++r) d[c][r] = m[r][c];
		}

		d[0][3] = 0.f;
		d[1][3] = 0.f;
		d[2][3] = 0.f;
		d[3][3] = 1.f;

		memcpy(dst, d, sizeof(dst));
	}

	void Math::transposeMat(const float32_t(&m)[4][4], float32_t(&dst)[4][4]) {
		float32_t d[4][4];

		for (uint8_t c = 0; c < 4; ++c) {
			for (uint8_t r = 0; r < 4; ++r) d[c][r] = m[r][c];
		}

		memcpy(dst, d, sizeof(dst));
	}

	void Math::appendMat(const float32_t(&lhs)[3][4], const float32_t(&rhs)[3][4], float32_t(&dst)[3][4]) {
		float32_t m[3][4];

		for (uint8_t c = 0; c < 3; ++c) {
			auto& mc = m[c];
			auto& rc = rhs[c];
			mc[0] = lhs[0][0] * rc[0] + lhs[1][0] * rc[1] + lhs[2][0] * rc[2];
			mc[1] = lhs[0][1] * rc[0] + lhs[1][1] * rc[1] + lhs[2][1] * rc[2];
			mc[2] = lhs[0][2] * rc[0] + lhs[1][2] * rc[1] + lhs[2][2] * rc[2];
			mc[3] = lhs[0][3] * rc[0] + lhs[1][3] * rc[1] + lhs[2][3] * rc[2] + rc[3];
		}

		memcpy(dst, m, sizeof(m));
	}

	void Math::appendMat(const float32_t(&lhs)[3][4], const float32_t(&rhs)[3][4], float32_t(&dst)[4][4]) {
		appendMat(lhs, rhs, (float32_t(&)[3][4])dst);

		dst[3][0] = 0.f;
		dst[3][1] = 0.f;
		dst[3][2] = 0.f;
		dst[3][3] = 1.f;
	}

	void Math::appendMat(const float32_t(&lhs)[3][4], const float32_t(&rhs)[4][4], float32_t(&dst)[3][4]) {
		appendMat(lhs, (float32_t(&)[3][4])rhs, (float32_t(&)[3][4])dst);
	}

	void Math::appendMat(const float32_t(&lhs)[3][4], const float32_t(&rhs)[4][4], float32_t(&dst)[4][4]) {
		float32_t m[4][4];

		for (uint8_t c = 0; c < 4; ++c) {
			auto& mc = m[c];
			auto& rc = rhs[c];
			mc[0] = lhs[0][0] * rc[0] + lhs[1][0] * rc[1] + lhs[2][0] * rc[2];
			mc[1] = lhs[0][1] * rc[0] + lhs[1][1] * rc[1] + lhs[2][1] * rc[2];
			mc[2] = lhs[0][2] * rc[0] + lhs[1][2] * rc[1] + lhs[2][2] * rc[2];
			mc[3] = lhs[0][3] * rc[0] + lhs[1][3] * rc[1] + lhs[2][3] * rc[2] + rc[3];
		}

		memcpy(dst, m, sizeof(m));
	}

	void Math::appendMat(const float32_t(&lhs)[4][4], const float32_t(&rhs)[3][4], float32_t(&dst)[3][4]) {
		float32_t m[3][4];

		for (uint8_t c = 0; c < 3; ++c) {
			auto& mc = m[c];
			auto& rc = rhs[c];
			mc[0] = lhs[0][0] * rc[0] + lhs[1][0] * rc[1] + lhs[2][0] * rc[2] + lhs[3][0] * rc[3];
			mc[1] = lhs[0][1] * rc[0] + lhs[1][1] * rc[1] + lhs[2][1] * rc[2] + lhs[3][1] * rc[3];
			mc[2] = lhs[0][2] * rc[0] + lhs[1][2] * rc[1] + lhs[2][2] * rc[2] + lhs[3][2] * rc[3];
			mc[3] = lhs[0][3] * rc[0] + lhs[1][3] * rc[1] + lhs[2][3] * rc[2] + lhs[3][3] * rc[3];
		}

		memcpy(dst, m, sizeof(m));
	}

	void Math::appendMat(const float32_t(&lhs)[4][4], const float32_t(&rhs)[3][4], float32_t(&dst)[4][4]) {
		appendMat(lhs, rhs, (float32_t(&)[3][4])dst);

		dst[3][0] = lhs[3][0];
		dst[3][1] = lhs[3][1];
		dst[3][2] = lhs[3][2];
		dst[3][3] = lhs[3][3];
	}

	void Math::appendMat(const float32_t(&lhs)[4][4], const float32_t(&rhs)[4][4], float32_t(&dst)[3][4]) {
		appendMat(lhs, (float32_t(&)[3][4])rhs, dst);
	}

	void Math::appendMat(const float32_t(&lhs)[4][4], const float32_t(&rhs)[4][4], float32_t(&dst)[4][4]) {
		float32_t m[4][4];

		for (uint8_t c = 0; c < 4; ++c) {
			auto& mc = m[c];
			auto& rc = rhs[c];
			mc[0] = lhs[0][0] * rc[0] + lhs[1][0] * rc[1] + lhs[2][0] * rc[2] + lhs[3][0] * rc[3];
			mc[1] = lhs[0][1] * rc[0] + lhs[1][1] * rc[1] + lhs[2][1] * rc[2] + lhs[3][1] * rc[3];
			mc[2] = lhs[0][2] * rc[0] + lhs[1][2] * rc[1] + lhs[2][2] * rc[2] + lhs[3][2] * rc[3];
			mc[3] = lhs[0][3] * rc[0] + lhs[1][3] * rc[1] + lhs[2][3] * rc[2] + lhs[3][3] * rc[3];
		}

		memcpy(dst, m, sizeof(dst));
	}

	bool Math::invertMat(const float32_t(&m)[3][4], float32_t(&dst)[3][4]) {
		auto tmp0 = m[2][2] * m[1][1] - m[2][1] * m[1][2];
		auto tmp1 = m[2][0] * m[1][2] - m[2][2] * m[1][0];
		auto tmp2 = m[2][1] * m[1][0] - m[2][0] * m[1][1];

		if (auto det = m[0][0] * tmp0 + m[0][1] * tmp1 + m[0][2] * tmp2; std::abs(det) > TOLERANCE<float32_t>) {
			det = ONE<float32_t> / det;

			float32_t d[3][4];

			d[0][0] = tmp0 * det;
			d[1][0] = tmp1 * det;
			d[2][0] = tmp2 * det;

			d[0][1] = (m[2][1] * m[0][2] - m[2][2] * m[0][1]) * det;
			d[1][1] = (m[2][2] * m[0][0] - m[2][0] * m[0][2]) * det;
			d[2][1] = (m[2][0] * m[0][1] - m[2][1] * m[0][0]) * det;

			tmp0 = m[0][2] * m[1][3];
			tmp1 = m[0][3] * m[1][2];
			tmp2 = m[0][1] * m[1][3];
			auto tmp3 = m[0][3] * m[1][1];
			auto tmp4 = m[0][1] * m[1][2];
			auto tmp5 = m[0][2] * m[1][1];
			auto tmp6 = m[0][0] * m[1][3];
			auto tmp7 = m[0][3] * m[1][0];
			auto tmp8 = m[0][0] * m[1][2];
			auto tmp9 = m[0][2] * m[1][0];
			auto tmp10 = m[0][0] * m[1][1];
			auto tmp11 = m[0][1] * m[1][0];

			d[0][2] = (tmp4 - tmp5) * det;
			d[1][2] = (tmp9 - tmp8) * det;
			d[2][2] = (tmp10 - tmp11) * det;
			d[0][3] = (tmp2 * m[2][2] + tmp5 * m[2][3] + tmp1 * m[2][1] - tmp4 * m[2][3] - tmp0 * m[2][1] - tmp3 * m[2][2]) * det;
			d[1][3] = (tmp8 * m[2][3] + tmp0 * m[2][0] + tmp7 * m[2][2] - tmp6 * m[2][2] - tmp9 * m[2][3] - tmp1 * m[2][0]) * det;
			d[2][3] = (tmp6 * m[2][1] + tmp11 * m[2][3] + tmp3 * m[2][0] - tmp10 * m[2][3] - tmp2 * m[2][0] - tmp7 * m[2][1]) * det;

			memcpy(dst, d, sizeof(dst));

			return true;
		} else {
			return false;
		}
	}

	bool Math::invertMat(const float32_t(&m)[4][4], float32_t(&dst)[4][4]) {
		auto tmp0 = m[2][2] * m[3][3];
		auto tmp1 = m[2][3] * m[3][2];
		auto tmp2 = m[2][1] * m[3][3];
		auto tmp3 = m[2][3] * m[3][1];
		auto tmp4 = m[2][1] * m[3][2];
		auto tmp5 = m[2][2] * m[3][1];
		auto tmp6 = m[2][0] * m[3][3];
		auto tmp7 = m[2][3] * m[3][0];
		auto tmp8 = m[2][0] * m[3][2];
		auto tmp9 = m[2][2] * m[3][0];
		auto tmp10 = m[2][0] * m[3][1];
		auto tmp11 = m[2][1] * m[3][0];

		auto d00 = tmp0 * m[1][1] + tmp3 * m[1][2] + tmp4 * m[1][3] - tmp1 * m[1][1] - tmp2 * m[1][2] - tmp5 * m[1][3];
		auto d10 = tmp1 * m[1][0] + tmp6 * m[1][2] + tmp9 * m[1][3] - tmp0 * m[1][0] - tmp7 * m[1][2] - tmp8 * m[1][3];
		auto d20 = tmp2 * m[1][0] + tmp7 * m[1][1] + tmp10 * m[1][3] - tmp3 * m[1][0] - tmp6 * m[1][1] - tmp11 * m[1][3];
		auto d30 = tmp5 * m[1][0] + tmp8 * m[1][1] + tmp11 * m[1][2] - tmp4 * m[1][0] - tmp9 * m[1][1] - tmp10 * m[1][2];

		if (auto det = m[0][0] * d00 + m[0][1] * d10 + m[0][2] * d20 + m[0][3] * d30; std::abs(det) > TOLERANCE<float32_t>) {
			det = ONE<float32_t> / det;

			float32_t d[4][4];

			d[0][0] = d00 * det;
			d[1][0] = d10 * det;
			d[2][0] = d20 * det;
			d[3][0] = d30 * det;

			d[0][1] = (tmp1 * m[0][1] + tmp2 * m[0][2] + tmp5 * m[0][3] - tmp0 * m[0][1] - tmp3 * m[0][2] - tmp4 * m[0][3]) * det;
			d[1][1] = (tmp0 * m[0][0] + tmp7 * m[0][2] + tmp8 * m[0][3] - tmp1 * m[0][0] - tmp6 * m[0][2] - tmp9 * m[0][3]) * det;
			d[2][1] = (tmp3 * m[0][0] + tmp6 * m[0][1] + tmp11 * m[0][3] - tmp2 * m[0][0] - tmp7 * m[0][1] - tmp10 * m[0][3]) * det;
			d[3][1] = (tmp4 * m[0][0] + tmp9 * m[0][1] + tmp10 * m[0][2] - tmp5 * m[0][0] - tmp8 * m[0][1] - tmp11 * m[0][2]) * det;

			tmp0 = m[0][2] * m[1][3];
			tmp1 = m[0][3] * m[1][2];
			tmp2 = m[0][1] * m[1][3];
			tmp3 = m[0][3] * m[1][1];
			tmp4 = m[0][1] * m[1][2];
			tmp5 = m[0][2] * m[1][1];
			tmp6 = m[0][0] * m[1][3];
			tmp7 = m[0][3] * m[1][0];
			tmp8 = m[0][0] * m[1][2];
			tmp9 = m[0][2] * m[1][0];
			tmp10 = m[0][0] * m[1][1];
			tmp11 = m[0][1] * m[1][0];

			d[0][2] = (tmp0 * m[3][1] + tmp3 * m[3][2] + tmp4 * m[3][3] - tmp1 * m[3][1] - tmp2 * m[3][2] - tmp5 * m[3][3]) * det;
			d[1][2] = (tmp1 * m[3][0] + tmp6 * m[3][2] + tmp9 * m[3][3] - tmp0 * m[3][0] - tmp7 * m[3][2] - tmp8 * m[3][3]) * det;
			d[2][2] = (tmp2 * m[3][0] + tmp7 * m[3][1] + tmp10 * m[3][3] - tmp3 * m[3][0] - tmp6 * m[3][1] - tmp11 * m[3][3]) * det;
			d[3][2] = (tmp5 * m[3][0] + tmp8 * m[3][1] + tmp11 * m[3][2] - tmp4 * m[3][0] - tmp9 * m[3][1] - tmp10 * m[3][2]) * det;
			d[0][3] = (tmp2 * m[2][2] + tmp5 * m[2][3] + tmp1 * m[2][1] - tmp4 * m[2][3] - tmp0 * m[2][1] - tmp3 * m[2][2]) * det;
			d[1][3] = (tmp8 * m[2][3] + tmp0 * m[2][0] + tmp7 * m[2][2] - tmp6 * m[2][2] - tmp9 * m[2][3] - tmp1 * m[2][0]) * det;
			d[2][3] = (tmp6 * m[2][1] + tmp11 * m[2][3] + tmp3 * m[2][0] - tmp10 * m[2][3] - tmp2 * m[2][0] - tmp7 * m[2][1]) * det;
			d[3][3] = (tmp10 * m[2][2] + tmp4 * m[2][0] + tmp9 * m[2][1] - tmp8 * m[2][1] - tmp11 * m[2][2] - tmp5 * m[2][0]) * det;

			memcpy(dst, d, sizeof(dst));

			return true;
		} else {
			return false;
		}
	}
}