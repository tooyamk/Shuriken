#pragma once

#include "srk/modules/inputs/IInputModule.h"
#include <optional>
#include <shared_mutex>

namespace srk::modules::inputs {
	class SRK_FW_DLL IGenericGamepadDriver : public Ref {
	public:
		using ReadWriteStateStartCallback = void(*)(void* custom);
		using ReadWriteStateEndCallback = void(*)(void* custom);
		using DispatchCallback = void(*)(DeviceEvent evt, void* data, void* custom);

		virtual size_t SRK_CALL getInputLength() const = 0;
		virtual size_t SRK_CALL getOutputLength() const = 0;

		virtual bool SRK_CALL init(void* inputState, void* outputState) = 0;

		virtual bool SRK_CALL isStateReady(const void* state) const = 0;

		virtual bool SRK_CALL readStateFromDevice(void* inputState) const = 0;
		virtual float32_t SRK_CALL readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const = 0;
		virtual float32_t SRK_CALL readDpadDataFromInputState(const void* inputState) const = 0;
		virtual DeviceState::CountType SRK_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count, 
			const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const = 0;
		virtual void SRK_CALL customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const = 0;

		virtual bool SRK_CALL writeStateToDevice(const void* outputState) const = 0;
		virtual DeviceState::CountType SRK_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
			ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const = 0;

		virtual void SRK_CALL setKeyMapping(GamepadKeyMapping& dst, const GamepadKeyMapping* src) const = 0;

		static float32_t SRK_CALL translate(float32_t state, GamepadKeyFlag flags);
	};

	class SRK_FW_DLL GenericGamepad : public IInputDevice {
	public:
		GenericGamepad(const DeviceInfo& info, IGenericGamepadDriver& driver, const GamepadKeyMapping* keyMapping = nullptr);
		virtual ~GenericGamepad();

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> SRK_CALL getEventDispatcher() override;
		virtual const DeviceInfo& SRK_CALL getInfo() const override;
		virtual DeviceState::CountType SRK_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType SRK_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;
		virtual void SRK_CALL poll(bool dispatchEvent) override;

	protected:
		using OUTPUT_FLAG_TYPE = uint8_t;
		static constexpr OUTPUT_FLAG_TYPE OUTPUT_DIRTY = 0b1;
		static constexpr OUTPUT_FLAG_TYPE OUTPUT_WRITING = 0b10;

		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		DeviceInfo _info;

		IntrusivePtr<IGenericGamepadDriver> _driver;

		std::atomic_bool _polling;
		uint8_t* _oldInputState;
		uint8_t* _inputBuffer;

		mutable std::shared_mutex _keyMappingMutex;
		GamepadKeyMapping _keyMapping;

		mutable std::shared_mutex _deadZoneMutex;
		std::unordered_map<GamepadVirtualKeyCode, Vec2<DeviceStateValue>> _deadZone;

		mutable std::shared_mutex _inputMutex;
		uint8_t* _inputState;

		mutable std::shared_mutex _outputMutex;
		size_t _outputLength;
		uint8_t* _outputState;
		uint8_t* _outputBuffer;
		std::atomic<OUTPUT_FLAG_TYPE> _outputFlags;
		bool _needOutput;

		void SRK_CALL _doInput(bool dispatchEvent);
		void SRK_CALL _doOutput();

		Vec2<DeviceStateValue> SRK_CALL _getDeadZone(GamepadVirtualKeyCode key) const;
		void SRK_CALL _setDeadZone(GamepadVirtualKeyCode keyCode, Vec2<DeviceStateValue>* deadZone);

		inline static DeviceStateValue SRK_CALL _normalizeStick(float32_t value) {
			return value * 2.0f - 1.0f;
		}

		void SRK_CALL _switchInputData();
	};
}