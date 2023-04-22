#pragma once

#include "srk/modules/windows/WindowModule.h"
#include <android/native_activity.h>

namespace srk::modules::windows::android_native {
	class Manager;

	class SRK_MODULE_DLL Window : public IWindow {
	public:
		Window(Manager& manager);
		virtual ~Window();

		void SRK_CALL operator delete(Window* p, std::destroying_delete_t) {
			auto m = p->_manager;
			p->~Window();
			::operator delete(p);
		}

		virtual IntrusivePtr<events::IEventDispatcher<WindowEvent>> SRK_CALL getEventDispatcher() const override;

		bool SRK_CALL create(const CreateWindowDescriptor& desc);

		virtual bool SRK_CALL isClosed() const override;
		virtual void* SRK_CALL getNative(const std::string_view& native) const override;
		virtual bool SRK_CALL isFullScreen() const override;
		virtual void SRK_CALL toggleFullScreen() override;
		virtual Vec4ui32 SRK_CALL getFrameExtents() const override;
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

	protected:
		IntrusivePtr<Manager> _manager;
		IntrusivePtr<events::IEventDispatcher<WindowEvent>> _eventDispatcher;

		//platform
		struct {
			bool isCreated = false;
            bool hasFocus = false;
			std::string title;
            ANativeWindow* wnd = nullptr;

			Vec2ui32 sentContentSize;
		} _data;

        
		void SRK_CALL _sendResizedEvent(const Vec2ui32& size);
	};
}