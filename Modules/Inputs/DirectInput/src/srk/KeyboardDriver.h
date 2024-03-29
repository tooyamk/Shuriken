#pragma once

#include "Base.h"
#include "srk/modules/inputs/GenericKeyboard.h"

namespace srk::modules::inputs::direct_input {
	class SRK_MODULE_DLL KeyboardDriver : public IGenericKeyboardDriver {
	public:
		virtual ~KeyboardDriver();

		static KeyboardDriver* SRK_CALL create(Input& input, srk_IDirectInputDevice* dev);

		virtual std::optional<bool> SRK_CALL readFromDevice(GenericKeyboardBuffer& buffer) const override;
		virtual void SRK_CALL close() override;

	private:
		KeyboardDriver(Input& input, srk_IDirectInputDevice* dev);

		IntrusivePtr<Input> _input;
		srk_IDirectInputDevice* _dev;

		inline static constexpr KeyboardVirtualKeyCode VK_MAPPER[] = {
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::ESCAPE,
			KeyboardVirtualKeyCode::_1,
			KeyboardVirtualKeyCode::_2,
			KeyboardVirtualKeyCode::_3,
			KeyboardVirtualKeyCode::_4,
			KeyboardVirtualKeyCode::_5,
			KeyboardVirtualKeyCode::_6,
			KeyboardVirtualKeyCode::_7,
			KeyboardVirtualKeyCode::_8,
			KeyboardVirtualKeyCode::_9,//10
			KeyboardVirtualKeyCode::_0,
			KeyboardVirtualKeyCode::MINUS,
			KeyboardVirtualKeyCode::EQUAL,
			KeyboardVirtualKeyCode::BACKSPACE,
			KeyboardVirtualKeyCode::TAB,
			KeyboardVirtualKeyCode::Q,
			KeyboardVirtualKeyCode::W,
			KeyboardVirtualKeyCode::E,
			KeyboardVirtualKeyCode::R,
			KeyboardVirtualKeyCode::T,//20
			KeyboardVirtualKeyCode::Y,
			KeyboardVirtualKeyCode::U,
			KeyboardVirtualKeyCode::I,
			KeyboardVirtualKeyCode::O,
			KeyboardVirtualKeyCode::P,
			KeyboardVirtualKeyCode::LEFT_BRACKET,
			KeyboardVirtualKeyCode::RIGHT_BRACKET,
			KeyboardVirtualKeyCode::ENTER,
			KeyboardVirtualKeyCode::L_CONTROL,
			KeyboardVirtualKeyCode::A,//30
			KeyboardVirtualKeyCode::S,
			KeyboardVirtualKeyCode::D,
			KeyboardVirtualKeyCode::F,
			KeyboardVirtualKeyCode::G,
			KeyboardVirtualKeyCode::H,
			KeyboardVirtualKeyCode::J,
			KeyboardVirtualKeyCode::K,
			KeyboardVirtualKeyCode::L,
			KeyboardVirtualKeyCode::SEMICOLON,
			KeyboardVirtualKeyCode::APOSTROPHE,//40
			KeyboardVirtualKeyCode::GRAVE,
			KeyboardVirtualKeyCode::L_SHIFT,
			KeyboardVirtualKeyCode::BACK_SLASH,
			KeyboardVirtualKeyCode::Z,
			KeyboardVirtualKeyCode::X,
			KeyboardVirtualKeyCode::C,
			KeyboardVirtualKeyCode::V,
			KeyboardVirtualKeyCode::B,
			KeyboardVirtualKeyCode::N,
			KeyboardVirtualKeyCode::M,//50
			KeyboardVirtualKeyCode::COMMA,
			KeyboardVirtualKeyCode::DOT,
			KeyboardVirtualKeyCode::SLASH,
			KeyboardVirtualKeyCode::R_SHIFT,
			KeyboardVirtualKeyCode::NUMPAD_MULTIPLY,
			KeyboardVirtualKeyCode::L_ALT,
			KeyboardVirtualKeyCode::SPACE,
			KeyboardVirtualKeyCode::CAPS_LOCK,
			KeyboardVirtualKeyCode::F1,
			KeyboardVirtualKeyCode::F2,//60
			KeyboardVirtualKeyCode::F3,
			KeyboardVirtualKeyCode::F4,
			KeyboardVirtualKeyCode::F5,
			KeyboardVirtualKeyCode::F6,
			KeyboardVirtualKeyCode::F7,
			KeyboardVirtualKeyCode::F8,
			KeyboardVirtualKeyCode::F9,
			KeyboardVirtualKeyCode::F10,
			KeyboardVirtualKeyCode::NUM_LOCK,
			KeyboardVirtualKeyCode::SCORLL_LOCK,//70
			KeyboardVirtualKeyCode::NUMPAD_7,
			KeyboardVirtualKeyCode::NUMPAD_8,
			KeyboardVirtualKeyCode::NUMPAD_9,
			KeyboardVirtualKeyCode::NUMPAD_MINUS,
			KeyboardVirtualKeyCode::NUMPAD_4,
			KeyboardVirtualKeyCode::NUMPAD_5,
			KeyboardVirtualKeyCode::NUMPAD_6,
			KeyboardVirtualKeyCode::NUMPAD_ADD,
			KeyboardVirtualKeyCode::NUMPAD_1,
			KeyboardVirtualKeyCode::NUMPAD_2,//80
			KeyboardVirtualKeyCode::NUMPAD_3,
			KeyboardVirtualKeyCode::NUMPAD_0,
			KeyboardVirtualKeyCode::NUMPAD_DOT,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_OEM_102
			KeyboardVirtualKeyCode::F11,
			KeyboardVirtualKeyCode::F12,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//90
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::F13,//100
			KeyboardVirtualKeyCode::F14,
			KeyboardVirtualKeyCode::F15,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//110
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_KANA
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_ABNT_C1
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//120
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_CONVERT
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_NOCONVERT
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_YEN
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_ABNT_C2
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//130
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//140
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_NUMPADEQUALS
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_PREVTRACK
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_AT
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_COLON
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_UNDERLINE
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_KANJI
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_STOP
			KeyboardVirtualKeyCode::UNDEFINED,//150, DIK_AX
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_UNLABELED
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_NEXTTRACK
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::NUMPAD_ENTER,
			KeyboardVirtualKeyCode::R_CONTROL,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//160, DIK_MUTE
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_CALCULATOR
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_PLAYPAUSE
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_MEDIASTOP
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//170
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_VOLUMEDOWN
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_VOLUMEUP
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_WEBHOME
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_NUMPADCOMMA
			KeyboardVirtualKeyCode::UNDEFINED,//180
			KeyboardVirtualKeyCode::NUMPAD_DIVIDE,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::PRINT_SCREEN,//DIK_SYSRQ
			KeyboardVirtualKeyCode::R_ALT,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//190
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::PAUSE,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::HOME,
			KeyboardVirtualKeyCode::UP,//200
			KeyboardVirtualKeyCode::PAGE_UP,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::LEFT,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::RIGHT,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::END,
			KeyboardVirtualKeyCode::DOWN,
			KeyboardVirtualKeyCode::PAGE_DOWN,
			KeyboardVirtualKeyCode::INSERT,//210
			KeyboardVirtualKeyCode::DEL,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::L_WIN,
			KeyboardVirtualKeyCode::R_WIN,//220
			KeyboardVirtualKeyCode::MENU,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_POWER
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_SLEEP
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_WAKE
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_WEBSEARCH
			KeyboardVirtualKeyCode::UNDEFINED,//230, DIK_WEBFAVORITES
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_WEBREFRESH
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_WEBSTOP
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_WEBFORWARD
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_WEBBACK
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_MYCOMPUTER
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_MAIL
			KeyboardVirtualKeyCode::UNDEFINED,//DIK_MEDIASELECT
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//240
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//250
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED
		};
	};
}