#pragma once

#include "../BaseTester.h"

class InputTester : public BaseTester {
public:
	static char* AE_CALL printGamepadKey(GamepadKeyCode code) {
		switch (code) {
		case GamepadKeyCode::LEFT_STICK:
			return "left stick";
		case GamepadKeyCode::RIGHT_STICK:
			return "right stick";
		case GamepadKeyCode::LEFT_THUMB:
			return "left thumb";
		case GamepadKeyCode::RIGHT_THUMB:
			return "right thumb";
		case GamepadKeyCode::DPAD:
			return "dpad";
		case GamepadKeyCode::LEFT_SHOULDER:
			return "left shoulder";
		case GamepadKeyCode::RIGHT_SHOULDER:
			return "right shoulder";
		case GamepadKeyCode::LEFT_TRIGGER:
			return "left trigger";
		case GamepadKeyCode::RIGHT_TRIGGER:
			return "right trigger";
		case GamepadKeyCode::SELECT:
			return "select";
		case GamepadKeyCode::START:
			return "start";
		case GamepadKeyCode::A:
			return "a";
		case GamepadKeyCode::B:
			return "b";
		case GamepadKeyCode::X:
			return "x";
		case GamepadKeyCode::Y:
			return "y";
		case GamepadKeyCode::TOUCH_PAD:
			return "touch pad";
		default:
			return "undefined";
		}
	}

	void AE_CALL initInputModule(std::vector<RefPtr<IInputModule>>& modules, const std::string_view& dll, const Args* args) {
		RefPtr loader = new InputModuleLoader();
		if (loader->load(dll)) {
			if (auto im = loader->create(args); im) modules.emplace_back(im);
		}
	}

	virtual int32_t AE_CALL run() override {
		RefPtr app = new Application(u8"TestApp", 1000. / 60.);

		Application::Style wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, u8"", Box2i32(Vec2i32({ 100, 100 }), Vec2i32({ 800, 600 })), false)) {
			Args args;
			args.add("app", &*app);

			std::vector<RefPtr<IInputModule>> inputModules;

#if AE_OS == AE_OS_WIN
			initInputModule(inputModules, getDLLName("ae-win-direct-input"), &args);
			initInputModule(inputModules, getDLLName("ae-win-xinput"), &args);
#endif

			std::vector<RefPtr<IInputDevice>> inputDevices;

			for (auto& im : inputModules) {
				im->getEventDispatcher().addEventListener(ModuleEvent::CONNECTED, new EventListener(std::function([&inputDevices, app](Event<ModuleEvent>& e) {
					auto getNumInputeDevice = [&inputDevices](DeviceType type) {
						uint32_t n = 0;
						for (auto& dev : inputDevices) {
							if (dev->getInfo().type == type) ++n;
						}
						return n;
					};

					auto info = e.getData<DeviceInfo>();
					if ((info->type & (DeviceType::KEYBOARD | DeviceType::GAMEPAD)) != DeviceType::UNKNOWN) {
						auto im = e.getTarget<IInputModule>();
						if (getNumInputeDevice(DeviceType::GAMEPAD) > 0) return;
						println("create device : ", (uint32_t)info->type, " guid size = ", info->guid.getSize());
						auto device = im->createDevice(info->guid);
						if (device) {
							device->getEventDispatcher().addEventListener(DeviceEvent::DOWN, new EventListener(std::function([app](Event<DeviceEvent>& e) {
								auto device = e.getTarget<IInputDevice>();
								switch (device->getInfo().type) {
								case DeviceType::KEYBOARD:
								{
									auto key = e.getData<Key>();
									if (key->code == (uint32_t)KeyboardVirtualKeyCode::KEY_ENTER) {
										f32 status;
										if (device->getKeyState((uint8_t)KeyboardVirtualKeyCode::KEY_RCTRL, &status, 1) && status != 0.f) {
											app->toggleFullscreen();
										}
									}

									println("keyboard down -> key : ", key->code, "    value : ", key->value[0]);

									break;
								}
								case DeviceType::GAMEPAD:
								{
									auto key = e.getData<Key>();
									println("gamepad down : ", printGamepadKey((GamepadKeyCode)key->code), "  ", key->value[0]);
									if (key->code == (uint32_t)GamepadKeyCode::CROSS) {
										device->setVibration(0.5f, 0.5f);
									}

									break;
								}
								}
							})));

							device->getEventDispatcher().addEventListener(DeviceEvent::UP, new EventListener(std::function([](Event<DeviceEvent>& e) {
								auto device = e.getTarget<IInputDevice>();
								switch (device->getInfo().type) {
								case DeviceType::KEYBOARD:
								{
									break;
								}
								case DeviceType::GAMEPAD:
								{
									auto key = e.getData<Key>();
									println("gamepad up : ", printGamepadKey((GamepadKeyCode)key->code), "  ", key->value[0]);
									if (key->code == (uint32_t)GamepadKeyCode::CROSS) {
										device->setVibration(0.0f, 0.0f);
									}

									break;
								}
								}
							})));

							device->getEventDispatcher().addEventListener(DeviceEvent::MOVE, new EventListener(std::function([](Event<DeviceEvent>& e) {
								switch (e.getTarget<IInputDevice>()->getInfo().type) {
								case DeviceType::MOUSE:
								{
									auto key = e.getData<Key>();
									if (key->code == 4) {
										//f32 curPos[2];
										//(e.getTarget<InputDevice>())->getKeyState(key->code, curPos, 2);
										println("input device move : ", key->code, key->value[0], key->value[1], key->value[2]);
									} else if (key->code == 1) {
										//println("input device wheel : %d   %f", key->code, *(f32*)key->value);
									}

									break;
								}
								case DeviceType::GAMEPAD:
								{
									auto key = e.getData<Key>();
									print("gamepad move : ", printGamepadKey((GamepadKeyCode)key->code), " ", key->value[0]);
									if (key->count > 1) print("  ", key->value[1]);
									println();

									break;
								}
								}

							})));

							inputDevices.emplace_back(device);
						}
					}
					println("input device connected : ", info->type);
				})));

				im->getEventDispatcher().addEventListener(ModuleEvent::DISCONNECTED, new EventListener(std::function([&inputDevices](Event<ModuleEvent>& e) {
					auto info = e.getData<DeviceInfo>();
					for (uint32_t i = 0, n = inputDevices.size(); i < n; ++i) {
						if (inputDevices[i]->getInfo().guid == info->guid) {
							inputDevices.erase(inputDevices.begin() + i);
							break;
						}
					}
					println("input device disconnected : ", info->type);
				})));
			}

			auto& evtDispatcher = app->getEventDispatcher();

			evtDispatcher.addEventListener(ApplicationEvent::TICKING, new EventListener(std::function([&inputModules, &inputDevices](Event<ApplicationEvent>& e) {
				for (auto& im : inputModules) im->poll();
				for (auto& dev : inputDevices) dev->poll(true);

				//app->setWindowTitle(String::toString(GetKeyboardType(0)) + "  " + String::toString(GetKeyboardType(1)) + "  " + String::toString(GetKeyboardType(2)));
			})));

			//evtDispatcher.addEventListener(ApplicationEvent::CLOSING, *appClosingListener);

			app->setVisible(true);
			app->run(true);
		}

		return 0;
	}
};