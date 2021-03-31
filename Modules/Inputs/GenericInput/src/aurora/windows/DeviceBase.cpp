#include "DeviceBase.h"
#include "Input.h"

namespace aurora::modules::inputs::generic_input {
	DeviceBase::DeviceBase(Input& input, const InternalDeviceInfo& info) :
		_input(input),
		_info(info),
		_handle(INVALID_HANDLE_VALUE),
		_inputBuffer(nullptr),
		_isReadPending(false),
		_receivedLength(0) {
		memset(&_oRead, 0, sizeof(_oRead));
		_oRead.hEvent = CreateEventW(nullptr, false, false, nullptr);
	}

	DeviceBase::~DeviceBase() {
		if (_inputBuffer) delete[] _inputBuffer;
		if (_oRead.hEvent) CloseHandle(_oRead.hEvent);
		if (_handle != INVALID_HANDLE_VALUE) CloseHandle(_handle);
	}

	events::IEventDispatcher<DeviceEvent>& DeviceBase::getEventDispatcher() {
		return _eventDispatcher;
	}

	const DeviceInfo& DeviceBase::getInfo() const {
		return _info;
	}

	bool DeviceBase::open() {
		auto path = _info.devicePath.data();
		DWORD desiredAccess = GENERIC_READ | GENERIC_WRITE;
		DWORD shareMode = FILE_SHARE_READ | FILE_SHARE_WRITE;

		_handle = CreateFileW(path, desiredAccess, shareMode, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
		if (_handle == INVALID_HANDLE_VALUE) {
			desiredAccess = STANDARD_RIGHTS_READ;
			_handle = CreateFileW(path, desiredAccess, shareMode, nullptr, OPEN_EXISTING, FILE_FLAG_OVERLAPPED, nullptr);
		}

		if (_handle == INVALID_HANDLE_VALUE) return false;

		PHIDP_PREPARSED_DATA preparsedData = nullptr;
		if (!HidD_GetPreparsedData(_handle, &preparsedData)) goto err;

		HIDP_CAPS cap;
		if (HidP_GetCaps(preparsedData, &cap) != HIDP_STATUS_SUCCESS) goto err;
		
		_inputBufferLength = cap.InputReportByteLength;
		_inputBuffer = new BYTE[_inputBufferLength];

		HidD_FreePreparsedData(preparsedData);

		return true;

	err:
		HidD_FreePreparsedData(preparsedData);
		return false;
	}

	void DeviceBase::_read() {
		if (!_isReadPending) {
			_isReadPending = true;
			_receivedLength = 0;

			DWORD n;
			ResetEvent(_oRead.hEvent);
			if (ReadFile(_handle, _inputBuffer, _inputBufferLength, &n, &_oRead)) {
				_receivedLength = n;
				_isReadPending = false;
			} else {
				if (GetLastError() != ERROR_IO_PENDING) {
					CancelIo(_handle);
					_isReadPending = false;
				}
			}
		}

		if (_isReadPending) {
			DWORD n;
			if (GetOverlappedResult(_handle, &_oRead, &n, false)) {
				_receivedLength = n;
				_isReadPending = false;
			} else {
				if (GetLastError() != ERROR_IO_INCOMPLETE) {
					CancelIo(_handle);
					_isReadPending = false;
				}
			}
		}

		if (_isReadPending || _receivedLength == 0) return;

		_parse();
	}
}