#pragma once

#include "srk/windows/IWindow.h"

#if SRK_OS == SRK_OS_WINDOWS
#define SRK_WINDOW_SUPPORTED
#include "srk/math/Box.h"

namespace srk {
	class WindowManager;

	class SRK_FW_DLL Window : public IWindow {
	public:
		Window();
		virtual ~Window();

		virtual IntrusivePtr<events::IEventDispatcher<WindowEvent>> SRK_CALL getEventDispatcher() override;

		virtual bool SRK_CALL create(const WindowStyle& style, const std::string_view& title, const Vec2ui32& contentSize, bool fullScreen) override;
		virtual bool SRK_CALL isCreated() const override;
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

		static std::atomic_uint32_t _counter;

		IntrusivePtr<events::IEventDispatcher<WindowEvent>> _eventDispatcher;

		//platform
		enum class WindowState : uint8_t {
			NORMAL,
			MAXIMUM,
			MINIMUM
		};


		struct {
			bool isCreated = false;
			bool isFullScreen = false;
			bool isVisible = false;
			bool wndDirty = false;
			bool ignoreEvtSize = false;

			WindowStyle style;
			WindowState wndState = WindowState::NORMAL;
			WindowState prevWndState = WindowState::NORMAL;
			
			HMODULE module = nullptr;
			HWND wnd = nullptr;
			HBRUSH bkBrush = nullptr;

			std::string title;
			std::wstring className;

			Box2i32ui32 contentRect;
			Vec2ui32 sentContentSize;
			Vec4i32 frameExtends;//left, right, top, bottom
		} _data;


		void SRK_CALL _calcBorder();
		Box2i32 SRK_CALL _calcWindowRect(bool fullscreen) const;
		void SRK_CALL _sendResizedEvent();
		Box2i32 SRK_CALL _calcWorkArea() const;
		bool SRK_CALL _setWndState(WindowState state);
		void SRK_CALL _updateWindowPlacement(UINT showCmd = (std::numeric_limits<UINT>::max)());
		static DWORD SRK_CALL _getNativeStyle(const WindowStyle& style, bool fullscreen);
		static DWORD SRK_CALL _getNativeExStyle(bool fullscreen);

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
			return _data.isFullScreen ? _getWindowPosFlags<true>() : _getWindowPosFlags<false>();
		}

		static LRESULT CALLBACK _wndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
	};
}
#endif