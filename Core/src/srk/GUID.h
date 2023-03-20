#pragma once

#include "srk/Global.h"

namespace srk::modules::inputs {
	template<uint32_t N>
	class GUID {
	public:
		using Data = uint8_t[N];

		GUID(nullptr_t) {}
		GUID() {
			memset(_data, 0, sizeof(Data));
		}
		GUID(const GUID& value) {
			memcpy(_data, value._data, sizeof(Data));
		}
		GUID(GUID&& value) {
			memcpy(_data, value._data, sizeof(Data));
		}
		~GUID() {
		}

		inline const Data& SRK_CALL getData() const {
			return _data;
		}

		inline constexpr uint32_t SRK_CALL getSize() const {
			return N;
		}

		template<bool LowCompare, bool HighCompare>
		bool SRK_CALL equal(const void* data, uint32_t len, uint32_t offset = 0, uint8_t lowCompareVal = 0, uint8_t highCompareVal = 0) const {
			auto data8 = (const uint8_t*)data;

			if constexpr (LowCompare) {
				if (offset) {
					uint32_t end = offset > N ? N : offset;
					for (uint32_t i = 0; i < end; ++i) {
						if (_data[i] != lowCompareVal) return false;
					}
				}
			}

			if (offset + 1 < N) {
				uint32_t end = offset + len;
				if (end > N) end = N;

				if (data8) {
					for (uint32_t i = offset; i < end; ++i) {
						if (_data[i] != data8[i - offset]) return false;
					}
				}

				if constexpr (HighCompare) {
					if (end < N) {
						for (uint32_t i = end; i < N; ++i) {
							if (_data[i] != highCompareVal) return false;
						}
					}
				}
			}

			return true;
		}

		template<bool LowFill, bool HighFill>
		void SRK_CALL set(const void* data, uint32_t len, uint32_t offset = 0, uint8_t lowFillVal = 0, uint8_t highFillVal = 0) {
			if constexpr (LowFill) {
				if (offset) memset(_data, lowFillVal, offset > N ? N : offset);
			}

			if (offset + 1 < N) {
				uint32_t end = offset + len;
				if (end > N) end = N;

				memcpy(_data + offset, data, end - offset);

				if constexpr (HighFill) {
					if (end < N) memset(_data + end, highFillVal, N - end);
				}
			}
		}

		void SRK_CALL set(uint8_t val, uint32_t len, uint32_t offset = 0) {
			if (offset + 1 < N) {
				uint32_t end = offset + len;
				if (end > N) end = N;
				memset(_data + offset, val, end - offset);
			}
		}

		inline GUID& SRK_CALL operator=(const GUID& value) {
			memcpy(_data, value._data, N);

			return *this;
		}

		inline GUID& SRK_CALL operator=(GUID&& value) noexcept {
			memcpy(_data, value._data, N);

			return *this;
		}

		inline bool SRK_CALL operator==(const GUID& val) const {
			return !memcmp(this, &val, sizeof(GUID));
		}

		inline bool SRK_CALL operator!=(const GUID& val) const {
			return memcmp(this, &val, sizeof(GUID));
		}

	private:
		Data _data;
	};
}