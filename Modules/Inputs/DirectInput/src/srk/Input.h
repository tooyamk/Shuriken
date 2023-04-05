#pragma once

#include "Base.h"
#include "srk/modules/windows/WindowModule.h"
#include <shared_mutex>

namespace srk::modules::inputs::direct_input {
	class SRK_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, const CreateInputModuleDescriptor& desc);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<ModuleEvent>> SRK_CALL getEventDispatcher() override;
		virtual void SRK_CALL poll() override;
		virtual IntrusivePtr<IInputDevice> SRK_CALL createDevice(const DeviceGUID& guid) override;

		inline HWND SRK_CALL getHWND() const {
			return _hwnd;
		}

		static bool SRK_CALL isXInputDevice(const ::GUID& guidProduct);

	private:
		struct EnumDevicesData {
			DeviceType filter;
			bool ignoreXInputDevices;
			std::vector<DeviceInfo>* devices;
		};


		IntrusivePtr<Ref> _loader;
		IntrusivePtr<windows::IWindow> _win;
		DeviceType _filters;
		IntrusivePtr<events::IEventDispatcher<ModuleEvent>> _eventDispatcher;

		HWND _hwnd;

		std::shared_mutex _mutex;
		std::vector<DeviceInfo> _devices;

		bool _ignoreXInputDevices;
		srk_IDirectInput* _di;

		static BOOL CALLBACK _enumDevicesCallback(const srk_DIDEVICEINSTANCE* pdidInstance, LPVOID pContext);

		inline static bool SRK_CALL _hasDevice(const DeviceInfo& info, const std::vector<DeviceInfo>& devices) {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}