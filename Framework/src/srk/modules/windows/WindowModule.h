#pragma once

#include "srk/modules/IModule.h"
#include "srk/math/Vector.h"
#include <functional>
#include <string_view>
#include <unordered_map>

namespace srk::events {
	template<typename EvtType> class IEventDispatcher;
}

namespace srk::modules::windows {
	enum class WindowEvent : uint8_t {
		RESIZED,
		FOCUS_IN,
		FOCUS_OUT,
		CLOSING,
		CLOSED,
		INPUT
	};


	struct SRK_FW_DLL WindowStyle {
		bool minimizable = true;
		bool maximizable = true;
		bool closable = true;
		bool resizable = true;
		Vec3<uint8_t> backgroundColor;
	};


	struct SRK_FW_DLL CreateWindowDescriptor {
		WindowStyle style;
		std::string_view title;
		Vec2ui32 contentSize;
		bool fullScreen = false;
	};


	class SRK_FW_DLL IWindow : public Ref {
	public:
		virtual ~IWindow() {};

		virtual IntrusivePtr<events::IEventDispatcher<WindowEvent>> SRK_CALL getEventDispatcher() const = 0;

		virtual bool SRK_CALL isClosed() const = 0;
		virtual void* SRK_CALL getNative(const std::string_view& native) const = 0;
		virtual bool SRK_CALL isFullScreen() const = 0;
		virtual void SRK_CALL toggleFullScreen() = 0;
		virtual Vec4ui32 SRK_CALL getFrameExtents() const = 0;
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
		virtual bool SRK_CALL isMaximized() const = 0;
		virtual void SRK_CALL setMaximized() = 0;
		virtual bool SRK_CALL isMinimized() const = 0;
		virtual void SRK_CALL setMinimized() = 0;
		virtual void SRK_CALL setRestore() = 0;
		virtual bool SRK_CALL isVisible() const = 0;
		virtual void SRK_CALL setVisible(bool b) = 0;
		virtual void SRK_CALL close() = 0;
		virtual void SRK_CALL processEvent(void* data) = 0;
	};


	class SRK_FW_DLL IWindowModule : public IModule {
	public:
		using EventFn = std::function<bool(IWindow*, void*)>;

		virtual ModuleType SRK_CALL getType() const override {
			return ModuleType::WINDOW;
		}

		virtual IntrusivePtr<IWindow> SRK_CALL crerate(const CreateWindowDescriptor& desc) = 0;

		virtual bool SRK_CALL processEvent() const = 0;
		virtual bool SRK_CALL processEvent(const EventFn& fn) const = 0;
		virtual bool SRK_CALL sendEvent(void* nativeWindow, void* data, const EventFn& fn) const = 0;
	};


	class SRK_FW_DLL DefaultWindowModule : public IWindowModule {
	public:
		virtual bool SRK_CALL processEvent() const override;
		using IWindowModule::processEvent;
		virtual bool SRK_CALL sendEvent(void* nativeWindow, void* data, const IWindowModule::EventFn& fn) const override;


	protected:
		std::unordered_map<void*, IWindow*> _windows;

		void SRK_CALL _add(void* nativeWindow, IWindow* window);
		void SRK_CALL _remove(void* nativeWindow);
	};
}