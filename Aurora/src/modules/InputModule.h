#pragma once

#include "modules/Module.h"

namespace aurora::modules {
	enum class InputModuleEvent : ui8 {
		CONNECTED,
		DISCONNECTED
	};


	enum class InputDeviceEvent : ui8 {
		DOWN,
		UP
	};


	class AE_DLL InputKey {
	public:
		i32 code;
		f32 value;
		i64 timestamp;
	};


	class AE_DLL InputDeviceType {
	public:
		InputDeviceType() = delete;
		InputDeviceType(const InputDeviceType&) = delete;
		InputDeviceType(InputDeviceType&&) = delete;

		static const ui32 KEYBOARD = 0b1;
		static const ui32 MOUSE = 0b10;
		static const ui32 GAMEPAD = 0b100;
	};


	class AE_DLL InputDeviceGUID {
	public:
		InputDeviceGUID();
		InputDeviceGUID(const InputDeviceGUID& value);
		InputDeviceGUID(InputDeviceGUID&& value);
		~InputDeviceGUID();

		inline const ui8* AE_CALL getData() const {
			return _data;
		}

		inline ui32 AE_CALL getDataLen() const {
			return _len;
		}

		void AE_CALL set(ui8* data, ui32 len);
		bool AE_CALL isEqual(ui8* data, ui32 len) const;

		InputDeviceGUID& operator=(const InputDeviceGUID& value);
		InputDeviceGUID& operator=(InputDeviceGUID&& value);
		bool operator==(const InputDeviceGUID& right) const;

	private:
		ui8* _data;
		ui32 _len;
	};


	class AE_DLL InputDeviceInfo {
	public:
		InputDeviceInfo();
		InputDeviceInfo(const InputDeviceGUID& guid, ui32 type);
		InputDeviceInfo(const InputDeviceInfo& value);
		InputDeviceInfo(InputDeviceInfo&& value);

		InputDeviceGUID guid;
		ui32 type;

		InputDeviceInfo& operator=(const InputDeviceInfo& value);
		InputDeviceInfo& operator=(InputDeviceInfo&& value);
	};


	class AE_DLL InputDevice {
	public:
		virtual ~InputDevice();

		virtual const InputDeviceGUID& AE_CALL getGUID() const = 0;
		virtual ui32 AE_CALL getType() const = 0;
	};


	class AE_DLL InputModule : public Module<InputModuleEvent> {
	public:
		InputModule();
		virtual ~InputModule();

		virtual ui32 AE_CALL getType() const override {
			return ModuleType::INPUT;
		}

		virtual events::IEventDispatcher<InputModuleEvent>& AE_CALL getEventDispatcher() = 0;
		virtual void AE_CALL poll() = 0;
		virtual InputDevice* AE_CALL createDevice(const InputDeviceGUID& guid) const = 0;

	protected:
	};
}