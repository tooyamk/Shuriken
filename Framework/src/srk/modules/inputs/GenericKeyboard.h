#pragma once

#include "srk/modules/inputs/InputModule.h"
#include <shared_mutex>

namespace srk::modules::inputs {
	class IGenericKeyboardDriver;

	class SRK_FW_DLL GenericKeyboard : public IInputDevice {
	public:
		class SRK_FW_DLL Buffer {
		public:
			static constexpr size_t COUNT = (size_t)KeyboardVirtualKeyCode::DEFINED_END - (size_t)KeyboardVirtualKeyCode::DEFINED_START + 1;
			using Data = uint8_t[(COUNT + 7) >> 3];

			Buffer();
			Buffer(nullptr_t);

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

		GenericKeyboard(const DeviceInfo& info, IGenericKeyboardDriver& driver);
		virtual ~GenericKeyboard();

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> SRK_CALL getEventDispatcher() override;
		virtual const DeviceInfo& SRK_CALL getInfo() const override;
		virtual DeviceState::CountType SRK_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType SRK_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;
		virtual DevicePollResult SRK_CALL poll(bool dispatchEvent) override;
		virtual void SRK_CALL close() override;

	protected:
		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		DeviceInfo _info;

		IntrusivePtr<IGenericKeyboardDriver> _driver;

		std::atomic_bool _closed;

		std::atomic_bool _polling;
		Buffer _inputBuffers[3];

		mutable std::shared_mutex _inputMutex;
		Buffer* _inputBuffer;
		Buffer* _oldInputBuffer;
		Buffer* _readInputBuffer;

		bool SRK_CALL _doInput(bool dispatchEvent);

		void SRK_CALL _switchInputData();
	};


	class SRK_FW_DLL IGenericKeyboardDriver : public Ref {
	public:
		virtual std::optional<bool> SRK_CALL readStateFromDevice(GenericKeyboard::Buffer& buffer) const = 0;
	};
}