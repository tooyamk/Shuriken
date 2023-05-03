#pragma once

#include "srk/Framework.h"
#include <string>
#include <vector>

namespace srk::modules::graphics {
	class SRK_FW_DLL GraphicsAdapter {
	public:
		GraphicsAdapter();
		GraphicsAdapter(GraphicsAdapter&& other) noexcept;

		uint32_t vendorId;
		uint32_t deviceId;
		uint64_t dedicatedSystemMemory;
		uint64_t dedicatedVideoMemory;
		uint64_t sharedSystemMemory;
		std::string description;

		GraphicsAdapter& SRK_CALL operator=(GraphicsAdapter&& other) noexcept;

		static void SRK_CALL query(std::vector<GraphicsAdapter>& dst);
		static GraphicsAdapter* SRK_CALL autoChoose(std::vector<GraphicsAdapter>& adapters);
		static void SRK_CALL autoSort(const std::vector<GraphicsAdapter>& adapters, std::vector<uint32_t>& dst);

	private:
		static float64_t SRK_CALL _calcScore(const GraphicsAdapter& adapter);
	};
}