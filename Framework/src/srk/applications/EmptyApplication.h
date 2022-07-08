#pragma once

#include "srk/applications/IApplication.h"
#include "srk/math/Box.h"

namespace srk {
	class SRK_FW_DLL EmptyApplication : public IApplication {
	public:
		EmptyApplication(const std::string_view& appId);
		EmptyApplication(const std::u8string_view& appId) : EmptyApplication((const std::string_view&)appId) {}

		virtual ~EmptyApplication();

		virtual IntrusivePtr<events::IEventDispatcher<ApplicationEvent>> SRK_CALL getEventDispatcher() override;

		virtual bool SRK_CALL createWindow(const ApplicationStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) override;
		virtual void* SRK_CALL getNative(ApplicationNative native) const override;
		virtual bool SRK_CALL isFullscreen() const override;
		virtual void SRK_CALL toggleFullscreen() override;
		virtual Vec4ui32 SRK_CALL getBorder() const override;
		virtual Vec2ui32 SRK_CALL getCurrentClientSize() const override;
		virtual Vec2ui32 SRK_CALL getClientSize() const override;
		virtual void SRK_CALL setClientSize(const Vec2ui32& size) override;
		virtual void SRK_CALL setWindowTitle(const std::string_view& title) override;
		virtual void SRK_CALL setWindowPosition(const Vec2i32& pos) override;
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
		virtual void SRK_CALL shutdown() override;

		virtual std::string_view SRK_CALL getAppId() const override;
		virtual const std::filesystem::path& SRK_CALL getAppPath() const override;

	protected:
		bool _isFullscreen;
		bool _isClosing;
		bool _isVisible;
		ApplicationStyle _style;
		std::string _appId;
		Vec2ui32 _clientSize;
		Vec4i32 _border;//left, right, top, bottom

		IntrusivePtr<events::IEventDispatcher<ApplicationEvent>> _eventDispatcher;

		std::filesystem::path _appPath;

		//platform
		bool _windowCreated = false;
	};
}