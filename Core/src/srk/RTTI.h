#pragma once

#include "srk/Core.h"

namespace srk::rtti {
	class ClassInfo {
	public:
		ClassInfo(const ClassInfo* const base) :
			_base(base),
			_depth(base ? base->_depth + 1 : 0) {
		}

		inline size_t SRK_CALL id() const {
			return (size_t)this;
		}

		template<typename T>
		inline bool SRK_CALL isKindOf() const {
			auto c = this;
			auto t = &T::rttiClassInfo;

			if (c->_depth > t->_depth) {
				for (size_t i = 0, n = c->_depth - t->_depth; i < n; ++i) c = c->_base;
			}

			return c == t;
		}

	protected:
		const ClassInfo* const _base;
		const size_t _depth;
	};
}


#define SRK_RTTI_DECLARE_BASE(__CLASS_INFO_T__, ...) \
public: \
inline static const __CLASS_INFO_T__ rttiClassInfo = __CLASS_INFO_T__(nullptr, ##__VA_ARGS__); \
template<typename RTTI_CLASS> \
inline bool SRK_CALL isKindOf() const { \
	return __rtti->isKindOf<RTTI_CLASS>(); \
}; \
inline const __CLASS_INFO_T__& SRK_CALL getRttiClassInfo() const { \
	return *__rtti; \
}; \
protected: \
friend srk::rtti::ClassInfo; \
friend __CLASS_INFO_T__; \
const __CLASS_INFO_T__* __rtti; \
private:

#define SRK_RTTI_DECLARE_DERIVED(__CLASS_INFO_T__, __BASE__, ...) \
public: \
inline static const __CLASS_INFO_T__ rttiClassInfo = __CLASS_INFO_T__(&__BASE__::rttiClassInfo, ##__VA_ARGS__); \
protected: \
friend srk::rtti::ClassInfo; \
friend __CLASS_INFO_T__; \
private:

#define SRK_RTTI_DEFINE() __rtti = &rttiClassInfo;

#define SRK_RTTI_INHERIT(__CLASS_INFO_T__, __BASE__, ...) \
__BASE__ { \
SRK_RTTI_DECLARE_DERIVED(__CLASS_INFO_T__, __BASE__, ##__VA_ARGS__)
