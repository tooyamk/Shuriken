#pragma once

#include "srk/windows/IWindow.h"
#include "srk/math/Box.h"

namespace srk {
	class SRK_FW_DLL EmptyWindow : public IWindow {
	public:
		EmptyWindow();

		virtual ~EmptyWindow();

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
		virtual void SRK_CALL setMinimum() override;
		virtual void SRK_CALL setRestore() override;
		virtual void SRK_CALL pollEvents() override;
		virtual bool SRK_CALL isVisible() const override;
		virtual void SRK_CALL setVisible(bool b) override;
		virtual void SRK_CALL close() override;

	protected:
		bool _isClosing;

		IntrusivePtr<events::IEventDispatcher<WindowEvent>> _eventDispatcher;
		IntrusivePtr<Application> _app;

		//platform
		bool _windowCreated = false;
	};
}