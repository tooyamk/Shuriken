#pragma once

#include "base/LowLevel.h"
#include <vector>

namespace aurora::modules::graphics {
	class AE_DLL GraphicsAdapter {
	public:
		GraphicsAdapter();

		ui32 vendorId;
		ui32 deviceId;
		ui64 dedicatedSystemMemory;
		ui64 dedicatedVideoMemory;
		ui64 sharedSystemMemory;
		std::string description;

		static void AE_CALL query(std::vector<GraphicsAdapter>& dst);
		static GraphicsAdapter* AE_CALL autoChoose(std::vector<GraphicsAdapter>& adapters);
		static void AE_CALL autoSort(const std::vector<GraphicsAdapter>& adapters, std::vector<ui32>& dst);

	private:
		static f64 AE_CALL _calcScore(const GraphicsAdapter& adapter);
	};
}