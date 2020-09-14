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
			bool maximizeButton = true;
			bool minimizeButton = true;
			bool thickFrame = true;
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

		bool AE_CALL createWindow(const Style& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen);
		inline bool AE_CALL isFullscreen() const {
			return _isFullscreen;
		}
		void AE_CALL toggleFullscreen();
		inline const Vec4ui32& AE_CALL getBorder() const {
			return _border;
		}
		Vec2ui32 AE_CALL getCurrentClientSize() const;
		inline const Vec2ui32& AE_CALL getClientSize() const {
			return _clientSize;
		}
		void AE_CALL setClientSize(const Vec2ui32& size);
		void AE_CALL setWindowTitle(const std::string_view& title);
		void AE_CALL setWindowPosition(const Vec2i32& pos);
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
		Vec2ui32 _clientSize;
		Vec4ui32 _border;//left, right, up, down

		events::EventDispatcher<ApplicationEvent> _eventDispatcher;

		mutable std::filesystem::path _appPath;

		void AE_CALL _calcBorder();

#if AE_OS == AE_OS_WIN
		struct {
			HINSTANCE hIns;
			HWND hWnd;
			Vec2i32 clinetPos;
			HBRUSH bkBrush;
		} _win;


		Box2i32ui32 AE_CALL _calcWindowRect() const;
		static DWORD AE_CALL _getWindowStyle(const Style& style, bool fullscreen);
		static DWORD AE_CALL _getWindowExStyle(bool fullscreen);

		template<bool FS>
		inline static consteval UINT AE_CALL _getWindowPosFlags() {
			UINT flags = SWP_NOACTIVATE;
			if constexpr (FS) {
				flags |= SWP_NOCOPYBITS;
			} else {
				flags |= SWP_NOOWNERZORDER | SWP_NOZORDER;
			}
			return flags;
		}
		inline UINT AE_CALL _getWindowPosFlags() {
			return _isFullscreen ? _getWindowPosFlags<true>() : _getWindowPosFlags<false>();
		}

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
			Window wnd;

			Atom MOTIF_WM_HINTS;
		} _linux;
#endif
	};
}