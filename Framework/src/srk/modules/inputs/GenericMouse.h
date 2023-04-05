#pragma once

#include "srk/modules/inputs/GenericDevice.h"

namespace srk::modules::inputs {
	class SRK_FW_DLL GenericMouseBuffer {
	public:
		static constexpr size_t BUTTON_COUNT = (size_t)MouseVirtualKeyCode::BUTTON_END - (size_t)MouseVirtualKeyCode::BUTTON_START + 1;
		using ButtonData = uint8_t[(BUTTON_COUNT + 7) >> 3];

		GenericMouseBuffer();
		GenericMouseBuffer(nullptr_t);

		Vec2f32 pos;
		float32_t wheel;
		ButtonData buttons;

		inline static bool SRK_CALL isValidButton(MouseVirtualKeyCode vk) {
			using namespace srk::enum_operators;

			return (size_t)(vk - MouseVirtualKeyCode::BUTTON_START) < BUTTON_COUNT;
		}

		inline void SRK_CALL setButton(MouseVirtualKeyCode vk, bool pressed) {
			using namespace srk::enum_operators;

			auto i = (size_t)(vk - MouseVirtualKeyCode::BUTTON_START);
			if (i >= BUTTON_COUNT) return;

			auto pos = i >> 3;
			auto mask = 1 << (i & 0b111);
			buttons[pos] &= ~mask;
			if (pressed) buttons[pos] |= mask;
		}

		inline bool SRK_CALL getButton(MouseVirtualKeyCode vk) const {
			using namespace srk::enum_operators;

			auto i = (size_t)(vk - MouseVirtualKeyCode::BUTTON_START);
			if (i >= BUTTON_COUNT) return false;

			return buttons[i >> 3] & (1 << (i & 0b111));
		}
	};


	class SRK_FW_DLL IGenericMouseDriver : public Ref {
	public:
		virtual std::optional<bool> SRK_CALL readFromDevice(GenericMouseBuffer& inputBuffer) const = 0;
		virtual void SRK_CALL close() = 0;
	};


	using GenericMouseBase = GenericDevice<IGenericMouseDriver, GenericMouseBuffer, void>;
	class SRK_FW_DLL GenericMouse : public GenericMouseBase {
	public:
		GenericMouse(const DeviceInfo& info, IGenericMouseDriver& driver);
		virtual ~GenericMouse();

		virtual DeviceState::CountType SRK_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType SRK_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;

	protected:
		GenericMouseBuffer _inputBuffers[3];

		virtual std::optional<bool> _readFromDevice() override;
		virtual bool _writeToDevice() override;
		virtual bool SRK_CALL _doInput() override;
		virtual void SRK_CALL _closeDevice() override;
	};
}