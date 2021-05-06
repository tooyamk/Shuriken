#pragma once

#include "aurora/modules/inputs/IInputModule.h"
#include <optional>
#include <shared_mutex>

namespace aurora::modules::inputs {
	class AE_FW_DLL IGenericGamepadDriver : public Ref {
	public:
		using ReadWriteStateStartCallback = void(*)(void* custom);
		using ReadWriteStateEndCallback = void(*)(void* custom);
		using DispatchCallback = void(*)(DeviceEvent evt, void* data, void* custom);

		virtual size_t AE_CALL getInputLength() const = 0;
		virtual size_t AE_CALL getOutputLength() const = 0;

		virtual bool AE_CALL init(void* inputState, void* outputState) = 0;

		virtual bool AE_CALL isStateReady(const void* state) const = 0;

		virtual bool AE_CALL readStateFromDevice(void* inputState) const = 0;
		virtual float32_t AE_CALL readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const = 0;
		virtual float32_t AE_CALL readDpadDataFromInputState(const void* inputState) const = 0;
		virtual DeviceState::CountType AE_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count, 
			const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const = 0;
		virtual void AE_CALL customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const = 0;

		virtual bool AE_CALL writeStateToDevice(const void* outputState) const = 0;
		virtual DeviceState::CountType AE_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
			ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const = 0;

		virtual void AE_CALL setKeyMapping(GamepadKeyMapping& dst, const GamepadKeyMapping* src) const = 0;

		static float32_t AE_CALL translate(float32_t state, GamepadKeyFlag flags);
	};

	class AE_FW_DLL GenericGamepad : public IInputDevice {
	public:
		GenericGamepad(const DeviceInfo& info, IGenericGamepadDriver& driver, const GamepadKeyMapping* keyMapping = nullptr);
		virtual ~GenericGamepad();

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> AE_CALL getEventDispatcher() override;
		virtual const DeviceInfo& AE_CALL getInfo() const override;
		virtual DeviceState::CountType AE_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType AE_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;
		virtual void AE_CALL poll(bool dispatchEvent) override;

	protected:
		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		DeviceInfo _info;

		IntrusivePtr<IGenericGamepadDriver> _driver;

		std::recursive_mutex _pollMutex;
		uint8_t* _oldInputState;
		uint8_t* _newInputState;

		mutable std::shared_mutex _keyMappingMutex;
		GamepadKeyMapping _keyMapping;

		mutable std::shared_mutex _deadZoneMutex;
		std::unordered_map<GamepadVirtualKeyCode, Vec2<DeviceStateValue>> _deadZone;

		mutable std::shared_mutex _inputMutex;
		size_t _inputLength;
		uint8_t* _inputState;

		mutable std::shared_mutex _outputMutex;
		size_t _outputLength;
		uint8_t* _outputState;
		uint8_t* _outputBuffer;
		std::atomic_bool _outputDirty;
		bool _needOutput;

		void AE_CALL _doInput(bool dispatchEvent);
		void AE_CALL _doOutput();

		inline Vec2<DeviceStateValue> AE_CALL _getDeadZone(GamepadVirtualKeyCode key) const {
			std::shared_lock lock(_deadZoneMutex);

			if (auto itr = _deadZone.find(key); itr == _deadZone.end()) {
				return Vec2<DeviceStateValue>::ZERO;
			} else {
				return itr->second;
			}
		}

		void AE_CALL _setDeadZone(GamepadVirtualKeyCode keyCode, Vec2<DeviceStateValue>* deadZone);

		inline static DeviceStateValue AE_CALL _normalizeStick(float32_t value) {
			return value * 2.0f - 1.0f;
		}
	};
}