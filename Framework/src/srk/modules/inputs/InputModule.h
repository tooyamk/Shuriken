#pragma once

#include "srk/modules/IModule.h"
#include "srk/modules/inputs/DeviceInfo.h"
#include "srk/math/Vector.h"

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
		KEY_BACKSPACE = 8,
		KEY_TAB,
		KEY_CLEAR = 12,
		KEY_ENTER,
		KEY_SHIFT = 16,
		KEY_CTRL,
		KEY_ALT,
		KEY_PAUSE,
		KEY_CAPS_LOCK,
		KEY_KANA,
		KEY_JUNJA = 23,
		KEY_FINAL,
		KEY_HANJA,
		KEY_ESCAPE = 27,
		KEY_CONVERT,
		KEY_NOCONVERT,
		KEY_ACCEPT,
		KEY_MODECHANGE,
		KEY_SPACE,
		KEY_PAGE_UP,
		KEY_PAGE_DOWN,
		KEY_END,
		KEY_HONE,
		KEY_LEFT,
		KEY_UP,
		KEY_RIGHT,
		KEY_DOWN,
		KEY_SELECT,
		KEY_PRINT,
		KEY_EXE,
		KEY_SNAPSHOT,
		KEY_INSERT,
		KEY_DELETE,
		KEY_HELP,
		KEY_0,
		KEY_1,
		KEY_2,
		KEY_3,
		KEY_4,
		KEY_5,
		KEY_6,
		KEY_7,
		KEY_8,
		KEY_9,
		KEY_A = 65,
		KEY_B,
		KEY_C,
		KEY_D,
		KEY_E,
		KEY_F,
		KEY_G,
		KEY_H,
		KEY_I,
		KEY_J,
		KEY_K,
		KEY_L,
		KEY_M,
		KEY_N,
		KEY_O,
		KEY_P,
		KEY_Q,
		KEY_R,
		KEY_S,
		KEY_T,
		KEY_U,
		KEY_V,
		KEY_W,
		KEY_X,
		KEY_Y,
		KEY_Z,
		KEY_LWIN,
		KEY_RWIN,
		KEY_APPS,
		KEY_SLEEP = 95,
		KEY_NUMPAD_0,
		KEY_NUMPAD_1,
		KEY_NUMPAD_2,
		KEY_NUMPAD_3,
		KEY_NUMPAD_4,
		KEY_NUMPAD_5,
		KEY_NUMPAD_6,
		KEY_NUMPAD_7,
		KEY_NUMPAD_8,
		KEY_NUMPAD_9,
		KEY_MULTIPLY,
		KEY_ADD,
		KEY_NUMPAD_ENTER,
		KEY_SUBTRACT,
		KEY_DECIMAL,
		KEY_DIVIDE,
		KEY_F1,
		KEY_F2,
		KEY_F3,
		KEY_F4,
		KEY_F5,
		KEY_F6,
		KEY_F7,
		KEY_F8,
		KEY_F9,
		KEY_F10,
		KEY_F11,
		KEY_F12,
		KEY_F13,
		KEY_F14,
		KEY_F15,
		KEY_F16,
		KEY_F17,
		KEY_F18,
		KEY_F19,
		KEY_F20,
		KEY_F21,
		KEY_F22,
		KEY_F23,
		KEY_F24,
		KEY_NUM_LOCK = 144,
		KEY_SCORLL,
		KEY_OEM_NEC_EQUAL,
		KEY_OEM_FJ_MASSHOU,
		KEY_OEM_FJ_TOUROKU,
		KEY_OEM_FJ_LOYA,
		KEY_OEM_FJ_ROYA,
		KEY_LSHIFT = 160,
		KEY_RSHIFT,
		KEY_LCTRL,
		KEY_RCTRL,
		KEY_LALT,
		KEY_RALT,
		KEY_WEB_BACK,
		KEY_WEB_FORWARD,
		KEY_WEB_REFRESH,
		KEY_WEB_STOP,
		KEY_WEB_SEARCH,
		KEY_WEB_FAVORITES,
		KEY_WEB_HOME,
		KEY_VOLUME_MUTE,
		KEY_VOLUME_DOWN,
		KEY_VOLUME_UP,
		KEY_MEDIA_NEXT_TRACK,
		KEY_MEDIA_PREV_TRACK,
		KEY_MEDIA_STOP,
		KEY_MEDIA_PLAY_PAUSE,
		KEY_LAUNCH_MAIL,
		KEY_LAUNCH_MEDIA_SELECT,
		KEY_LAUNCH_APP1,
		KEY_LAUNCH_APP2,
		KEY_SEMICOLON = 186,//;
		KEY_EQUALS,//=
		KEY_COMMA,//,
		KEY_MINUS,//-
		KEY_PERIOD,//.
		KEY_SLASH,///
		KEY_GRAVE,//`
		KEY_LBRACKET = 219,//[
		KEY_BACK_SLASH,//|
		KEY_RBRACKET,//]
		KEY_APOSTROPHE,//'
		KEY_OEM_8,
		KEY_AX = 225,
		KEY_OEM_102,
		KEY_ICO_HELP,
		KEY_ICO_00,
		KEY_PROCESSKEY,
		KEY_ICO_CLEAR,
		KEY_PACKET,
		KEY_OEM_RESET = 233,
		KEY_OEM_JUMP,
		KEY_OEM_PA1,
		KEY_OEM_PA2,
		KEY_OEM_PA3,
		KEY_OEM_WSCTRL,
		KEY_OEM_CUSEL,
		KEY_OEM_ATTN,
		KEY_OEM_FINISH,
		KEY_OEM_COPY,
		KEY_OEM_AUTO,
		KEY_OEM_ENLW,
		KEY_OEM_BACKTAB,
		KEY_ATTN,
		KEY_CRSEL,
		KEY_EXSEL,
		KEY_EREOF,
		KEY_PLAY,
		KEY_ZOOM,
		KEY_NONAME,
		KEY_PA1,
		KEY_OEM_CLEAR
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
		AXIS_END = AXIS_1 + 15,

		BUTTON_START,
		BUTTON_1 = BUTTON_START,
		BUTTON_END = BUTTON_1 + 31
	};


	enum class GamepadKeyFlag : uint8_t {
		NONE,

		HALF_SMALL = 0b1,
		HALF_BIG = 0b10,
		FLIP = 0b100
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

		AXIS_START,

		L_STICK = AXIS_START,
		R_STICK,

		DPAD,

		SEPARATE_AXIS_START,
		L_STICK_X_LEFT = SEPARATE_AXIS_START,
		L_STICK_X_RIGHT,
		L_STICK_Y_DOWN,
		L_STICK_Y_UP,
		R_STICK_X_LEFT,
		R_STICK_X_RIGHT,
		R_STICK_Y_DOWN,
		R_STICK_Y_UP,

		DPAD_LEFT,
		DPAD_RIGHT,
		DPAD_DOWN,
		DPAD_UP,

		L_TRIGGER,
		L2 = L_TRIGGER,//DualShock
		R_TRIGGER,
		R2 = R_TRIGGER,//DualShock

		UNDEFINED_AXIS_1,
		UNDEFINED_AXIS_END = UNDEFINED_AXIS_1 + 15,
		SEPARATE_AXIS_END = UNDEFINED_AXIS_END,
		AXIS_END = UNDEFINED_AXIS_END,

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
		UNDEFINED_BUTTON_END = UNDEFINED_BUTTON_1 + 31,
		BUTTON_END = UNDEFINED_BUTTON_END
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

		void SRK_CALL set(GamepadVirtualKeyCode vk, GamepadKeyCode k, GamepadKeyFlag flags = GamepadKeyFlag::NONE) {
			if (vk >= VK_MIN && vk <= VK_MAX) _mapping[_getIndex(vk)].set(k, flags);
		}

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

		void SRK_CALL undefinedCompletion(size_t maxAxes, size_t maxButtons);

	private:
		static constexpr GamepadVirtualKeyCode VK_MIN = GamepadVirtualKeyCode::SEPARATE_AXIS_START;
		static constexpr GamepadVirtualKeyCode VK_MAX = GamepadVirtualKeyCode::BUTTON_END;
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
		static constexpr GamepadVirtualKeyCode VK_MIN = GamepadVirtualKeyCode::AXIS_START;
		static constexpr GamepadVirtualKeyCode VK_MAX = GamepadVirtualKeyCode::BUTTON_END;
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
		const void** argv = nullptr;
	};
}