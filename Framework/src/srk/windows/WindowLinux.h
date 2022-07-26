#pragma once

#include "srk/windows/IWindow.h"

#if SRK_OS == SRK_OS_LINUX
#include "srk/math/Box.h"

#ifndef SRK_HAS_X11
#	if __has_include(<X11/Xlib.h>)
#		define SRK_HAS_X11
#	endif
#endif

namespace srk {
#ifdef SRK_HAS_X11
#	define SRK_WINDOW_SUPPORTED
	class SRK_FW_DLL Window : public IWindow {
	public:
		Window();
		virtual ~Window();

		virtual IntrusivePtr<events::IEventDispatcher<WindowEvent>> SRK_CALL getEventDispatcher() override;

		virtual bool SRK_CALL create(const WindowStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) override;
		virtual bool SRK_CALL isCreated() const override;
		virtual void* SRK_CALL getNative(WindowNative native) const override;
		virtual bool SRK_CALL isFullscreen() const override;
		virtual void SRK_CALL toggleFullscreen() override;
		virtual Vec4ui32 SRK_CALL getBorder() const override;
		virtual Vec2ui32 SRK_CALL getCurrentClientSize() const override;
		virtual Vec2ui32 SRK_CALL getClientSize() const override;
		virtual void SRK_CALL setClientSize(const Vec2ui32& size) override;
		virtual std::string_view SRK_CALL getTitle() const override;
		virtual void SRK_CALL setTitle(const std::string_view& title) override;
		virtual void SRK_CALL setPosition(const Vec2i32& pos) override;
		virtual void SRK_CALL setCursorVisible(bool visible) override;
		virtual bool SRK_CALL hasFocus() const override;
		virtual void SRK_CALL setFocus() override;
		virtual bool SRK_CALL isMaximzed() const override;
		virtual void SRK_CALL setMaximum() override;
		virtual bool SRK_CALL isMinimzed() const override;
		virtual void SRK_CALL setMinimum() override;
		virtual void SRK_CALL setRestore() override;
		virtual bool SRK_CALL isVisible() const override;
		virtual void SRK_CALL setVisible(bool b) override;
		virtual void SRK_CALL close() override;
		virtual void SRK_CALL processEvent(void* data) override;

		inline static WindowManager* getManager() {
			return _manager;
		}

	protected:
		static WindowManager* _manager;

		friend WindowManager;
		using X11_Atom = size_t;
		using X11_Window = size_t;

		IntrusivePtr<events::IEventDispatcher<WindowEvent>> _eventDispatcher;

		//platform
		enum class WindowState : uint8_t {
			NORMAL,
			MAXIMUM,
			MINIMUM
		};


		struct MwmHints {
			uint64_t flags;
			uint64_t functions;
			uint64_t decorations;
			int64_t input_mode;
			uint64_t status;

			static const uint64_t MWM_HINTS_FUNCTIONS = 1L << 0;
			static const uint64_t MWM_HINTS_DECORATIONS = 1L << 1;
			static const uint64_t MWM_HINTS_INPUT_MODE = 1L << 2;
			static const uint64_t MWM_HINTS_STATUS = 1L << 3;

			static const uint64_t MWM_FUNC_ALL = 1L << 0;
			static const uint64_t MWM_FUNC_RESIZE = 1L << 1;
			static const uint64_t MWM_FUNC_MOVE = 1L << 2;
			static const uint64_t MWM_FUNC_MINIMIZE = 1L << 3;
			static const uint64_t MWM_FUNC_MAXIMIZE = 1L << 4;
			static const uint64_t MWM_FUNC_CLOSE = 1L << 5;

			static const uint64_t MWM_DECOR_ALL = 1L << 0;
			static const uint64_t MWM_DECOR_BORDER = 1L << 1;
			static const uint64_t MWM_DECOR_RESIZEH = 1L << 2;
			static const uint64_t MWM_DECOR_TITLE = 1L << 3;
			static const uint64_t MWM_DECOR_MENU = 1L << 4;
			static const uint64_t MWM_DECOR_MINIMIZE = 1L << 5;
			static const uint64_t MWM_DECOR_MAXIMIZE = 1L << 6;
		};


		struct {
			bool isCreated = false;
			bool isFullscreen = false;
			bool isVisible = false;
			bool wndDirty = false;
			bool xMapped = false;
			bool xFullscreen = false;
			bool ignoreEvtPos = false;
			bool useDisplay = false;

			WindowStyle style;
			WindowState wndState = WindowState::NORMAL;
			WindowState prevWndState = WindowState::NORMAL;
			WindowState xWndState = WindowState::NORMAL;

			std::string title;

			int32_t screen = 0;
			X11_Window root = 0;
			X11_Window wnd = 0;
			Vec2i32 wndPos;
			Vec2ui32 clientSize;
			uint32_t bgColor = 0;
			Vec2ui32 sentSize;
			Vec4i32 border;//left, right, top, bottom

			bool waitFrameEXTENTS = false;
			bool waitVisibility = false;

			X11_Atom MOTIF_WM_HINTS;
			X11_Atom WM_STATE;
			X11_Atom WM_DELETE_WINDOW;
			X11_Atom WM_PROTOCOLS;
			X11_Atom NET_WM_PING;
			X11_Atom NET_WM_WINDOW_TYPE;
			X11_Atom NET_WM_WINDOW_TYPE_NORMAL;
			X11_Atom NET_WM_STATE;
			X11_Atom NET_WM_STATE_FULLSCREEN;
			X11_Atom NET_WM_STATE_MAXIMIZED_HORZ;
			X11_Atom NET_WM_STATE_MAXIMIZED_VERT;
			X11_Atom NET_REQUEST_FRAME_EXTENTS;
			X11_Atom NET_FRAME_EXTENTS;
			X11_Atom NET_WORKAREA;
			X11_Atom NET_CURRENT_DESKTOP;
		} _data;

		static uint32_t _displayRefCount;
		static void* _display;

		void SRK_CALL _sendClientEventToWM(X11_Atom msgType, int64_t a = 0, int64_t b = 0, int64_t c = 0, int64_t d = 0, int64_t e = 0);
		void SRK_CALL _sendResizedEvent();
		//void SRK_CALL _waitEvent(bool& value);
		Box2i32 SRK_CALL _calcWorkArea() const;
		bool SRK_CALL _setWndState(WindowState state);
		size_t SRK_CALL _getXWndProperty(X11_Window wnd, X11_Atom property, X11_Atom type, uint8_t** value) const;
		bool SRK_CALL _isMaximized() const;
		bool SRK_CALL _isMinimized() const;
		int32_t SRK_CALL _getXWndState() const;
		void SRK_CALL _updateWindowPlacement();

		void SRK_CALL _doEvent(void* evt);//XEvent*

		static bool SRK_CALL _checkIfEvent(void* evt);//XEvent*
	};
#endif
}

#endif