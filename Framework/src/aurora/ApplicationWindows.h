#pragma once

#include "aurora/IApplication.h"

#if AE_OS == AE_OS_WINDOWS
#include "aurora/math/Box.h"

namespace aurora {
	class AE_FW_DLL Application : public IApplication {
	public:
		Application(const std::string_view& appId);
		Application(const std::u8string_view& appId) : Application((const std::string_view&)appId) {}

		virtual ~Application();

		virtual IntrusivePtr<events::IEventDispatcher<ApplicationEvent>> AE_CALL getEventDispatcher() override;
		//virtual const events::IEventDispatcher<ApplicationEvent>& AE_CALL getEventDispatcher() const override;

		virtual bool AE_CALL createWindow(const ApplicationStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) override;
		virtual void* AE_CALL getNative(ApplicationNative native) const override;
		virtual bool AE_CALL isFullscreen() const override;
		virtual void AE_CALL toggleFullscreen() override;
		virtual Vec4ui32 AE_CALL getBorder() const override;
		virtual Vec2ui32 AE_CALL getCurrentClientSize() const override;
		virtual Vec2ui32 AE_CALL getClientSize() const override;
		virtual void AE_CALL setClientSize(const Vec2ui32& size) override;
		virtual void AE_CALL setWindowTitle(const std::string_view& title) override;
		inline void AE_CALL setWindowTitle(const std::u8string_view& title) {
			setWindowTitle((const std::string_view&)title);
		}
		virtual void AE_CALL setWindowPosition(const Vec2i32& pos) override;
		virtual void AE_CALL setCursorVisible(bool visible) override;
		virtual bool AE_CALL hasFocus() const override;
		virtual void AE_CALL setFocus() override;
		virtual bool AE_CALL isMaximzed() const override;
		virtual void AE_CALL setMaximum() override;
		virtual bool AE_CALL isMinimzed() const override;
		virtual void AE_CALL pollEvents() override;
		virtual void AE_CALL setMinimum() override;
		virtual void AE_CALL setRestore() override;
		virtual bool AE_CALL isVisible() const override;
		virtual void AE_CALL setVisible(bool b) override;
		virtual void AE_CALL shutdown() override;

		virtual std::string_view AE_CALL getAppId() const override;
		virtual const std::filesystem::path& AE_CALL getAppPath() const override;

	protected:
		bool _isFullscreen;
		bool _isClosing;
		bool _isVisible;
		ApplicationStyle _style;
		std::string _appId;
		Vec2ui32 _clientSize;
		Vec4i32 _border;//left, right, top, bottom

		IntrusivePtr<events::IEventDispatcher<ApplicationEvent>> _eventDispatcher;

		mutable std::filesystem::path _appPath;

		//platform
		enum class WindowState : uint8_t {
			NORMAL,
			MAXIMUM,
			MINIMUM
		};


		struct {
			WindowState wndState = WindowState::NORMAL;
			WindowState prevWndState = WindowState::NORMAL;
			bool wndDirty = false;
			bool ignoreEvtSize = false;
			HINSTANCE ins = nullptr;
			HWND wnd = nullptr;
			Vec2i32 clinetPos;
			Vec2ui32 sentSize;
			HBRUSH bkBrush = nullptr;
		} _win;


		void AE_CALL _calcBorder();
		Box2i32 AE_CALL _calcWindowRect(bool fullscreen) const;
		void AE_CALL _sendResizedEvent();
		Box2i32 AE_CALL _calcWorkArea() const;
		bool AE_CALL _setWndState(WindowState state);
		void AE_CALL _updateWindowPlacement(UINT showCmd = (std::numeric_limits<UINT>::max)());
		static DWORD AE_CALL _getWindowStyle(const ApplicationStyle& style, bool fullscreen);
		static DWORD AE_CALL _getWindowExStyle(bool fullscreen);

		template<bool FS>
		inline static constexpr UINT AE_CALL _getWindowPosFlags() {
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
	};
}
#endif