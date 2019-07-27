#pragma once

#include "base/LowLevel.h"

namespace aurora::modules::inputs {
	class AE_DLL GUID {
	public:
		GUID();
		GUID(const GUID& value);
		GUID(GUID&& value);
		~GUID();

		inline const uint8_t* AE_CALL getData() const {
			return _data;
		}

		inline uint32_t AE_CALL getSize() const {
			return _len;
		}

		void AE_CALL set(const uint8_t* data, uint32_t len);
		bool AE_CALL isEqual(const uint8_t* data, uint32_t len) const;

		inline GUID& AE_CALL operator=(const GUID& value) {
			_len = value._len;
			_data = new uint8_t[_len];
			memcpy(_data, value._data, _len);

			return *this;
		}

		inline GUID& AE_CALL operator=(GUID&& value) noexcept {
			_data = value._data;
			_len = value._len;
			value._data = nullptr;

			return *this;
		}

		inline bool AE_CALL operator==(const GUID& right) const {
			return isEqual(right._data, right._len);
		}

	private:
		uint8_t* _data;
		uint32_t _len;
	};
}