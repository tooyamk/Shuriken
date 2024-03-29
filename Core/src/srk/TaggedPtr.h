#pragma once

#include "srk/Core.h"

namespace srk {
	template<typename T>
	class
#if ((SRK_ARCH == SRK_ARCH_X86 || SRK_ARCH == SRK_ARCH_ARM) && SRK_ARCH_WORD_BITS == SRK_ARCH_WORD_BITS_64)
		alignas(8) TaggedPtr {
	public:
		using CompressedPtr = uint64_t;
		using Tag = uint16_t;

		TaggedPtr() :
			_ptr(0) {
		}

		TaggedPtr(T* ptr, Tag tag = 0) {
			_packPtr(_ptr, ptr, tag);
		}

		inline bool SRK_CALL operator==(const TaggedPtr& value) const {
			return _ptr == value._ptr;
		}

		inline bool SRK_CALL operator!=(const TaggedPtr& value) const {
			return _ptr != value._ptr;
		}

		inline T* SRK_CALL operator->() const {
			return _extractPtr(_ptr);
		}

		inline T& SRK_CALL operator*() const {
			return *_extractPtr(_ptr);
		}

		inline T* SRK_CALL getPtr() const {
			return _extractPtr(_ptr);
		}

		inline Tag SRK_CALL getTag() const {
			return _extractTag(_ptr);
		}

		inline void SRK_CALL setTag(Tag tag) {
			_packTag(_ptr, tag);
		}

		inline void SRK_CALL set(T* ptr, Tag tag) {
			_packPtr(_ptr, ptr, tag);
		}

	private:
		static constexpr CompressedPtr PTR_MASK = 0xFFFFFFFFFFFFULL;

		CompressedPtr _ptr;

		inline static void _packPtr(volatile CompressedPtr& i, T* ptr) {
			auto tag = _extractTag(i);
			i = (CompressedPtr)ptr;
			_packTag(i, tag);
		}

		inline static void _packPtr(volatile CompressedPtr& i, T* ptr, Tag tag) {
			i = (CompressedPtr)ptr;
			_packTag(i, tag);
		}

		inline static void _packTag(volatile CompressedPtr& i, Tag tag) {
			((Tag*)&i)[3] = tag;
		}

		inline static T* _extractPtr(volatile CompressedPtr const& i) {
			return (T*)(i & PTR_MASK);
		}

		inline static Tag _extractTag(volatile CompressedPtr const& i) {
			return ((Tag*)&i)[3];
		}
	};
#else
#	if SRK_ARCH_WORD_BITS == SRK_ARCH_WORD_BITS_64
		alignas(16)
#	else
		alignas(8)
#	endif
		TaggedPtr {
	public:
		using Tag = size_t;

		TaggedPtr() :
			_ptr(nullptr),
			_tag(0) {
		}

		TaggedPtr(T* ptr, Tag tag = 0) :
			_ptr(ptr),
			_tag(tag) {
		}

		inline bool SRK_CALL operator==(const TaggedPtr& value) const {
			return _ptr == value._ptr && _tag == value._tag;
		}

		inline bool SRK_CALL operator!=(const TaggedPtr& value) const {
			return _ptr != value._ptr || _tag != value._tag;
		}

		inline T* SRK_CALL operator->() const {
			return _ptr;
		}

		inline T& SRK_CALL operator*() const {
			return *_ptr;
		}

		inline T* SRK_CALL getPtr() const {
			return _ptr;
		}

		inline Tag SRK_CALL getTag() const {
			return _tag;
		}

		inline void SRK_CALL setTag(Tag tag) {
			_tag = tag;
		}

		inline void SRK_CALL set(T* ptr, Tag tag) {
			_ptr = ptr;
			_tag = tag;
		}

	private:
		T* _ptr;
		Tag _tag;
	};
#endif
}