#include "Input.h"
#include "GamepadDriver.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "CreateModule.h"

#include <wbemidl.h>
#include <oleauto.h>

namespace aurora::modules::inputs::direct_input {
	Input::Input(Ref* loader, IApplication* app, DeviceType filter, bool ignoreXInputDevices) :
		_loader(loader),
		_app(app),
		_filter(filter),
		_eventDispatcher(new events::EventDispatcher<ModuleEvent>()),
		_ignoreXInputDevices(ignoreXInputDevices),
		_di(nullptr) {
	}

	Input::~Input() {
		SAFE_RELEASE(_di);
	}

	IntrusivePtr<events::IEventDispatcher<ModuleEvent>> Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		if (!_di && FAILED(DirectInput8Create((HINSTANCE)_app->getNative(ApplicationNative::HINSTANCE), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID*)&_di, nullptr))) return;

		std::vector<InternalDeviceInfo> newDevices;

		EnumDevicesData data;
		data.filter = _filter;
		data.ignoreXInputDevices = _ignoreXInputDevices;
		data.devices = &newDevices;

		_di->EnumDevices(DI8DEVCLASS_ALL, _enumDevicesCallback, &data, DIEDFL_ATTACHEDONLY);

		std::vector<DeviceInfo> add;
		std::vector<DeviceInfo> remove;
		{
			std::scoped_lock lock(_mutex);

			for (auto& info : newDevices) {
				if (!_hasDevice(info, _devices)) add.emplace_back(info);
			}

			for (auto& info : _devices) {
				if (!_hasDevice(info, newDevices)) remove.emplace_back(info);
			}

			_devices = std::move(newDevices);
		}

		for (auto& info : remove) _eventDispatcher->dispatchEvent(this, ModuleEvent::DISCONNECTED, &info);
		for (auto& info : add) _eventDispatcher->dispatchEvent(this, ModuleEvent::CONNECTED, &info);
	}

	IntrusivePtr<IInputDevice> Input::createDevice(const DeviceGUID& guid) {
		std::shared_lock lock(_mutex);

		for (auto& info : _devices) {
			if (info.guid == guid) {
				LPDIRECTINPUTDEVICE8 dev = nullptr;
				if (FAILED(_di->CreateDevice(*(const ::GUID*)guid.getData(), &dev, nullptr))) return nullptr;

				switch (info.type) {
				case DeviceType::GAMEPAD:
					return new GenericGamepad(info, *new GamepadDriver(*this, dev));
				case DeviceType::KEYBOARD:
					return new Keyboard(*this, dev, info);
				case DeviceType::MOUSE:
					return new Mouse(*this, dev, info);
				default:
					dev->Release();
					return nullptr;
				}
			}
		}

		return nullptr;
	}

	HWND Input::getHWND() const {
		return (HWND)_app->getNative(ApplicationNative::HWND);
	}

	bool Input::isXInputDevice(const ::GUID& guidProduct) {
		IWbemLocator* pIWbemLocator = nullptr;
		IEnumWbemClassObject* pEnumDevices = nullptr;
		IWbemClassObject* pDevices[20] = { 0 };
		IWbemServices* pIWbemServices = nullptr;
		BSTR                    bstrNamespace = nullptr;
		BSTR                    bstrDeviceID = nullptr;
		BSTR                    bstrClassName = nullptr;
		DWORD                   uReturned = 0;
		bool                    bIsXinputDevice = false;
		UINT                    iDevice = 0;
		VARIANT                 var;
		HRESULT                 hr;

		// CoInit if needed
		hr = CoInitialize(nullptr);
		bool bCleanupCOM = SUCCEEDED(hr);

		// So we can call VariantClear() later, even if we never had a successful IWbemClassObject::Get().
		VariantInit(&var);

		// Create WMI
		hr = CoCreateInstance(__uuidof(WbemLocator), nullptr, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID*)&pIWbemLocator);
		if (FAILED(hr) || !pIWbemLocator) goto LCleanup;

		bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2"); if (!bstrNamespace) goto LCleanup;
		bstrClassName = SysAllocString(L"Win32_PNPEntity");   if (!bstrClassName) goto LCleanup;
		bstrDeviceID = SysAllocString(L"DeviceID");          if (!bstrDeviceID)  goto LCleanup;

		// Connect to WMI 
		hr = pIWbemLocator->ConnectServer(bstrNamespace, nullptr, nullptr, 0L, 0L, nullptr, nullptr, &pIWbemServices);
		if (FAILED(hr) || !pIWbemServices) goto LCleanup;

		// Switch security level to IMPERSONATE. 
		CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, nullptr, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, nullptr, EOAC_NONE);

		hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, nullptr, &pEnumDevices);
		if (FAILED(hr) || pEnumDevices == nullptr) goto LCleanup;

		// Loop over all devices
		for (;;) {
			// Get 20 at a time
			hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
			if (FAILED(hr)) goto LCleanup;
			if (uReturned == 0) break;

			for (iDevice = 0; iDevice < uReturned; ++iDevice) {
				// For each device, get its device ID
				hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, nullptr, nullptr);
				if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal) {
					// Check if the device ID contains "IG_".  If it does, then it's an XInput device
						// This information can not be found from DirectInput 
					if (wcsstr(var.bstrVal, L"IG_")) {
						// If it does, then get the VID/PID from var.bstrVal
						DWORD dwPid = 0, dwVid = 0;
						WCHAR* strVid = wcsstr(var.bstrVal, L"VID_");
						if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1) dwVid = 0;
						WCHAR* strPid = wcsstr(var.bstrVal, L"PID_");
						if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1) dwPid = 0;

						// Compare the VID/PID to the DInput device
						DWORD dwVidPid = MAKELONG(dwVid, dwPid);
						if (dwVidPid == guidProduct.Data1) {
							bIsXinputDevice = true;
							goto LCleanup;
						}
					}
				}
				VariantClear(&var);
				SAFE_RELEASE(pDevices[iDevice]);
			}
		}

	LCleanup:
		VariantClear(&var);
		if (bstrNamespace) SysFreeString(bstrNamespace);
		if (bstrDeviceID) SysFreeString(bstrDeviceID);
		if (bstrClassName) SysFreeString(bstrClassName);
		for (iDevice = 0; iDevice < 20; ++iDevice) SAFE_RELEASE(pDevices[iDevice]);
		SAFE_RELEASE(pEnumDevices);
		SAFE_RELEASE(pIWbemLocator);
		SAFE_RELEASE(pIWbemServices);

		if (bCleanupCOM) CoUninitialize();

		return bIsXinputDevice;
	}

	BOOL Input::_enumDevicesCallback(const DIDEVICEINSTANCE* pdidInstance, LPVOID pContext) {
		using namespace aurora::enum_operators;

		uint32_t type = pdidInstance->dwDevType & 0xFF;
		if (type == DI8DEVTYPE_MOUSE || type == DI8DEVTYPE_KEYBOARD || type == DI8DEVTYPE_GAMEPAD) {
			auto data = (EnumDevicesData*)pContext;

			DeviceType dt = DeviceType::UNKNOWN;
			switch (type) {
			case DI8DEVTYPE_MOUSE:
				dt |= DeviceType::MOUSE;
				break;
			case DI8DEVTYPE_KEYBOARD:
				dt |= DeviceType::KEYBOARD;
				break;
			case DI8DEVTYPE_GAMEPAD:
				dt |= DeviceType::GAMEPAD;
				break;
			default:
				break;
			}

			if ((dt & data->filter) == DeviceType::UNKNOWN) return DIENUM_CONTINUE;

			bool isXInput = false;
			if (type == DI8DEVTYPE_GAMEPAD) {
				isXInput = isXInputDevice(pdidInstance->guidProduct);
				if (isXInput && data->ignoreXInputDevices) return DIENUM_CONTINUE;
			}

			auto& info = data->devices->emplace_back();
			info.guid.set<false, true>(&pdidInstance->guidInstance, sizeof(::GUID));
			info.type = dt;
			info.vendorID = pdidInstance->guidProduct.Data1 & 0xFFFF;
			info.productID = pdidInstance->guidProduct.Data1 >> 16 & 0xFFFF;
			info.isXInput = isXInput;
		}

		return DIENUM_CONTINUE;
	}
}