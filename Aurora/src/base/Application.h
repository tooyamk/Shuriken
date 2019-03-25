#pragma once

#include "base/Ref.h"
#include "math/Rect.h"
#include <string>

namespace aurora {
	namespace events {
		template<typename T> class IEventDispatcher;
	}

	class AE_DLL Application : public Ref {
	public:
		class AE_DLL Style {
		public:
			Style();
		};


		enum class Event : ui8 {
			UPDATE,
			RESIZE
		};


		Application(const i8* appId, f64 frameInterval);
		virtual ~Application();

		inline events::IEventDispatcher<Event>* AE_CALL getEventDispatcher() const {
			return _eventDispatcher;
		}

		void AE_CALL setEventDispatcher(events::IEventDispatcher<Event>* eventDispatcher);

		bool AE_CALL createWindow(const Style& style, const i8* title, const Rect<i32>& windowedRect, bool fullscreen);
		bool AE_CALL isWindowed() const;
		void AE_CALL toggleFullscreen();
		void AE_CALL getWindowedRect(Rect<i32>& dst) const;
		void AE_CALL setWindowedRect(const Rect<i32>& rect);

		void AE_CALL setVisible(bool b);
		void AE_CALL run();
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
		std::string _appId;
		Style _style;

		events::IEventDispatcher<Event>* _eventDispatcher;

		mutable Rect<i32> _windowedRect;
		Rect<i32> _curRect;

		f64 _frameInterval; //microsecond
		i64 _time;

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
		HINSTANCE _hIns;
		HWND _hWnd;
		ui32 _dwStyle;

		void AE_CALL _updateWindowParams();
		void AE_CALL _updateWindowedRect() const;
		void AE_CALL _changeWindow();
#endif
	};
}