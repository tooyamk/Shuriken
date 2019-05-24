#include "Gamepad.h"
#include "Input.h"

#include <wbemidl.h>
#include <oleauto.h>
#include <dinput.h>

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)       { if (p) { (p)->Release();  (p) = nullptr; } }
#endif

namespace aurora::modules::inputs::win_direct_input {
	const Gamepad::KeyMapping Gamepad::DIRECT{
			0, 1, 2, 5, 3, 4,
			{
			{ 0, GamepadKeyCode::X },
			{ 1, GamepadKeyCode::A },
			{ 2, GamepadKeyCode::B },
			{ 3, GamepadKeyCode::Y },
			{ 4, GamepadKeyCode::LEFT_SHOULDER },
			{ 5, GamepadKeyCode::RIGHT_SHOULDER },
			{ 6, GamepadKeyCode::LEFT_TRIGGER },
			{ 7, GamepadKeyCode::RIGHT_TRIGGER },
			{ 8, GamepadKeyCode::SELECT },
			{ 9, GamepadKeyCode::START },
			{ 10, GamepadKeyCode::LEFT_THUMB },
			{ 11, GamepadKeyCode::RIGHT_THUMB }
			}
	};

	const Gamepad::KeyMapping Gamepad::XINPUT{
			0, 1, 3, 4, 2, 2,
			{
			{ 0, GamepadKeyCode::A },
			{ 1, GamepadKeyCode::B },
			{ 2, GamepadKeyCode::X },
			{ 3, GamepadKeyCode::Y },
			{ 4, GamepadKeyCode::LEFT_SHOULDER },
			{ 5, GamepadKeyCode::RIGHT_SHOULDER },
			{ 6, GamepadKeyCode::SELECT },
			{ 7, GamepadKeyCode::START },
			{ 8, GamepadKeyCode::LEFT_THUMB },
			{ 9, GamepadKeyCode::RIGHT_THUMB }
			}
	};

	const Gamepad::KeyMapping Gamepad::DS4{
			0, 1, 2, 5, 3, 4,
			{
			{ 0, GamepadKeyCode::X },
			{ 1, GamepadKeyCode::A },
			{ 2, GamepadKeyCode::B },
			{ 3, GamepadKeyCode::Y },
			{ 4, GamepadKeyCode::LEFT_SHOULDER },
			{ 5, GamepadKeyCode::RIGHT_SHOULDER },
			{ 6, GamepadKeyCode::LEFT_TRIGGER },
			{ 7, GamepadKeyCode::RIGHT_TRIGGER },
			{ 8, GamepadKeyCode::SELECT },
			{ 9, GamepadKeyCode::START },
			{ 10, GamepadKeyCode::LEFT_THUMB },
			{ 11, GamepadKeyCode::RIGHT_THUMB },
			{ 13, GamepadKeyCode::TOUCH_PAD }
			}
	};

	Gamepad::Gamepad(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info) : DeviceBase(input, dev, info),
		_keyMapping(nullptr) {
		_dev->SetDataFormat(&c_dfDIJoystick2);
		_dev->SetCooperativeLevel(_input.get()->getHWND(), DISCL_NONEXCLUSIVE | DISCL_BACKGROUND);

		if (_isXInputDevice(*(const ::GUID*)info.guid.getData())) {
			_keyMapping = &XINPUT;
		} else {
			auto data = (const ui16*)info.guid.getData();
			auto vender = data[0];
			auto product = data[1];
			switch (data[0]) {
			case 0x054C:
			{
				if (product == 0x05C4 || product == 0x09CC) {
					_keyMapping = &DS4;
				}

				break;
			}
			default:
				break;
			}
		}

		if (!_keyMapping) _keyMapping = &DIRECT;
		for (auto& itr : _keyMapping->BUTTONS) _enumToKeyMapping.try_emplace(itr.second, itr.first);

		memset(&_state, 0, sizeof(_state));
		memset(&_state.rgdwPOV, 0xFF, sizeof(_state.rgdwPOV));
		auto axis = &_state.lX;
		axis[_keyMapping->LSTICK_X] = 32767;
		axis[_keyMapping->LSTICK_Y] = 32767;
		axis[_keyMapping->RSTICK_X] = 32767;
		axis[_keyMapping->RSTICK_Y] = 32767;
		if (_keyMapping->LTRIGGER == _keyMapping->RTRIGGER) axis[_keyMapping->LTRIGGER] = 32767;

		setDeadZone((ui8)GamepadKeyCode::LEFT_STICK, .05f);
		setDeadZone((ui8)GamepadKeyCode::RIGHT_STICK, .05f);
		setDeadZone((ui8)GamepadKeyCode::LEFT_TRIGGER, .05f);
		setDeadZone((ui8)GamepadKeyCode::RIGHT_TRIGGER, .05f);
	}

	ui32 Gamepad::getKeyState(ui32 keyCode, f32* data, ui32 count) const {
		if (data && count) {
			switch ((GamepadKeyCode)keyCode) {
			case GamepadKeyCode::LEFT_STICK:
			{
				auto axis = &_state.lX;
				return _getStick(axis[_keyMapping->LSTICK_X], axis[_keyMapping->LSTICK_Y], (GamepadKeyCode)keyCode, data, count);
			}
			case GamepadKeyCode::RIGHT_STICK:
			{
				auto axis = &_state.lX;
				return _getStick(axis[_keyMapping->RSTICK_X], axis[_keyMapping->RSTICK_Y], (GamepadKeyCode)keyCode, data, count);
			}
			case GamepadKeyCode::LEFT_TRIGGER:
			{
				auto axis = &_state.lX;
				return _keyMapping->LTRIGGER == _keyMapping->RTRIGGER ? _getTrigger(axis[_keyMapping->LTRIGGER], (GamepadKeyCode)keyCode, 0, data[0]) : _getTriggerSeparate(axis[_keyMapping->LTRIGGER], (GamepadKeyCode)keyCode, data[0]);
			}
			case GamepadKeyCode::RIGHT_TRIGGER:
			{
				auto axis = &_state.lX;
				return _keyMapping->LTRIGGER == _keyMapping->RTRIGGER ? _getTrigger(axis[_keyMapping->RTRIGGER], (GamepadKeyCode)keyCode, 1, data[0]) : _getTriggerSeparate(axis[_keyMapping->RTRIGGER], (GamepadKeyCode)keyCode, data[0]);
			}
			case GamepadKeyCode::DPAD:
				data[0] = _translateDpad(_state.rgdwPOV[0]);
				return 1;
			default:
			{
				if (auto itr = _enumToKeyMapping.find((GamepadKeyCode)keyCode); itr != _enumToKeyMapping.end()) {
					data[0] = _translateButton(_state.rgbButtons[itr->second]);

					return 1;
				}

				if (keyCode >= (ui8)GamepadKeyCode::UNDEFINED) {
					data[0] = _translateButton(_state.rgbButtons[keyCode - (ui8)GamepadKeyCode::UNDEFINED]);

					return 1;
				}

				return 0;
			}
			}
		}
		return 0;
	}

	void Gamepad::poll(bool dispatchEvent) {
		if (auto hr = _dev->Poll(); hr == DIERR_NOTACQUIRED || DIERR_INPUTLOST) {
			if (FAILED(_dev->Acquire())) return;
			if (FAILED(_dev->Poll())) return;
		}

		DIJOYSTATE2 state;
		if (FAILED(_dev->GetDeviceState(sizeof(state), &state))) return;
		if (_checkInvalidData(state)) return;

		if (!dispatchEvent) {
			memcpy(&_state, &state, sizeof(_state));
			return;
		}

		auto oriAxis = &_state.lX;
		auto curAxis = &state.lX;

		LONG oriLStickX, oriLStickY, oriRStickX, oriRStickY;
		auto ls = false, rs = false;
		if (oriAxis[_keyMapping->LSTICK_X] != curAxis[_keyMapping->LSTICK_X] || oriAxis[_keyMapping->LSTICK_Y] != curAxis[_keyMapping->LSTICK_Y]) {
			oriLStickX = oriAxis[_keyMapping->LSTICK_X];
			oriLStickY = oriAxis[_keyMapping->LSTICK_Y];
			oriAxis[_keyMapping->LSTICK_X] = curAxis[_keyMapping->LSTICK_X];
			oriAxis[_keyMapping->LSTICK_Y] = curAxis[_keyMapping->LSTICK_Y];
			ls = true;
		}
		if (oriAxis[_keyMapping->RSTICK_X] != curAxis[_keyMapping->RSTICK_X] || oriAxis[_keyMapping->RSTICK_Y] != curAxis[_keyMapping->RSTICK_Y]) {
			oriRStickX = oriAxis[_keyMapping->RSTICK_X];
			oriRStickY = oriAxis[_keyMapping->RSTICK_Y];
			oriAxis[_keyMapping->RSTICK_X] = curAxis[_keyMapping->RSTICK_X];
			oriAxis[_keyMapping->RSTICK_Y] = curAxis[_keyMapping->RSTICK_Y];
			rs = true;
		}

		auto lt = false, rt = false;
		LONG oriLT, oriRT;
		if (oriAxis[_keyMapping->LTRIGGER] != curAxis[_keyMapping->LTRIGGER]) {
			oriLT = oriAxis[_keyMapping->LTRIGGER];
			oriAxis[_keyMapping->LTRIGGER] = curAxis[_keyMapping->LTRIGGER];
			lt = true;
		}
		if (_keyMapping->LTRIGGER != _keyMapping->RTRIGGER && oriAxis[_keyMapping->RTRIGGER] != curAxis[_keyMapping->RTRIGGER]) {
			oriRT = oriAxis[_keyMapping->RTRIGGER];
			oriAxis[_keyMapping->RTRIGGER] = curAxis[_keyMapping->RTRIGGER];
			rt = true;
		}

		ui8 changedBtns[sizeof(state.rgbButtons)];
		ui8 changedBtnsLen = 0;
		for (ui8 i = 0; i < sizeof(state.rgbButtons); ++i) {
			if (_state.rgbButtons[i] != state.rgbButtons[i]) {
				_state.rgbButtons[i] = state.rgbButtons[i];
				changedBtns[changedBtnsLen++] = i;
			}
		}

		ui8 changedPov[4];
		ui8 changedPovLen = 0;
		for (ui8 i = 0; i < sizeof(state.rgdwPOV); ++i) {
			if (_state.rgdwPOV[i] != state.rgdwPOV[i]) {
				_state.rgdwPOV[i] = state.rgdwPOV[i];
				changedPov[changedPovLen++] = i;
			}
		}

		if (ls) _updateStick(oriLStickX, oriLStickY, curAxis[_keyMapping->LSTICK_X], curAxis[_keyMapping->LSTICK_Y], GamepadKeyCode::LEFT_STICK);
		if (rs) _updateStick(oriRStickX, oriRStickY, curAxis[_keyMapping->RSTICK_X], curAxis[_keyMapping->RSTICK_Y], GamepadKeyCode::RIGHT_STICK);

		if (_keyMapping->LTRIGGER == _keyMapping->RTRIGGER) {
			if (lt) _updateTrigger(oriLT, curAxis[_keyMapping->LTRIGGER], GamepadKeyCode::LEFT_TRIGGER, GamepadKeyCode::RIGHT_TRIGGER);
		} else {
			if (lt) _updateTriggerSeparate(oriLT, curAxis[_keyMapping->LTRIGGER], GamepadKeyCode::LEFT_TRIGGER);
			if (rt) _updateTriggerSeparate(oriRT, curAxis[_keyMapping->RTRIGGER], GamepadKeyCode::RIGHT_TRIGGER);
		}

		if (changedPovLen) {
			for (ui8 i = 0; i < changedPovLen; ++i) {
				ui8 key = changedPov[i];
				if (key == 0) {
					f32 value = _translateDpad(state.rgdwPOV[key]);
					_eventDispatcher.dispatchEvent(this, value >= 0.f ? DeviceEvent::DOWN : DeviceEvent::UP, &Key({ (ui8)GamepadKeyCode::DPAD, 1, &value }));
				}
			}
		}

		if (changedBtnsLen) {
			for (ui8 i = 0; i < changedBtnsLen; ++i) {
				ui8 key = changedBtns[i];
				f32 value = _translateButton(state.rgbButtons[key]);

				auto itr = _keyMapping->BUTTONS.find(key);
				key = itr == _keyMapping->BUTTONS.end() ? key + (ui8)GamepadKeyCode::UNDEFINED : (ui8)itr->second;

				_eventDispatcher.dispatchEvent(this, value > 0.f ? DeviceEvent::DOWN : DeviceEvent::UP, &Key({ key, 1, &value }));
			}
		}
	}

	void Gamepad::setDeadZone(ui32 keyCode, f32 deadZone) {
		if (deadZone < 0.f) deadZone = -deadZone;

		if (auto itr = _deadZone.find(keyCode); itr == _deadZone.end()) {
			_deadZone.emplace(keyCode, deadZone);
		} else {
			itr->second = deadZone;
		}
	}

	bool Gamepad::_checkInvalidData(const DIJOYSTATE2& state) {
		auto axis = &state.lX;
		for (ui32 i = 0; i < 6; ++i) {
			if (axis[i] != 32767) return false;
		}

		const auto numButtons = sizeof(state.rgbButtons) / sizeof(state.rgbButtons[0]);
		for (ui32 i = 0; i < numButtons; ++i) {
			if (state.rgbButtons[i] != 0) return false;
		}

		return true;
	}

	f32 Gamepad::_translateStick(LONG value) {
		auto v = f32(value - 32767);
		if (v < 0.f) {
			v /= 32767.f;
		} else if (v > 0.f) {
			v /= 32768.f;
		}
		return v;
	}

	void Gamepad::_translateTrigger(LONG value, f32& l, f32& r) {
		if (value < 32767) {
			l = 0.f;
			r = f32(32767 - value) / 32767.f;
		} else if (value > 32767) {
			l =  f32(value - 32767) / 32768.f;
			r = 0.f;
		} else {
			l = 0.f;
			r = 0.f;
		}
	}

	ui32 Gamepad::_getStick(LONG x, LONG y, GamepadKeyCode key, f32* data, ui32 count) const {
		ui32 c = 1;

		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;

		auto dx = _translateStick(x);
		auto dy = _translateStick(y);

		auto d2 = dx * dx + dy * dy;
		if (d2 > 1.f) d2 = 1.f;

		if (d2 <= dz2) {
			data[0] = -1.0f;
			if (count > 1) data[c++] = 0.f;
		} else {
			data[0] = std::atan2(dy, dx) + Math::PI_2<f32>;
			if (data[0] < 0.f) data[0] += Math::PI2<f32>;
			if (count > 1) data[c++] = _translateDeadZone0_1(d2 < 1.f ? std::sqrt(d2) : 1.f, dz, false);
		}

		return c;
	}

	ui32 Gamepad::_getTrigger(LONG t, GamepadKeyCode key, ui8 index, f32& data) const {
		auto dz = _getDeadZone(key);

		f32 values[2];
		_translateTrigger(t, values[0], values[1]);
		data = values[index];
		data = _translateDeadZone0_1(data, dz, data <= dz);

		return 1;
	}

	ui32 Gamepad::_getTriggerSeparate(LONG t, GamepadKeyCode key, f32& data) const {
		auto dz = _getDeadZone(key);

		data = _translateTriggerSeparate(t);
		data = _translateDeadZone0_1(data, dz, data <= dz);

		return 1;
	}

	void Gamepad::_updateStick(LONG oriX, LONG oriY, LONG curX, LONG curY, GamepadKeyCode key) {
		f32 value[] = { _translateStick(curX) , _translateStick(curY) };
		auto dz = _getDeadZone(key);
		auto dz2 = dz * dz;
		auto x = _translateStick(oriX), y = _translateStick(oriY);
		auto oriDz = x * x + y * y <= dz2;
		auto d2 = value[0] * value[0] + value[1] * value[1];
		if (d2 > 1.f) d2 = 1.f;
		auto curDz = d2 <= dz2;
		if (!oriDz || oriDz != curDz) {
			if (curDz) {
				value[0] = -1.f;
				value[1] = 0.f;
			} else {
				value[0] = std::atan2(value[1], value[0]) + Math::PI_2<f32>;
				if (value[0] < 0.f) value[0] += Math::PI2<f32>;
				value[1] = _translateDeadZone0_1(d2 < 1.f ? std::sqrt(d2) : 1.f, dz, false);
			}
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &Key({ (ui8)key, 2, value }));
		}
	}

	void Gamepad::_updateTrigger(LONG ori, LONG cur, GamepadKeyCode lkey, GamepadKeyCode rkey) {
		f32 oriValues[2], curValues[2];
		_translateTrigger(ori, oriValues[0], oriValues[1]);
		_translateTrigger(cur, curValues[0], curValues[1]);
		auto ldz = _getDeadZone(lkey);
		auto rdz = _getDeadZone(rkey);
		auto oriLDz = oriValues[0] <= ldz;
		auto curLDz = curValues[0] <= ldz;
		auto oriRDz = oriValues[1] <= rdz;
		auto curRDz = curValues[1] <= rdz;
		if (!curLDz || oriLDz != curLDz) {
			curValues[0] = _translateDeadZone0_1(curValues[0], ldz, curLDz);
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &Key({ (ui8)lkey, 1, &curValues[0] }));
		}
		if (!curRDz || oriRDz != curRDz) {
			curValues[1] = _translateDeadZone0_1(curValues[1], rdz, curRDz);
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &Key({ (ui8)rkey, 1, &curValues[1] }));
		}
	}

	void Gamepad::_updateTriggerSeparate(LONG ori, LONG cur, GamepadKeyCode key) {
		f32 value = _translateTriggerSeparate(cur);
		auto dz = _getDeadZone(key);
		auto oriDz = _translateTriggerSeparate(ori) <= dz;
		auto curDz = value <= dz;
		ori = cur;
		if (!curDz || oriDz != curDz) {
			value = _translateDeadZone0_1(value, dz, curDz);
			_eventDispatcher.dispatchEvent(this, DeviceEvent::MOVE, &Key({ (ui8)key, 1, &value }));
		}
	}

	bool Gamepad::_isXInputDevice(const ::GUID& guid) {
		IWbemLocator*           pIWbemLocator = NULL;
		IEnumWbemClassObject*   pEnumDevices = NULL;
		IWbemClassObject*       pDevices[20] = { 0 };
		IWbemServices*          pIWbemServices = NULL;
		BSTR                    bstrNamespace = NULL;
		BSTR                    bstrDeviceID = NULL;
		BSTR                    bstrClassName = NULL;
		DWORD                   uReturned = 0;
		bool                    bIsXinputDevice = false;
		UINT                    iDevice = 0;
		VARIANT                 var;
		HRESULT                 hr;

		// CoInit if needed
		hr = CoInitialize(NULL);
		bool bCleanupCOM = SUCCEEDED(hr);

		// Create WMI
		hr = CoCreateInstance(__uuidof(WbemLocator),
			NULL,
			CLSCTX_INPROC_SERVER,
			__uuidof(IWbemLocator),
			(LPVOID*)&pIWbemLocator);
		if (FAILED(hr) || pIWbemLocator == NULL)
			goto LCleanup;

		bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2"); if (bstrNamespace == NULL) goto LCleanup;
		bstrClassName = SysAllocString(L"Win32_PNPEntity");   if (bstrClassName == NULL) goto LCleanup;
		bstrDeviceID = SysAllocString(L"DeviceID");          if (bstrDeviceID == NULL)  goto LCleanup;

		// Connect to WMI 
		hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L,
			0L, NULL, NULL, &pIWbemServices);
		if (FAILED(hr) || pIWbemServices == NULL)
			goto LCleanup;

		// Switch security level to IMPERSONATE. 
		CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
			RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

		hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices);
		if (FAILED(hr) || pEnumDevices == NULL)
			goto LCleanup;

		// Loop over all devices
		for (;; ) {
			// Get 20 at a time
			hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
			if (FAILED(hr))
				goto LCleanup;
			if (uReturned == 0)
				break;

			for (iDevice = 0; iDevice < uReturned; iDevice++) {
				// For each device, get its device ID
				hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
				if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL) {
					// Check if the device ID contains "IG_".  If it does, then it's an XInput device
						// This information can not be found from DirectInput 
					if (wcsstr(var.bstrVal, L"IG_")) {
						// If it does, then get the VID/PID from var.bstrVal
						DWORD dwPid = 0, dwVid = 0;
						WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
						if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1)
							dwVid = 0;
						WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
						if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1)
							dwPid = 0;

						// Compare the VID/PID to the DInput device
						DWORD dwVidPid = MAKELONG(dwVid, dwPid);
						if (dwVidPid == guid.Data1) {
							bIsXinputDevice = true;
							goto LCleanup;
						}
					}
				}
				SAFE_RELEASE(pDevices[iDevice]);
			}
		}

	LCleanup:
		if (bstrNamespace) SysFreeString(bstrNamespace);
		if (bstrDeviceID) SysFreeString(bstrDeviceID);
		if (bstrClassName) SysFreeString(bstrClassName);
		for (iDevice = 0; iDevice < 20; iDevice++) SAFE_RELEASE(pDevices[iDevice]);
		SAFE_RELEASE(pEnumDevices);
		SAFE_RELEASE(pIWbemLocator);
		SAFE_RELEASE(pIWbemServices);
		

		if (bCleanupCOM) CoUninitialize();

		return bIsXinputDevice;
	}
}