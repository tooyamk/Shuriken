#pragma once

#include "srk/modules/inputs/GenericDevice.h"

namespace srk::modules::inputs {
	class SRK_FW_DLL GenericKeyboardBuffer {
	public:
		static constexpr size_t COUNT = (size_t)KeyboardVirtualKeyCode::DEFINED_END - (size_t)KeyboardVirtualKeyCode::DEFINED_START + 1;
		using Data = uint8_t[(COUNT + 7) >> 3];

		GenericKeyboardBuffer();
		GenericKeyboardBuffer(nullptr_t);

		Data data;

		inline static bool SRK_CALL isValid(KeyboardVirtualKeyCode vk) {
			using namespace srk::enum_operators;

			return (size_t)(vk - KeyboardVirtualKeyCode::DEFINED_START) < COUNT;
		}

		inline void SRK_CALL set(KeyboardVirtualKeyCode vk, bool pressed) {
			using namespace srk::enum_operators;

			auto i = (size_t)(vk - KeyboardVirtualKeyCode::DEFINED_START);
			if (i >= COUNT) return;

			auto pos = i >> 3;
			auto mask = 1 << (i & 0b111);
			data[pos] &= ~mask;
			if (pressed) data[pos] |= mask;
		}

		inline bool SRK_CALL get(KeyboardVirtualKeyCode vk) const {
			using namespace srk::enum_operators;

			auto i = (size_t)(vk - KeyboardVirtualKeyCode::DEFINED_START);
			if (i >= COUNT) return false;

			return data[i >> 3] & (1 << (i & 0b111));
		}
	};


	class SRK_FW_DLL IGenericKeyboardDriver : public Ref {
	public:
		virtual std::optional<bool> SRK_CALL readFromDevice(GenericKeyboardBuffer& inputBuffer) const = 0;
		virtual void SRK_CALL close() = 0;
	};


	using GenericKeyboardBase = GenericDevice<IGenericKeyboardDriver, GenericKeyboardBuffer, void>;
	class SRK_FW_DLL GenericKeyboard : public GenericKeyboardBase {
	public:
		GenericKeyboard(const DeviceInfo& info, IGenericKeyboardDriver& driver);
		virtual ~GenericKeyboard();

		virtual DeviceState::CountType SRK_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType SRK_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;

	protected:
		GenericKeyboardBuffer _inputBuffers[3];

		virtual std::optional<bool> _readFromDevice() override;
		virtual bool _writeToDevice() override;
		virtual bool SRK_CALL _doInput() override;
		virtual void SRK_CALL _closeDevice() override;
	};
}