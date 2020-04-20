#pragma once

#include "aurora/Global.h"

namespace aurora::rtti {
	class AE_DLL ClassInfo {
	public:
		ClassInfo(const ClassInfo* const base) :
			_base(base),
			_depth(base ? base->_depth + 1 : 0) {
		}

		inline size_t AE_CALL id() const {
			return (size_t)this;
		}

		template<typename T>
		inline bool AE_CALL isKindOf() const {
			auto c = this;
			auto t = &T::__rttiClassInfo;
			auto td = t->_depth;

			if (c->_depth < td) return false;
			if (c->_depth > td) {
				for (size_t i = 0, n = c->_depth - td; i < n; ++i) c = c->_base;
			}

			return c == t;
		}

	private:
		const ClassInfo* const _base;
		const uint64_t _depth;
	};
}


#define AE_RTTI_DECLARE_BASE() \
inline static aurora::rtti::ClassInfo __rttiClassInfo = aurora::rtti::ClassInfo(nullptr); \
friend aurora::rtti::ClassInfo; \
aurora::rtti::ClassInfo* __rtti;

#define AE_RTTI_DECLARE_DERIVED(__BASE__) \
inline static aurora::rtti::ClassInfo __rttiClassInfo = aurora::rtti::ClassInfo(&__BASE__::__rttiClassInfo); \
friend aurora::rtti::ClassInfo;

#define AE_RTTI_DEFINE() __rtti = &__rttiClassInfo;

#define AE_RTTI_GEN_IS_KIND_OF_METHOD() \
template<typename RTTI_CLASS> \
inline bool AE_CALL isKindOf() const { \
	return __rtti->isKindOf<RTTI_CLASS>(); \
};