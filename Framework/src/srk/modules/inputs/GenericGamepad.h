#pragma once

#include "srk/modules/inputs/GenericDevice.h"

namespace srk::modules::inputs {
	class SRK_FW_DLL IGenericGamepadDriver : public Ref {
	public:
		using ReadWriteStateStartCallback = void(SRK_CALL*)(void* userData);
		using ReadWriteStateEndCallback = void(SRK_CALL*)(void* userData);
		using DispatchCallback = void(SRK_CALL*)(DeviceEvent evt, void* data, void* userData);

		virtual size_t SRK_CALL getInputBufferLength() const = 0;
		virtual size_t SRK_CALL getOutputBufferLength() const = 0;

		virtual bool SRK_CALL init(void* inputBuffer, void* outputBuffer) = 0;

		virtual bool SRK_CALL isBufferReady(const void* buffer) const = 0;

		virtual std::optional<bool> SRK_CALL readFromDevice(void* inputBuffer) const = 0;
		virtual float32_t SRK_CALL readFromInputBuffer(const void* inputBuffer, GamepadKeyCode keyCode) const = 0;
		virtual DeviceState::CountType SRK_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count, 
			const void* inputBuffer, void* userData, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateEndCallback readStateEndCallback) const = 0;
		virtual void SRK_CALL customDispatch(const void* oldInputBuffer, const void* newInputBuffer, void* userData, DispatchCallback dispatchCallback) const = 0;

		virtual bool SRK_CALL writeToDevice(const void* outputBuffer) const = 0;
		virtual DeviceState::CountType SRK_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputBuffer, void* userData,
			ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateEndCallback writeStateEndCallback) const = 0;

		virtual void SRK_CALL setKeyMapper(GamepadKeyMapper& dst, const GamepadKeyMapper* src) const = 0;

		virtual void SRK_CALL close() = 0;
	};


	using GenericGamepadBase = GenericDevice<IGenericGamepadDriver, uint8_t, uint8_t>;
	class SRK_FW_DLL GenericGamepad : public GenericGamepadBase {
	public:
		GenericGamepad(const DeviceInfo& info, IGenericGamepadDriver& driver, const GamepadKeyMapper* keyMapper = nullptr);
		virtual ~GenericGamepad();

		virtual DeviceState::CountType SRK_CALL getState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count) const override;
		virtual DeviceState::CountType SRK_CALL setState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count) override;

	protected:
		GamepadKeyMapper _keyMapper;
		GamepadKeyDeadZone _deadZone;

		uint8_t* _buffers;

		virtual std::optional<bool> _readFromDevice() override;
		virtual bool _writeToDevice() override;
		virtual bool SRK_CALL _doInput() override;
		virtual void SRK_CALL _closeDevice() override;

		void SRK_CALL _doInputMove(const GamepadKeyMapper& mapper, GamepadVirtualKeyCode combined, GamepadVirtualKeyCode separatedBegin);

		void SRK_CALL _setDeadZone(GamepadVirtualKeyCode keyCode, Vec2<DeviceStateValue>* deadZone);

		inline DeviceStateValue _readFromInputBuffer(const void* inputBuffer, const GamepadKeyCodeAndFlags& cf, DeviceStateValue defaultValue) const {
			return translate(_driver->readFromInputBuffer(inputBuffer, cf.code), cf.flags, defaultValue);
		}

		inline static DeviceStateValue SRK_CALL _normalizeStick(float32_t smallVal, float32_t bigVal) {
			return bigVal - smallVal;
		}

		void SRK_CALL _switchInputData();
	};
}