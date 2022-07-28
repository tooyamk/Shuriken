#pragma once

#include "srk/windows/IWindow.h"
#include "srk/math/Vector.h"

namespace srk {
	class SRK_FW_DLL EmptyWindow : public IWindow {
	public:
		EmptyWindow();

		virtual ~EmptyWindow();

		virtual IntrusivePtr<events::IEventDispatcher<WindowEvent>> SRK_CALL getEventDispatcher() override;

		virtual bool SRK_CALL create(const WindowStyle& style, const std::string_view& title, const Vec2ui32& contentSize, bool fullscreen) override;
		virtual bool SRK_CALL isCreated() const override;
		virtual void* SRK_CALL getNative(WindowNative native) const override;
		virtual bool SRK_CALL isFullscreen() const override;
		virtual void SRK_CALL toggleFullscreen() override;
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

	protected:
		IntrusivePtr<events::IEventDispatcher<WindowEvent>> _eventDispatcher;

		//platform
		struct {
			bool isCreated = false;
			std::string title;
		} _data;
	};
}