#pragma once

#include "aurora/Global.h"

namespace aurora::modules::inputs {
	template<uint32_t N>
	class AE_TEMPLATE_DLL GUID {
	public:
		using Data = uint8_t[N];

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

		inline const Data& AE_CALL getData() const {
			return _data;
		}

		inline constexpr uint32_t AE_CALL getSize() const {
			return N;
		}

		template<bool LowCompare, bool HighCompare>
		bool AE_CALL isEqual(const uint8_t* data, uint32_t len, uint32_t offset = 0, uint8_t lowCompareVal = 0, uint8_t highCompareVal = 0) const {
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

				if (data) {
					for (uint32_t i = offset; i < end; ++i) {
						if (_data[i] != data[i - offset]) return false;
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
		void AE_CALL set(const uint8_t* data, uint32_t len, uint32_t offset = 0, uint8_t lowFillVal = 0, uint8_t highFillVal = 0) {
			if constexpr (LowFill) {
				if (offset) {
					uint32_t end = offset > N ? N : offset;
					for (uint32_t i = 0; i < end; ++i) _data[i] = lowFillVal;
				}
			}

			if (offset + 1 < N) {
				uint32_t end = offset + len;
				if (end > N) end = N;

				if (data) {
					for (uint32_t i = offset; i < end; ++i) _data[i] = data[i - offset];
				}

				if constexpr (HighFill) {
					if (end < N) {
						for (uint32_t i = end; i < N; ++i) _data[i] = highFillVal;
					}
				}
			}
		}

		void AE_CALL set(uint8_t val, uint32_t len, uint32_t offset = 0) {
			if (offset + 1 < N) {
				uint32_t end = offset + len;
				if (end > N) end = N;
				for (uint32_t i = offset; i < end; ++i) _data[i] = val;
			}
		}

		inline GUID& AE_CALL operator=(const GUID& value) {
			memcpy(_data, value._data, N);

			return *this;
		}

		inline GUID& AE_CALL operator=(GUID&& value) noexcept {
			memcpy(_data, value._data, N);

			return *this;
		}

		inline bool AE_CALL operator==(const GUID& val) const {
			return memEqual<sizeof(GUID)>(this, &val);
		}

		inline bool AE_CALL operator!=(const GUID& val) const {
			return !memEqual<sizeof(GUID)>(this, &val);
		}

	private:
		Data _data;
	};
}