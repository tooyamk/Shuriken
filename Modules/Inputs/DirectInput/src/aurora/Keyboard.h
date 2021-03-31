#pragma once

#include "DeviceBase.h"

namespace aurora::modules::inputs::direct_input {
	class AE_MODULE_DLL Keyboard : public DeviceBase {
	public:
		Keyboard(Input& input, LPDIRECTINPUTDEVICE8 dev, const DeviceInfo& info);

		virtual uint32_t AE_CALL getKeyState(uint32_t keyCode, float32_t* data, uint32_t count) const override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	private:
		using StateBuffer = uint8_t[256];

		StateBuffer _state;

		inline static const uint8_t SK_VK[] = {
			0,
			VK_ESCAPE,
			0x31,
			0x32,
			0x33,
			0x34,
			0x35,
			0x36,
			0x37,
			0x38,///10/0x09
			0x39,
			0x30,
			VK_OEM_MINUS,
			VK_OEM_PLUS,
			VK_BACK,
			VK_TAB,
			0x51,
			0x57,
			0x45,
			0x52,///20/0x13
			0x54,
			0x59,
			0x55,
			0x49,
			0x4F,
			0x50,
			VK_OEM_4,
			VK_OEM_6,
			VK_RETURN,
			VK_LCONTROL,///30/0x1D
			0x41,
			0x53,
			0x44,
			0x46,
			0x47,
			0x48,
			0x4A,
			0x4B,
			0x4C,
			VK_OEM_1,///40/0x27
			VK_OEM_7,
			VK_OEM_3,
			VK_LSHIFT,
			VK_OEM_5,
			0x5A,
			0x58,
			0x43,
			0x56,
			0x42,
			0x4E,///50/0x31
			0x4D,
			VK_OEM_COMMA,
			VK_OEM_PERIOD,
			VK_OEM_2,
			VK_RSHIFT,
			VK_MULTIPLY,
			VK_LMENU,
			VK_SPACE,
			VK_CAPITAL,
			VK_F1,///60/0x3B
			VK_F2,
			VK_F3,
			VK_F4,
			VK_F5,
			VK_F6,
			VK_F7,
			VK_F8,
			VK_F9,
			VK_F10,
			VK_NUMLOCK,///70/0x45
			VK_SCROLL,
			VK_NUMPAD7,
			VK_NUMPAD8,
			VK_NUMPAD9,
			VK_SUBTRACT,
			VK_NUMPAD4,
			VK_NUMPAD5,
			VK_NUMPAD6,
			VK_ADD,
			VK_NUMPAD1,///80/0x4F
			VK_NUMPAD2,
			VK_NUMPAD3,
			VK_NUMPAD0,
			VK_DECIMAL,
			0,
			0,
			VK_OEM_102,
			VK_F11,
			VK_F12,
			0,///90/0x59
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,///100/0x63
			VK_F13,
			VK_F14,
			VK_F15,
			0,
			0,
			0,
			0,
			0,
			0,
			0,///110/0x6D
			0,
			0,
			VK_KANA,
			0,
			0,
			0,//DIK_ABNT_C1
			0,
			0,
			0,
			0,///120/0x77
			0,
			VK_CONVERT,
			0,
			VK_NONCONVERT,
			0,
			0,//DIK_YEN
			0,//DIK_ABNT_C2
			0,
			0,
			0,///130/0x81
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,///140/0x8B
			0,
			0,//DIK_NUMPADEQUALS
			0,
			0,
			VK_MEDIA_PREV_TRACK,
			0,//DIK_AT
			0,//DIK_COLON
			0,//DIK_UNDERLINE
			0,//DIK_KANJI
			0,//DIK_STOP ///150/0x95
			VK_OEM_AX,
			0,//DIK_UNLABELED
			0,
			VK_MEDIA_NEXT_TRACK,
			0,
			0,
			VK_SEPARATOR,
			VK_RCONTROL,
			0,
			0,///160/0x9F
			VK_VOLUME_MUTE,
			0,//DIK_CALCULATOR
			VK_MEDIA_PLAY_PAUSE,
			0,
			VK_MEDIA_STOP,
			0,
			0,
			0,
			0,
			0,///170/0xA9
			0,
			0,
			0,
			0,
			VK_VOLUME_DOWN,
			0,
			VK_VOLUME_UP,
			0,
			VK_BROWSER_HOME,
			0,//DIK_NUMPADCOMMA ///180/0xB3
			0,
			VK_DIVIDE,
			0,
			0,//DIK_SYSRQ
			VK_RMENU,
			0,
			0,
			0,
			0,
			0,///190/0xBD
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			VK_PAUSE,
			0,
			VK_HOME,///200/0xC7
			VK_UP,
			VK_PRIOR,
			0,
			VK_LEFT,
			0,
			VK_RIGHT,
			0,
			VK_END,
			VK_DOWN,
			VK_NEXT,///210/0xD1
			VK_INSERT,
			VK_DELETE,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			VK_LWIN,///220/0xDB
			VK_RWIN,
			VK_APPS,
			0,//DIK_POWER
			VK_SLEEP,
			0,
			0,
			0,
			0,//WAKE
			0,
			VK_BROWSER_SEARCH,///230/0xE5
			VK_BROWSER_FAVORITES,
			VK_BROWSER_REFRESH,
			VK_BROWSER_STOP,
			VK_BROWSER_FORWARD,
			VK_BROWSER_BACK,
			0,//DIK_MYCOMPUTER
			VK_LAUNCH_MAIL,
			VK_LAUNCH_MEDIA_SELECT,
			0,
			0,///240/0xE9
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,///250/0xF9
			0,
			0,
			0,
			0,
			0,
			0
		};

		inline static const uint8_t VK_SK[] = {
			0,
			0,//VK_LBUTTON
			0,//VK_RBUTTON
			0,//VK_CANCEL
			0,//VK_MBUTTON
			0,//VK_XBUTTON1DIK_KANA
			0,//VK_XBUTTON2
			0,
			DIK_BACK,
			DIK_TAB,///10/0x09
			0,
			0,
			0,//VK_CLEAR
			DIK_RETURN,
			0,
			0,
			0,//VK_SHIFT
			0,//VK_CONTROL
			0,//VK_MENU
			DIK_PAUSE,///20/0x13
			DIK_CAPITAL,
			DIK_KANA,
			0,
			0,//VK_JUNJA
			0,//VK_FINAL
			0,//VK_HANJA
			0,
			DIK_ESCAPE,
			DIK_CONVERT,
			DIK_NOCONVERT,///30/0x1D
			0,//VK_ACCEPT
			0,//VK_MODECHANGE
			DIK_SPACE,
			DIK_PRIOR,
			DIK_NEXT,
			DIK_END,
			DIK_HOME,
			DIK_LEFT,
			DIK_UP,
			DIK_RIGHT,///40/0x27
			DIK_DOWN,
			0,//VK_SELECT,
			0,//VK_PRINT,
			0,//VK_EXECUTE,
			0,//VK_SNAPSHOT,
			DIK_INSERT,
			DIK_DELETE,
			0,//VK_HELP
			DIK_0,
			DIK_1,///50/0x31
			DIK_2,
			DIK_3,
			DIK_4,
			DIK_5,
			DIK_6,
			DIK_7,
			DIK_8,
			DIK_9,
			0,
			0,///60/0x3B
			0,
			0,
			0,
			0,
			0,
			DIK_A,
			DIK_B,
			DIK_C,
			DIK_D,
			DIK_E,///70/0x45
			DIK_F,
			DIK_G,
			DIK_H,
			DIK_I,
			DIK_J,
			DIK_K,
			DIK_L,
			DIK_M,
			DIK_N,
			DIK_O,///80/0x4F
			DIK_P,
			DIK_Q,
			DIK_R,
			DIK_S,
			DIK_T,
			DIK_U,
			DIK_V,
			DIK_W,
			DIK_X,
			DIK_Y,///90/0x59
			DIK_Z,
			DIK_LWIN,
			DIK_RWIN,
			DIK_APPS,
			0,
			DIK_SLEEP,
			DIK_NUMPAD0,
			DIK_NUMPAD1,
			DIK_NUMPAD2,
			DIK_NUMPAD3,///100/0x63
			DIK_NUMPAD4,
			DIK_NUMPAD5,
			DIK_NUMPAD6,
			DIK_NUMPAD7,
			DIK_NUMPAD8,
			DIK_NUMPAD9,
			DIK_MULTIPLY,
			DIK_ADD,
			DIK_NUMPADENTER,
			DIK_SUBTRACT,///110/0x6D
			DIK_DECIMAL,
			DIK_DIVIDE,
			DIK_F1,
			DIK_F2,
			DIK_F3,
			DIK_F4,
			DIK_F5,
			DIK_F6,
			DIK_F7,
			DIK_F8,///120/0x77
			DIK_F9,
			DIK_F10,
			DIK_F11,
			DIK_F12,
			DIK_F13,
			DIK_F14,
			DIK_F15,
			0,//VK_F16
			0,//VK_F17
			0,//VK_F18 ///130/0x81
			0,//VK_F19
			0,//VK_F20
			0,//VK_F21
			0,//VK_F22
			0,//VK_F23
			0,//VK_F24
			0,
			0,
			0,
			0,///140/0x8B
			0,
			0,
			0,
			0,
			DIK_NUMLOCK,
			DIK_SCROLL,
			0,//VK_OEM_NEC_EQUAL
			0,//VK_OEM_FJ_MASSHOU
			0,//VK_OEM_FJ_TOUROKU
			0,//VK_OEM_FJ_LOYA ///150/0x95
			0,//VK_OEM_FJ_ROYA
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,///160/0x9F
			DIK_LSHIFT,
			DIK_RSHIFT,
			DIK_LCONTROL,
			DIK_RCONTROL,
			DIK_LMENU,
			DIK_RMENU,
			DIK_WEBBACK,
			DIK_WEBFORWARD,
			DIK_WEBREFRESH,
			DIK_WEBSTOP,///170/0xA9
			DIK_WEBSEARCH,
			DIK_WEBFAVORITES,
			DIK_WEBHOME,
			DIK_MUTE,
			DIK_VOLUMEDOWN,
			DIK_VOLUMEUP,
			DIK_NEXTTRACK,
			DIK_PREVTRACK,
			DIK_MEDIASTOP,
			DIK_PLAYPAUSE,///180/0xB3
			DIK_MAIL,
			DIK_MEDIASELECT,
			0,//VK_LAUNCH_APP1
			0,//VK_LAUNCH_APP2
			0,
			0,
			DIK_SEMICOLON,
			DIK_EQUALS,
			DIK_COMMA,
			DIK_MINUS,///190/0xBD
			DIK_PERIOD,
			DIK_SLASH,
			DIK_GRAVE,
			0,
			0,
			0,
			0,
			0,
			0,
			0,///200/0xC7
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,///210/0xD1
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			0,
			DIK_LBRACKET,///220/0xDB
			DIK_BACKSLASH,
			DIK_RBRACKET,
			DIK_APOSTROPHE,
			0,//VK_OEM_8
			0,
			DIK_AX,
			DIK_OEM_102,
			0,//VK_ICO_HELP
			0,//VK_ICO_00
			0,//VK_PROCESSKEY ///230/0xE5
			0,//VK_ICO_CLEAR
			0,//VK_PACKET
			0,
			0,//VK_OEM_RESET
			0,//VK_OEM_JUMP
			0,//VK_OEM_PA1
			0,//VK_OEM_PA2
			0,//VK_OEM_PA3
			0,//VK_OEM_WSCTRL
			0,//VK_OEM_CUSEL ///240/0xE9
			0,//VK_OEM_ATTN
			0,//VK_OEM_FINISH
			0,//VK_OEM_COPY
			0,//VK_OEM_AUTO
			0,//VK_OEM_ENLW
			0,//VK_OEM_BACKTAB
			0,//VK_ATTN
			0,//VK_CRSEL
			0,//VK_EXSEL
			0,//VK_EREOF ///250/0xF9
			0,//VK_PLAY
			0,//VK_ZOOM
			0,//VK_NONAME
			0,//VK_PA1
			0,//VK_OEM_CLEAR
			0
		};
	};
}