#pragma once

#include "Base.h"
#include "srk/Lock.h"
#include "srk/modules/inputs/GenericKeyboard.h"

namespace srk::modules::inputs::evdev_input {
	class Input;

	class SRK_MODULE_DLL KeyboardDriver : public IGenericKeyboardDriver {
	public:
		virtual ~KeyboardDriver();

		static KeyboardDriver* SRK_CALL create(Input& input, int32_t fd);

		virtual std::optional<bool> SRK_CALL readFromDevice(GenericKeyboardBuffer& buffer) const override;
		virtual void SRK_CALL close() override;

	private:
		KeyboardDriver(Input& input, int32_t fd);

		int32_t _fd;

		mutable AtomicLock _lock;
		mutable GenericKeyboardBuffer _inputBuffer;

		inline static constexpr KeyboardVirtualKeyCode VK_MAPPER[] = {
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_RESERVED
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
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_ZENKAKUHANKAKU
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_102ND
			KeyboardVirtualKeyCode::F11,
			KeyboardVirtualKeyCode::F12,
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_RO
			KeyboardVirtualKeyCode::UNDEFINED,//90, KEY_KATAKANA
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_HIRAGANA
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_HENKAN
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_KATAKANAHIRAGANA
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_MUHENKAN
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_KPJPCOMMA
			KeyboardVirtualKeyCode::NUMPAD_ENTER,
			KeyboardVirtualKeyCode::R_CONTROL,
			KeyboardVirtualKeyCode::NUMPAD_DIVIDE,
			KeyboardVirtualKeyCode::PRINT_SCREEN,//KEY_SYSRQ
			KeyboardVirtualKeyCode::R_ALT,//100
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_LINEFEED
			KeyboardVirtualKeyCode::HOME,
			KeyboardVirtualKeyCode::UP,
			KeyboardVirtualKeyCode::PAGE_UP,
			KeyboardVirtualKeyCode::LEFT,
			KeyboardVirtualKeyCode::RIGHT,
			KeyboardVirtualKeyCode::END,
			KeyboardVirtualKeyCode::DOWN,
			KeyboardVirtualKeyCode::PAGE_DOWN,
			KeyboardVirtualKeyCode::INSERT,//110
			KeyboardVirtualKeyCode::DEL,
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_MACRO
			KeyboardVirtualKeyCode::VOLUME_MUTE,
			KeyboardVirtualKeyCode::VOLUME_DOWN,
			KeyboardVirtualKeyCode::VOLUME_UP,
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_POWER
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_KPEQUAL
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_KPPLUSMINUS
			KeyboardVirtualKeyCode::PAUSE,
			KeyboardVirtualKeyCode::UNDEFINED,//120, KEY_SCALE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_KPCOMMA
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_HANGEUL or KEY_HANGUEL
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_HANJA
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_YEN
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_LEFTMETA
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_RIGHTMETA
			KeyboardVirtualKeyCode::MENU,
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_STOP
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_AGAIN
			KeyboardVirtualKeyCode::UNDEFINED,//130, KEY_PROPS
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_UNDO
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_FRONT
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_COPY
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_OPEN
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_PASTE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_FIND
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_CUT
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_HELP
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_MENU
			KeyboardVirtualKeyCode::LAUNCH_CALC,//140
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SETUP
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SLEEP
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_WAKEUP
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_FILE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SENDFILE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_DELETEFILE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_XFER
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_PROG1
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_PROG2
			KeyboardVirtualKeyCode::UNDEFINED,//150, KEY_WWW
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_MSDOS
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_COFFEE or KEY_SCREENLOCK
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_ROTATE_DISPLAY or KEY_DIRECTION
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_CYCLEWINDOWS
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_MAIL
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_BOOKMARKS
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_COMPUTER
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_BACK
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_FORWARD
			KeyboardVirtualKeyCode::UNDEFINED,//160, KEY_CLOSECD
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_EJECTCD
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_EJECTCLOSECD
			KeyboardVirtualKeyCode::MEDIA_NEXT_TRACK,
			KeyboardVirtualKeyCode::MEDIA_PLAY_PAUSE,
			KeyboardVirtualKeyCode::MEDIA_PREV_TRACK,
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_STOPCD
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_RECORD
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_REWIND
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_PHONE
			KeyboardVirtualKeyCode::UNDEFINED,//170, KEY_ISO
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_CONFIG
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_HOMEPAGE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_REFRESH
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_EXIT
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_MOVE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_EDIT
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SCROLLUP
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SCROLLDOWN
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_KPLEFTPAREN
			KeyboardVirtualKeyCode::UNDEFINED,//180, KEY_KPRIGHTPAREN
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_NEW
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_REDO
			KeyboardVirtualKeyCode::F13,
			KeyboardVirtualKeyCode::F14,
			KeyboardVirtualKeyCode::F15,
			KeyboardVirtualKeyCode::F16,
			KeyboardVirtualKeyCode::F17,
			KeyboardVirtualKeyCode::F18,
			KeyboardVirtualKeyCode::F19,
			KeyboardVirtualKeyCode::F20,//190
			KeyboardVirtualKeyCode::F21,
			KeyboardVirtualKeyCode::F22,
			KeyboardVirtualKeyCode::F23,
			KeyboardVirtualKeyCode::F24,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,
			KeyboardVirtualKeyCode::UNDEFINED,//200, KEY_PLAYCD
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_PAUSECD
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_PROG3
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_PROG4
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_ALL_APPLICATIONS or KEY_DASHBOARD
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SUSPEND
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_CLOSE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_PLAY
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_FASTFORWARD
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_BASSBOOST
			KeyboardVirtualKeyCode::UNDEFINED,//210, KEY_PRINT
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_HP
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_CAMERA
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SOUND
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_QUESTION
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_EMAIL
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_CHAT
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SEARCH
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_CONNECT
			KeyboardVirtualKeyCode::LEFT_BRACKET,//KEY_FINANCE
			KeyboardVirtualKeyCode::BACK_SLASH,//220, KEY_SPORT
			KeyboardVirtualKeyCode::RIGHT_BRACKET,//KEY_SHOP
			KeyboardVirtualKeyCode::APOSTROPHE,//KEY_ALTERASE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_CANCEL
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_BRIGHTNESSDOWN
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_BRIGHTNESSUP
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_MEDIA
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SWITCHVIDEOMODE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_KBDILLUMTOGGLE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_KBDILLUMDOWN
			KeyboardVirtualKeyCode::UNDEFINED,//230, KEY_KBDILLUMUP
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SEND
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_REPLY
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_FORWARDMAIL
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_SAVE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_DOCUMENTS
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_BATTERY
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_BLUETOOTH
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_WLAN
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_UWB
			KeyboardVirtualKeyCode::UNDEFINED,//240, KEY_UNKNOWN
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_VIDEO_NEXT
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_VIDEO_PREV
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_BRIGHTNESS_CYCLE
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_BRIGHTNESS_AUTO or KEY_BRIGHTNESS_ZERO
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_DISPLAY_OFF
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_WWAN or KEY_WIMAX
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_RFKILL
			KeyboardVirtualKeyCode::UNDEFINED,//KEY_MICMUTE
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