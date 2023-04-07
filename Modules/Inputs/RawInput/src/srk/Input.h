#pragma once

#include "Base.h"
#include <shared_mutex>

namespace srk::modules::inputs::raw_input {
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

		void SRK_CALL registerRawInputDevices(DeviceType type);
		void SRK_CALL unregisterRawInputDevices(DeviceType type);

	private:
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<windows::IWindow> _win;
		DeviceType _filters;
		IntrusivePtr<events::IEventDispatcher<ModuleEvent>> _eventDispatcher;

		HWND _hwnd;

		std::shared_mutex _mutex;
		std::vector<InternalDeviceInfo> _devices;

		std::mutex _numMutex;
		uint32_t _numKeyboards, _numMouses;

		uint32_t* SRK_CALL _getNumVal(DeviceType type);

		void SRK_CALL _registerDevices(DeviceType type, bool remove);

		inline static bool SRK_CALL _hasDevice(const InternalDeviceInfo& info, const std::vector<InternalDeviceInfo>& devices) {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}