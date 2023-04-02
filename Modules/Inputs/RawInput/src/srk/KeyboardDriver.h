#pragma once

#include "InputListener.h"
#include "srk/Lock.h"
#include "srk/modules/inputs/GenericKeyboard.h"

namespace srk::modules::inputs::raw_input {
	class SRK_MODULE_DLL KeyboardDriver : public IGenericKeyboardDriver {
	public:
		virtual ~KeyboardDriver();

		static KeyboardDriver* SRK_CALL create(Input& input, windows::IWindow& win, HANDLE handle);

		virtual bool SRK_CALL readStateFromDevice(GenericKeyboard::Buffer& buffer) const override;

	private:
		KeyboardDriver(Input& input, windows::IWindow& win, HANDLE handle);

		InputListener _listener;

		mutable AtomicLock<true, false> _lock;
		GenericKeyboard::Buffer _inputBuffer;
		std::atomic_bool _changed;

		static void SRK_CALL _callback(const RAWINPUT& rawInput, void* target);
		static KeyboardVirtualKeyCode SRK_CALL _getVirtualKey(const RAWKEYBOARD& raw);

		inline static constexpr KeyboardVirtualKeyCode VK_MAPPER[] = {
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_LBUTTON
			KeyboardVirtualKeyCode::UNDEFINED,//VK_RBUTTON
			KeyboardVirtualKeyCode::UNDEFINED,//VK_CANCEL
			KeyboardVirtualKeyCode::UNDEFINED,//VK_MBUTTON
			KeyboardVirtualKeyCode::UNDEFINED,//VK_XBUTTON1
			KeyboardVirtualKeyCode::UNDEFINED,//VK_XBUTTON2
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::BACKSPACE,
			KeyboardVirtualKeyCode::TAB,
			KeyboardVirtualKeyCode::UNDEFINED,//10
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_CLEAR
			KeyboardVirtualKeyCode::ENTER,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_SHIFT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_CONTROL
			KeyboardVirtualKeyCode::UNDEFINED,//VK_MENU
			KeyboardVirtualKeyCode::PAUSE,
			KeyboardVirtualKeyCode::CAPS_LOCK,//20
			KeyboardVirtualKeyCode::UNDEFINED,//VK_KANA or VK_HANGEUL or VK_HANGUL
			KeyboardVirtualKeyCode::UNDEFINED,//VK_IME_ON
			KeyboardVirtualKeyCode::UNDEFINED,//VK_JUNJA
			KeyboardVirtualKeyCode::UNDEFINED,//VK_FINAL
			KeyboardVirtualKeyCode::UNDEFINED,//VK_KANJI or VK_KANJI
			KeyboardVirtualKeyCode::UNDEFINED,//VK_IME_OFF
			KeyboardVirtualKeyCode::ESCAPE,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_CONVERT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_NONCONVERT
			KeyboardVirtualKeyCode::UNDEFINED,//30, VK_ACCEPT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_MODECHANGE
			KeyboardVirtualKeyCode::SPACE,
			KeyboardVirtualKeyCode::PAGE_UP,
			KeyboardVirtualKeyCode::PAGE_DOWN,
			KeyboardVirtualKeyCode::END,
			KeyboardVirtualKeyCode::HONE,
			KeyboardVirtualKeyCode::LEFT,
			KeyboardVirtualKeyCode::UP,
			KeyboardVirtualKeyCode::RIGHT,
			KeyboardVirtualKeyCode::DOWN,//40
			KeyboardVirtualKeyCode::UNDEFINED,//VK_SELECT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_PRINT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_EXECUTE
			KeyboardVirtualKeyCode::PRINT_SCREEN,
			KeyboardVirtualKeyCode::INSERT,
			KeyboardVirtualKeyCode::DEL,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_HELP
			KeyboardVirtualKeyCode::_0,
			KeyboardVirtualKeyCode::_1,
			KeyboardVirtualKeyCode::_2,//50
			KeyboardVirtualKeyCode::_3,
			KeyboardVirtualKeyCode::_4,
			KeyboardVirtualKeyCode::_5,
			KeyboardVirtualKeyCode::_6,
			KeyboardVirtualKeyCode::_7,
			KeyboardVirtualKeyCode::_8,
			KeyboardVirtualKeyCode::_9,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::A,
			KeyboardVirtualKeyCode::B,
			KeyboardVirtualKeyCode::C,
			KeyboardVirtualKeyCode::D,
			KeyboardVirtualKeyCode::E,
			KeyboardVirtualKeyCode::F,//70
			KeyboardVirtualKeyCode::G,
			KeyboardVirtualKeyCode::H,
			KeyboardVirtualKeyCode::I,
			KeyboardVirtualKeyCode::J,
			KeyboardVirtualKeyCode::K,
			KeyboardVirtualKeyCode::L,
			KeyboardVirtualKeyCode::M,
			KeyboardVirtualKeyCode::N,
			KeyboardVirtualKeyCode::O,
			KeyboardVirtualKeyCode::P,//80
			KeyboardVirtualKeyCode::Q,
			KeyboardVirtualKeyCode::R,
			KeyboardVirtualKeyCode::S,
			KeyboardVirtualKeyCode::T,
			KeyboardVirtualKeyCode::U,
			KeyboardVirtualKeyCode::V,
			KeyboardVirtualKeyCode::W,
			KeyboardVirtualKeyCode::X,
			KeyboardVirtualKeyCode::Y,
			KeyboardVirtualKeyCode::Z,//90
			KeyboardVirtualKeyCode::L_WIN,
			KeyboardVirtualKeyCode::R_WIN,
			KeyboardVirtualKeyCode::APPS,//APPS
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_SLEEP
			KeyboardVirtualKeyCode::NUMPAD_0,
			KeyboardVirtualKeyCode::NUMPAD_1,
			KeyboardVirtualKeyCode::NUMPAD_2,
			KeyboardVirtualKeyCode::NUMPAD_3,
			KeyboardVirtualKeyCode::NUMPAD_4,//100
			KeyboardVirtualKeyCode::NUMPAD_5,
			KeyboardVirtualKeyCode::NUMPAD_6,
			KeyboardVirtualKeyCode::NUMPAD_7,
			KeyboardVirtualKeyCode::NUMPAD_8,
			KeyboardVirtualKeyCode::NUMPAD_9,
			KeyboardVirtualKeyCode::NUMPAD_MULTIPLY,
			KeyboardVirtualKeyCode::NUMPAD_ADD,
			KeyboardVirtualKeyCode::NUMPAD_ENTER,
			KeyboardVirtualKeyCode::NUMPAD_MINUS,
			KeyboardVirtualKeyCode::NUMPAD_DOT,//110
			KeyboardVirtualKeyCode::NUMPAD_DIVIDE,
			KeyboardVirtualKeyCode::F1,
			KeyboardVirtualKeyCode::F2,
			KeyboardVirtualKeyCode::F3,
			KeyboardVirtualKeyCode::F4,
			KeyboardVirtualKeyCode::F5,
			KeyboardVirtualKeyCode::F6,
			KeyboardVirtualKeyCode::F7,
			KeyboardVirtualKeyCode::F8,
			KeyboardVirtualKeyCode::F9,//120
			KeyboardVirtualKeyCode::F10,
			KeyboardVirtualKeyCode::F11,
			KeyboardVirtualKeyCode::F12,
			KeyboardVirtualKeyCode::F13,
			KeyboardVirtualKeyCode::F14,
			KeyboardVirtualKeyCode::F15,
			KeyboardVirtualKeyCode::F16,
			KeyboardVirtualKeyCode::F17,
			KeyboardVirtualKeyCode::F18,
			KeyboardVirtualKeyCode::F19,//130
			KeyboardVirtualKeyCode::F20,
			KeyboardVirtualKeyCode::F21,
			KeyboardVirtualKeyCode::F22,
			KeyboardVirtualKeyCode::F23,
			KeyboardVirtualKeyCode::F24,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_NAVIGATION_VIEW
			KeyboardVirtualKeyCode::UNDEFINED,//VK_NAVIGATION_MENU
			KeyboardVirtualKeyCode::UNDEFINED,//VK_NAVIGATION_UP
			KeyboardVirtualKeyCode::UNDEFINED,//VK_NAVIGATION_DOWN
			KeyboardVirtualKeyCode::UNDEFINED,//VK_NAVIGATION_LEFT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_NAVIGATION_RIGHT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_NAVIGATION_ACCEPT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_NAVIGATION_CANCEL
			KeyboardVirtualKeyCode::NUM_LOCK,
			KeyboardVirtualKeyCode::SCORLL_LOCK,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_NEC_EQUAL or VK_OEM_FJ_JISHO
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_FJ_MASSHOU
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_FJ_TOUROKU
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_FJ_LOYA
			KeyboardVirtualKeyCode::UNDEFINED,//150, VK_OEM_FJ_ROYA
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::L_SHIFT,
			KeyboardVirtualKeyCode::R_SHIFT,
			KeyboardVirtualKeyCode::L_CONTROL,
			KeyboardVirtualKeyCode::R_CONTROL,
			KeyboardVirtualKeyCode::L_ALT,
			KeyboardVirtualKeyCode::R_ALT,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_BROWSER_BACK
			KeyboardVirtualKeyCode::UNDEFINED,//VK_BROWSER_FORWARD
			KeyboardVirtualKeyCode::UNDEFINED,//VK_BROWSER_REFRESH
			KeyboardVirtualKeyCode::UNDEFINED,//VK_BROWSER_STOP
			KeyboardVirtualKeyCode::UNDEFINED,//170, VK_BROWSER_SEARCH
			KeyboardVirtualKeyCode::UNDEFINED,//VK_BROWSER_FAVORITES
			KeyboardVirtualKeyCode::UNDEFINED,//VK_BROWSER_HOME
			KeyboardVirtualKeyCode::UNDEFINED,//VK_VOLUME_MUTE
			KeyboardVirtualKeyCode::UNDEFINED,//VK_VOLUME_DOWN
			KeyboardVirtualKeyCode::UNDEFINED,//VK_VOLUME_UP
			KeyboardVirtualKeyCode::UNDEFINED,//VK_MEDIA_NEXT_TRACK
			KeyboardVirtualKeyCode::UNDEFINED,//VK_MEDIA_PREV_TRACK
			KeyboardVirtualKeyCode::UNDEFINED,//VK_MEDIA_STOP
			KeyboardVirtualKeyCode::UNDEFINED,//VK_MEDIA_PLAY_PAUSE
			KeyboardVirtualKeyCode::UNDEFINED,//180, VK_LAUNCH_MAIL
			KeyboardVirtualKeyCode::UNDEFINED,//VK_LAUNCH_MEDIA_SELECT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_LAUNCH_APP1
			KeyboardVirtualKeyCode::UNDEFINED,//VK_LAUNCH_APP2
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::SEMICOLON,//;
			KeyboardVirtualKeyCode::EQUAL,//=
			KeyboardVirtualKeyCode::COMMA,//,
			KeyboardVirtualKeyCode::MINUS,//-
			KeyboardVirtualKeyCode::DOT,//. 190
			KeyboardVirtualKeyCode::SLASH,///
			KeyboardVirtualKeyCode::GRAVE,//`
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_A
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_B
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_X
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_Y
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_RIGHT_SHOULDER
			KeyboardVirtualKeyCode::UNDEFINED,//200, VK_GAMEPAD_LEFT_SHOULDER
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_LEFT_TRIGGER
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_RIGHT_TRIGGER
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_DPAD_UP
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_DPAD_DOWN
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_DPAD_LEFT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_DPAD_RIGHT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_MENU
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_VIEW
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_LEFT_THUMBSTICK_BUTTON
			KeyboardVirtualKeyCode::UNDEFINED,//210, VK_GAMEPAD_RIGHT_THUMBSTICK_BUTTON
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_LEFT_THUMBSTICK_UP
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_LEFT_THUMBSTICK_DOWN
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_LEFT_THUMBSTICK_RIGHT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_LEFT_THUMBSTICK_LEFT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_RIGHT_THUMBSTICK_UP
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_RIGHT_THUMBSTICK_DOWN
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_RIGHT_THUMBSTICK_RIGHT
			KeyboardVirtualKeyCode::UNDEFINED,//VK_GAMEPAD_RIGHT_THUMBSTICK_LEFT
			KeyboardVirtualKeyCode::LEFT_BRACKET,//[
			KeyboardVirtualKeyCode::BACK_SLASH,//220, |
			KeyboardVirtualKeyCode::RIGHT_BRACKET,//]
			KeyboardVirtualKeyCode::APOSTROPHE,//'
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_8
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_AX
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_102
			KeyboardVirtualKeyCode::UNDEFINED,//VK_ICO_HELP
			KeyboardVirtualKeyCode::UNDEFINED,//VK_ICO_00
			KeyboardVirtualKeyCode::UNDEFINED,//VK_PROCESSKEY
			KeyboardVirtualKeyCode::UNDEFINED,//230, VK_ICO_CLEAR
			KeyboardVirtualKeyCode::UNDEFINED,//VK_PACKET,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_RESET
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_JUMP
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_PA1
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_PA2
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_PA3
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_WSCTRL
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_CUSEL
			KeyboardVirtualKeyCode::UNDEFINED,//240, VK_OEM_ATTN
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_FINISH
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_COPY
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_AUTO
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_ENLW
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_BACKTAB
			KeyboardVirtualKeyCode::UNDEFINED,//VK_ATTN
			KeyboardVirtualKeyCode::UNDEFINED,//VK_CRSEL
			KeyboardVirtualKeyCode::UNDEFINED,//VK_EXSEL
			KeyboardVirtualKeyCode::UNDEFINED,//VK_EREOF
			KeyboardVirtualKeyCode::UNDEFINED,//250, VK_PLAY
			KeyboardVirtualKeyCode::UNDEFINED,//VK_ZOOM
			KeyboardVirtualKeyCode::UNDEFINED,//VK_NONAME
			KeyboardVirtualKeyCode::UNDEFINED,//VK_PA1
			KeyboardVirtualKeyCode::UNDEFINED,//VK_OEM_CLEAR
			KeyboardVirtualKeyCode::UNDEFINED
		};
	};
}