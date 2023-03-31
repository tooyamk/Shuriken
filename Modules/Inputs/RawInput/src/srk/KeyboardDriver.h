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
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::BACKSPACE,
			KeyboardVirtualKeyCode::TAB,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//CLEAR = 12
			KeyboardVirtualKeyCode::ENTER,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//SHIFT = 16
			KeyboardVirtualKeyCode::UNDEFINED,//CTRL
			KeyboardVirtualKeyCode::UNDEFINED,//ALT
			KeyboardVirtualKeyCode::PAUSE,
			KeyboardVirtualKeyCode::CAPS_LOCK,//20
			KeyboardVirtualKeyCode::UNDEFINED,//KANA
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//JUNJA = 23
			KeyboardVirtualKeyCode::UNDEFINED,//FINAL
			KeyboardVirtualKeyCode::UNDEFINED,//HANJA
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::ESCAPE,
			KeyboardVirtualKeyCode::UNDEFINED,//CONVERT
			KeyboardVirtualKeyCode::UNDEFINED,//NOCONVERT
			KeyboardVirtualKeyCode::UNDEFINED,//ACCEPT,//30
			KeyboardVirtualKeyCode::UNDEFINED,//MODECHANGE
			KeyboardVirtualKeyCode::SPACE,
			KeyboardVirtualKeyCode::PAGE_UP,
			KeyboardVirtualKeyCode::PAGE_DOWN,
			KeyboardVirtualKeyCode::END,
			KeyboardVirtualKeyCode::HONE,
			KeyboardVirtualKeyCode::LEFT,
			KeyboardVirtualKeyCode::UP,
			KeyboardVirtualKeyCode::RIGHT,
			KeyboardVirtualKeyCode::DOWN,//40
			KeyboardVirtualKeyCode::UNDEFINED,//SELECT
			KeyboardVirtualKeyCode::UNDEFINED,//PRINT
			KeyboardVirtualKeyCode::UNDEFINED,//EXE
			KeyboardVirtualKeyCode::PRINT_SCREEN,
			KeyboardVirtualKeyCode::INSERT,
			KeyboardVirtualKeyCode::DEL,
			KeyboardVirtualKeyCode::UNDEFINED,//HELP
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
			KeyboardVirtualKeyCode::UNDEFINED,//SLEEP = 95
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
			KeyboardVirtualKeyCode::NUMPAD_SUBTRACT,
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
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::NUM_LOCK,
			KeyboardVirtualKeyCode::SCORLL_LOCK,
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_NEC_EQUAL
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_FJ_MASSHOU
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_FJ_TOUROKU
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_FJ_LOYA
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_FJ_ROYA,//150
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
			KeyboardVirtualKeyCode::UNDEFINED,//WEB_BACK
			KeyboardVirtualKeyCode::UNDEFINED,//WEB_FORWARD
			KeyboardVirtualKeyCode::UNDEFINED,//WEB_REFRESH
			KeyboardVirtualKeyCode::UNDEFINED,//WEB_STOP
			KeyboardVirtualKeyCode::UNDEFINED,//WEB_SEARCH,//170
			KeyboardVirtualKeyCode::UNDEFINED,//WEB_FAVORITES
			KeyboardVirtualKeyCode::UNDEFINED,//WEB_HOME
			KeyboardVirtualKeyCode::UNDEFINED,//VOLUME_MUTE
			KeyboardVirtualKeyCode::UNDEFINED,//VOLUME_DOWN
			KeyboardVirtualKeyCode::UNDEFINED,//VOLUME_UP
			KeyboardVirtualKeyCode::UNDEFINED,//MEDIA_NEXT_TRACK
			KeyboardVirtualKeyCode::UNDEFINED,//MEDIA_PREV_TRACK
			KeyboardVirtualKeyCode::UNDEFINED,//MEDIA_STOP
			KeyboardVirtualKeyCode::UNDEFINED,//MEDIA_PLAY_PAUSE
			KeyboardVirtualKeyCode::UNDEFINED,//LAUNCH_MAIL,//180
			KeyboardVirtualKeyCode::UNDEFINED,//LAUNCH_MEDIA_SELECT
			KeyboardVirtualKeyCode::UNDEFINED,//LAUNCH_APP1
			KeyboardVirtualKeyCode::UNDEFINED,//LAUNCH_APP2
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
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::LEFT_BRACKET,//[
			KeyboardVirtualKeyCode::BACK_SLASH,//|
			KeyboardVirtualKeyCode::RIGHT_BRACKET,//]
			KeyboardVirtualKeyCode::APOSTROPHE,//'
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_8
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//AX = 225
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_102
			KeyboardVirtualKeyCode::UNDEFINED,//ICO_HELP
			KeyboardVirtualKeyCode::UNDEFINED,//ICO_00
			KeyboardVirtualKeyCode::UNDEFINED,//PROCESSKEY
			KeyboardVirtualKeyCode::UNDEFINED,//ICO_CLEAR,//230
			KeyboardVirtualKeyCode::UNDEFINED,//PACKET,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_RESET = 233,
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_JUMP
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_PA1
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_PA2
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_PA3
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_WSCTRL
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_CUSEL
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_ATTN,//240
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_FINISH
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_COPY
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_AUTO
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_ENLW
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_BACKTAB
			KeyboardVirtualKeyCode::UNDEFINED,//ATTN
			KeyboardVirtualKeyCode::UNDEFINED,//CRSEL
			KeyboardVirtualKeyCode::UNDEFINED,//EXSEL
			KeyboardVirtualKeyCode::UNDEFINED,//EREOF
			KeyboardVirtualKeyCode::UNDEFINED,//PLAY,//250
			KeyboardVirtualKeyCode::UNDEFINED,//ZOOM
			KeyboardVirtualKeyCode::UNDEFINED,//NONAME
			KeyboardVirtualKeyCode::UNDEFINED,//PA1
			KeyboardVirtualKeyCode::UNDEFINED,//OEM_CLEAR
			KeyboardVirtualKeyCode::UNDEFINED
		};
	};
}