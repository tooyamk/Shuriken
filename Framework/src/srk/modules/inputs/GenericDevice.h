#pragma once

#include "srk/modules/inputs/InputModule.h"
#include "srk/events/EventDispatcher.h"
#include <optional>
#include <shared_mutex>

namespace srk::modules::inputs {
	template<typename Driver, typename InputBuffer, typename OutputBuffer>
	class GenericDevice : public IInputDevice {
	public:
		GenericDevice(const DeviceInfo& info, Driver& driver) :
			_eventDispatcher(new events::EventDispatcher<DeviceEvent>()),
			_info(info),
			_driver(driver),
			_polling(false),
			_closeFlag(CLOSE_FLAG_OPENING),
			_curInputBuffer(nullptr),
			_prevInputBuffer(nullptr),
			_readDeviceInputBuffer(nullptr),
			_outputBufferLength(0),
			_outputBuffer(nullptr),
			_writingOutputBuffer(nullptr),
			_outputFlag(0),
			_needOutput(false) {
		}

		virtual ~GenericDevice() {
			close();
		}

		virtual IntrusivePtr<events::IEventDispatcher<DeviceEvent>> SRK_CALL getEventDispatcher() override {
			return _eventDispatcher;
		}

		virtual const DeviceInfo& SRK_CALL getInfo() const override {
			return _info;
		}

		virtual DevicePollResult SRK_CALL poll(bool dispatchEvent) override {
			using namespace srk::enum_operators;

			if (_closeFlag.load() == CLOSE_FLAG_CLOSED) return DevicePollResult::CLOSED;
			if (_polling.exchange(true, std::memory_order::acquire)) return DevicePollResult::EMPTY;

			auto rst = DevicePollResult::ACQUIRED;

			if (_doInput(dispatchEvent)) rst |= DevicePollResult::INPUT_COMPLETE;
			if (_doOutput()) rst |= DevicePollResult::OUTPUT__COMPLETE;

			_polling.store(false, std::memory_order::release);

			return rst;
		}

		virtual void SRK_CALL close() override {
			do {
				auto expected = CLOSE_FLAG_OPENING;
				if (_closeFlag.compare_exchange_strong(expected, CLOSE_FLAG_LOCKING, std::memory_order::release, std::memory_order::relaxed)) {
					_closeDevice();

					_closeFlag.store(CLOSE_FLAG_CLOSED);

					break;
				} else {
					if (expected == CLOSE_FLAG_CLOSED) break;

					std::this_thread::yield();
				}
			} while (true);
		}

	protected:
		using CLOSE_FLAG_TYPE = uint8_t;
		static constexpr CLOSE_FLAG_TYPE CLOSE_FLAG_OPENING = 0;
		static constexpr CLOSE_FLAG_TYPE CLOSE_FLAG_LOCKING = 0b1;
		static constexpr CLOSE_FLAG_TYPE CLOSE_FLAG_CLOSED = 0b10;

		using OUTPUT_FLAG_TYPE = uint8_t;
		static constexpr OUTPUT_FLAG_TYPE OUTPUT_FLAG_DIRTY = 0b1;
		static constexpr OUTPUT_FLAG_TYPE OUTPUT_FLAG_WRITING = 0b10;

		IntrusivePtr<events::IEventDispatcher<DeviceEvent>> _eventDispatcher;
		DeviceInfo _info;

		IntrusivePtr<Driver> _driver;

		std::atomic_uint8_t _closeFlag;

		std::atomic_bool _polling;

		mutable std::shared_mutex _inputMutex;
		InputBuffer* _curInputBuffer;
		InputBuffer* _prevInputBuffer;
		InputBuffer* _readDeviceInputBuffer;

		mutable std::shared_mutex _outputMutex;
		size_t _outputBufferLength;
		OutputBuffer* _outputBuffer;
		OutputBuffer* _writingOutputBuffer;
		std::atomic<OUTPUT_FLAG_TYPE> _outputFlag;
		std::atomic_bool _needOutput;

		bool SRK_CALL _doInput(bool dispatchEvent) {
			auto expected = CLOSE_FLAG_OPENING;
			auto lock = _closeFlag.compare_exchange_strong(expected, CLOSE_FLAG_LOCKING, std::memory_order::release, std::memory_order::relaxed);
			if (!lock) return false;

			auto opt = _readFromDevice();
			_closeFlag.store(CLOSE_FLAG_OPENING);

			if (!opt) return false;
			if (!(*opt)) return true;

			if (!dispatchEvent) {
				_switchInputBuffer();

				return true;
			}

			_switchInputBuffer();

			return _doInput();
		}

		bool SRK_CALL _doOutput() {
			auto expected = OUTPUT_FLAG_DIRTY;
			if (_outputFlag.compare_exchange_strong(expected, OUTPUT_FLAG_WRITING, std::memory_order::release, std::memory_order::relaxed)) {
				_needOutput.store(true);

				std::shared_lock lock(_outputMutex);

				memcpy(_writingOutputBuffer, _outputBuffer, _outputBufferLength);
			}

			if (_needOutput.load()) {
				if (_writeToDevice()) {
					_needOutput.store(false);
					_outputFlag.fetch_and(~OUTPUT_FLAG_WRITING);

					return true;
				} else {
					return false;
				}
			} else {
				return true;
			}
		}

		virtual std::optional<bool> _readFromDevice() = 0;
		virtual bool _writeToDevice() = 0;
		virtual bool SRK_CALL _doInput() = 0;
		virtual void SRK_CALL _closeDevice() = 0;

		void SRK_CALL _switchInputBuffer() {
			std::scoped_lock lock(_inputMutex);

			auto tmp = _prevInputBuffer;
			_prevInputBuffer = _curInputBuffer;
			_curInputBuffer = _readDeviceInputBuffer;
			_readDeviceInputBuffer = tmp;
		}
	};
}