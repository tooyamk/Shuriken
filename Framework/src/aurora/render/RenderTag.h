#pragma once

#include "aurora/Intrusive.h"
#include "aurora/hash/CRC.h"
#include <unordered_set>

namespace aurora::render {
	class AE_FW_DLL RenderTag {
	public:
		struct AE_FW_DLL StdComparer {
			inline bool AE_CALL operator()(const RenderTag& value1, const RenderTag& value2) const {
				return value1._value < value2._value;
			}
		};


		struct AE_FW_DLL StdUnorderedComparer {
			inline bool AE_CALL operator()(const RenderTag& value1, const RenderTag& value2) const {
				return value1 == value2;
			}
		};


		struct AE_FW_DLL StdUnorderedHasher {
			inline size_t AE_CALL operator()(const RenderTag& value) const {
				return std::hash<uint64_t>{}(value._value);
			}
		};


		RenderTag();
		RenderTag(const std::string_view& tag);
		RenderTag(const RenderTag& tag);

		inline RenderTag& AE_CALL operator=(const RenderTag& value) {
			_value = value._value;
			return *this;
		}

		inline RenderTag& AE_CALL operator=(const std::string_view& value) {
			_value = _gen(value.data(), value.size());
			return *this;
		}

		inline bool AE_CALL operator==(const RenderTag& value) const {
			return _value == value._value;
		}

		inline bool AE_CALL operator!=(const RenderTag& value) const {
			return _value != value._value;
		}

	private:
		uint64_t _value;

		inline static auto _crcTable1 = hash::CRC::createTable<32>(0x04C11DB7U);
		inline static auto _crcTable2 = hash::CRC::createTable<32>(0x814141ABU);

		static uint64_t AE_CALL _gen(const void* data, size_t size);
	};


	class AE_FW_DLL RenderTagCollection {
		AE_REF_OBJECT(RenderTagCollection)
	public:
		inline void AE_CALL add(const RenderTag& tag) {
			_tags.emplace(tag);
		}

		inline void AE_CALL remove(const RenderTag& tag) {
			_tags.erase(tag);
		}

		inline bool AE_CALL has(const RenderTag& tag) const {
			return _tags.find(tag) != _tags.end();
		}

	private:
		std::unordered_set<RenderTag, RenderTag::StdUnorderedHasher, RenderTag::StdUnorderedComparer> _tags;
	};
}