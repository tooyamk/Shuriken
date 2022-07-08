#pragma once

#include "srk/Intrusive.h"
#include "srk/math/Vector.h"
#include "srk/events/EventDispatcher.h"

namespace srk {
	enum class ApplicationEvent : uint8_t {
		RESIZED,
		FOCUS_IN,
		FOCUS_OUT,
		CLOSING,
		CLOSED,
#if SRK_OS == SRK_OS_WINDOWS
		RAW_INPUT
#endif
	};


	struct SRK_FW_DLL ApplicationStyle {
		bool maximizeButton = true;
		bool minimizeButton = true;
		bool thickFrame = true;
		Vec3<uint8_t> backgroundColor;
	};


	enum class ApplicationNative : uint8_t {
		INSTANCE,
		WINDOW
	};


	class SRK_FW_DLL IApplication : public Ref {
	public:
		virtual ~IApplication() {};

		virtual IntrusivePtr<events::IEventDispatcher<ApplicationEvent>> SRK_CALL getEventDispatcher() = 0;
		//virtual const events::IEventDispatcher<ApplicationEvent>& SRK_CALL getEventDispatcher() const = 0;

		virtual bool SRK_CALL createWindow(const ApplicationStyle& style, const std::string_view& title, const Vec2ui32& clientSize, bool fullscreen) = 0;
		virtual void* SRK_CALL getNative(ApplicationNative native) const = 0;
		virtual bool SRK_CALL isFullscreen() const = 0;
		virtual void SRK_CALL toggleFullscreen() = 0;
		virtual Vec4ui32 SRK_CALL getBorder() const = 0;
		virtual Vec2ui32 SRK_CALL getCurrentClientSize() const = 0;
		virtual Vec2ui32 SRK_CALL getClientSize() const = 0;
		virtual void SRK_CALL setClientSize(const Vec2ui32& size) = 0;
		virtual void SRK_CALL setWindowTitle(const std::string_view& title) = 0;
		inline void SRK_CALL setWindowTitle(const std::u8string_view& title) {
			setWindowTitle((const std::string_view&)title);
		}
		virtual void SRK_CALL setWindowPosition(const Vec2i32& pos) = 0;
		virtual void SRK_CALL setCursorVisible(bool visible) = 0;
		virtual bool SRK_CALL hasFocus() const = 0;
		virtual void SRK_CALL setFocus() = 0;
		virtual bool SRK_CALL isMaximzed() const = 0;
		virtual void SRK_CALL setMaximum() = 0;
		virtual bool SRK_CALL isMinimzed() const = 0;
		virtual void SRK_CALL setMinimum() = 0;
		virtual void SRK_CALL setRestore() = 0;
		virtual void SRK_CALL pollEvents() = 0;

		virtual bool SRK_CALL isVisible() const = 0;
		virtual void SRK_CALL setVisible(bool b) = 0;
		virtual void SRK_CALL shutdown() = 0;

		virtual std::string_view SRK_CALL getAppId() const = 0;
		virtual const std::filesystem::path& SRK_CALL getAppPath() const = 0;

	protected:
		IApplication() {};
	};
}