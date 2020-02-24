#pragma once

#include "aurora/Global.h"

namespace aurora::rtti {
	class AE_DLL ClassInfo {
	public:
		ClassInfo(const ClassInfo* const base) :
			_base(base) {
		}

		inline size_t AE_CALL id() const {
			return (size_t)this;
		}

		template<typename T>
		inline bool AE_CALL isKindOf() const {
			auto c = this;
			do {
				if (c == &T::__rttiClassInfo) return true;
				c = c->_base;
			} while (c);
			return false;
		}

	private:
		const ClassInfo* const _base;
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