#pragma once

#include "aurora/Global.h"
#include <vector>

namespace aurora::modules::graphics {
	class AE_FW_DLL GraphicsAdapter {
	public:
		GraphicsAdapter();
		GraphicsAdapter(GraphicsAdapter&& other) noexcept;

		uint32_t vendorId;
		uint32_t deviceId;
		uint64_t dedicatedSystemMemory;
		uint64_t dedicatedVideoMemory;
		uint64_t sharedSystemMemory;
		std::string description;

		GraphicsAdapter& AE_CALL operator=(GraphicsAdapter&& other) noexcept;

		static void AE_CALL query(std::vector<GraphicsAdapter>& dst);
		static GraphicsAdapter* AE_CALL autoChoose(std::vector<GraphicsAdapter>& adapters);
		static void AE_CALL autoSort(const std::vector<GraphicsAdapter>& adapters, std::vector<uint32_t>& dst);

	private:
		static float64_t AE_CALL _calcScore(const GraphicsAdapter& adapter);
	};
}