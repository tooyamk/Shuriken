#pragma once

#include "base/Ref.h"
#include "math/Rect.h"
#include <string>

namespace aurora {
	namespace events {
		template<typename EvtType> class IEventDispatcher;
		template<typename EvtType> class IEventDispatcherAllocator;
	}

	class AE_DLL Application : public Ref {
	public:
		class AE_DLL Style {
		public:
			bool maximizeButton = false;
			bool minimizeButton = true;
			bool thickFrame = false;
		};


		enum class Event : ui8 {
			UPDATE,
			RESIZE,
			FOCUS_IN,
			FOCUS_OUT,
			CLOSING
		};


		Application(const i8* appId, f64 frameInterval, const events::IEventDispatcherAllocator<Event>& eventDispatcherAllocator);
		virtual ~Application();

		inline events::IEventDispatcher<Event>* AE_CALL getEventDispatcher() const {
			return _eventDispatcher;
		}

		bool AE_CALL createWindow(const Style& style, const std::string& title, const Rect<i32>& windowedRect, bool fullscreen);
		bool AE_CALL isWindowed() const;
		void AE_CALL toggleFullscreen();
		void AE_CALL getInnerSize(i32& w, i32& h);
		void AE_CALL getWindowedRect(Rect<i32>& dst) const;
		void AE_CALL setWindowedRect(const Rect<i32>& rect);
		void AE_CALL setWindowTitle(const std::string& title);
		void AE_CALL setCursorVisible(bool isVisible);
		bool AE_CALL hasFocus() const;
		void AE_CALL pollEvents();

		void AE_CALL setVisible(bool b);
		void AE_CALL run();
		f64 AE_CALL getFrameInterval() const;
		void AE_CALL setFrameInterval(f64 frameInterval);
		void AE_CALL resetDeltaRecord();
		void AE_CALL update(bool autoSleep);
		void AE_CALL shutdown();

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		inline HWND AE_CALL getHWND() const {
			return _hWnd;
		}
#endif

	protected:
		bool _isWindowed;
		bool _isClosing;
		std::string _appId;
		Style _style;

		const events::IEventDispatcherAllocator<Event>& _eventDispatcherAllocator;
		events::IEventDispatcher<Event>* _eventDispatcher;

		mutable Rect<i32> _windowedRect;
		Rect<i32> _wndRect;

		f64 _frameInterval; //microsecond
		i64 _time;

		void AE_CALL _adjustWindowRect(const Rect<i32>& in, Rect<i32>& out);
		void AE_CALL _recordWindowedRect() const;

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		HINSTANCE _hIns;
		HWND _hWnd;

		DWORD AE_CALL _getWindowStyle() const;
		DWORD AE_CALL _getWindowExStyle() const;
		void AE_CALL _updateWindowRectValue();
		void AE_CALL _changeWindow(bool style, bool posOrSize);

		static LRESULT CALLBACK _wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif
	};
}