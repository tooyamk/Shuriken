#pragma once

#include "Base.h"
#include "srk/events/EventDispatcher.h"

#define SRK_MODULE_INPUT_EVDEV_PATH_BUFFER {"/dev/input/eventxxx" }

namespace srk::modules::inputs::evdev_input {
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

		template<typename Fn, typename... Args>
		static void SRK_CALL traverseBits(const uint8_t* bits, size_t len, size_t begin, Fn&& fn, Args&&... args) {
			size_t i = begin >> 3;
			size_t n = begin & 0b111;

			begin = 0;
			while (i < len) {
				auto val = bits[i] >> n;

				while (val) {
					if (val & 0b1) {
						if (begin = fn((i << 3) + n, std::forward<Args>(args)...); begin > 0) break;
					}

					val >>= 1;
					++n;
				}

				if (begin > 0) {
					i = begin >> 3;
					n = begin & 0b111;
					begin = 0;
				} else {
					n = 0;
					++i;
				}
			}
		}

	private:
		static constexpr size_t EVENT_STR_LEN = 5;
		static constexpr size_t EVENT_NUMBER_BUFFER_LEN = 3;

		IntrusivePtr<Ref> _loader;
		IntrusivePtr<windows::IWindow> _win;
		DeviceType _filters;
		IntrusivePtr<events::IEventDispatcher<ModuleEvent>> _eventDispatcher;

		std::shared_mutex _mutex;
		std::vector<InternalDeviceInfo> _devices;

		inline static bool SRK_CALL _hasDevice(const InternalDeviceInfo& info, const std::vector<InternalDeviceInfo>& devices) {
			for (auto& di : devices) {
				if (info.guid == di.guid) return true;
			}

			return false;
		}
	};
}