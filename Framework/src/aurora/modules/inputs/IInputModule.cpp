#include "IInputModule.h"
#include "aurora/events/IEventDispatcher.h"

namespace aurora::modules::inputs {
	IInputDevice::~IInputDevice() {
	}


	IInputModule::IInputModule() {
	}

	IInputModule::~IInputModule() {
	}


	void IInputDevice::getStates(DeviceStateType type, DeviceState* states, size_t count) const {
		for (size_t i = 0; i < count; ++i) {
			auto& state = states[i];
			state.count = getState(type, state.code, state.values, state.count);
		}
	}

	void IInputDevice::setStates(DeviceStateType type, DeviceState* states, size_t count) {
		for (size_t i = 0; i < count; ++i) {
			auto& state = states[i];
			state.count = setState(type, state.code, state.values, state.count);
		}
	}
}