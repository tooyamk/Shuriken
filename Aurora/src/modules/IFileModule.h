#pragma once

#include "modules/IModule.h"

namespace aurora::modules {
	class AE_DLL IFileModule : public IModule {
	public:
		IFileModule();
		virtual ~IFileModule();

		virtual ui32 AE_CALL getType() const override {
			return ModuleType::INPUT;
		}

		virtual void execute(const Args* args) = 0;
	};
}