#pragma once

#include "base/LowLevel.h"
#include "math/Math.h"

namespace aurora {
	template<ui32 N, typename T>
	class AE_TEMPLATE_DLL Vec {
	public:
		using Type = Vec<N, T>;

		Vec() {
			memset(this, 0, sizeof(T) * N);
		}

		Vec(const Type& vec) {
			set(vec);
		}

		Vec(Type&& vec) {
			set(vec);
		}

		Vec(const T values[]) {
			set(values);
		}

		inline Type& AE_CALL set(const Type& vec) {
			set(vec.data);
			return *this;
		}

		inline Type& AE_CALL set(const T values[]) {
			for (ui32 i = 0; i < N; ++i) data[i] = values[i];
			return *this;
		}

		inline void AE_CALL normalize() {
			Math::normalize<N, T, T>(data, data);
		}

		template<typename Ret = T>
		inline Ret AE_CALL getLengthSq() const {
			return Math::dot<Ret>(data, data);
		}

		template<typename Ret = T>
		inline Ret AE_CALL getLength() const {
			return std::sqrt(getLengthSq<Ret>());
		}

		template<typename Ret = T>
		inline static void AE_CALL normalize(const Type& v, Vec<N, Ret>& dst) {
			Math::normalize<N, T, Ret>(v.data, dst.data);
		}

		template<typename Ret = T>
		inline static Ret AE_CALL dot(const Type& v1, const Type& v2) {
			return Math::dot<Ret>(v1.data, v2.data);
		}

		template<typename Ret>
		inline static void AE_CALL lerp(const Type& from, const Type& to, f32 t, Vec<N, Ret>& dst) {
			Math::lerp<N>(from.data, to.data, t, dst.data);
		}

		T data[N];
	};


	/*
	template<typename T>
	class AE_TEMPLATE_DLL Vec3 : public Vec<T, 3> {
	public:

	};


	template<typename T>
	class AE_TEMPLATE_DLL Vec4 : public Vec<T, 3> {
	public:

	};
	*/
}