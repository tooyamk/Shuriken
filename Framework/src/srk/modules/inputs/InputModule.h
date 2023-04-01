#pragma once

#include "srk/modules/IModule.h"
#include "srk/modules/inputs/DeviceInfo.h"
#include "srk/math/Vector.h"
#include <bitset>

namespace srk::events {
	template<typename EvtType> class IEventDispatcher;
}

namespace srk::modules::windows {
	class IWindow;
}

namespace srk::modules::inputs {
	/*++
	DeviceStateType and DeviceEvent:
	  KEY_MAPPING, values is GamepadKeyMapping*
	  TOUCH, values is DeviceTouchStateValue*
	  others, values is DeviceStateValue*
	--*/


	enum class ModuleEvent : uint8_t {
		CONNECTED,
		DISCONNECTED
	};


	enum class DeviceEvent : uint8_t {
		DOWN,
		UP,
		MOVE,
		TOUCH
	};


	using DeviceStateValue = float32_t;


	class SRK_FW_DLL DeviceTouchStateValue {
	public:
		using FingerIDType = uint32_t;
		using CodeType = uint32_t;
		using CountType = uint32_t;

		bool isTouched = false;
		FingerIDType fingerID = 0;
		CodeType code = 0;
		Vec2<DeviceStateValue> position;

		inline bool SRK_CALL operator==(const DeviceTouchStateValue& value) const {
			return fingerID == value.fingerID && isTouched == value.isTouched && code == value.code && position == value.position;
		}
	};


	class SRK_FW_DLL DeviceState {
	public:
		using CodeType = uint32_t;
		using CountType = uint32_t;

		CodeType code;
		CountType count;
		void* values;
	};


	enum class DeviceFlag : uint8_t {
		NONE = 0,
		SPECIFIC = 0b1
	};


	enum class DeviceType : uint8_t {
		UNKNOWN,
		KEYBOARD = 0b1,
		MOUSE = 0b10,
		GAMEPAD = 0b100,
	};


	enum class DeviceStateType : uint8_t {
		UNKNOWN,
		KEY_MAPPER,
		KEY,
		TOUCH,
		TOUCH_RESOLUTION,
		DEAD_ZONE,
		VIBRATION,
		LED
	};


	enum class KeyboardVirtualKeyCode : uint8_t {
		UNKNOWN,

		DEFINED_START,
		A = DEFINED_START,
		B,
		C,
		D,
		E,
		F,
		G,
		H,
		I,
		J,
		K,
		L,
		M,
		N,
		O,
		P,
		Q,
		R,
		S,
		T,
		U,
		V,
		W,
		X,
		Y,
		Z,

		_0,
		_1,
		_2,
		_3,
		_4,
		_5,
		_6,
		_7,
		_8,
		_9,

		NUM_LOCK,
		NUMPAD_0,
		NUMPAD_1,
		NUMPAD_2,
		NUMPAD_3,
		NUMPAD_4,
		NUMPAD_5,
		NUMPAD_6,
		NUMPAD_7,
		NUMPAD_8,
		NUMPAD_9,
		NUMPAD_ADD,
		NUMPAD_MINUS,
		NUMPAD_DIVIDE,
		NUMPAD_MULTIPLY,
		NUMPAD_DOT,//DECIMAL
		NUMPAD_ENTER,

		F1,
		F2,
		F3,
		F4,
		F5,
		F6,
		F7,
		F8,
		F9,
		F10,
		F11,
		F12,
		F13,
		F14,
		F15,
		F16,
		F17,
		F18,
		F19,
		F20,
		F21,
		F22,
		F23,
		F24,

		L_SHIFT,
		R_SHIFT,
		L_CONTROL,
		R_CONTROL,
		L_ALT,
		R_ALT,
		L_WIN,
		R_WIN,

		LEFT_BRACKET,//[
		RIGHT_BRACKET,//]

		SLASH,///
		BACK_SLASH,//|

		MINUS,//-
		EQUAL,//=

		SEMICOLON,//;
		APOSTROPHE,//'

		COMMA,//,
		DOT,//. PERIOD

		GRAVE,//`

		PRINT_SCREEN,
		SCORLL_LOCK,
		PAUSE,

		INSERT,
		DEL,

		HONE,
		END,

		PAGE_UP,
		PAGE_DOWN,

		LEFT,
		RIGHT,
		DOWN,
		UP,
		
		ESCAPE,
		BACKSPACE,
		TAB,
		CAPS_LOCK,
		ENTER,
		SPACE,

		APPS,

		VOLUME_DOWN,
		VOLUME_UP,

		NEXT_SONG,
		PLAY_PAUSE,
		PREV_SONG,

		DEFINED_END = PREV_SONG,

		UNDEFINED
	};


	enum class MouseKeyCode : uint8_t {
		POSITION,
		WHEEL,
		L_BUTTON,
		R_BUTTON,
		M_BUTTON,
		FN_BUTTON_0
	};


	enum class GamepadKeyCode : uint8_t {
		UNDEFINED,

		AXIS_START,
		AXIS_1 = AXIS_START,
		AXIS_END = AXIS_1 + 12,

		HAT_START,
		HAT_1 = HAT_START,
		HAT_END = HAT_1 + 4,

		BUTTON_START,
		BUTTON_1 = BUTTON_START,
		BUTTON_END = BUTTON_1 + 31
	};


	enum class GamepadKeyFlag : uint8_t {
		NONE,

		AXIS_X = 0b1,
		AXIS_Y = 0b10,
		HALF_SMALL = 0b100,
		HALF_BIG = 0b1000,
		FLIP = 0b10000
	};


	struct SRK_FW_DLL GamepadKeyCodeAndFlags {
		GamepadKeyCode code;
		GamepadKeyFlag flags;

		GamepadKeyCodeAndFlags() :
			code(GamepadKeyCode::UNDEFINED),
			flags(GamepadKeyFlag::NONE) {
		}

		GamepadKeyCodeAndFlags(const GamepadKeyCodeAndFlags& other) :
			code(other.code),
			flags(other.flags) {
		}

		GamepadKeyCodeAndFlags(GamepadKeyCode code, GamepadKeyFlag flags) :
			code(code),
			flags(flags) {
		}

		inline void SRK_CALL clear() {
			code = GamepadKeyCode::UNDEFINED;
			flags = GamepadKeyFlag::NONE;
		}

		inline void SRK_CALL set(GamepadKeyCode code, GamepadKeyFlag flags = GamepadKeyFlag::NONE) {
			this->code = code;
			this->flags = flags;
		}
	};


	enum class GamepadVirtualKeyCode : uint8_t {
		UNKNOWN,

		COMBINED_START,

		COMBINED_AXIS_START = COMBINED_START,

		L_STICK = COMBINED_AXIS_START,
		R_STICK,

		COMBINED_AXIS_END = R_STICK,

		COMBINED_HAT_START,

		DPAD = COMBINED_HAT_START,

		COMBINED_HAT_END = DPAD,

		COMBINED_END = COMBINED_HAT_END,

		SEPARATED_START,

		SEPARATED_AXIS_START = SEPARATED_START,

		L_STICK_X_LEFT = SEPARATED_AXIS_START,
		L_STICK_X_RIGHT,
		L_STICK_Y_DOWN,
		L_STICK_Y_UP,
		R_STICK_X_LEFT,
		R_STICK_X_RIGHT,
		R_STICK_Y_DOWN,
		R_STICK_Y_UP,

		L_TRIGGER,
		L2 = L_TRIGGER,//DualShock
		R_TRIGGER,
		R2 = R_TRIGGER,//DualShock

		UNDEFINED_AXIS_1,
		UNDEFINED_AXIS_END = UNDEFINED_AXIS_1 + ((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::AXIS_END - (std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::AXIS_1),
		SEPARATED_AXIS_END = UNDEFINED_AXIS_END,

		SEPARATE_HAT_START,

		DPAD_LEFT = SEPARATE_HAT_START,
		DPAD_RIGHT,
		DPAD_DOWN,
		DPAD_UP,

		UNDEFINED_HAT_1,
		UNDEFINED_HAT_END = UNDEFINED_HAT_1 + ((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::HAT_END - (std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::HAT_1),
		SEPARATE_HAT_END = UNDEFINED_HAT_END,

		//DPAD_CENTER,

		BUTTON_START,

		A = BUTTON_START,
		CROSS = A,//DualShock
		B,
		CIRCLE = B,//DualShock
		//C,
		X,
		SQUARE = X,//DualShock
		Y,
		TRIANGLE = Y,//DualShock
		//Z,

		L_SHOULDER,
		L_BUMPER = L_SHOULDER,//Xbox
		L1 = L_SHOULDER,//DualShock
		R_SHOULDER,
		R_BUMPER = R_SHOULDER,//Xbox
		R1 = R_SHOULDER,//DualShock

		L_TRIGGER_BUTTON,
		L2_BUTTON = L_TRIGGER_BUTTON,//DualShock
		R_TRIGGER_BUTTON,
		R2_BUTTON = R_TRIGGER_BUTTON,//DualShock

		SELECT,
		BACK = SELECT,//XBOX360
		VIEW = SELECT,//XBOXONE
		SHARE = SELECT,//DualShock4
		MENU = SELECT,//XBOXONE

		START,
		OPTIONS = START,//DualShock4

		L_THUMB,
		L3 = L_THUMB,//DualShock
		R_THUMB,
		R3 = R_THUMB,//DualShock

		TOUCH_PAD,//DualShock4

		UNDEFINED_BUTTON_1,
		UNDEFINED_BUTTON_END = UNDEFINED_BUTTON_1 + ((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::BUTTON_END - (std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::BUTTON_1),
		BUTTON_END = UNDEFINED_BUTTON_END,

		SEPARATED_END = BUTTON_END
	};


	class SRK_FW_DLL GamepadKeyMapper {
	private:
		class Value {
		public:
			Value() :
				_val(0) {
			}

			Value(const Value& other) :
				_val(other._val.load()) {}

			inline Value& SRK_CALL operator=(const Value& other) {
				_val = other._val.load();
				return *this;
			}

			inline void SRK_CALL clear() {
				_val = 0;
			}

			inline void SRK_CALL set(GamepadKeyCode code, GamepadKeyFlag flags = GamepadKeyFlag::NONE) {
				_val = ((uint8_t)code << 8) | (uint8_t)flags;
			}

			inline GamepadKeyCodeAndFlags SRK_CALL get() const {
				auto v = _val.load();
				return GamepadKeyCodeAndFlags((GamepadKeyCode)(v >> 8 &0xFF), (GamepadKeyFlag)(v & 0xFF));
			}

		private:
			std::atomic_uint16_t _val;
		};

	public:
		GamepadKeyMapper(nullptr_t) {};
		GamepadKeyMapper();
		GamepadKeyMapper(const GamepadKeyMapper& other);

		GamepadKeyMapper& SRK_CALL operator=(const GamepadKeyMapper& other);

		inline void SRK_CALL set(GamepadVirtualKeyCode vk, GamepadKeyCode k, GamepadKeyFlag flags = GamepadKeyFlag::NONE) {
			if (vk >= VK_MIN && vk <= VK_MAX) _mapping[_getIndex(vk)].set(k, flags);
		}

		void SRK_CALL setDefault(uint8_t numAxes, uint8_t numDPads, uint8_t numButtons, bool axisUpIsBigValue);

		bool SRK_CALL remove(GamepadVirtualKeyCode vk);
		void SRK_CALL removeUndefined();
		void SRK_CALL clear();

		inline GamepadKeyCodeAndFlags SRK_CALL get(GamepadVirtualKeyCode vk) const {
			return vk >= VK_MIN && vk <= VK_MAX ? _mapping[_getIndex(vk)].get() : GamepadKeyCodeAndFlags();
		}

		inline void SRK_CALL get(GamepadVirtualKeyCode vkBegin, size_t n, GamepadKeyCodeAndFlags* out) const {
			using namespace srk::enum_operators;

			for (size_t i = 0; i < n; ++i) out[i] = get(vkBegin + i);
		}

		template<std::invocable<GamepadVirtualKeyCode, GamepadKeyCodeAndFlags> Fn>
		void SRK_CALL forEach(Fn&& fn) const {
			for (size_t i = 0; i < _mapping.size(); ++i) {
				auto cf = _mapping[i].get();
				if (cf.code != GamepadKeyCode::UNDEFINED) fn((GamepadVirtualKeyCode)((std::underlying_type_t<GamepadVirtualKeyCode>)VK_MIN + i), cf);
			}
		}

		template<GamepadKeyCode kBegin, GamepadKeyCode kEnd, GamepadVirtualKeyCode vkUndefinedBegin>
		void SRK_CALL undefinedCompletion(size_t num) {
			using namespace srk::enum_operators;

			if (!num) return;
			if (num > 64) num = 64;

			auto maxKey = Math::clamp(kBegin + (num - 1), kBegin, kEnd);
			constexpr auto maxUndefined = std::min<size_t>((size_t)(kEnd - kBegin) + 1, 64);
			constexpr auto vkEnd = vkUndefinedBegin + (size_t)(kEnd - kBegin);

			std::bitset<maxUndefined> vkeys;
			std::bitset<64> keys;

			for (size_t i = 0; i < _mapping.size(); ++i) {
				auto cf = _mapping[i].get();
				if (cf.code == GamepadKeyCode::UNDEFINED) continue;

				auto vk = (GamepadVirtualKeyCode)((size_t)VK_MIN + i);

				if (vk >= vkUndefinedBegin && vk <= vkEnd) vkeys[(size_t)(vk - vkUndefinedBegin)] = true;
				if (cf.code >= kBegin && cf.code <= maxKey) keys[(size_t)(cf.code - kBegin)] = true;
			}

			for (size_t i = 0; i < num; ++i) {
				if (!keys[i]) {
					for (size_t j = 0; j < vkeys.size(); ++j) {
						if (!vkeys[j]) {
							vkeys[j] = true;
							_mapping[_getIndex((GamepadVirtualKeyCode)((size_t)vkUndefinedBegin + j))].set((GamepadKeyCode)((size_t)kBegin + i));
							break;
						}
					}
				}
			}
		}

	private:
		static constexpr GamepadVirtualKeyCode VK_MIN = GamepadVirtualKeyCode::SEPARATED_START;
		static constexpr GamepadVirtualKeyCode VK_MAX = GamepadVirtualKeyCode::SEPARATED_END;
		static constexpr size_t COUNT = (std::underlying_type_t<GamepadVirtualKeyCode>)VK_MAX - (std::underlying_type_t<GamepadVirtualKeyCode>)VK_MIN + 1;

		std::array<Value, COUNT> _mapping;

		inline static size_t SRK_CALL _getIndex(GamepadVirtualKeyCode vk) {
			return (std::underlying_type_t<GamepadVirtualKeyCode>)vk - (std::underlying_type_t<GamepadVirtualKeyCode>)VK_MIN;
		}
	};


	class SRK_FW_DLL GamepadKeyDeadZone {
	private:
		class Value {
		public:
			Value() :
				_val(0) {
			}

			Value(const Value& other) :
				_val(other._val.load()) {
			}

			Value& SRK_CALL operator=(const Value& other) {
				_val = other._val.load();
				return *this;
			}

			Value& SRK_CALL operator=(const Vec2<DeviceStateValue>& val) {
				set(val);
				return *this;
			}

			inline void SRK_CALL clear() {
				_val = 0;
			}

			inline void SRK_CALL set(const Vec2<DeviceStateValue>& val) {
				float32_t x = val[0];
				float32_t y = val[1];
				_val = ((uint64_t)(*(uint32_t*)(&x)) << 32) | (uint64_t)(*(uint32_t*)(&y));
			}

			inline Vec2<DeviceStateValue> SRK_CALL get() const {
				auto v = _val.load();
				uint32_t x = v >> 32 & 0xFFFFFFFF;
				uint32_t y = v & 0xFFFFFFFF;
				return Vec2<DeviceStateValue>(*(float32_t*)(&x), *(float32_t*)(&y));
			}

		private:
			std::atomic_uint64_t _val;
		};

	public:
		GamepadKeyDeadZone(nullptr_t) {};
		GamepadKeyDeadZone();
		GamepadKeyDeadZone(const GamepadKeyDeadZone& other);

		GamepadKeyDeadZone& SRK_CALL operator=(const GamepadKeyDeadZone& other);

		void SRK_CALL set(GamepadVirtualKeyCode vk, const Vec2<DeviceStateValue>& val) {
			if (vk >= VK_MIN && vk <= VK_MAX) _values[_getIndex(vk)] = val;
		}

		bool SRK_CALL remove(GamepadVirtualKeyCode vk);
		void SRK_CALL clear();

		inline Vec2<DeviceStateValue> SRK_CALL get(GamepadVirtualKeyCode vk) const {
			return vk >= VK_MIN && vk <= VK_MAX ? _values[_getIndex(vk)].get() : Vec2<DeviceStateValue>();
		}

		inline void SRK_CALL get(GamepadVirtualKeyCode vkBegin, size_t n, Vec2<DeviceStateValue>* out) const {
			using namespace srk::enum_operators;

			for (size_t i = 0; i < n; ++i) out[i] = get(vkBegin + i);
		}

	private:
		static constexpr GamepadVirtualKeyCode VK_MIN = GamepadVirtualKeyCode::COMBINED_START;
		static constexpr GamepadVirtualKeyCode VK_MAX = GamepadVirtualKeyCode::SEPARATED_END;
		static constexpr size_t COUNT = (std::underlying_type_t<GamepadVirtualKeyCode>)VK_MAX - (std::underlying_type_t<GamepadVirtualKeyCode>)VK_MIN + 1;

		std::array<Value, COUNT> _values;

		inline static size_t SRK_CALL _getIndex(GamepadVirtualKeyCode vk) {
			return (std::underlying_type_t<GamepadVirtualKeyCode>)vk - (std::underlying_type_t<GamepadVirtualKeyCode>)VK_MIN;
		}
	};


	template<typename T> concept DeviceCode = SameAnyOf<T, KeyboardVirtualKeyCode, MouseKeyCode, GamepadVirtualKeyCode>;


	class SRK_FW_DLL IInputDevice : public Ref {
	public:
		virtual ~IInputDevice();

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> SRK_CALL getEventDispatcher() = 0;
		virtual const DeviceInfo& SRK_CALL getInfo() const = 0;

		virtual DeviceState::CountType SRK_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const = 0;
		
		template<DeviceCode T>
		inline DeviceState::CountType SRK_CALL getState(DeviceStateType type, T code, void* values, DeviceState::CountType count) const {
			return getState(type, (DeviceState::CodeType)code, values, count);
		}

		void SRK_CALL getStates(DeviceStateType type, DeviceState* states, size_t count) const;

		virtual DeviceState::CountType SRK_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) = 0;

		template<DeviceCode T>
		inline DeviceState::CountType SRK_CALL setState(DeviceStateType type, T code, const void* values, DeviceState::CountType count) {
			return setState(type, (DeviceState::CodeType)code, values, count);
		}

		void SRK_CALL setStates(DeviceStateType type, DeviceState* states, size_t count);

		virtual void SRK_CALL poll(bool dispatchEvent) = 0;

		/*++
		value is normalized,[0, 1].
		--*/
		static DeviceStateValue SRK_CALL translate(DeviceStateValue value, const Vec2<DeviceStateValue>& deadZone);

		static DeviceStateValue SRK_CALL translate(DeviceStateValue state, GamepadKeyFlag flags, DeviceStateValue defaultValue);

		/*++
		x and y is normalized,[-1, 1], left down is (-1, -1).
		--*/
		static DeviceState::CountType SRK_CALL translate(DeviceStateValue x, DeviceStateValue y, const Vec2<DeviceStateValue>& deadZone, DeviceStateValue* out, DeviceState::CountType outCount);
	};


	class SRK_FW_DLL IInputModule : public IModule {
	public:
		IInputModule();
		virtual ~IInputModule();

		virtual ModuleType SRK_CALL getType() const override {
			return ModuleType::INPUT;
		}

		virtual IntrusivePtr<events::IEventDispatcher<ModuleEvent>> SRK_CALL getEventDispatcher() = 0;
		virtual void SRK_CALL poll() = 0;
		virtual IntrusivePtr<IInputDevice> SRK_CALL createDevice(const DeviceGUID& guid) = 0;
	};


	class SRK_FW_DLL CreateInputModuleDesc {
	public:
		DeviceType filters = DeviceType::UNKNOWN;
		windows::IWindow* window = nullptr;
		size_t argc = 0;
		const std::string_view* argv = nullptr;
	};
}