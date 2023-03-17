#pragma once

#include "../BaseTester.h"
#include <shared_mutex>
//#include <sys/epoll.h>
//#include <sys/eventfd.h>
//#include <sys/inotify.h>

#if SRK_OS == SRK_OS_LINUX
#include <dirent.h>

#include <fcntl.h>
#include <linux/input.h>
#undef KEY_ENTER
#endif

class InputTester : public BaseTester {
public:
	static std::string SRK_CALL getGamepadKeyString(GamepadVirtualKeyCode code) {
		switch (code) {
		case GamepadVirtualKeyCode::L_STICK:
			return "left stick";
		case GamepadVirtualKeyCode::R_STICK:
			return "right stick";
		case GamepadVirtualKeyCode::L_STICK_X_LEFT:
			return "left stick -x";
		case GamepadVirtualKeyCode::L_STICK_X_RIGHT:
			return "left stick +x";
		case GamepadVirtualKeyCode::L_STICK_Y_DOWN:
			return "left stick -y";
		case GamepadVirtualKeyCode::L_STICK_Y_UP:
			return "left stick +y";
		case GamepadVirtualKeyCode::R_STICK_X_LEFT:
			return "right stick -x";
		case GamepadVirtualKeyCode::R_STICK_X_RIGHT:
			return "right stick +x";
		case GamepadVirtualKeyCode::R_STICK_Y_DOWN:
			return "right stick -y";
		case GamepadVirtualKeyCode::R_STICK_Y_UP:
			return "right stick +y";
		case GamepadVirtualKeyCode::DPAD_LEFT:
			return "dpad -x";
		case GamepadVirtualKeyCode::DPAD_RIGHT:
			return "dpad +x";
		case GamepadVirtualKeyCode::DPAD_DOWN:
			return "dpad -y";
		case GamepadVirtualKeyCode::DPAD_UP:
			return "dpad +y";
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

			return "unknown " + String::toString((size_t)code);
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

#if SRK_OS == SRK_OS_LINUX
	static inline bool bit_is_set(const uint64_t* bits, int32_t bit) {
		constexpr auto LONG_BITS (sizeof(bits[0]) * 8);
    	return !!(bits[bit / LONG_BITS] & (1LL << (bit % LONG_BITS)));
	}

	static inline void set_bit(uint64_t* bits, int bit) {
		constexpr auto LONG_BITS (sizeof(bits[0]) * 8);
		bits[bit / LONG_BITS] |= (1LL << (bit % LONG_BITS));
	}

	static bool has_event_type(const uint64_t* bits, uint32_t type) {
		return bit_is_set(bits, type);
	}

	/*static int32_t type_to_mask_const(const struct libevdev *dev, uint32_t type, const uint64_t** mask) {
		int32_t max;

		switch(type) {
		case EV_ABS:
			*mask = 
			max_mask(ABS, abs);
			max_mask(REL, rel);
			max_mask(KEY, key);
			max_mask(LED, led);
			max_mask(MSC, msc);
			max_mask(SW, sw);
			max_mask(FF, ff);
			max_mask(REP, rep);
			max_mask(SND, snd);
			default:
				max = -1;
				break;
		}

		return max;
	}

	static bool has_event_code(const uint64_t* bits, uint32_t type, uint32_t code) {
		const uint64_t* mask = nullptr;

		if (!has_event_type(bits, type)) return false;

		if (type == EV_SYN) return true;

		auto max = type_to_mask_const(dev, type, &mask);

		if (max == -1 || code > (unsigned int)max)
			return 0;

		return bit_is_set(mask, code);
	}*/

	struct PrintFormater {
		template<typename F, typename T>
		bool SRK_CALL operator()(Printer::OutputBuffer& buf, F&& formater, T&& value) const {
			using Type = std::remove_cvref_t<T>;

			if constexpr (std::is_same_v<Type, input_absinfo>) {
				buf.write(L"input_absinfo{"sv);
				buf.write(L"value:"sv);
				buf.write(std::to_wstring(value.value));
				buf.write(L",minimum:"sv);
				buf.write(std::to_wstring(value.minimum));
				buf.write(L",maximum:"sv);
				buf.write(std::to_wstring(value.maximum));
				buf.write(L",fuzz:"sv);
				buf.write(std::to_wstring(value.fuzz));
				buf.write(L",flat:"sv);
				buf.write(std::to_wstring(value.flat));
				buf.write(L",resolution:"sv);
				buf.write(std::to_wstring(value.resolution));
				buf.write(L'}');

				return true;
			}

			return false;
		}
	};
#endif

	virtual int32_t SRK_CALL run() override {
#if SRK_OS != SRK_OS_LINUX
		std::thread aaa([]() {
			std::filesystem::path dir("/dev/input/");
			//printaln(std::filesystem::exists(dir));
			if (std::filesystem::exists(dir) && std::filesystem::is_directory(dir)) {
				for (auto& itr : std::filesystem::directory_iterator(dir)) {
					if (auto& path = itr.path(); path.has_filename()) {
						if (std::string_view(path.filename().string()).substr(0, 5) == "event") {
							//printaln(path);

							auto fd = open(path.string().c_str(), O_RDONLY|O_NONBLOCK);
							if (fd < 0) {
								//printaln(path, L" open error"sv);
								continue;
							}

							printaln(path);

							uint64_t bits[EV_CNT];
							if (auto rc = ioctl(fd, EVIOCGBIT(0, sizeof(bits)), bits); rc < 0) {
								printaln(L"EVIOCGBIT 0 error"sv);
								continue;
							} else {
								printaln(L"bits : "sv, bits);
							}

							char buf[256];

							if (auto rc = ioctl(fd, EVIOCGNAME(sizeof(buf) - 1), buf); rc < 0) {
								printaln(L"EVIOCGNAME error"sv);
								continue;
							} else {
								printaln(L"name : "sv, std::string_view(buf));
							}

							if (auto rc = ioctl(fd, EVIOCGPHYS(sizeof(buf) - 1), buf); rc < 0) {
								printaln(L"EVIOCGPHYS error"sv);
								continue;
							} else {
								printaln(L"phys : "sv, std::string_view(buf));
							}
							if (auto rc = ioctl(fd, EVIOCGUNIQ(sizeof(buf) - 1), buf); rc < 0) {
								printaln(L"EVIOCGUNIQ error"sv);
								continue;
							} else {
								printaln(L"uniq : "sv, std::string_view(buf));
							}

							input_id ids;
							if (auto rc = ioctl(fd, EVIOCGID, &ids); rc < 0) {
								printaln(L"EVIOCGID error"sv);
								continue;
							} else {
								printaln(L"bustype : "sv, ids.bustype, L"  vendor : "sv, ids.vendor, L"  product : "sv, ids.product, L"  version : "sv, ids.version);
							}
							
							int32_t driverVersion;
							if (auto rc = ioctl(fd, EVIOCGVERSION, &driverVersion); rc < 0) {
								printaln(L"EVIOCGVERSION error"sv);
								continue;
							} else {
								printaln(L"driverVersion : "sv, driverVersion);
							}

							uint64_t props[INPUT_PROP_CNT];
							if (auto rc = ioctl(fd, EVIOCGPROP(sizeof(props)), props); rc < 0) {
								printaln(L"EVIOCGPROP error"sv);
								continue;
							} else {
								printaln(L"props : "sv, props);
							}

							uint64_t relBits[REL_CNT];
							if (auto rc = ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relBits)), relBits); rc < 0) {
								printaln(L"EVIOCGBIT EV_REL error"sv);
								continue;
							} else {
								printaln(L"relBits : "sv, relBits);
							}

							uint64_t absBits[ABS_CNT];
							if (auto rc = ioctl(fd, EVIOCGBIT(EV_ABS, sizeof(absBits)), absBits); rc < 0) {
								printaln(L"EVIOCGBIT EV_ABS error"sv);
								continue;
							} else {
								printaln(L"absBits : "sv, absBits);
							}

							uint64_t ledBits[LED_CNT];
							if (auto rc = ioctl(fd, EVIOCGBIT(EV_LED, sizeof(ledBits)), ledBits); rc < 0) {
								printaln(L"EVIOCGBIT EV_LED error"sv);
								continue;
							} else {
								printaln(L"ledBits : "sv, ledBits);
							}

							uint64_t keyBits[KEY_CNT];
							if (auto rc = ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keyBits)), keyBits); rc < 0) {
								printaln(L"EVIOCGBIT EV_KEY error"sv);
								continue;
							} else {
								printaln(L"keyBits : "sv, keyBits);
							}

							uint64_t swBits[SW_CNT];
							if (auto rc = ioctl(fd, EVIOCGBIT(EV_SW, sizeof(swBits)), swBits); rc < 0) {
								printaln(L"EVIOCGBIT EV_SW error"sv);
								continue;
							} else {
								printaln(L"swBits : "sv, swBits);
							}

							uint64_t mscBits[MSC_CNT];
							if (auto rc = ioctl(fd, EVIOCGBIT(EV_MSC, sizeof(mscBits)), mscBits); rc < 0) {
								printaln(L"EVIOCGBIT EV_MSC error"sv);
								continue;
							} else {
								printaln(L"mscBits : "sv, mscBits);
							}

							uint64_t ffBits[FF_CNT];
							if (auto rc = ioctl(fd, EVIOCGBIT(EV_FF, sizeof(ffBits)), ffBits); rc < 0) {
								printaln(L"EVIOCGBIT EV_FF error"sv);
								continue;
							} else {
								printaln(L"ffBits : "sv, ffBits);
							}

							uint64_t sndBits[SND_CNT];
							if (auto rc = ioctl(fd, EVIOCGBIT(EV_SND, sizeof(sndBits)), sndBits); rc < 0) {
								printaln(L"EVIOCGBIT EV_SND error"sv);
								continue;
							} else {
								printaln(L"sndBits : "sv, sndBits);
							}

							uint64_t keyValues[KEY_CNT];
							if (auto rc = ioctl(fd, EVIOCGKEY(sizeof(keyValues)), keyValues); rc < 0) {
								printaln(L"EVIOCGKEY EV_SND error"sv);
								continue;
							} else {
								printaln(L"keyValues : "sv, keyValues);
							}

							uint64_t ledValues[LED_CNT];
							if (auto rc = ioctl(fd, EVIOCGLED(sizeof(ledValues)), ledValues); rc < 0) {
								printaln(L"EVIOCGLED error"sv);
								continue;
							} else {
								printaln(L"ledValues : "sv, ledValues);
							}

							uint64_t swValues[KEY_CNT];
							if (auto rc = ioctl(fd, EVIOCGSW(sizeof(swValues)), swValues); rc < 0) {
								printaln(L"EVIOCGSW error"sv);
								continue;
							} else {
								printaln(L"swValues : "sv, swValues);
							}

							uint64_t repBits[REP_CNT];
							int32_t repValues[REP_CNT];
							if (bit_is_set(bits, EV_REP)) {
								for (decltype(REP_CNT) i = 0; i < REP_CNT; ++i) set_bit(repBits, i);
								if (auto rc = ioctl(fd, EVIOCGREP, repValues); rc < 0) {
									printaln(L"EVIOCGREP error"sv);
									continue;
								} else {
									printaln(L"repBits : "sv, repBits);
									printaln(L"repValues : "sv, repValues);
								}
							} else {
								printaln(L"EV_REP bit not set"sv);
							}

							input_absinfo absInfos[ABS_CNT];
							auto absErr = false;
							for (decltype(ABS_X) i = ABS_X; i <= ABS_MAX; ++i) {
								if (bit_is_set(absBits, i)) {
									input_absinfo absInfo;
									if (auto rc = ioctl(fd, EVIOCGABS(i), &absInfo); rc < 0) {
										absErr = true;
										break;
									}

									if (i == ABS_MT_TRACKING_ID && absInfo.maximum == absInfo.minimum) {
										absInfo.minimum = -1;
										absInfo.maximum = 0xFFFF;
									}

									absInfos[i] = absInfo;
								}
							}

							if (absErr) {
								printaln(L"EVIOCGABS error"sv);
								continue;
							} else {
								printaln<Printer::PriorityFormater<PrintFormater, Printer::DefaultFormater>>(L"absInfos : "sv, absInfos);
							}
						}
					}
				}
			}

			printaln(L"=================================="sv);

			/*auto dirp = opendir("/dev/input/by-id/");
			struct dirent* dp;
			if (!dirp) return;

			size_t len = strlen("event-joystick");
			while((dp = readdir(dirp)) != NULL) {
				size_t devlen = strlen(dp->d_name);
				if(devlen >= len) {
					const char* const start = dp->d_name + devlen - len;
					if(strncmp(start, "event-joystick", len) == 0) printaln(dp->d_name);
				}
			}

			int fd = -1;
			if ((fd = open("/dev/input/by-id/usb-Sony_Computer_Entertainment_Wireless_Controller-event-joystick", O_RDONLY)) < 0) {
				printaln(L"open error"sv);
			}*/
		});
		aaa.detach();
#endif

		{
			

			/*char event_buf[512];
			struct inotify_event* event;

			auto res = read(mINotifyFd, event_buf, sizeof(event_buf));
			if (res < static_cast<int>(sizeof(*event))) {
				printaln(L"read failed"sv);
			}*/
		}

		IntrusivePtr wml = new WindowModuleLoader();
		if (!wml->load(getWindowDllPath())) return 0;

		auto wm = wml->create();
		if (!wm) return 0;

		CreateWindowDesc desc;
		desc.style.resizable = true;
		desc.contentSize.set(800, 600);
		auto win = wm->crerateWindow(desc);
		if (!win) return 0;

		printaln(L"UNDEFINED_AXIS_1 : "sv,  GamepadVirtualKeyCode::UNDEFINED_AXIS_1, L" UNDEFINED_BUTTON_1 : "sv, GamepadVirtualKeyCode::UNDEFINED_BUTTON_1);

		win->getEventDispatcher()->addEventListener(WindowEvent::CLOSED, createEventListener<WindowEvent>([](Event<WindowEvent>& e) {
			std::exit(0);
			}));

		std::vector<IntrusivePtr<IInputModule>> inputModules;

		CreateInputModuleDesc createInputModuleDesc;
		createInputModuleDesc.window = win;

		if constexpr (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::WINDOWS) {
			if (0) {
				createInputModuleDesc.filters = DeviceType::GAMEPAD;
				auto ignoreXInputDevices = false;
				const void* argv[2];
				argv[0] = "ignore-xinput-devices";
				argv[1] = &ignoreXInputDevices;
				createInputModuleDesc.argc = std::extent_v<decltype(argv)>;
				createInputModuleDesc.argv = argv;
				initInputModule(inputModules, getDllPath("srk-module-input-direct-input"), createInputModuleDesc);
			}
			if (0) {
				createInputModuleDesc.filters = DeviceType::KEYBOARD;
				initInputModule(inputModules, getDllPath("srk-module-input-raw-input"), createInputModuleDesc);
			}
			if (0) {
				createInputModuleDesc.filters = DeviceType::GAMEPAD;
				auto useHiddenAPI1_4 = true;
				const void* argv[2];
				argv[0] = "use-hidden-api-1-4";
				argv[1] = &useHiddenAPI1_4;
				createInputModuleDesc.argc = std::extent_v<decltype(argv)>;
				createInputModuleDesc.argv = argv;
				initInputModule(inputModules, getDllPath("srk-module-input-xinput"), createInputModuleDesc);
			}
			if (1) {
				createInputModuleDesc.filters = DeviceType::GAMEPAD;
				initInputModule(inputModules, getDllPath("srk-module-input-hid-input"), createInputModuleDesc);
			}
		} else if constexpr (Environment::OPERATING_SYSTEM == Environment::OperatingSystem::LINUX) {
			if (1) {
				createInputModuleDesc.filters = DeviceType::GAMEPAD;
				initInputModule(inputModules, getDllPath("srk-module-input-evdev"), createInputModuleDesc);
			}

			if (0) {
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
				printaln(L"input device connected : "sv, getDeviceTypeString(info->type), L" vid = "sv, info->vendorID, L" pid = "sv, info->productID, L" name = "sv, info->name, L" guid = "sv, String::toString(info->guid.getData(), info->guid.getSize()));

				//if ((info->type & (DeviceType::KEYBOARD)) != DeviceType::UNKNOWN) {
				if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN) {
				//if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN && info->vendorID == 0x54C) {
				//if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN && info->vendorID == 0xF0D) {
				//if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN && info->vendorID == 0x45E) {
					auto im = e.getTarget<IInputModule>();
					//if (getNumInputeDevice(DeviceType::GAMEPAD) > 0) return;
					printaln(L"createing device : "sv, getDeviceTypeString(info->type), L" vid = "sv, info->vendorID, L" pid = "sv, info->productID, L" name = "sv, info->name, L" guid = "sv, String::toString(info->guid.getData(), info->guid.getSize()));
					if (auto device = im->createDevice(info->guid); device) {
						printaln(L"created device : "sv, getDeviceTypeString(info->type), L" vid = "sv, info->vendorID, L" pid = "sv, info->productID, L" name = "sv, info->name, L" guid = "sv, String::toString(info->guid.getData(), info->guid.getSize()));
						{
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::L_STICK, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::R_STICK, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::L_STICK_X_LEFT, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::L_STICK_X_RIGHT, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::L_STICK_Y_DOWN, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::L_STICK_Y_UP, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::R_STICK_X_LEFT, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::R_STICK_X_RIGHT, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::R_STICK_Y_DOWN, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::R_STICK_Y_UP, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::R_STICK, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::L_TRIGGER, &Math::TENTH<DeviceStateValue>, 1);
							device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::R_TRIGGER, &Math::TENTH<DeviceStateValue>, 1);

							/*GamepadKeyMapper km;
							km.set(GamepadVirtualKeyCode::L_STICK_X_LEFT, GamepadKeyCode::AXIS_1, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
							km.set(GamepadVirtualKeyCode::L_STICK_X_RIGHT, GamepadKeyCode::AXIS_1, GamepadKeyFlag::HALF_BIG);
							km.set(GamepadVirtualKeyCode::L_STICK_Y_DOWN, GamepadKeyCode::AXIS_1 + 1, GamepadKeyFlag::HALF_BIG);
							km.set(GamepadVirtualKeyCode::L_STICK_Y_UP, GamepadKeyCode::AXIS_1 + 1, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);

							km.set(GamepadVirtualKeyCode::R_STICK_X_LEFT, GamepadKeyCode::AXIS_1 + 2, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
							km.set(GamepadVirtualKeyCode::R_STICK_X_RIGHT, GamepadKeyCode::AXIS_1 + 2, GamepadKeyFlag::HALF_BIG);
							km.set(GamepadVirtualKeyCode::R_STICK_Y_DOWN, GamepadKeyCode::AXIS_1 + 5, GamepadKeyFlag::HALF_BIG);
							km.set(GamepadVirtualKeyCode::R_STICK_Y_UP, GamepadKeyCode::AXIS_1 + 5, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);

							km.set(GamepadVirtualKeyCode::DPAD_LEFT, GamepadKeyCode::AXIS_1 + 6, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
							km.set(GamepadVirtualKeyCode::DPAD_RIGHT, GamepadKeyCode::AXIS_1 + 6, GamepadKeyFlag::HALF_BIG);
							km.set(GamepadVirtualKeyCode::DPAD_DOWN, GamepadKeyCode::AXIS_1 + 7, GamepadKeyFlag::HALF_SMALL | GamepadKeyFlag::FLIP);
							km.set(GamepadVirtualKeyCode::DPAD_UP, GamepadKeyCode::AXIS_1 + 7, GamepadKeyFlag::HALF_BIG);

							km.set(GamepadVirtualKeyCode::L_TRIGGER, GamepadKeyCode::AXIS_1 + 3);
							km.set(GamepadVirtualKeyCode::R_TRIGGER, GamepadKeyCode::AXIS_1 + 4);

							km.set(GamepadVirtualKeyCode::SQUARE, GamepadKeyCode::BUTTON_1);
							km.set(GamepadVirtualKeyCode::CROSS, GamepadKeyCode::BUTTON_1 + 1);
							km.set(GamepadVirtualKeyCode::CIRCLE, GamepadKeyCode::BUTTON_1 + 2);
							km.set(GamepadVirtualKeyCode::TRIANGLE, GamepadKeyCode::BUTTON_1 + 3);
							km.set(GamepadVirtualKeyCode::L1, GamepadKeyCode::BUTTON_1 + 4);
							km.set(GamepadVirtualKeyCode::R1, GamepadKeyCode::BUTTON_1 + 5);
							km.set(GamepadVirtualKeyCode::L2_BUTTON, GamepadKeyCode::BUTTON_1 + 6);
							km.set(GamepadVirtualKeyCode::R2_BUTTON, GamepadKeyCode::BUTTON_1 + 7);
							km.set(GamepadVirtualKeyCode::SHARE, GamepadKeyCode::BUTTON_1 + 8);
							km.set(GamepadVirtualKeyCode::OPTIONS, GamepadKeyCode::BUTTON_1 + 9);
							km.set(GamepadVirtualKeyCode::L3, GamepadKeyCode::BUTTON_1 + 10);
							km.set(GamepadVirtualKeyCode::R3, GamepadKeyCode::BUTTON_1 + 11);
							km.set(GamepadVirtualKeyCode::UNDEFINED_BUTTON_1, GamepadKeyCode::BUTTON_1 + 12);
							km.set(GamepadVirtualKeyCode::TOUCH_PAD, GamepadKeyCode::BUTTON_1 + 13);
							device->setState(DeviceStateType::KEY_MAPPER, GamepadVirtualKeyCode::UNKNOWN, &km, 1);*/
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

								printaln(L"keyboard down -> key : "sv, state->code, L"    value : "sv, ((DeviceStateValue*)state->values)[0]);

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
								printaln(L"gamepad down : "sv, L" vid = "sv, info.vendorID, L" pid = "sv, info.productID, L" name = "sv, info.name, L" "sv, getGamepadKeyString((GamepadVirtualKeyCode)state->code), L"  "sv, ((DeviceStateValue*)state->values)[0]);
								if (state->code == GamepadVirtualKeyCode::CROSS) {
									//DeviceStateValue vals[] = { 1.f, 1.f };
									//device->setState(DeviceStateType::VIBRATION, 0, vals, 2);
								}

								break;
							}
							case DeviceType::MOUSE:
							{
								auto state = e.getData<DeviceState>();

								printaln(L"mouse down -> key : "sv, state->code, L"    value : "sv, ((DeviceStateValue*)state->values)[0]);

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
								printaln(L"gamepad up : "sv, info.vendorID, L" pid = "sv, info.productID, L" name = "sv, info.name, L" "sv, getGamepadKeyString((GamepadVirtualKeyCode)state->code), L"  "sv, ((DeviceStateValue*)state->values)[0]);
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
									//printaln(L"input device move : "sv, key->value[0], L" "sv, key->value[1]);
								} else if (state->code == MouseKeyCode::WHEEL) {
									printaln(L"input device wheel : "sv, ((DeviceStateValue*)state->values)[0]);
								}

								break;
							}
							case DeviceType::GAMEPAD:
							{
								auto state = e.getData<DeviceState>();
								auto vk = (GamepadVirtualKeyCode)state->code;
								if (vk >= GamepadVirtualKeyCode::L_STICK_X_LEFT && vk <= GamepadVirtualKeyCode::L_STICK_Y_UP) break;
								if (vk >= GamepadVirtualKeyCode::R_STICK_X_LEFT && vk <= GamepadVirtualKeyCode::R_STICK_Y_UP) break;
								//if (vk >= GamepadVirtualKeyCode::L_STICK && vk <= GamepadVirtualKeyCode::R_STICK) break;
								if (vk >= GamepadVirtualKeyCode::DPAD_LEFT && vk <= GamepadVirtualKeyCode::DPAD_UP) break;
								//if (vk >= GamepadVirtualKeyCode::L_TRIGGER && vk <= GamepadVirtualKeyCode::R_TRIGGER) break;
								
								//if (key->code != GamepadKeyCode::R_STICK) break;
								printa(L"gamepad move : vid = "sv, info.vendorID, L" pid = "sv, info.productID, L" name = "sv, info.name, L" "sv, getGamepadKeyString((GamepadVirtualKeyCode)state->code), L" "sv, ((DeviceStateValue*)state->values)[0]);
								if (state->count > 1) printa(L"  "sv, ((DeviceStateValue*)state->values)[1]);
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
									printaln(L"gamepad touch : vid = "sv, info.vendorID, L" pid = "sv, info.productID, L" name = "sv, info.name, L" id = "sv, touch.fingerID, L" isTouched = "sv, touch.isTouched, L" x = "sv, touch.position[0], L" y = "sv, touch.position[1]);
								}

								break;
							}
							}
							}));

						std::scoped_lock lock(inputDevicesMutex);

						inputDevices.emplace_back(device);
					} else {
						printaln(L"create device failed : "sv, getDeviceTypeString(info->type), L" vid = "sv, info->vendorID, L" pid = "sv, info->productID, L" name = "sv, info->name, L" guid = "sv, String::toString(info->guid.getData(), info->guid.getSize()));
					}
				}
				}));

			im->getEventDispatcher()->addEventListener(ModuleEvent::DISCONNECTED, createEventListener<ModuleEvent>([&inputDevices, &inputDevicesMutex](Event<ModuleEvent>& e) {
				auto info = e.getData<DeviceInfo>();
				printaln(L"input device disconnected : "sv, getDeviceTypeString(info->type));

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
								//dev->setState(DeviceStateType::VIBRATION, 0, vibration, 2);
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