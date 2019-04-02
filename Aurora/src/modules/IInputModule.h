#pragma once

#include "modules/IModule.h"

namespace aurora::events {
	template<typename EvtType> class IEventDispatcher;
}

namespace aurora::modules {
	enum class InputModuleEvent : ui8 {
		CONNECTED,
		DISCONNECTED
	};


	enum class InputDeviceEvent : ui8 {
		DOWN,
		UP,
		MOVE
	};


	class AE_DLL InputKey {
	public:
		ui32 code;
		ui32 count;
		f32* value;
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


	class AE_DLL IInputDevice : public Ref {
	public:
		virtual ~IInputDevice();

		virtual events::IEventDispatcher<InputDeviceEvent>& AE_CALL getEventDispatcher() = 0;
		virtual const InputDeviceInfo& AE_CALL getInfo() const = 0;
		virtual ui32 AE_CALL getKeyState(ui32 keyCode, f32* data, ui32 count) const = 0;
		virtual void AE_CALL poll(bool dispatchEvent) = 0;
	};


	class AE_DLL IInputModule : public IModule {
	public:
		IInputModule();
		virtual ~IInputModule();

		virtual ui32 AE_CALL getType() const override {
			return ModuleType::INPUT;
		}

		virtual events::IEventDispatcher<InputModuleEvent>& AE_CALL getEventDispatcher() = 0;
		virtual void AE_CALL poll() = 0;
		virtual IInputDevice* AE_CALL createDevice(const InputDeviceGUID& guid) = 0;

	protected:
	};
}