#pragma once

#include "../BaseTester.h"
#include <shared_mutex>

class InputTester : public BaseTester {
public:
	static std::string_view AE_CALL getGamepadKeyString(GamepadKeyCode code) {
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

	static std::string_view AE_CALL getDeviceTypeString(DeviceType type) {
		switch (type) {
		case DeviceType::KEYBOARD:
			return "keyboard";
		case DeviceType::MOUSE:
			return "mouse";
		case DeviceType::GAMEPAD:
			return "gamepad";
		default:
			return "undefined";
		}
	}

	void AE_CALL initInputModule(std::vector<IntrusivePtr<IInputModule>>& modules, const std::string_view& dll, const SerializableObject* args) {
		IntrusivePtr loader = new InputModuleLoader();
		if (loader->load(dll)) {
			if (auto im = loader->create(args); im) modules.emplace_back(im);
		}
	}

	virtual int32_t AE_CALL run() override {
		IntrusivePtr app = new Application("TestApp");

		ApplicationStyle wndStype;
		wndStype.thickFrame = true;
		if (app->createWindow(wndStype, "", Vec2ui32(800, 600), false)) {
			std::vector<IntrusivePtr<IInputModule>> inputModules;

			if constexpr (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::WINDOWS) {
				if (1) {
					SerializableObject args;
					args.insert("app", app.uintptr());
					args.insert("ignoreXInputDevices", true);
					args.insert("filter", DeviceType::GAMEPAD);
					initInputModule(inputModules, "libs/" + getDLLName("ae-input-direct-input"), &args);
				}
				if (1) {
					SerializableObject args;
					args.insert("app", app.uintptr());
					args.insert("filter", DeviceType::KEYBOARD | DeviceType::MOUSE);
					initInputModule(inputModules, "libs/" + getDLLName("ae-input-raw-input"), &args);
				}
				if (1) {
					SerializableObject args;
					args.insert("app", app.uintptr());
					args.insert("filter", DeviceType::GAMEPAD);
					initInputModule(inputModules, "libs/" + getDLLName("ae-input-xinput"), &args);
				}
			}

			std::shared_mutex inputDevicesMutex;
			std::vector<IntrusivePtr<IInputDevice>> inputDevices;

			for (auto& im : inputModules) {
				im->getEventDispatcher().addEventListener(ModuleEvent::CONNECTED, createEventListener<ModuleEvent>([&inputDevices, &inputDevicesMutex, app](Event<ModuleEvent>& e) {
					auto getNumInputeDevice = [&inputDevices, &inputDevicesMutex](DeviceType type) {
						uint32_t n = 0;

						std::shared_lock lock(inputDevicesMutex);

						for (auto& dev : inputDevices) {
							if (dev->getInfo().type == type) ++n;
						}
						return n;
					};

					auto info = e.getData<DeviceInfo>();
					printaln("input device connected : ", getDeviceTypeString(info->type), " vid = ", info->vendorID, " pid = ", info->productID, " guid = ", String::toString(info->guid.getData(), info->guid.getSize()));

					if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN) {
						auto im = e.getTarget<IInputModule>();
						//if (getNumInputeDevice(DeviceType::GAMEPAD) > 0) return;
						printaln("create device : ", getDeviceTypeString(info->type), " guid = ", String::toString(info->guid.getData(), info->guid.getSize()));
						if (auto device = im->createDevice(info->guid); device) {
							device->getEventDispatcher().addEventListener(DeviceEvent::DOWN, createEventListener<DeviceEvent>([app](Event<DeviceEvent>& e) {
								auto device = e.getTarget<IInputDevice>();
								switch (device->getInfo().type) {
								case DeviceType::KEYBOARD:
								{
									auto key = e.getData<Key>();
									if (key->code == KeyboardVirtualKeyCode::KEY_ENTER) {
										float32_t state = 0.0f;
										if (device->getKeyState((uint8_t)KeyboardVirtualKeyCode::KEY_RCTRL, &state, 1) && state != 0.f) {
											app->toggleFullscreen();
										}
									}

									printaln("keyboard down -> key : ", key->code, "    value : ", key->value[0]);

									break;
								}
								case DeviceType::GAMEPAD:
								{
									auto key = e.getData<Key>();
									printaln("gamepad down : ", getGamepadKeyString((GamepadKeyCode)key->code), "  ", key->value[0]);
									if (key->code == GamepadKeyCode::CROSS) {
										device->setVibration(0.5f, 0.5f);
									}

									break;
								}
								case DeviceType::MOUSE:
								{
									auto key = e.getData<Key>();
									
									printaln("mouse down -> key : ", key->code, "    value : ", key->value[0]);

									break;
								}
								}
							}));

							device->getEventDispatcher().addEventListener(DeviceEvent::UP, createEventListener<DeviceEvent>([](Event<DeviceEvent>& e) {
								auto device = e.getTarget<IInputDevice>();
								switch (device->getInfo().type) {
								case DeviceType::KEYBOARD:
								{
									break;
								}
								case DeviceType::GAMEPAD:
								{
									auto key = e.getData<Key>();
									printaln("gamepad up : ", getGamepadKeyString((GamepadKeyCode)key->code), "  ", key->value[0]);
									if (key->code == GamepadKeyCode::CROSS) {
										device->setVibration(0.0f, 0.0f);
									}

									break;
								}
								}
							}));

							device->getEventDispatcher().addEventListener(DeviceEvent::MOVE, createEventListener<DeviceEvent>([](Event<DeviceEvent>& e) {
								switch (e.getTarget<IInputDevice>()->getInfo().type) {
								case DeviceType::MOUSE:
								{
									auto key = e.getData<Key>();
									if (key->code == MouseKeyCode::POSITION) {
										//f32 curPos[2];
										//(e.getTarget<InputDevice>())->getKeyState(key->code, curPos, 2);
										//printdln("input device move : ", key->value[0], " ", key->value[1]);
									} else if (key->code == MouseKeyCode::WHEEL) {
										printaln("input device wheel : ", key->value[0]);
									}

									break;
								}
								case DeviceType::GAMEPAD:
								{
									auto key = e.getData<Key>();
									printd("gamepad move : ", getGamepadKeyString((GamepadKeyCode)key->code), " ", key->value[0]);
									if (key->count > 1) printd("  ", key->value[1]);
									printaln();

									break;
								}
								}

							}));

							std::scoped_lock lock(inputDevicesMutex);

							inputDevices.emplace_back(device);
						}
					}
				}));

				im->getEventDispatcher().addEventListener(ModuleEvent::DISCONNECTED, createEventListener<ModuleEvent>([&inputDevices, &inputDevicesMutex](Event<ModuleEvent>& e) {
					auto info = e.getData<DeviceInfo>();
					printaln("input device disconnected : ", getDeviceTypeString(info->type));

					std::scoped_lock lock(inputDevicesMutex);

					for (uint32_t i = 0, n = inputDevices.size(); i < n; ++i) {
						if (inputDevices[i]->getInfo().guid == info->guid) {
							inputDevices.erase(inputDevices.begin() + i);
							break;
						}
					}
				}));
			}

			IntrusivePtr looper = new Looper(1000.0 / 60.0);

			app->getEventDispatcher().addEventListener(ApplicationEvent::CLOSED, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				looper->stop();
			}));

			looper->getEventDispatcher().addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([app, &inputModules, &inputDevices, &inputDevicesMutex](Event<LooperEvent>& e) {
				app->pollEvents();

				for (auto& im : inputModules) im->poll();
				//for (auto& dev : inputDevices) dev->poll(true);

				//app->setWindowTitle(String::toString(GetKeyboardType(0)) + "  " + String::toString(GetKeyboardType(1)) + "  " + String::toString(GetKeyboardType(2)));
			}));

			//evtDispatcher.addEventListener(ApplicationEvent::CLOSING, *appClosingListener);

			app->setVisible(true);

			std::thread([&inputDevices, &inputDevicesMutex]() {
				while (true) {
					{
						std::shared_lock lock(inputDevicesMutex);

						for (auto& dev : inputDevices) dev->poll(true);
					}

					std::this_thread::sleep_for(1ms);
				}
			}).detach();

			looper->run(true);
		}

		return 0;
	}
};