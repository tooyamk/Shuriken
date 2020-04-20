#pragma once

#include "aurora/hash/CRC.h"

namespace aurora::render {
	class AE_DLL RenderTag {
	public:
		struct std_compare {
			inline bool operator()(const RenderTag& value1, const RenderTag& value2) const {
				return value1._value < value2._value;
			}
		};


		struct std_unordered_compare {
			inline bool operator()(const RenderTag& value1, const RenderTag& value2) const {
				return value1 == value2;
			}
		};


		struct std_unordered_key {
			inline size_t operator()(const RenderTag& value) const {
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
}