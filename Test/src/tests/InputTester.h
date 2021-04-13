#pragma once

#include "../BaseTester.h"
#include <shared_mutex>

class InputTester : public BaseTester {
public:
	static std::string_view AE_CALL getGamepadKeyString(GamepadKeyCode code) {
		switch (code) {
		case GamepadKeyCode::L_STICK:
			return "left stick";
		case GamepadKeyCode::R_STICK:
			return "right stick";
		case GamepadKeyCode::L_THUMB:
			return "left thumb";
		case GamepadKeyCode::R_THUMB:
			return "right thumb";
		case GamepadKeyCode::DPAD:
			return "dpad";
		case GamepadKeyCode::L_SHOULDER:
			return "left shoulder";
		case GamepadKeyCode::R_SHOULDER:
			return "right shoulder";
		case GamepadKeyCode::L_TRIGGER:
			return "left trigger";
		case GamepadKeyCode::R_TRIGGER:
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

	static std::string_view AE_CALL getDeviceTouchPhaseString(DeviceTouchPhase type) {
		switch (type) {
		case DeviceTouchPhase::BEGIN:
			return "begin";
		case DeviceTouchPhase::MOVE:
			return "move";
		case DeviceTouchPhase::END:
			return "end";
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
					//initInputModule(inputModules, "libs/" + getDLLName("ae-input-direct-input"), &args);
				}
				if (1) {
					SerializableObject args;
					args.insert("app", app.uintptr());
					args.insert("filter", DeviceType::KEYBOARD | DeviceType::MOUSE);
					//initInputModule(inputModules, "libs/" + getDLLName("ae-input-raw-input"), &args);
				}
				if (1) {
					SerializableObject args;
					args.insert("app", app.uintptr());
					args.insert("filter", DeviceType::GAMEPAD);
					//initInputModule(inputModules, "libs/" + getDLLName("ae-input-xinput"), &args);
				}
				if (1) {
					SerializableObject args;
					args.insert("app", app.uintptr());
					args.insert("filter", DeviceType::GAMEPAD);
					initInputModule(inputModules, "libs/" + getDLLName("ae-input-hid-input"), &args);
				}
			}

			std::shared_mutex inputDevicesMutex;
			std::vector<IntrusivePtr<IInputDevice>> inputDevices;

			for (auto& im : inputModules) {
				im->getEventDispatcher()->addEventListener(ModuleEvent::CONNECTED, createEventListener<ModuleEvent>([&inputDevices, &inputDevicesMutex, app](Event<ModuleEvent>& e) {
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

					if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN && info->vendorID != 0x54C) {
						auto im = e.getTarget<IInputModule>();
						//if (getNumInputeDevice(DeviceType::GAMEPAD) > 0) return;
						printaln("create device : ", getDeviceTypeString(info->type), " guid = ", String::toString(info->guid.getData(), info->guid.getSize()));
						if (auto device = im->createDevice(info->guid); device) {
							auto eventDispatcher = device->getEventDispatcher();

							eventDispatcher->addEventListener(DeviceEvent::DOWN, createEventListener<DeviceEvent>([app](Event<DeviceEvent>& e) {
								auto device = e.getTarget<IInputDevice>();
								auto& info = device->getInfo();
								switch (info.type) {
								case DeviceType::KEYBOARD:
								{
									auto state = e.getData<DeviceState>();
									if (state->code == KeyboardVirtualKeyCode::KEY_ENTER) {
										float32_t state = 0.0f;
										if (device->getState(DeviceStateType::KEY, KeyboardVirtualKeyCode::KEY_RCTRL, &state, 1) && state != 0.f) {
											app->toggleFullscreen();
										}
									}

									printaln("keyboard down -> key : ", state->code, "    value : ", ((DeviceStateValue*)state->values)[0]);

									break;
								}
								case DeviceType::GAMEPAD:
								{
									auto state = e.getData<DeviceState>();
									printaln("gamepad down : ", " vid = ", info.vendorID, " pid = ", info.productID, getGamepadKeyString((GamepadKeyCode)state->code), "  ", ((DeviceStateValue*)state->values)[0]);
									if (state->code == GamepadKeyCode::CROSS) {
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
									printaln("gamepad up : ", info.vendorID, " pid = ", info.productID, getGamepadKeyString((GamepadKeyCode)state->code), "  ", ((DeviceStateValue*)state->values)[0]);
									if (state->code == GamepadKeyCode::CROSS) {
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
										//printdln("input device move : ", key->value[0], " ", key->value[1]);
									} else if (state->code == MouseKeyCode::WHEEL) {
										printaln("input device wheel : ", ((DeviceStateValue*)state->values)[0]);
									}

									break;
								}
								case DeviceType::GAMEPAD:
								{
									auto state = e.getData<DeviceState>();
									//if (key->code != GamepadKeyCode::R_STICK) break;
									printd("gamepad move : ", info.vendorID, " pid = ", info.productID, getGamepadKeyString((GamepadKeyCode)state->code), " ", ((DeviceStateValue*)state->values)[0]);
									if (state->count > 1) printd("  ", ((DeviceStateValue*)state->values)[1]);
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
										printdln("gamepad touch : ", info.vendorID, " pid = ", info.productID, "id = ", touch.fingerID, " phase = ", getDeviceTouchPhaseString(touch.phase), " x = ", touch.position[0], " y = ", touch.position[1]);
									}

									break;
								}
								}
							}));

							std::scoped_lock lock(inputDevicesMutex);

							inputDevices.emplace_back(device);
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

			IntrusivePtr looper = new Looper(1000.0 / 60.0);

			app->getEventDispatcher()->addEventListener(ApplicationEvent::CLOSED, createEventListener<ApplicationEvent>([looper](Event<ApplicationEvent>& e) {
				looper->stop();
			}));

			looper->getEventDispatcher()->addEventListener(LooperEvent::TICKING, createEventListener<LooperEvent>([app, &inputModules, &inputDevices, &inputDevicesMutex](Event<LooperEvent>& e) {
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

						for (auto& dev : inputDevices) {
							dev->poll(true);

							if (dev->getInfo().type == DeviceType::GAMEPAD) {
								DeviceStateValue vals[2];
								dev->getState(DeviceStateType::KEY, GamepadKeyCode::SQUARE, vals, 2);
								
								DeviceStateValue vibration[2];
								vibration[0] = vals[0];
								vibration[1] = vals[1];
								dev->setState(DeviceStateType::VIBRATION, 0, vibration, 2);
							}
						}
					}

					std::this_thread::sleep_for(1ms);
				}
			}).detach();

			looper->run(true);
		}

		return 0;
	}
};