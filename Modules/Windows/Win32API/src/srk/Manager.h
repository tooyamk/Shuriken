#pragma once

#include "srk/modules/windows/IWindowModule.h"

namespace srk::modules::windows::win32api {
	class SRK_MODULE_DLL Manager : public DefaultWindowModule {
	public:
		Manager(Ref* loader);
		virtual ~Manager();

		void operator delete(Manager* p, std::destroying_delete_t) {
			auto l = p->_loader;
			p->~Manager();
			::operator delete(p);
		}

		virtual IntrusivePtr<IWindow> SRK_CALL crerateWindow(const CreateWindowDesc& desc) override;
		virtual bool SRK_CALL processEvent(const IWindowModule::EventFn& fn) const override;

		inline void SRK_CALL add(void* nativeWindow, IWindow* window) {
			_add(nativeWindow, window);
		}

		inline void SRK_CALL remove(void* nativeWindow) {
			_remove(nativeWindow);
		}

	private:
		IntrusivePtr<Ref> _loader;
	};
}