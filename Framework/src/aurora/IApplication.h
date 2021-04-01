#pragma once

#include "aurora/Intrusive.h"
#include "aurora/math/Vector.h"
#include "aurora/events/EventDispatcher.h"

namespace aurora {
	enum class ApplicationEvent : uint8_t {
		RESIZED,
		FOCUS_IN,
		FOCUS_OUT,
		CLOSING,
		CLOSED,
#if AE_OS == AE_OS_WIN
		RAW_INPUT
#endif
	};


	struct AE_FW_DLL ApplicationStyle {
		bool maximizeButton = true;
		bool minimizeButton = true;
		bool thickFrame = true;
		Vec3<uint8_t> backgroundColor;
	};


	enum class ApplicationNative : uint8_t {
#if AE_OS == AE_OS_WIN
		HINSTANCE,
		HWND
#elif AE_OS == AE_OS_LINUX
		DISPLAY,
		WINDOW
#endif
	};


	class AE_FW_DLL IApplication : public Ref {
	public:
		virtual ~IApplication() {};

		virtual events::IEventDispatcher<ApplicationEvent>& AE_CALL getEventDispatcher() = 0;
		virtual const events::IEventDispatcher<ApplicationEvent>& AE_CALL getEventDispatcher() const = 0;

		virtual bool AE_CALL createWindow(const ApplicationStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) = 0;
		virtual void* AE_CALL getNative(ApplicationNative native) const = 0;
		virtual bool AE_CALL isFullscreen() const = 0;
		virtual void AE_CALL toggleFullscreen() = 0;
		virtual Vec4ui32 AE_CALL getBorder() const = 0;
		virtual Vec2ui32 AE_CALL getCurrentClientSize() const = 0;
		virtual Vec2ui32 AE_CALL getClientSize() const = 0;
		virtual void AE_CALL setClientSize(const Vec2ui32& size) = 0;
		virtual void AE_CALL setWindowTitle(const std::string_view& title) = 0;
		virtual void AE_CALL setWindowPosition(const Vec2i32& pos) = 0;
		virtual void AE_CALL setCursorVisible(bool visible) = 0;
		virtual bool AE_CALL hasFocus() const = 0;
		virtual void AE_CALL setFocus() = 0;
		virtual bool AE_CALL isMaximzed() const = 0;
		virtual void AE_CALL setMaximum() = 0;
		virtual bool AE_CALL isMinimzed() const = 0;
		virtual void AE_CALL setMinimum() = 0;
		virtual void AE_CALL setRestore() = 0;
		virtual void AE_CALL pollEvents() = 0;

		virtual bool AE_CALL isVisible() const = 0;
		virtual void AE_CALL setVisible(bool b) = 0;
		virtual void AE_CALL shutdown() = 0;

		virtual std::string_view AE_CALL getAppId() const = 0;
		virtual const std::filesystem::path& AE_CALL getAppPath() const = 0;

	protected:
		IApplication() {};
	};
}