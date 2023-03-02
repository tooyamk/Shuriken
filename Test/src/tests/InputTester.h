#pragma once

#include "../BaseTester.h"
#include <shared_mutex>

class InputTester : public BaseTester {
public:
	static std::string SRK_CALL getGamepadKeyString(GamepadVirtualKeyCode code) {
		switch (code) {
		case GamepadVirtualKeyCode::L_STICK:
			return "left stick";
		case GamepadVirtualKeyCode::R_STICK:
			return "right stick";
		case GamepadVirtualKeyCode::L_THUMB:
			return "left thumb";
		case GamepadVirtualKeyCode::R_THUMB:
			return "right thumb";
		case GamepadVirtualKeyCode::DPAD:
			return "dpad";
		case GamepadVirtualKeyCode::L_SHOULDER:
			return "left shoulder";
		case GamepadVirtualKeyCode::R_SHOULDER:
			return "right shoulder";
		case GamepadVirtualKeyCode::L_TRIGGER:
			return "left trigger";
		case GamepadVirtualKeyCode::R_TRIGGER:
			return "right trigger";
		case GamepadVirtualKeyCode::L_TRIGGER_BUTTON:
			return "left trigger button";
		case GamepadVirtualKeyCode::R_TRIGGER_BUTTON:
			return "right trigger button";
		case GamepadVirtualKeyCode::SELECT:
			return "select";
		case GamepadVirtualKeyCode::START:
			return "start";
		case GamepadVirtualKeyCode::A:
			return "a";
		case GamepadVirtualKeyCode::B:
			return "b";
		case GamepadVirtualKeyCode::X:
			return "x";
		case GamepadVirtualKeyCode::Y:
			return "y";
		case GamepadVirtualKeyCode::TOUCH_PAD:
			return "touch pad";
		default:
		{
			if (code >= GamepadVirtualKeyCode::UNDEFINED_AXIS_1 && code <= GamepadVirtualKeyCode::UNDEFINED_AXIS_END) {
				return "undefined axis " + String::toString((uint32_t)(code - GamepadVirtualKeyCode::UNDEFINED_AXIS_1) + 1);
			} else if (code >= GamepadVirtualKeyCode::UNDEFINED_BUTTON_1 && code <= GamepadVirtualKeyCode::UNDEFINED_BUTTON_END) {
				return "undefined button " + String::toString((uint32_t)(code - GamepadVirtualKeyCode::UNDEFINED_BUTTON_1) + 1);
			}

			return "unknown";
		}
		}
	}

	static std::string_view SRK_CALL getDeviceTypeString(DeviceType type) {
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

	void SRK_CALL initInputModule(std::vector<IntrusivePtr<IInputModule>>& modules, const std::string_view& dll, const CreateInputModuleDesc& desc) {
		IntrusivePtr loader = new InputModuleLoader();
		if (loader->load(dll)) {
			if (auto im = loader->create(desc); im) modules.emplace_back(im);
		}
	}

	virtual int32_t SRK_CALL run() override {
		IntrusivePtr wml = new WindowModuleLoader();
		if (!wml->load(getWindowDllPath())) return 0;

		auto wm = wml->create();
		if (!wm) return 0;

		CreateWindowDesc desc;
		desc.style.resizable = true;
		desc.contentSize.set(800, 600);
		auto win = wm->crerateWindow(desc);
		if (!win) return 0;

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
			std::exit(0);
			}));

		std::vector<IntrusivePtr<IInputModule>> inputModules;

		CreateInputModuleDesc createInputModuleDesc;
		createInputModuleDesc.window = win;

		if constexpr (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::WINDOWS) {
			if (1) {
				createInputModuleDesc.filters = DeviceType::GAMEPAD;
				auto ignoreXInputDevices = false;
				void* argv[1];
				argv[0] = &ignoreXInputDevices;
				createInputModuleDesc.argc = 0;
				createInputModuleDesc.argv = argv;
				//initInputModule(inputModules, getDLLName("srk-module-input-direct-input"), inputModuleDesc);
			}
			if (1) {
				createInputModuleDesc.filters = DeviceType::KEYBOARD;
				initInputModule(inputModules, getDllPath("srk-module-input-raw-input"), createInputModuleDesc);
			}
			if (1) {
				createInputModuleDesc.filters = DeviceType::GAMEPAD;
				//initInputModule(inputModules, getDLLName("srk-module-input-xinput"), inputModuleDesc);
			}
			if (1) {
				createInputModuleDesc.filters = DeviceType::GAMEPAD;
				//initInputModule(inputModules, getDLLName("srk-module-input-hid-input"), inputModuleDesc);
			}
		} else if constexpr (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::LINUX) {
			if (1) {
				createInputModuleDesc.filters = DeviceType::GAMEPAD;
				initInputModule(inputModules, getDllPath("srk-module-input-hid-input"), createInputModuleDesc);
			}
		}

		std::shared_mutex inputDevicesMutex;
		std::vector<IntrusivePtr<IInputDevice>> inputDevices;

		for (auto& im : inputModules) {
			im->getEventDispatcher()->addEventListener(ModuleEvent::CONNECTED, createEventListener<ModuleEvent>([&inputDevices, &inputDevicesMutex, win](Event<ModuleEvent>& e) {
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

				if ((info->type & (DeviceType::KEYBOARD)) != DeviceType::UNKNOWN) {
					//if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN) {
					//if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN && info->vendorID == 0x54C) {
					//if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN && info->vendorID == 0xF0D) {
					//if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN && info->vendorID == 0x45E) {
					auto im = e.getTarget<IInputModule>();
					//if (getNumInputeDevice(DeviceType::GAMEPAD) > 0) return;
					printaln("createing device : ", getDeviceTypeString(info->type), " vid = ", info->vendorID, " pid = ", info->productID, " guid = ", String::toString(info->guid.getData(), info->guid.getSize()));
					if (auto device = im->createDevice(info->guid); device) {
						printaln("created device : ", getDeviceTypeString(info->type), " vid = ", info->vendorID, " pid = ", info->productID, " guid = ", String::toString(info->guid.getData(), info->guid.getSize()));
						{
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::L_STICK, &Math::TWENTIETH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::R_STICK, &Math::TWENTIETH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::L_TRIGGER, &Math::TWENTIETH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::R_TRIGGER, &Math::TWENTIETH<DeviceStateValue>, 1);
						}

						/*
						GamepadKeyMapping keyMapping;
						device->getState(DeviceStateType::KEY_MAPPING, 0, &keyMapping, 1);
						keyMapping.removeUndefined();

						keyMapping.set(GamepadVirtualKeyCode::R_STICK_X, GamepadKeyCode::AXIS_1 + 2);
						keyMapping.set(GamepadVirtualKeyCode::R_STICK_Y, GamepadKeyCode::AXIS_1 + 5);
						keyMapping.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + 3);
						keyMapping.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + 4);
						keyMapping.set(GamepadVirtualKeyCode::L2_BUTTON, GamepadKeyCode::BUTTON_1 + 6);
						keyMapping.set(GamepadVirtualKeyCode::R2_BUTTON, GamepadKeyCode::BUTTON_1 + 7);
						keyMapping.set(GamepadVirtualKeyCode::SELECT, GamepadKeyCode::BUTTON_1 + 8);
						keyMapping.set(GamepadVirtualKeyCode::START, GamepadKeyCode::BUTTON_1 + 9);
						keyMapping.set(GamepadVirtualKeyCode::L_THUMB, GamepadKeyCode::BUTTON_1 + 10);
						keyMapping.set(GamepadVirtualKeyCode::R_THUMB, GamepadKeyCode::BUTTON_1 + 11);
						device->setState(DeviceStateType::KEY_MAPPING, 0, &keyMapping, 1);
						*/

						{
							float32_t dz[] = { Math::ONE_HALF<DeviceStateValue> -Math::FORTIETH<DeviceStateValue>, Math::ONE_HALF<DeviceStateValue> +Math::FORTIETH<DeviceStateValue> };
							for (size_t i = 0; i < 10; ++i) device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::UNDEFINED_AXIS_1 + i, dz, 2);
						}

						auto eventDispatcher = device->getEventDispatcher();

						eventDispatcher->addEventListener(DeviceEvent::DOWN, createEventListener<DeviceEvent>([win](Event<DeviceEvent>& e) {
							auto device = e.getTarget<IInputDevice>();
							auto& info = device->getInfo();
							switch (info.type) {
							case DeviceType::KEYBOARD:
							{
								auto state = e.getData<DeviceState>();
								if (state->code == KeyboardVirtualKeyCode::KEY_ENTER) {
									float32_t state = 0.0f;
									if (device->getState(DeviceStateType::KEY, KeyboardVirtualKeyCode::KEY_RCTRL, &state, 1) && state != 0.f) {
										win->toggleFullScreen();
									}
								}

								printaln("keyboard down -> key : ", state->code, "    value : ", ((DeviceStateValue*)state->values)[0]);

								if (state->code == KeyboardVirtualKeyCode::KEY_ENTER) {
									float val[2];
									device->getState(DeviceStateType::KEY, KeyboardVirtualKeyCode::KEY_CTRL, &val[0], 1);
									device->getState(DeviceStateType::KEY, KeyboardVirtualKeyCode::KEY_ALT, &val[1], 1);
									if (val[0] == 1.0f && val[1] == 1.0f) {
										win->toggleFullScreen();
									}
								}

								break;
							}
							case DeviceType::GAMEPAD:
							{
								auto state = e.getData<DeviceState>();
								printaln("gamepad down : ", " vid = ", info.vendorID, " pid = ", info.productID, " ", getGamepadKeyString((GamepadVirtualKeyCode)state->code), "  ", ((DeviceStateValue*)state->values)[0]);
								if (state->code == GamepadVirtualKeyCode::CROSS) {
									//DeviceStateValue vals[] = { 1.f, 1.f };
									//device->setState(DeviceStateType::VIBRATION, 0, vals, 2);
								}

								break;
							}
							case DeviceType::MOUSE:
							{
								auto state = e.getData<DeviceState>();

								printaln("mouse down -> key : ", state->code, "    value : ", ((DeviceStateValue*)state->values)[0]);

								break;
							}
							}
							}));

						eventDispatcher->addEventListener(DeviceEvent::UP, createEventListener<DeviceEvent>([](Event<DeviceEvent>& e) {
							auto device = e.getTarget<IInputDevice>();
							auto& info = device->getInfo();
							switch (info.type) {
							case DeviceType::KEYBOARD:
							{
								break;
							}
							case DeviceType::GAMEPAD:
							{
								auto state = e.getData<DeviceState>();
								printaln("gamepad up : ", info.vendorID, " pid = ", info.productID, " ", getGamepadKeyString((GamepadVirtualKeyCode)state->code), "  ", ((DeviceStateValue*)state->values)[0]);
								if (state->code == GamepadVirtualKeyCode::CROSS) {
									//DeviceStateValue vals[] = { 0.f, 0.f };
									//device->setState(DeviceStateType::VIBRATION, 0, vals, 2);
								}

								break;
							}
							}
							}));

						eventDispatcher->addEventListener(DeviceEvent::MOVE, createEventListener<DeviceEvent>([](Event<DeviceEvent>& e) {
							auto device = e.getTarget<IInputDevice>();
							auto& info = device->getInfo();
							switch (info.type) {
							case DeviceType::MOUSE:
							{
								auto state = e.getData<DeviceState>();
								if (state->code == MouseKeyCode::POSITION) {
									//f32 curPos[2];
									//(e.getTarget<InputDevice>())->getKeyState(key->code, curPos, 2);
									//printaln("input device move : ", key->value[0], " ", key->value[1]);
								} else if (state->code == MouseKeyCode::WHEEL) {
									printaln("input device wheel : ", ((DeviceStateValue*)state->values)[0]);
								}

								break;
							}
							case DeviceType::GAMEPAD:
							{
								auto state = e.getData<DeviceState>();
								//if (key->code != GamepadKeyCode::R_STICK) break;
								printa("gamepad move : ", info.vendorID, " pid = ", info.productID, " ", getGamepadKeyString((GamepadVirtualKeyCode)state->code), " ", ((DeviceStateValue*)state->values)[0]);
								if (state->count > 1) printa("  ", ((DeviceStateValue*)state->values)[1]);
								printaln();

								break;
							}
							}

							}));

						eventDispatcher->addEventListener(DeviceEvent::TOUCH, createEventListener<DeviceEvent>([](Event<DeviceEvent>& e) {
							auto device = e.getTarget<IInputDevice>();
							auto& info = device->getInfo();
							switch (info.type) {
							case DeviceType::MOUSE:
							{
								break;
							}
							case DeviceType::GAMEPAD:
							{
								auto state = e.getData<DeviceState>();
								auto touches = (DeviceTouchStateValue*)state->values;
								for (size_t i = 0; i < state->count; ++i) {
									auto& touch = touches[i];
									printaln("gamepad touch : ", info.vendorID, " pid = ", info.productID, " id = ", touch.fingerID, " isTouched = ", touch.isTouched, " x = ", touch.position[0], " y = ", touch.position[1]);
								}

								break;
							}
							}
							}));

						std::scoped_lock lock(inputDevicesMutex);

						inputDevices.emplace_back(device);
					} else {
						printaln("create device failed : ", getDeviceTypeString(info->type), " vid = ", info->vendorID, " pid = ", info->productID, " guid = ", String::toString(info->guid.getData(), info->guid.getSize()));
					}
				}
				}));

			im->getEventDispatcher()->addEventListener(ModuleEvent::DISCONNECTED, createEventListener<ModuleEvent>([&inputDevices, &inputDevicesMutex](Event<ModuleEvent>& e) {
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

		IntrusivePtr looper = new Looper(1.0 / 60.0);

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([looper](Event<WindowEvent>& e) {
			looper->stop();
			}));

		looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([wm, &inputModules, &inputDevices, &inputDevicesMutex](Event<LooperEvent>& e) {
			while (wm->processEvent()) {};

			for (auto& im : inputModules) im->poll();
			//for (auto& dev : inputDevices) dev->poll(true);

			//app->setWindowTitle(String::toString(GetKeyboardType(0)) + "  " + String::toString(GetKeyboardType(1)) + "  " + String::toString(GetKeyboardType(2)));
			}));

		//evtDispatcher.addEventListener(ApplicationEvent::CLOSING, *appClosingListener);

		win->setVisible(true);

		std::thread([&inputDevices, &inputDevicesMutex]() {
			while (true) {
				{
					std::shared_lock lock(inputDevicesMutex);

					for (auto& dev : inputDevices) {
						dev->poll(true);

						if (dev->getInfo().type == DeviceType::GAMEPAD) {
							DeviceStateValue vals[2];
							if (dev->getState(DeviceStateType::KEY, GamepadVirtualKeyCode::SQUARE, vals, 2)) {

								DeviceStateValue vibration[2];
								vibration[0] = vals[0];
								vibration[1] = vals[0];
								dev->setState(DeviceStateType::VIBRATION, 0, vibration, 2);
							}
						}
					}
				}

				std::this_thread::sleep_for(1ms);
			}
			}).detach();

			looper->run(true);

		return 0;
	}
};