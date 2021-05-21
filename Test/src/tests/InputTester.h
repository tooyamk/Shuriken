#pragma once

#include "../BaseTester.h"
#include "aurora/predefine/Architecture.h"
#include <shared_mutex>

namespace std {
#if AE_ARCH_WORD_BITS == AE_ARCH_WORD_BITS_64 && (AE_COMPILER == AE_COMPILER_MSVC || AE_COMPILER == AE_COMPILER_CLANG || AE_COMPILER == AE_COMPILER_GCC)
	template<typename T>
	requires (sizeof(T) == 16)
		class atomic<T> {
		public:
			atomic() {
			}

			atomic(const T& value) :
				_value(value) {
			}

			atomic(T&& value) :
				_value(std::move(value)) {
			}

			inline T AE_CALL load(std::memory_order order = std::memory_order::seq_cst) const {
				T val;
				_compareExchange(_value, val, val);
				return std::move(val);
			}

			inline void AE_CALL store(T desired, std::memory_order order = std::memory_order_seq_cst) {
				T val = _value;
				while (!_compareExchange(_value, val, desired)) {}
			}

			inline T AE_CALL exchange(T desired, std::memory_order order = std::memory_order_seq_cst) {
				T old = _value;
				while (!compare_exchange_strong(old, desired, order)) {}
				return std::move(old);
			}

			inline bool AE_CALL compare_exchange_strong(T& expected, T desired, std::memory_order order = std::memory_order_seq_cst) {
				return _compareExchange(_value, expected, desired);
			}

			inline bool AE_CALL compare_exchange_strong(T& expected, T desired, std::memory_order success, std::memory_order failure) {
				return compare_exchange_strong(expected, desired, success);
			}

			inline bool AE_CALL compare_exchange_weak(T& expected, T desired, std::memory_order order = std::memory_order_seq_cst) {
				return compare_exchange_strong(expected, desired, order);
			}

			inline bool AE_CALL compare_exchange_weak(T& expected, T desired, std::memory_order success, std::memory_order failure) {
				return compare_exchange_strong(expected, desired, success, failure);
			}

		private:
			T _value;

			inline static bool AE_CALL _compareExchange(volatile T& dst, T& expected, const T& desired) {
#	if AE_ARCH_WORD_BITS == AE_ARCH_WORD_BITS_64
#		if AE_COMPILER == AE_COMPILER_MSVC
				return _InterlockedCompareExchange128((volatile LONG64*)&dst, ((LONG64*)(&desired))[1], ((LONG64*)(&desired))[0], (LONG64*)(&expected));
#		elif AE_COMPILER == AE_COMPILER_CLANG || AE_COMPILER == AE_COMPILER_GCC
				return __sync_bool_compare_and_swap((volatile __uint128_t*)&storage, (__uint128_t&)expected, (const __uint128_t&)desired);
#		endif
#	else
				static std::atomic_flag lock = ATOMIC_FLAG_INIT;

				auto pdst = (volatile uint64_t*)&dst;
				auto pexpected = (uint64_t*)&expected;
				auto pdesired = (const uint64_t*)&desired;
				bool rst;

				while (lock.test_and_set(std::memory_order::acquire)) {}

				if (pdst[0] == pexpected[0] && pdst[1] == pexpected[1]) {
					pdst[0] = pdesired[0];
					pdst[1] = pdesired[1];
					rst = true;
				} else {
					pexpected[0] = pdst[0];
					pexpected[1] = pdst[1];
					rst = false;
				}

				lock.clear(std::memory_order::release);

				return rst;
#	endif
			}
	};
#endif
}

template<typename T>
class
#if (AE_ARCH == AE_ARCH_X86 && AE_ARCH_WORD_BITS == AE_ARCH_WORD_BITS_64) || defined (__aarch64__)
	alignas(8) TaggedPtr {
public:
	using CompressedPtr = uint64_t;
	using Tag = uint16_t;

	TaggedPtr() :
		_ptr(0) {
	}

	TaggedPtr(const TaggedPtr& p) :
		_ptr(p._ptr) {
	}

	TaggedPtr(T* ptr, Tag tag = 0) {
		_packPtr(_ptr, ptr, tag);
	}

	inline T* AE_CALL operator->() const {
		return _extractPtr(_ptr);
	}

	inline T& AE_CALL operator*() const {
		return *_extractPtr(_ptr);
	}

	inline T* AE_CALL getPtr() const {
		return _extractPtr(_ptr);
	}

	inline Tag AE_CALL getTag() const {
		return _extractTag(_ptr);
	}

	inline void AE_CALL setTag(Tag tag) {
		_packTag(_ptr, tag);
	}

	inline void AE_CALL set(T* ptr, Tag tag) {
		_packPtr(_ptr, ptr, tag);
	}

private:
	static constexpr CompressedPtr PTR_MASK = 0xFFFFFFFFFFFFULL;

	CompressedPtr _ptr;

	inline static CompressedPtr _packPtr(volatile CompressedPtr& i, T* ptr) {
		auto tag = _extractTag(i);
		i = (CompressedPtr)ptr;
		_packTag(i, tag);
	}

	inline static CompressedPtr _packPtr(volatile CompressedPtr& i, T* ptr, Tag tag) {
		i = (CompressedPtr)ptr;
		_packTag(i, tag);
	}

	inline static CompressedPtr _packTag(volatile CompressedPtr& i, Tag tag) {
		((Tag*)&i)[3] = tag;
	}

	inline static T* _extractPtr(volatile CompressedPtr const& i) {
		return (T*)(i & PTR_MASK);
	}

	inline static Tag _extractTag(volatile CompressedPtr const& i) {
		return ((Tag*)&i)[3];
	}
};
#else
#	if AE_ARCH_WORD_BITS == AE_ARCH_WORD_BITS_64
	alignas(16)
#	else
	alignas(8)
#	endif
	TaggedPtr {
public:
	using Tag = size_t;

	TaggedPtr() :
		_ptr(nullptr),
		_tag(0) {
	}

	TaggedPtr(const TaggedPtr& p) :
		_ptr(p._ptr),
		_tag(p._tag) {
	}

	TaggedPtr(T* ptr, Tag tag = 0) :
		_ptr(ptr),
		_tag(tag) {
	}

	inline T* AE_CALL operator->() const {
		return _ptr;
	}

	inline T& AE_CALL operator*() const {
		return *_ptr;
	}

	inline T* AE_CALL getPtr() const {
		return _ptr;
	}

	inline Tag AE_CALL getTag() const {
		return _tag;
	}

	inline void AE_CALL setTag(Tag tag) {
		_tag = tag;
	}

	inline void AE_CALL set(T* ptr, Tag tag) {
		_ptr = ptr;
		_tag = tag;
	}

private:
	T* _ptr;
	Tag _tag;
};
#endif

enum class LockFreeQueueMode : uint8_t {
	SPSC,
	MPSC,
	MPMC
};

template<typename T, LockFreeQueueMode Mode>
class LockFreeLinkedQueue;

template<typename T>
class LockFreeLinkedQueue<T, LockFreeQueueMode::SPSC> {
private:
	struct Node {
		Node* volatile next;
		T item;

		Node() :
			next(nullptr) {
		}

		explicit Node(const T& item) :
			next(nullptr),
			item(item) {
		}

		explicit Node(T&& item) :
			next(nullptr),
			item(std::move(item)) {
		}
	};

	Node* _head;
	Node* _tail;

public:
	LockFreeLinkedQueue():
		_head(new Node()),
		_tail(_head) {

	}

	~LockFreeLinkedQueue() {
		while (_head) {
			auto n = _head;
			_head = _head->next;

			delete n;
		}
	}

	bool enqueue(T&& item) {
		auto n = new Node(std::forward<T>(item));
		if (!n) return false;

		auto oldTail = _tail;
		_tail = n;
		oldTail->next = n;

		return true;	
	}

	bool dequeue(T& out) {
		auto poped = _head->next;
		if (!poped) return false;

		out = std::move(poped->item);

		auto oldHead = _head;
		_head = poped;
		_head->item = T();
		delete oldHead;

		return true;
	}
};

template<typename T>
class LockFreeLinkedQueue<T, LockFreeQueueMode::MPSC> {
private:
	struct Node {
		std::atomic<Node*> volatile next;
		T item;

		Node() :
			next(nullptr) {
		}

		explicit Node(const T& item) :
			next(nullptr),
			item(item) {
		}

		explicit Node(T&& item) :
			next(nullptr),
			item(std::move(item)) {
		}
	};

	Node* _head;
	std::atomic<Node*> _tail;

public:
	LockFreeLinkedQueue() :
		_head(new Node()),
		_tail(_head) {

	}

	~LockFreeLinkedQueue() {
		while (_head) {
			auto n = _head;
			_head = _head->next.load(std::memory_order::acquire);

			delete n;
		}
	}

	bool enqueue(T&& item) {
		auto n = new Node(std::forward<T>(item));
		if (!n) return false;

		auto oldTail = _tail.exchange(n, std::memory_order::acquire);
		oldTail->next.store(n, std::memory_order::release);

		return true;
	}

	bool dequeue(T& out) {
		auto poped = _head->next.load(std::memory_order::acquire);
		if (!poped) return false;

		out = std::move(poped->item);

		auto oldHead = _head;
		_head = poped;
		_head->item = T();
		delete oldHead;

		return true;
	}
};

template<typename T>
class LockFreeLinkedQueue<T, LockFreeQueueMode::MPMC> {
private:
	struct Node {
		std::atomic<Node*> volatile next;
		T item;

		Node() :
			next(nullptr) {
		}

		explicit Node(const T& item) :
			next(nullptr),
			item(item) {
		}

		explicit Node(T&& item) :
			next(nullptr),
			item(std::move(item)) {
		}
	};

	std::atomic<TaggedPtr<Node>> _head;
	std::atomic<Node*> _tail;

public:
	LockFreeLinkedQueue() :
		_head(new Node()),
		_tail(_head) {

	}

	~LockFreeLinkedQueue() {
		while (_head) {
			auto n = _head;
			_head = _head->next.load(std::memory_order::acquire);

			delete n;
		}
	}

	bool enqueue(T&& item) {
		auto n = new Node(std::forward<T>(item));
		if (!n) return false;

		auto oldTail = _tail.exchange(n, std::memory_order::acquire);
		oldTail->next.store(n, std::memory_order::release);

		return true;
	}

	bool dequeue(T& out) {
		TaggedPtr head = _head.load(std::memory_order::acquire), newHead;
		Node* poped;

		do {
			poped = head.getPtr()->next.load(std::memory_order::acquire);
			if (!poped) return false;

			out = std::move(poped->item);
			newHead.set(poped, head.getTag() + 1);
		} while (!_head.compare_exchange_strong(head, newHead, std::memory_order::release, std::memory_order::relaxed));

		out = std::move(poped->item);
		delete poped;

		return true;
	}
};

class InputTester : public BaseTester {
public:
	static std::string AE_CALL getGamepadKeyString(GamepadVirtualKeyCode code) {
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
					//args.insert("ignoreXInputDevices", true);
					args.insert("filter", DeviceType::GAMEPAD);
					//initInputModule(inputModules, "libs/" + getDLLName("ae-input-direct-input"), &args);
				}
				if (1) {
					SerializableObject args;
					args.insert("app", app.uintptr());
					args.insert("filter", DeviceType::GAMEPAD);
					//initInputModule(inputModules, "libs/" + getDLLName("ae-input-raw-input"), &args);
				}
				if (1) {
					SerializableObject args;
					args.insert("app", app.uintptr());
					args.insert("filter", DeviceType::GAMEPAD);
					initInputModule(inputModules, "libs/" + getDLLName("ae-input-xinput"), &args);
				}
				if (1) {
					SerializableObject args;
					args.insert("app", app.uintptr());
					args.insert("filter", DeviceType::GAMEPAD);
					//initInputModule(inputModules, "libs/" + getDLLName("ae-input-hid-input"), &args);
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

					if ((info->type & (DeviceType::GAMEPAD)) != DeviceType::UNKNOWN) {
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
								float32_t dz[] = { Math::ONE_HALF<DeviceStateValue> - Math::FORTIETH<DeviceStateValue>, Math::ONE_HALF<DeviceStateValue> + Math::FORTIETH<DeviceStateValue> };
								for (size_t i = 0; i < 10; ++i) device->setState(DeviceStateType::DEAD_ZONE, GamepadVirtualKeyCode::UNDEFINED_AXIS_1 + i, dz, 2);
							}

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
									printd("gamepad move : ", info.vendorID, " pid = ", info.productID, " ", getGamepadKeyString((GamepadVirtualKeyCode)state->code), " ", ((DeviceStateValue*)state->values)[0]);
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
										printdln("gamepad touch : ", info.vendorID, " pid = ", info.productID, " id = ", touch.fingerID, " isTouched = ", touch.isTouched, " x = ", touch.position[0], " y = ", touch.position[1]);
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
		}

		return 0;
	}
};