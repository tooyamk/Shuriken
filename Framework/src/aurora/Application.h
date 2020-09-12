#pragma once

#include "aurora/Ref.h"
#include "aurora/math/Box.h"
#include "aurora/events/EventDispatcher.h"

#if AE_OS == AE_OS_WIN
#	include "mmsystem.h"
#	pragma comment( lib, "Winmm.lib")
#elif AE_OS == AE_OS_LINUX
#	include <X11/Xlib.h>
#	include <X11/Xatom.h>
#	include <X11/Xutil.h>
#endif

namespace aurora {
	enum class ApplicationEvent : uint8_t {
		RESIZED,
		FOCUS_IN,
		FOCUS_OUT,
		CLOSING,
		CLOSED
	};


	class AE_FW_DLL Application : public Ref {
	public:
		class AE_FW_DLL Style {
		public:
			bool maximizeButton = false;
			bool minimizeButton = true;
			bool thickFrame = false;
			Vec3<uint8_t> backgroundColor;
		};

		Application(const std::string_view& appId);
		virtual ~Application();

		inline events::IEventDispatcher<ApplicationEvent>& AE_CALL getEventDispatcher() {
			return _eventDispatcher;
		}
		inline const events::IEventDispatcher<ApplicationEvent>& AE_CALL getEventDispatcher() const {
			return _eventDispatcher;
		}

		bool AE_CALL createWindow(const Style& style, const std::string_view& title, const Box2i32ui32& clientRect, bool fullscreen);
		inline bool AE_CALL isFullscreen() const {
			return _isFullscreen;
		}
		void AE_CALL toggleFullscreen();
		inline Vec2ui32 AE_CALL getClientSize() const {
			Vec2ui32 size(NO_INIT);
			getClientSize(size);
			return size;
		}
		void AE_CALL getClientSize(Vec2ui32& size) const;
		void AE_CALL getWindowRect(Box2i32ui32& dst) const;
		void AE_CALL setWindowRect(const Box2i32ui32& clientRect);
		void AE_CALL setWindowTitle(const std::string_view& title);
		void AE_CALL setCursorVisible(bool visible);
		bool AE_CALL hasFocus() const;
		void AE_CALL pollEvents();

		bool AE_CALL isVisible() const;
		void AE_CALL setVisible(bool b);
		void AE_CALL shutdown();

		inline const std::string& getAppId() const {
			return _appId;
		}

		inline const std::filesystem::path& getAppPath() const {
			if (_appPath.empty()) _appPath = ::aurora::getAppPath();
			return _appPath;
		}

#if AE_OS == AE_OS_WIN
		inline HWND AE_CALL Win_getHWnd() const {
			return _win.hWnd;
		}
#endif

	protected:
		bool _isFullscreen;
		bool _isClosing;
		Style _style;
		std::string _appId;

		events::EventDispatcher<ApplicationEvent> _eventDispatcher;

		mutable Box2i32ui32 _windowRect;
		Box2i32ui32 _wndRect;

		mutable std::filesystem::path _appPath;

		bool AE_CALL _adjustWindowRect(const Box2i32ui32& in, Box2i32ui32& out);
		void AE_CALL _recordWindowRect() const;

#if AE_OS == AE_OS_WIN
		struct {
			HINSTANCE hIns;
			HWND hWnd;
			RECT lastWndClientRect;
			HBRUSH bkBrush;
		} _win;


		DWORD AE_CALL _getWindowStyle() const;
		DWORD AE_CALL _getWindowExStyle() const;
		void AE_CALL _updateWindowRectValue();
		void AE_CALL _changeWindow(bool style, bool posOrSize);

		static LRESULT CALLBACK _wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#elif AE_OS == AE_OS_LINUX
		struct MwmHints {
			uint64_t flags;
			uint64_t functions;
			uint64_t decorations;
			int64_t input_mode;
			uint64_t status;

			static const uint64_t MWM_HINTS_FUNCTIONS = 1L << 0;
			static const uint64_t MWM_HINTS_DECORATIONS = 1L << 1;

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
			bool isVisible;
			Display* dis;
			Window window;

			Atom MOTIF_WM_HINTS;
		} _linux;
#endif
	};
}