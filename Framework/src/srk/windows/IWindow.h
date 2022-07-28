#pragma once

#include "srk/Intrusive.h"
#include "srk/math/Vector.h"
#include "srk/events/EventDispatcher.h"

namespace srk {
	class WindowManager;

	enum class WindowEvent : uint8_t {
		RESIZED,
		FOCUS_IN,
		FOCUS_OUT,
		CLOSING,
		CLOSED,
#if SRK_OS == SRK_OS_WINDOWS
		RAW_INPUT
#endif
	};


	struct SRK_FW_DLL WindowStyle {
		bool minimizable = true;
		bool maximizable = true;
		bool closable = true;
		bool resizable = true;
		Vec3<uint8_t> backgroundColor;
	};


	enum class WindowNative : uint8_t {
		MODULE,
		X_DISPLAY,
		WINDOW
	};


	class SRK_FW_DLL IWindow : public Ref {
	public:
		virtual ~IWindow() {};

		virtual IntrusivePtr<events::IEventDispatcher<WindowEvent>> SRK_CALL getEventDispatcher() = 0;

		virtual bool SRK_CALL create(const WindowStyle& style, const std::string_view& title, const Vec2ui32& contentSize, bool fullscreen) = 0;
		virtual bool SRK_CALL isCreated() const = 0;
		virtual void* SRK_CALL getNative(WindowNative native) const = 0;
		virtual bool SRK_CALL isFullscreen() const = 0;
		virtual void SRK_CALL toggleFullscreen() = 0;
		virtual Vec4ui32 SRK_CALL getFrameExtents() const = 0;
		virtual Vec2ui32 SRK_CALL getCurrentContentSize() const = 0;
		virtual Vec2ui32 SRK_CALL getContentSize() const = 0;
		virtual void SRK_CALL setContentSize(const Vec2ui32& size) = 0;
		virtual std::string_view SRK_CALL getTitle() const = 0;
		virtual void SRK_CALL setTitle(const std::string_view& title) = 0;
		inline void SRK_CALL setTitle(const std::u8string_view& title) {
			setTitle((const std::string_view&)title);
		}
		virtual void SRK_CALL setPosition(const Vec2i32& pos) = 0;
		virtual void SRK_CALL setCursorVisible(bool visible) = 0;
		virtual bool SRK_CALL hasFocus() const = 0;
		virtual void SRK_CALL setFocus() = 0;
		virtual bool SRK_CALL isMaximzed() const = 0;
		virtual void SRK_CALL setMaximum() = 0;
		virtual bool SRK_CALL isMinimzed() const = 0;
		virtual void SRK_CALL setMinimum() = 0;
		virtual void SRK_CALL setRestore() = 0;
		virtual bool SRK_CALL isVisible() const = 0;
		virtual void SRK_CALL setVisible(bool b) = 0;
		virtual void SRK_CALL close() = 0;
		virtual void SRK_CALL processEvent(void* data) = 0;
	};
}