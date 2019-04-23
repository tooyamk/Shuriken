#include "PNG.h"
#include "zlib.h"
#include <vector>

namespace aurora::file {
	Image* PNG::parse(ByteArray& source) {
		ByteArray::Endian srcEndian = source.getEndian();
		ui32 srcPos = source.getPosition();
		source.setEndian(ByteArray::Endian::BIG);
		source.setPosition(0);

		ui32 width = 0, height = 0;
		ui8 bitDepth = 0;
		Color colorType;
		ui8 compressionMethod = 0;
		std::vector<ui32> idatSegments;
		ui32 dataLen = 0;
		ui32 plte = 0, trns = 0;

		if (source.readUInt64() == 0x89504E470D0A1A0Aui64) {
			bool isReadDone = false;
			while (!isReadDone && source.getBytesAvailable() >= 4) {
				auto length = source.readUInt32();
				auto chunk = (Chunk)source.readUInt32();
				auto pos = source.getPosition();

				switch (chunk) {
				case Chunk::IHDR:
				{
					width = source.readUInt32();
					height = source.readUInt32();

					//_image = new BitmapData(_width, _height, true, 0x0);
					bitDepth = source.readUInt8();
					colorType = (Color)source.readUInt8();
					compressionMethod = source.readUInt8();
					source.setPosition(source.getPosition() + 2);

					int a = 1;

					break;
				}
				case Chunk::IDAT:
				{
					idatSegments.emplace_back(pos);
					idatSegments.emplace_back(length);
					dataLen += length;

					break;
				}
				case Chunk::PLTE:
					plte = pos;
					break;
				case Chunk::TRNS:
					trns = pos;
					break;
				case Chunk::IEND:
					isReadDone = true;
					break;
				default:
					break;
				}

				source.setPosition(pos + length + 4);
			}
		}

		ByteArray compressedData;
		if (idatSegments.size() == 2) {
			compressedData = ByteArray((i8*)source.getBytes() + idatSegments[0], dataLen, ByteArray::ExtMemMode::EXT);
		} else {
			compressedData = ByteArray(dataLen);
			for (ui32 i = 0, n = idatSegments.size(); i < n; i += 2) compressedData.writeBytes(source.getBytes(), idatSegments[i], idatSegments[i + 1]);
		}

		ByteArray uncompressedData(((width * height) << 2) + height);
		do {
			uLongf size = uncompressedData.getCapacity();
			auto rst = ::uncompress((ui8*)uncompressedData.getBytes(), &size, (ui8*)compressedData.getBytes(), dataLen);
			if (rst == Z_BUF_ERROR) {
				uncompressedData.setCapacity(uncompressedData.getCapacity() << 1);
			} else {
				if (rst == Z_OK) uncompressedData.setLength(size);
				compressedData.dispose();

				break;
			}
		} while (true);
		
		Image* img = nullptr;
		if (uncompressedData.getLength() > 0) {
			switch (colorType) {
			case Color::RGB:
			{
				if (bitDepth == 8) {
					img = _parseColor<3>(width, height, uncompressedData);
					if (img) img->format = modules::graphics::TextureFormat::UNKNOWN;
				}

				break;
			}
			case Color::INDEX_RGB:
			{
				if (bitDepth == 1 || bitDepth == 2 || bitDepth == 4 || bitDepth == 8) {
					img = _parseIndexColor(width, height, bitDepth, uncompressedData, source, plte, trns);
					if (img) img->format = trns ? modules::graphics::TextureFormat::R8G8B8A8 : modules::graphics::TextureFormat::UNKNOWN;
				}

				break;
			}
			case Color::RGBA:
			{
				if (bitDepth == 8) {
					img = _parseColor<4>(width, height, uncompressedData);
					if (img) img->format = modules::graphics::TextureFormat::R8G8B8A8;
				}

				break;
			}
			default:
				break;
			}
		}

		source.setPosition(srcPos);
		source.setEndian(srcEndian);

		return img;
	}

	Image* PNG::_parseIndexColor(ui32 width, ui32 height, ui8 bitDepth, ByteArray& data, ByteArray& source, ui32 plte, ui32 trns) {
		auto img = new Image();
		img->width = width;
		img->height = height;
		ui32 area = width * height;
		img->source = ByteArray(trns ?  area << 2 : area * 3);

		ui8 bits = 0;
		ui32 bitsLength = 0;

		if (trns) {
			for (ui32 y = 0; y < height; ++y) {
				auto filter = (Filter)data.readUInt8();

				if (filter == Filter::NONE) {
					for (ui32 x = 0; x < width; ++x) {
						auto index = _readBits(data, bitDepth, bits, bitsLength);
						img->source.writeBytes(source.getBytes(), plte + index, 3);
						img->source.writeBytes(source.getBytes(), trns + index, 1);
					}
				}
			}
		} else {
			for (ui32 y = 0; y < height; ++y) {
				auto filter = (Filter)data.readUInt8();

				if (filter == Filter::NONE) {
					for (ui32 x = 0; x < width; ++x) img->source.writeBytes(source.getBytes(), plte + _readBits(data, bitDepth, bits, bitsLength), 3);
				}
			}
		}

		return img;
	}

	ui8 PNG::_paethPredictor(ui8 c0, ui8 c1, ui8 c2) {
		i16 pa = c1 - c2;
		i16 pb = c0 - c2;
		i16 pc = pa + pb;

		if (pa < 0) pa = -pa;
		if (pb < 0) pb = -pb;
		if (pc < 0) pc = -pc;

		if (pa <= pb && pa <= pc) {
			return c0;
		} else if (pb <= pc) {
			return c1;
		} else {
			return c2;
		}
	}

	ui32 PNG::_readBits(ByteArray& data, ui32 length, ui8& bits, ui32& bitsLength) {
		if (bitsLength == 0) {
			bits = data.readUInt8();
			bitsLength = 8 - length;
			return bits >> bitsLength & ((2 << (length - 1)) - 1);
		} else if (bitsLength >= length) {
			bitsLength -= length;
			return bits >> bitsLength & ((2 << (length - 1)) - 1);
		} else {
			auto temp = bits;
			bits = data.readUInt8();
			auto l = length - bitsLength;
			auto out = (ui32)(temp & ((2 << (l - 1)) - 1) << bitsLength | bits >> (8 - l) & ((2 << (l - 1)) - 1));
			bitsLength = 8 - l;
			return out;
		}
	}
}