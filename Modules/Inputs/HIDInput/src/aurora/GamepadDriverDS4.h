#pragma once

#include "GamepadDriverBase.h"

namespace aurora::modules::inputs::hid_input {
	class AE_MODULE_DLL GamepadDriverDS4 : public GamepadDriverBase {
	public:
		GamepadDriverDS4(Input& input, extensions::HIDDevice& hid);
		virtual ~GamepadDriverDS4();

		virtual size_t AE_CALL getInputLength() const override;
		virtual size_t AE_CALL getOutputLength() const override;

		virtual bool AE_CALL init(void* inputState, void* outputState) override;

		virtual bool AE_CALL isStateReady(const void* state) const override;

		virtual bool AE_CALL readStateFromDevice(void* inputState) const override;
		virtual float32_t AE_CALL readDataFromInputState(const void* inputState, GamepadKeyCodeAndFlags cf, float32_t defaultVal) const override;
		virtual float32_t AE_CALL readDpadDataFromInputState(const void* inputState) const override;
		virtual DeviceState::CountType AE_CALL customGetState(DeviceStateType type, DeviceState::CodeType code, void* values, DeviceState::CountType count,
			const void* inputState, void* custom, ReadWriteStateStartCallback readStateStartCallback, ReadWriteStateStartCallback readStateEndCallback) const override;
		virtual void AE_CALL customDispatch(const void* oldInputState, const void* newInputState, void* custom, DispatchCallback dispatchCallback) const override;

		virtual bool AE_CALL writeStateToDevice(const void* outputState) const override;
		virtual DeviceState::CountType AE_CALL customSetState(DeviceStateType type, DeviceState::CodeType code, const void* values, DeviceState::CountType count, void* outputState, void* custom,
			ReadWriteStateStartCallback writeStateStartCallback, ReadWriteStateStartCallback writeStateEndCallback) const override;

		virtual void AE_CALL setKeyMapping(GamepadKeyMapping& dst, const GamepadKeyMapping* src) const override;

	private:
		enum class InputOffset : uint8_t {
			LX = 0,
			LY,
			RX,
			RY,
			D_PAD = 4,
			SQUARE = 4,
			CROSS = 4,
			CIRCLE = 4,
			TRIANGLE = 4,
			L1 = 5,
			R1 = 5,
			L2 = 5,
			R2 = 5,
			SHARE = 5,
			OPTIONS = 5,
			L3 = 5,
			R3 = 5,
			PS = 6,
			TOUTCH_PAD = 6,
			COUNTER = 6,
			L_TRIGGER = 7,
			R_TRIGGER,
			BATTERY_LEVEL = 11,
			GYRO_X = 12,
			GYRO_Y = 14,
			GYRO_Z = 16,
			ACCEL_X = 18,
			ACCEL_Y = 20,
			ACCEL_Z = 22,
			//BATTERY_LEVEL = 29,
			TOUCHES = 32,
			TOUCH_PACKET_COUNTER = 33,
			FINGER1 = 34,
			FINGER2 = 38,
		};


		enum class InputMask : uint8_t {
			NONE = 0,

			SQUARE = 0b10000,
			CROSS = 0b100000,
			CIRCLE = 0b1000000,
			TRIANGLE = 0b10000000,

			L1 = 0b1,
			R1 = 0b10,
			L2 = 0b100,
			R2 = 0b1000,
			SHARE = 0b10000,
			OPTIONS = 0b100000,
			L3 = 0b1000000,
			R3 = 0b10000000,

			PS = 0b1,
			TOUTCH_PAD_CLICK = 0b10
		};


		template<size_t N>
		class TouchCollection {
		public:
			TouchCollection(DeviceTouchStateValue* touches) :
				_touches(touches) {
				auto idsLen = 0;
				for (size_t i = 0; i < N; ++i) {
					auto& touch = touches[i];
					if (touch.fingerID == 0) continue;

					auto found = false;
					for (size_t j = 0; j < idsLen; ++j) {
						if (_ids[j] == touch.fingerID) {
							_idIndices[i] = j;
							found = true;

							break;
						}
					}

					if (!found) {
						_idIndices[i] = idsLen;
						_ids[idsLen++] = touch.fingerID;
					}
				}

				memset(_map, 0, sizeof(_map));
				for (size_t i = 0; i < N; ++i) {
					auto& touch = touches[i];
					if (touch.fingerID != 0) {
						auto idIdx = _idIndices[i];

						for (size_t j = 0, n = N >> 1; j < n; ++j) {
							if (_map[idIdx][j] == 0) {
								_map[idIdx][j] = i + 1;

								break;
							}
						}
					}
				}
			}

			template<std::invocable<const uint8_t*, size_t> Fn>
			void AE_CALL query(Fn&& fn) const {
				size_t count;
				uint8_t states[N];

				for (size_t i = 0; i < N; ++i) {
					count = 0;
					for (size_t j = 0, n = N >> 1; j < n; ++j) {
						auto idx = _map[i][j];
						if (idx == 0) break;
						
						states[count++] = idx - 1;
					}

					if (count) fn(states, count);
				}
			}

		private:
			const DeviceTouchStateValue* _touches;

			uint16_t _ids[N];

			uint8_t _idIndices[N];

			uint8_t _map[N][N >> 1];
		};


		static constexpr size_t INPUT_BUFFER_LENGTH = 42;
		static constexpr size_t OUTPUT_BUFFER_LENGTH = 11;
		static constexpr size_t MAX_AXES = 6;
		static constexpr size_t MAX_BUTTONS = 14;
		static constexpr auto MAX_AXIS_KEY = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::AXIS_1 + (MAX_AXES - 1));
		static constexpr auto MAX_BUTTON_KEY = (GamepadKeyCode)((std::underlying_type_t<GamepadKeyCode>)GamepadKeyCode::BUTTON_1 + (MAX_BUTTONS - 1));
		static constexpr uint8_t BUTTON_OFFSET[] = {
			(uint8_t)InputOffset::SQUARE, (uint8_t)InputOffset::CROSS, (uint8_t)InputOffset::CIRCLE, (uint8_t)InputOffset::TRIANGLE,
			(uint8_t)InputOffset::L1, (uint8_t)InputOffset::R1,
			(uint8_t)InputOffset::L2, (uint8_t)InputOffset::R2,
			(uint8_t)InputOffset::SHARE, (uint8_t)InputOffset::OPTIONS,
			(uint8_t)InputOffset::L3, (uint8_t)InputOffset::R3,
			(uint8_t)InputOffset::PS, (uint8_t)InputOffset::TOUTCH_PAD
		};
		static constexpr uint16_t BUTTON_MASK[] = {
			(uint8_t)InputMask::SQUARE, (uint8_t)InputMask::CROSS, (uint8_t)InputMask::CIRCLE, (uint8_t)InputMask::TRIANGLE,
			(uint8_t)InputMask::L1, (uint8_t)InputMask::R1,
			(uint8_t)InputMask::L2, (uint8_t)InputMask::R2,
			(uint8_t)InputMask::SHARE, (uint8_t)InputMask::OPTIONS,
			(uint8_t)InputMask::L3, (uint8_t)InputMask::R3,
			(uint8_t)InputMask::PS, (uint8_t)InputMask::TOUTCH_PAD_CLICK
		};

		inline static constexpr DeviceStateValue TOUCH_PAD_RESOLUTION_X = 1920;
		inline static constexpr DeviceStateValue TOUCH_PAD_RESOLUTION_Y = 943;
		inline static constexpr DeviceStateValue TOUCH_PAD_MAX_X = TOUCH_PAD_RESOLUTION_X - Math::ONE<DeviceStateValue>;
		inline static constexpr DeviceStateValue TOUCH_PAD_MAX_Y = TOUCH_PAD_RESOLUTION_Y - Math::ONE<DeviceStateValue>;


		mutable std::atomic_uint8_t _state;

		inline uint8_t AE_CALL _getOutputOffset(uint8_t state) const {
			return _state >> 4 & 0b1111;
		}
		void AE_CALL _writeOutputStateInit(void* outputState, uint8_t offset) const;
		static void AE_CALL _parseTouches(const uint8_t* data, DeviceTouchStateValue* states);
	};
}