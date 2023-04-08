#include "Input.h"
#include "GamepadDriver.h"
#include "CreateModule.h"
#include "srk/String.h"

#include <hidsdi.h>
#include <SetupAPI.h>

namespace srk::modules::inputs::xinput {
	Input::Input(Ref* loader, const CreateInputModuleDescriptor& desc) :
		_loader(loader),
		_filters(desc.filters),
		_eventDispatcher(new events::EventDispatcher<ModuleEvent>()),
		//_XInputGetCapabilitiesEx(nullptr) {
		_XInputGetCapabilitiesEx(((XInputGetCapabilitiesEx)GetProcAddress(GetModuleHandleW(L"XInput1_4.dll"), (LPCSTR)108))) {
	}

	Input::~Input() {
	}

	IntrusivePtr<events::IEventDispatcher<ModuleEvent>> Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		using namespace srk::enum_operators;

		if ((DeviceType::GAMEPAD & _filters) == DeviceType::UNKNOWN) return;

		std::vector<DeviceInfo> newDevices;

		InternalGUID guid;

		XINPUT_STATE state;
		XINPUT_CAPABILITIES_EX capsEx;
		size_t hasIdCount = 0;
		for (decltype(XUSER_MAX_COUNT) i = 0; i < XUSER_MAX_COUNT; ++i) {
			guid.index = i + 1;

			if (XInputGetState(i, &state) == ERROR_SUCCESS) {
				auto found = false;

				auto& info = newDevices.emplace_back();
				info.type = DeviceType::GAMEPAD;

				if (_XInputGetCapabilitiesEx && _XInputGetCapabilitiesEx(1, i, 0, &capsEx) == ERROR_SUCCESS) {
					info.vendorID = capsEx.vendorId;
					info.productID = capsEx.productId;

					guid.vendorID = info.vendorID;
					guid.productID = info.productID;

					++hasIdCount;
				} else {
					guid.vendorID = 0;
					guid.productID = 0;
				}

				info.guid.set<false, true>(&guid, sizeof(guid));
			}
		}

		if (hasIdCount) {
			size_t foundCount = 0;

			::GUID guid;
			HidD_GetHidGuid(&guid);

			if (auto hDevInfo = SetupDiGetClassDevsW(&guid, nullptr, nullptr, DIGCF_PRESENT | DIGCF_DEVICEINTERFACE); hDevInfo) {
				SP_DEVICE_INTERFACE_DATA deviceInterfaceData;
				deviceInterfaceData.cbSize = sizeof(SP_DEVICE_INTERFACE_DATA);

				constexpr size_t stackDetailMemSize = 256;
				uint8_t stackDetailMem[stackDetailMemSize];
				auto detail = (PSP_DEVICE_INTERFACE_DETAIL_DATA_A)stackDetailMem;
				detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
				size_t mallocDetailSize = stackDetailMemSize;
				void* heapDetailMem = nullptr;

				HIDD_ATTRIBUTES attrib;
				attrib.Size = sizeof(HIDD_ATTRIBUTES);

				WCHAR devNameBuf[256];
				std::string devName;

				for (DWORD i = 0; SetupDiEnumDeviceInterfaces(hDevInfo, nullptr, &guid, i, &deviceInterfaceData) != 0; ++i) {
					DWORD requiredSize = 0;

					if (SetupDiGetDeviceInterfaceDetailA(hDevInfo, &deviceInterfaceData, nullptr, 0, &requiredSize, nullptr); requiredSize == 0) continue;

					if (mallocDetailSize < requiredSize) {
						if (heapDetailMem) free(heapDetailMem);

						mallocDetailSize = requiredSize;
						if (heapDetailMem = malloc(requiredSize); !heapDetailMem) break;

						detail = (PSP_INTERFACE_DEVICE_DETAIL_DATA_A)heapDetailMem;
						detail->cbSize = sizeof(SP_DEVICE_INTERFACE_DETAIL_DATA_A);
					}

					if (!SetupDiGetDeviceInterfaceDetailA(hDevInfo, &deviceInterfaceData, detail, requiredSize, nullptr, nullptr)) continue;

					auto hidHandle = CreateFileA(detail->DevicePath, 0, FILE_SHARE_READ, nullptr, OPEN_EXISTING, 0, nullptr);
					if (hidHandle == INVALID_HANDLE_VALUE) continue;

					if (HidD_GetAttributes(hidHandle, &attrib)) {
						for (auto& info : newDevices) {
							if (info.vendorID == attrib.VendorID && info.productID == attrib.ProductID) {
								if (devName.empty()) {
									if (!HidD_GetProductString(hidHandle, devNameBuf, sizeof(devNameBuf))) break;

									devName = String::wideToUtf8<std::string>(devNameBuf);
								}
								info.name = devName;

								if (++foundCount == hasIdCount) break;
							}
						}
					}

					CloseHandle(hidHandle);

					if (foundCount >= hasIdCount) break;
				}

				if (heapDetailMem) free(heapDetailMem);
				SetupDiDestroyDeviceInfoList(hDevInfo);
			}
		}

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
		DeviceInfo info;
		auto found = false;

		{
			std::shared_lock lock(_mutex);

			for (auto& i : _devices) {
				if (i.guid == guid) {
					info = i;
					found = true;

					break;
				}
			}
		}

		if (!found) return nullptr;

		auto& data = (const InternalGUID&)*info.guid.getData();

		XINPUT_CAPABILITIES_EX capsEx;
		uint16_t vendorID = 0, productID = 0;
		if (_XInputGetCapabilitiesEx && _XInputGetCapabilitiesEx(1, data.index - 1, 0, &capsEx) == ERROR_SUCCESS) {
			vendorID = capsEx.vendorId;
			productID = capsEx.productId;
		}
		if (info.vendorID != vendorID || info.productID != productID) return nullptr;

		return new GenericGamepad(info, *new GamepadDriver(*this, data.index - 1));
	}
}