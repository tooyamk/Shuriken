#include "RenderTag.h"

namespace aurora::render {
	RenderTag::RenderTag() :
		_value(0) {
	}

	RenderTag::RenderTag(const std::string_view& tag) :
		_value(_gen(tag.data(), tag.size())) {
	}

	RenderTag::RenderTag(const RenderTag& tag) :
		_value(tag._value) {
	}

	uint64_t RenderTag::_gen(const void* data, size_t size) {
		uint64_t val;
		auto p = (uint32_t*)&val;
		*p = hash::CRC::calc<32>(data, size, 0xFFFFFFFFU, 0xFFFFFFFFU, false, false, _crcTable1);
		*(p + 1) = hash::CRC::calc<32>(data, size, 0x00000000U, 0x00000000U, false, false, _crcTable2);
		return val;
	}
}