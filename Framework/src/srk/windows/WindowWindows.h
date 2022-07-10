#pragma once

#include "srk/windows/IWindow.h"

#if SRK_OS == SRK_OS_WINDOWS
#include "srk/math/Box.h"

namespace srk {
	class SRK_FW_DLL Window : public IWindow {
	public:
		Window();

		virtual ~Window();

		virtual IntrusivePtr<Application> SRK_CALL getApplication() const override;
		virtual IntrusivePtr<events::IEventDispatcher<WindowEvent>> SRK_CALL getEventDispatcher() override;

		virtual bool SRK_CALL create(Application& app, const WindowStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) override;
		virtual void* SRK_CALL getNative(WindowNative native) const override;
		virtual bool SRK_CALL isFullscreen() const override;
		virtual void SRK_CALL toggleFullscreen() override;
		virtual Vec4ui32 SRK_CALL getBorder() const override;
		virtual Vec2ui32 SRK_CALL getCurrentClientSize() const override;
		virtual Vec2ui32 SRK_CALL getClientSize() const override;
		virtual void SRK_CALL setClientSize(const Vec2ui32& size) override;
		virtual void SRK_CALL setTitle(const std::string_view& title) override;
		virtual void SRK_CALL setPosition(const Vec2i32& pos) override;
		virtual void SRK_CALL setCursorVisible(bool visible) override;
		virtual bool SRK_CALL hasFocus() const override;
		virtual void SRK_CALL setFocus() override;
		virtual bool SRK_CALL isMaximzed() const override;
		virtual void SRK_CALL setMaximum() override;
		virtual bool SRK_CALL isMinimzed() const override;
		virtual void SRK_CALL pollEvents() override;
		virtual void SRK_CALL setMinimum() override;
		virtual void SRK_CALL setRestore() override;
		virtual bool SRK_CALL isVisible() const override;
		virtual void SRK_CALL setVisible(bool b) override;
		virtual void SRK_CALL close() override;

	protected:
		static std::atomic_uint32_t _counter;
		bool _isFullscreen;
		bool _isClosing;
		bool _isVisible;
		WindowStyle _style;
		Vec2ui32 _clientSize;
		Vec4i32 _border;//left, right, top, bottom

		IntrusivePtr<events::IEventDispatcher<WindowEvent>> _eventDispatcher;

		IntrusivePtr<Application> _app;

		//platform
		std::wstring _className;

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
			HWND wnd = nullptr;
			Vec2i32 clinetPos;
			Vec2ui32 sentSize;
			HBRUSH bkBrush = nullptr;
		} _win;


		void SRK_CALL _calcBorder();
		Box2i32 SRK_CALL _calcWindowRect(bool fullscreen) const;
		void SRK_CALL _sendResizedEvent();
		Box2i32 SRK_CALL _calcWorkArea() const;
		bool SRK_CALL _setWndState(WindowState state);
		void SRK_CALL _updateWindowPlacement(UINT showCmd = (std::numeric_limits<UINT>::max)());
		static DWORD SRK_CALL _getWindowStyle(const WindowStyle& style, bool fullscreen);
		static DWORD SRK_CALL _getWindowExStyle(bool fullscreen);

		template<bool FS>
		inline static constexpr UINT SRK_CALL _getWindowPosFlags() {
			UINT flags = SWP_NOACTIVATE;
			if constexpr (FS) {
				flags |= SWP_NOCOPYBITS;
			} else {
				flags |= SWP_NOOWNERZORDER | SWP_NOZORDER;
			}
			return flags;
		}

		inline UINT SRK_CALL _getWindowPosFlags() {
			return _isFullscreen ? _getWindowPosFlags<true>() : _getWindowPosFlags<false>();
		}

		static LRESULT CALLBACK _wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};
}
#endif