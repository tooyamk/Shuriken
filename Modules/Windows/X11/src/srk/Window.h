#pragma once

#include "srk/modules/windows/IWindowModule.h"
#include "srk/math/Box.h"
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

namespace srk::modules::windows::x11 {
	class Manager;

	class SRK_MODULE_DLL Window : public IWindow {
	public:
		Window(Manager& manager);
		virtual ~Window();

		void operator delete(Window* p, std::destroying_delete_t) {
			auto m = p->_manager;
			p->~Window();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<WindowEvent>> SRK_CALL getEventDispatcher() const override;

		bool SRK_CALL create(const CreateWindowDesc& desc);

		virtual bool SRK_CALL isValid() const override;
		virtual void* SRK_CALL getNative(WindowNative native) const override;
		virtual bool SRK_CALL isFullScreen() const override;
		virtual void SRK_CALL toggleFullScreen() override;
		virtual Vec4ui32 SRK_CALL getFrameExtents() const override;
		virtual Vec2ui32 SRK_CALL getCurrentContentSize() const override;
		virtual Vec2ui32 SRK_CALL getContentSize() const override;
		virtual void SRK_CALL setContentSize(const Vec2ui32& size) override;
		virtual std::string_view SRK_CALL getTitle() const override;
		virtual void SRK_CALL setTitle(const std::string_view& title) override;
		virtual void SRK_CALL setPosition(const Vec2i32& pos) override;
		virtual void SRK_CALL setCursorVisible(bool visible) override;
		virtual bool SRK_CALL hasFocus() const override;
		virtual void SRK_CALL setFocus() override;
		virtual bool SRK_CALL isMaximized() const override;
		virtual void SRK_CALL setMaximized() override;
		virtual bool SRK_CALL isMinimized() const override;
		virtual void SRK_CALL setMinimized() override;
		virtual void SRK_CALL setRestore() override;
		virtual bool SRK_CALL isVisible() const override;
		virtual void SRK_CALL setVisible(bool b) override;
		virtual void SRK_CALL close() override;
		virtual void SRK_CALL processEvent(void* data) override;

		static bool SRK_CALL checkIfEvent(XEvent* evt);

	protected:
		IntrusivePtr<Manager> _manager;
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
			bool isFullScreen = false;
			bool isVisible = false;
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
			::Window root = 0;
			::Window wnd = 0;
			Vec2i32 wndPos;
			Vec2ui32 contentSize;
			uint32_t bgColor = 0;
			Vec2ui32 sentContentSize;
			Vec4i32 frameExtends;//left, right, top, bottom

			bool waitFrameEXTENTS = false;
			bool waitVisibility = false;

			Atom MOTIF_WM_HINTS;
			Atom WM_STATE;
			Atom WM_DELETE_WINDOW;
			Atom WM_PROTOCOLS;
			Atom NET_WM_PING;
			Atom NET_WM_WINDOW_TYPE;
			Atom NET_WM_WINDOW_TYPE_NORMAL;
			Atom NET_WM_STATE;
			Atom NET_WM_STATE_FULLSCREEN;
			Atom NET_WM_STATE_MAXIMIZED_HORZ;
			Atom NET_WM_STATE_MAXIMIZED_VERT;
			Atom NET_REQUEST_FRAME_EXTENTS;
			Atom NET_FRAME_EXTENTS;
			Atom NET_WORKAREA;
			Atom NET_CURRENT_DESKTOP;
		} _data;

		static uint32_t _displayRefCount;
		static void* _display;

		void SRK_CALL _sendClientEventToWM(Atom msgType, int64_t a = 0, int64_t b = 0, int64_t c = 0, int64_t d = 0, int64_t e = 0);
		void SRK_CALL _sendResizedEvent();
		void SRK_CALL _waitEvent(bool& value, bool canBreak);
		Box2i32 SRK_CALL _calcWorkArea() const;
		bool SRK_CALL _setWndState(WindowState state);
		size_t SRK_CALL _getXWndProperty(::Window wnd, Atom property, Atom type, uint8_t** value) const;
		bool SRK_CALL _isMaximized() const;
		bool SRK_CALL _isMinimized() const;
		int32_t SRK_CALL _getXWndState() const;
		void SRK_CALL _updateWindowPlacement();
		void SRK_CALL _waitFrameExtents();

		void SRK_CALL _doEvent(XEvent* evt);
	};
}