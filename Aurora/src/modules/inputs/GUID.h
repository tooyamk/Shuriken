#pragma once

#include "base/LowLevel.h"

namespace aurora::modules::inputs {
	class AE_DLL GUID {
	public:
		GUID();
		GUID(const GUID& value);
		GUID(GUID&& value);
		~GUID();

		inline const i8* AE_CALL getData() const {
			return _data;
		}

		inline ui32 AE_CALL getSize() const {
			return _len;
		}

		void AE_CALL set(const i8* data, ui32 len);
		bool AE_CALL isEqual(const i8* data, ui32 len) const;

		inline GUID& AE_CALL operator=(const GUID& value) {
			_len = value._len;
			_data = new i8[_len];
			memcpy(_data, value._data, _len);

			return *this;
		}

		inline GUID& AE_CALL operator=(GUID&& value) {
			_data = value._data;
			_len = value._len;
			value._data = nullptr;

			return *this;
		}

		inline bool AE_CALL operator==(const GUID& right) const {
			return isEqual(right._data, right._len);
		}

	private:
		i8* _data;
		ui32 _len;
	};
}