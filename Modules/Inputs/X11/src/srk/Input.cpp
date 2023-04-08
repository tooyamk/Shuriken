#include "Input.h"
#include "KeyboardDriver.h"
#include "MouseDriver.h"
#include "CreateModule.h"

namespace srk::modules::inputs::x11 {
	Input::Input(Ref* loader, const CreateInputModuleDescriptor& desc) :
		_loader(loader),
		_win(desc.window),
		_firstPoll(true),
		_eventDispatcher(new events::EventDispatcher<ModuleEvent>()) {
		using namespace std::string_view_literals;
		using namespace srk::enum_operators;

		if (_win->getNative("XWindow"sv)) {
			if ((desc.filters & DeviceType::KEYBOARD) != DeviceType::UNKNOWN) {
				auto& info = _devices.emplace_back();
				info.type = DeviceType::KEYBOARD;
				constexpr auto id = "X11InputKbd"sv;
				info.guid.set<false, true>(id.data(), id.size(), 0, 0, 0);
			}

			if ((desc.filters & DeviceType::MOUSE) != DeviceType::UNKNOWN) {
				auto& info = _devices.emplace_back();
				info.type = DeviceType::MOUSE;
				constexpr auto id = "X11InputMouse"sv;
				info.guid.set<false, true>(id.data(), id.size(), 0, 0, 0);
			}
		}
	}

	Input::~Input() {
	}

	IntrusivePtr<events::IEventDispatcher<ModuleEvent>> Input::getEventDispatcher() {
		return _eventDispatcher;
	}

	void Input::poll() {
		if (_firstPoll.exchange(false)) {
			for (auto& info : _devices) _eventDispatcher->dispatchEvent(this, ModuleEvent::CONNECTED, &info);
		}
	}

	IntrusivePtr<IInputDevice> Input::createDevice(const DeviceGUID& guid) {
		DeviceInfo info;
		auto found = false;

		{
			for (auto& i : _devices) {
				if (i.guid == guid) {
					info = i;
					found = true;

					break;
				}
			}
		}

		if (!found) return nullptr;

		switch (info.type) {
		case DeviceType::KEYBOARD:
		{
			auto driver = KeyboardDriver::create(*this, *_win);
			if (driver) return new GenericKeyboard(info, *driver);

			break;
		}
		case DeviceType::MOUSE:
		{
			auto driver = MouseDriver::create(*this, *_win);
			if (driver) return new GenericMouse(info, *driver);

			break;
		}
		}

		return nullptr;
	}
}