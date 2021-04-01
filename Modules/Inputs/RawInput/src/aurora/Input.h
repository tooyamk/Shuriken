#pragma once

#include "Base.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora::modules::inputs::raw_input {
	using namespace std::literals;

	class AE_MODULE_DLL Input : public IInputModule {
	public:
		Input(Ref* loader, IApplication* app);
		virtual ~Input();

		void operator delete(Input* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Input();
			::operator delete(p);
		}

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() override;
		virtual void AE_CALL poll() override;
		virtual IntrusivePtr<IInputDevice> AE_CALL createDevice(const DeviceGUID& guid) override;

		void AE_CALL registerRawInputDevices(DeviceType type);
		void AE_CALL unregisterRawInputDevices(DeviceType type);

	private:
		IntrusivePtr<Ref> _loader;
		IntrusivePtr<IApplication> _app;

		events::EventDispatcher<ModuleEvent> _eventDispatcher;

		std::shared_mutex _mutex;
		std::vector<InternalDeviceInfo> _devices;

		std::mutex _numMutex;
		std::uint32_t _numKeyboards, _numMouses;

		inline std::uint32_t* AE_CALL _getNumVal(DeviceType type) {
			if (type == DeviceType::KEYBOARD) return &_numKeyboards;
			if (type == DeviceType::MOUSE) return &_numKeyboards;
			return nullptr;
		}

		template<bool Remove>
		void AE_CALL _registerDevices(DeviceType type) {
			RAWINPUTDEVICE dev;
			dev.usUsagePage = 0x1;
			dev.usUsage = type == DeviceType::KEYBOARD ? 0x6 : 0x2;
			if constexpr (Remove) {
				dev.dwFlags = RIDEV_REMOVE;
			} else {
				dev.dwFlags = RIDEV_INPUTSINK;
			}
			dev.hwndTarget = (HWND)_app->getNative(ApplicationNative::HWND);

			RegisterRawInputDevices(&dev, 1, sizeof(RAWINPUTDEVICE));
		}

		inline bool AE_CALL _hasDevice(const InternalDeviceInfo& info, const std::vector<InternalDeviceInfo>& devices) const {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}