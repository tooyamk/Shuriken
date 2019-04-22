#include "PNG.h"

namespace aurora::file {
	Image PNG::parse(ByteArray& source) {
		ByteArray::Endian srcEndian = source.getEndian();
		ui32 srcPos = source.getPosition();
		source.setEndian(ByteArray::Endian::LITTLE);
		source.setPosition(0);

		if (source.readUInt64() == 0x89504E470D0A1A0Aui64) {
		}

		return std::move(Image());
	}
}