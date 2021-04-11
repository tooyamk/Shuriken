#pragma once

#include "aurora/modules/IModule.h"
#include "aurora/modules/inputs/DeviceInfo.h"

namespace aurora::events {
	template<typename EvtType> class IEventDispatcher;
}

namespace aurora::modules::inputs {
	enum class ModuleEvent : uint8_t {
		CONNECTED,
		DISCONNECTED
	};


	enum class DeviceEvent : uint8_t {
		DOWN,
		UP,
		MOVE
	};


	class AE_FW_DLL DeviceState {
	public:
		using CodeType = uint32_t;
		using CountType = uint32_t;
		using ValueType = float32_t;

		CodeType code;
		CountType count;
		ValueType* value;
	};


	enum class DeviceType : uint8_t {
		UNKNOWN,
		KEYBOARD = 0b1,
		MOUSE = 0b10,
		GAMEPAD = 0b100,
	};


	enum class DeviceStateType : uint8_t {
		KEY,
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
		L_STICK,
		R_STICK,

		L_THUMB,
		L3 = L_THUMB,//DualShock
		R_THUMB,
		R3 = R_THUMB,//DualShock

		DPAD,
		//DPAD_CENTER,

		L_SHOULDER,
		L1 = L_SHOULDER,//DualShock
		R_SHOULDER,
		R1 = R_SHOULDER,//DualShock

		L_TRIGGER,
		L2 = L_TRIGGER,//DualShock
		R_TRIGGER,
		R2 = R_TRIGGER,//DualShock

		//LEFT_THUMBSTICK,
		//RIGHT_THUMBSTICK,

		SELECT,
		BACK = SELECT,//XBOX360
		VIEW = SELECT,//XBOXONE
		SHARE = SELECT,//DualShock4
		MENU = SELECT,//XBOXONE

		START,
		OPTIONS = START,//DualShock4

		A,
		CROSS = A,//DualShock
		B,
		CIRCLE = B,//DualShock
		//C,
		X,
		SQUARE = X,//DualShock
		Y,
		TRIANGLE = Y,//DualShock
		//Z,

		TOUCH_PAD,//DualShock4

		UNDEFINED
	};


	template<typename T> concept DeviceKeyCode = std::same_as<T, KeyboardVirtualKeyCode> || std::same_as<T, MouseKeyCode> || std::same_as<T, GamepadKeyCode>;;


	class AE_FW_DLL IInputDevice : public Ref {
	public:
		virtual ~IInputDevice();

		virtual events::IEventDispatcher<DeviceEvent>& AE_CALL getEventDispatcher() = 0;
		virtual const DeviceInfo& AE_CALL getInfo() const = 0;

		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) const = 0;
		
		template<DeviceKeyCode T>
		inline DeviceState::CountType AE_CALL getState(DeviceStateType type, T code, DeviceState::ValueType* data, DeviceState::CountType count) const {
			return getState(type, (DeviceState::CodeType)code, data, count);
		}

		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, DeviceState::ValueType* data, DeviceState::CountType count) = 0;

		template<DeviceKeyCode T>
		inline DeviceState::CountType AE_CALL setState(DeviceStateType type, T code, DeviceState::ValueType* data, DeviceState::CountType count) {
			return setState(type, (DeviceState::CodeType)code, data, count);
		}

		virtual void AE_CALL poll(bool dispatchEvent) = 0;
	};


	class AE_FW_DLL IInputModule : public IModule {
	public:
		IInputModule();
		virtual ~IInputModule();

		virtual ModuleType AE_CALL getType() const override {
			return ModuleType::INPUT;
		}

		virtual events::IEventDispatcher<ModuleEvent>& AE_CALL getEventDispatcher() = 0;
		virtual void AE_CALL poll() = 0;
		virtual IntrusivePtr<IInputDevice> AE_CALL createDevice(const DeviceGUID& guid) = 0;
	};
}