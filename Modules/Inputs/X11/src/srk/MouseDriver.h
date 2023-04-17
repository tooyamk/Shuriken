#pragma once

#include "InputListener.h"
#include "srk/modules/inputs/GenericMouse.h"

namespace srk::modules::inputs::x11 {
	class SRK_MODULE_DLL MouseDriver : public IGenericMouseDriver {
	public:
		virtual ~MouseDriver();

		static MouseDriver* SRK_CALL create(Input& input, windows::IWindow& win);

		virtual std::optional<bool> SRK_CALL readFromDevice(GenericMouseBuffer& buffer) const override;
		virtual void SRK_CALL close() override;

	private:
		MouseDriver(Input& input, windows::IWindow& win);

		InputListener _listener;

		mutable AtomicLock _lock;
		mutable GenericMouseBuffer _inputBuffer;
		mutable std::atomic_bool _changed;

		void SRK_CALL _setWheel(float32_t val);
		void SRK_CALL _setButton(MouseVirtualKeyCode vk, bool pressed);
		static void SRK_CALL _callback(const XEvent& evt, void* target);
	};
}