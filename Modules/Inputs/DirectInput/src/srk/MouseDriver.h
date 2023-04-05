#pragma once

#include "Base.h"
#include "srk/modules/inputs/GenericMouse.h"

namespace srk::modules::inputs::direct_input {
	class SRK_MODULE_DLL MouseDriver : public IGenericMouseDriver {
	public:
		virtual ~MouseDriver();

		static MouseDriver* SRK_CALL create(Input& input, srk_IDirectInputDevice* dev);

		virtual std::optional<bool> SRK_CALL readFromDevice(GenericMouseBuffer& buffer) const override;
		virtual void SRK_CALL close() override;

	private:
		MouseDriver(Input& input, srk_IDirectInputDevice* dev);

		IntrusivePtr<Input> _input;
		srk_IDirectInputDevice* _dev;
	};
}