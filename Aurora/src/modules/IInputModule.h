#pragma once

#include "modules/IModule.h"

namespace aurora::events {
	template<typename EvtType> class IEventDispatcher;
}

namespace aurora::modules {
	enum class InputModuleEvent : ui8 {
		CONNECTED,
		DISCONNECTED
	};


	enum class InputDeviceEvent : ui8 {
		DOWN,
		UP,
		MOVE
	};


	class AE_DLL InputKey {
	public:
		ui32 code;
		ui32 count;
		f32* value;
	};


	class AE_DLL InputDeviceType {
	public:
		InputDeviceType() = delete;
		InputDeviceType(const InputDeviceType&) = delete;
		InputDeviceType(InputDeviceType&&) = delete;

		static const ui32 KEYBOARD = 0b1;
		static const ui32 MOUSE = 0b10;
		static const ui32 GAMEPAD = 0b100;
	};


	class AE_DLL InputDeviceGUID {
	public:
		InputDeviceGUID();
		InputDeviceGUID(const InputDeviceGUID& value);
		InputDeviceGUID(InputDeviceGUID&& value);
		~InputDeviceGUID();

		inline const ui8* AE_CALL getData() const {
			return _data;
		}

		inline ui32 AE_CALL getDataLen() const {
			return _len;
		}

		void AE_CALL set(ui8* data, ui32 len);
		bool AE_CALL isEqual(ui8* data, ui32 len) const;

		InputDeviceGUID& operator=(const InputDeviceGUID& value);
		InputDeviceGUID& operator=(InputDeviceGUID&& value);
		bool operator==(const InputDeviceGUID& right) const;

	private:
		ui8* _data;
		ui32 _len;
	};


	class AE_DLL InputDeviceInfo {
	public:
		InputDeviceInfo();
		InputDeviceInfo(const InputDeviceGUID& guid, ui32 type);
		InputDeviceInfo(const InputDeviceInfo& value);
		InputDeviceInfo(InputDeviceInfo&& value);

		InputDeviceGUID guid;
		ui32 type;

		InputDeviceInfo& operator=(const InputDeviceInfo& value);
		InputDeviceInfo& operator=(InputDeviceInfo&& value);
	};


	class AE_DLL IInputDevice : public Ref {
	public:
		virtual ~IInputDevice();

		virtual events::IEventDispatcher<InputDeviceEvent>& AE_CALL getEventDispatcher() = 0;
		virtual const InputDeviceInfo& AE_CALL getInfo() const = 0;
		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const = 0;
		virtual void AE_CALL poll(bool dispatchEvent) = 0;
	};


	class AE_DLL IInputModule : public IModule {
	public:
		IInputModule();
		virtual ~IInputModule();

		virtual ui32 AE_CALL getType() const override {
			return ModuleType::INPUT;
		}

		virtual events::IEventDispatcher<InputModuleEvent>& AE_CALL getEventDispatcher() = 0;
		virtual void AE_CALL poll() = 0;
		virtual IInputDevice* AE_CALL createDevice(const InputDeviceGUID& guid) = 0;
	};


	enum class KeyboardVirtualKeyCode : ui8 {
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
}