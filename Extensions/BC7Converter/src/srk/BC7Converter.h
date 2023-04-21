#pragma once

#include "srk/Image.h"
#include <functional>
#include <future>

#ifdef SRK_EXT_BC7_CONV_EXPORTS
#	define SRK_EXT_BC7_CONV_DLL SRK_DLL_EXPORT
#else
#	define SRK_EXT_BC7_CONV_DLL SRK_DLL_IMPORT
#endif

namespace srk::extensions {
	class SRK_EXT_BC7_CONV_DLL BC7Converter {
	public:
		static constexpr size_t DDS_HEADER_SIZE = 148;
		static constexpr uint32_t DDS_HEADER_MAGIC_ID = 0x20534444;

		using Job = std::function<std::shared_future<void>(const std::function<void()>&)>;

		enum class Flags : uint8_t {
			NONE = 0,
			PERCEPTUAL = 1 << 0,

			WRITE_DDS_HEADER = 1 << 7
		};

		static bool SRK_CALL encode(const Image& img, uint32_t uberLevel, uint32_t maxPartitionsToScan, Flags flags, size_t threadCount, void** outBuffer, size_t& outBufferSize, const Job& job = nullptr);

		static ByteArray SRK_CALL encode(const Image& img, uint32_t uberLevel, uint32_t maxPartitionsToScan, Flags flags, size_t threadCount, const Job& job = nullptr);
	};
}