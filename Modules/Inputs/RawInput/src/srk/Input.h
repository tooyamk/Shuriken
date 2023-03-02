#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"

namespace srk::modules::inputs::raw_input {
	class SRK_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, const CreateInputModuleDesc& desc);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<ModuleEvent>> SRK_CALL getEventDispatcher() override;
		virtual void SRK_CALL poll() override;
		virtual IntrusivePtr<IInputDevice> SRK_CALL createDevice(const DeviceGUID& guid) override;

		HWND SRK_CALL getHWND() const;

		void SRK_CALL registerRawInputDevices(DeviceType type);
		void SRK_CALL unregisterRawInputDevices(DeviceType type);

	private:
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<windows::IWindow> _win;
		DeviceType _filters;
		IntrusivePtr<events::IEventDispatcher<ModuleEvent>> _eventDispatcher;

		std::shared_mutex _mutex;
		std::vector<InternalDeviceInfo> _devices;

		std::mutex _numMutex;
		std::uint32_t _numKeyboards, _numMouses;

		inline std::uint32_t* SRK_CALL _getNumVal(DeviceType type) {
			if (type == DeviceType::KEYBOARD) return &_numKeyboards;
			if (type == DeviceType::MOUSE) return &_numKeyboards;
			return nullptr;
		}

		template<bool Remove>
		void SRK_CALL _registerDevices(DeviceType type) {
			RAWINPUTDEVICE dev;
			dev.usUsagePage = 0x1;
			dev.usUsage = type == DeviceType::KEYBOARD ? 0x6 : 0x2;
			if constexpr (Remove) {
				dev.dwFlags = RIDEV_REMOVE;
			} else {
				dev.dwFlags = RIDEV_INPUTSINK;
			}
			dev.hwndTarget = getHWND();

			RegisterRawInputDevices(&dev, 1, sizeof(RAWINPUTDEVICE));
		}

		inline bool SRK_CALL _hasDevice(const InternalDeviceInfo& info, const std::vector<InternalDeviceInfo>& devices) const {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}