#pragma once

#include "srk/math/Math.h"

namespace srk {
	template<std::floating_point T>
	class Quaternion {
	public:
		using ElementType = T;
		using Data = T[4];

		constexpr Quaternion() noexcept : Quaternion(Math::ZERO<T>, Math::ZERO<T>, Math::ZERO<T>, Math::ONE<T>) {}

		constexpr Quaternion(nullptr_t) noexcept {}

		constexpr Quaternion(const Quaternion& q) noexcept : Quaternion(q.x, q.y, q.z, q.w) {}

		constexpr Quaternion(Quaternion&& q) noexcept : Quaternion(q.x, q.y, q.z, q.w) {}

		constexpr Quaternion(T x, T y = Math::ZERO<T>, T z = Math::ZERO<T>, T w = Math::ONE<T>) noexcept :
			x(x),
			y(y),
			z(z),
			w(w) {}

		~Quaternion() {}

		inline SRK_CALL operator Data& () {
			return data;
		}
		inline SRK_CALL operator const Data& () const {
			return data;
		}

		template<std::floating_point K>
		inline constexpr Quaternion& SRK_CALL operator=(const Quaternion<K>& q) noexcept {
			set(q.data);
			return *this;
		}

		template<std::floating_point K>
		inline constexpr Quaternion& SRK_CALL operator=(Quaternion<K>&& q) noexcept {
			set(q.data);
			return *this;
		}

		template<std::floating_point K>
		inline constexpr Quaternion& SRK_CALL operator=(const K(&q)[4]) noexcept {
			set(q);
			return *this;
		}

		template<std::floating_point K>
		inline constexpr void SRK_CALL operator*=(const Quaternion<K>& rhs) noexcept {
			append(rhs);
		}

		inline T SRK_CALL getLength() const {
			return std::sqrt(getLengthSq());
		}
		inline T SRK_CALL getLengthSq() const {
			return Math::dot(data, data);
		}

		template<std::floating_point K>
		inline constexpr Quaternion& SRK_CALL set(const Quaternion<K>& q) {
			set(q.data);
			return *this;
		}

		inline constexpr Quaternion& SRK_CALL set(T x = Math::ZERO<T>, T y = Math::ZERO<T>, T z = Math::ZERO<T>, T w = Math::ONE<T>) {
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
			return *this;
		}

		template<std::floating_point K>
		inline constexpr Quaternion& SRK_CALL set(const K(&q)[4]) {
			x = q[0];
			y = q[1];
			z = q[2];
			w = q[3];
			return *this;
		}

		Quaternion& SRK_CALL normalize() {
			auto n = Math::dot(data, data);

			if (n <= Math::TOLERANCE<T> || Math::equal(n, Math::ONE<T>, Math::TOLERANCE<T>)) return *this;
			n = std::sqrt(n);

			x *= n;
			y *= n;
			z *= n;
			w *= n;
			return *this;
		}

		inline constexpr Quaternion& SRK_CALL conjugate() {
			x = -x;
			y = -y;
			z = -z;
			return *this;
		}

		inline constexpr Quaternion& SRK_CALL invert() {
			Math::invert<Math::DataDesc(Math::DataType::QUATERNION), Math::DataDesc(Math::DataType::QUATERNION)>(data, data);
			return *this;
		}

		template<size_t N, std::floating_point K>
		inline constexpr void SRK_CALL invert(K(&dst)[N]) const {
			Math::invert<Math::DataDesc(Math::DataType::QUATERNION), Math::DataDesc(Math::DataType::QUATERNION), N, K, 4, T>(data, dst);
		}

		template<std::floating_point K>
		inline constexpr void SRK_CALL invert(Quaternion<K>& dst) const {
			invert(dst.data);
		}

		template<std::floating_point K>
		void constexpr SRK_CALL toEuler(K(&dst)[3]) const {
			auto y2 = y * y;
			auto ex = std::atan2(Math::TWO<T> *(w * x + y * z), (Math::ONE<T> - Math::TWO<T> *(x * x + y2)));
			auto ey = std::asin(Math::TWO<T> *(w * y - z * x));
			auto ez = std::atan2(Math::TWO<T> *(w * z + x * y), (Math::ONE<T> - Math::TWO<T> *(y2 + z * z)));
			dst[0] = ex;
			dst[1] = ey;
			dst[2] = ez;
		}

		inline constexpr bool SRK_CALL isIdentity() const {
			return Math::equal(x, Math::ZERO<T>, Math::TOLERANCE<T>) && Math::equal(y, Math::ZERO<T>, Math::TOLERANCE<T>) && Math::equal(z, Math::ZERO<T>, Math::TOLERANCE<T>) && Math::equal(w, Math::ONE<T>, Math::TOLERANCE<T>);
		}

		inline constexpr T SRK_CALL getRadian() const {
			return std::acos(w);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point LT>
		inline Quaternion& SRK_CALL append(const LT(&lhs)[4]) {
			using namespace srk::enum_operators;

			constexpr auto ldesc = Math::DataDesc(Math::DataType::QUATERNION, LDesc);
			constexpr auto rdesc = Math::DataDesc(Math::DataType::QUATERNION, RDesc);
			constexpr auto ddesc = Math::DataDesc(Math::DataType::QUATERNION, DDesc.hints | Math::Hint::MEM_OVERLAP, DDesc.range);

			Math::mul<ldesc, rdesc, ddesc>(lhs, data, data);
			return *this;
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point LT>
		inline Quaternion& SRK_CALL append(const Quaternion<LT>& lhs) {
			return append<LDesc, RDesc, DDesc>(lhs.data);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point LT, std::floating_point DT>
		inline void SRK_CALL append(const LT(&lhs)[4], DT(&dst)[4]) const {
			using namespace srk::enum_operators;

			constexpr auto ldesc = Math::DataDesc(Math::DataType::QUATERNION, LDesc);
			constexpr auto rdesc = Math::DataDesc(Math::DataType::QUATERNION, RDesc);
			constexpr auto ddesc = Math::DataDesc(Math::DataType::QUATERNION, DDesc.hints | Math::Hint::MEM_OVERLAP, DDesc.range);

			Math::mul<ldesc, rdesc, ddesc>(lhs, data, dst);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point LT, std::floating_point DT>
		inline void SRK_CALL append(const Quaternion<LT>& lhs, DT(&dst)[4]) const {
			append<LDesc, RDesc, DDesc>(lhs.data, dst);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point LT, std::floating_point DT>
		inline void SRK_CALL append(const LT(&lhs)[4], Quaternion<DT>& dst) const {
			append<LDesc, RDesc, DDesc>(lhs, dst.data);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point LT, std::floating_point DT>
		inline void SRK_CALL append(const Quaternion<LT>& lhs, Quaternion<DT>& dst) const {
			append<LDesc, RDesc, DDesc>(lhs.data, dst.data);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point RT>
		inline Quaternion& SRK_CALL prepend(const RT(&rhs)[4]) {
			using namespace srk::enum_operators;

			constexpr auto ldesc = Math::DataDesc(Math::DataType::QUATERNION, LDesc);
			constexpr auto rdesc = Math::DataDesc(Math::DataType::QUATERNION, RDesc);
			constexpr auto ddesc = Math::DataDesc(Math::DataType::QUATERNION, DDesc.hints | Math::Hint::MEM_OVERLAP, DDesc.range);

			Math::mul<ldesc, rdesc, ddesc>(data, rhs, data);
			return *this;
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point RT>
		inline Quaternion& SRK_CALL prepend(const Quaternion<RT>& rhs) {
			return prepend<LDesc, RDesc, DDesc>(rhs.data);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point RT, std::floating_point DT>
		inline void SRK_CALL prepend(const RT(&rhs)[4], DT(&dst)[4]) const {
			using namespace srk::enum_operators;

			constexpr auto ldesc = Math::DataDesc(Math::DataType::QUATERNION, LDesc);
			constexpr auto rdesc = Math::DataDesc(Math::DataType::QUATERNION, RDesc);
			constexpr auto ddesc = Math::DataDesc(Math::DataType::QUATERNION, DDesc.hints | Math::Hint::MEM_OVERLAP, DDesc.range);

			Math::mul<ldesc, rdesc, ddesc>(data, rhs, dst);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point RT, std::floating_point DT>
		inline void SRK_CALL prepend(const Quaternion<RT>& rhs, DT(&dst)[4]) const {
			prepend<LDesc, RDesc, DDesc>(rhs.data, dst);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point RT, std::floating_point DT>
		inline void SRK_CALL prepend(const RT(&rhs)[4], Quaternion<DT>& dst) const {
			prepend<LDesc, RDesc, DDesc>(rhs, dst.data);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = nullptr, std::floating_point RT, std::floating_point DT>
		inline void SRK_CALL prepend(const Quaternion<RT>& rhs, Quaternion<DT>& dst) const {
			prepend<LDesc, RDesc, DDesc>(rhs.data, dst.data);
		}

		template<Math::DataDesc LDesc = nullptr, Math::DataDesc RDesc = nullptr, Math::DataDesc DDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, size_t RN, std::floating_point RT, size_t DN, std::floating_point DT>
		inline void SRK_CALL transformPoint(const RT(&rhs)[RN], DT(&dst)[DN]) const {
			constexpr auto ldesc = Math::Data2DDesc(Math::DataType::QUATERNION, LDesc);
			constexpr auto rdesc = Math::DataDesc(Math::DataType::VECTOR, RDesc);
			constexpr auto ddesc = Math::DataDesc(Math::DataType::VECTOR, DDesc);

			Math::transform<ldesc, rdesc, ddesc>(data, rhs, dst);
		}

		template<Math::DataDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point RadT>
		inline Quaternion& SRK_CALL rotationX(RadT radian) {
			Math::rotationX<Math::DataDesc(Math::DataType::QUATERNION, DstDesc)>(radian, data);
			return *this;
		}

		template<Math::DataDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point RadT>
		inline Quaternion& SRK_CALL rotationY(RadT radian) {
			Math::rotationY<Math::DataDesc(Math::DataType::QUATERNION, DstDesc)>(radian, data);
			return *this;
		}

		template<Math::DataDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point RadT>
		inline Quaternion& SRK_CALL rotationZ(RadT radian) {
			Math::rotationZ<Math::DataDesc(Math::DataType::QUATERNION, DstDesc)>(radian, data);
			return *this;
		}

		template<Math::DataDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point RadT>
		inline Quaternion& SRK_CALL rotation(const RadT(&radians)[3]) {
			Math::rotation<Math::DataDesc(Math::DataType::QUATERNION, DstDesc)>(radians, data);
			return *this;
		}

		template<Math::DataDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point AxisT, std::floating_point RadT>
		inline Quaternion& SRK_CALL rotation(const AxisT(&axis)[3], RadT radian) {
			Math::rotation<Math::DataDesc(Math::DataType::QUATERNION, DstDesc)>(axis, radian, data);
			return *this;
		}

		template<Math::DataDesc DstDesc = Math::Hint::IDENTITY_IF_NOT_EXIST, std::floating_point FwdT, std::floating_point UwdT>
		inline Quaternion& SRK_CALL lookAt(const FwdT(&forward)[3], const UwdT(&upward)[3]) {
			Math::lookAt<Math::DataDesc(Math::DataType::QUATERNION, DstDesc)>(forward, upward, data);
			return *this;
		}

		union {
			Data data;

			struct {
				T x;
				T y;
				T z;
				T w;
			};
		};
	};

	template<std::floating_point LT, std::floating_point RT>
	inline Quaternion<decltype((*(LT*)0) + (*(RT*)0))> SRK_CALL operator*(const Quaternion<LT>& lhs, const Quaternion<RT>& rhs) {
		Quaternion<decltype((*(LT*)0) + (*(RT*)0))> q(nullptr);
		Math::mul<Math::DataDesc(Math::DataType::QUATERNION), Math::DataDesc(Math::DataType::QUATERNION), Math::DataDesc(Math::DataType::QUATERNION)>(lhs.data, rhs.data, q.data);
		return q;
	}
}