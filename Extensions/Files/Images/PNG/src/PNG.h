#pragma once

#include "base/Image.h"

namespace aurora::file {
	class AE_EXTENSION_DLL PNG {
	public:
		enum class Chunk {
			IHDR = 0x49484452,
			PLTE = 0x504C5445,
			TRNS = 0x74524E53,
			IDAT = 0x49444154,
			IEND = 0x49454E44
		};

		enum class Color : ui8 {
			GRAY = 0,
			RGB = 2,
			INDEX_RGB = 3,
			ALPHA_GRAY = 4,
			RGBA = 6
		};

		enum class Filter : ui8 {
			NONE = 0,
			SUB = 1,
			UP = 2,
			AVERAGE = 3,
			PAETH = 4
		};

		static Image* AE_CALL parse(ByteArray& source);

		template<ui8 Channel>
		static Image* AE_CALL _parseColor(ui32 width, ui32 height, ByteArray& data) {
			auto img = new Image();
			img->width = width;
			img->height = height;
			img->source = ByteArray((width * height) * Channel);
			ui8 c0[Channel], c1[Channel];

			for (ui32 y = 0; y < height; ++y) {
				auto filter = (Filter)data.readUInt8();

				switch (filter) {
				case Filter::NONE:
				{
					ui32 size = width * Channel;
					img->source.writeBytes(data.getBytes(), data.getPosition(), size);
					data.setPosition(data.getPosition() + size);

					break;
				}
				case Filter::SUB:
				{
					data.readBytes((i8*)c0, 0, Channel);
					img->source.writeBytes((i8*)c0, 0, Channel);

					for (ui32 x = 1; x < width; ++x) {
						for (ui8 i = 0; i < Channel; ++i) c0[i] = (c0[i] + data.readUInt8()) & 0xFF;
						img->source.writeBytes((i8*)c0, 0, Channel);
					}

					break;
				}
				case Filter::UP:
				{
					ui32 base = (y - 1) * width;
					for (ui32 x = 0; x < width; ++x) {
						data.readBytes((i8*)c0, 0, Channel);
						auto c = img->source.getBytes() + (ui32)((base + x) * Channel);
						for (ui8 i = 0; i < Channel; ++i) c0[i] = (c0[i] + c[i]) & 0xFF;
						img->source.writeBytes((i8*)c0, 0, Channel);
					}

					break;
				}
				case Filter::AVERAGE:
				{
					data.readBytes((i8*)c0, 0, Channel);
					ui32 base = (y - 1) * width;
					auto c = img->source.getBytes() + (ui32)(base * Channel);
					for (ui8 i = 0; i < Channel; ++i) c0[i] = (c0[i] + (c[i] >> 1)) & 0xFF;
					img->source.writeBytes((i8*)c0, 0, Channel);

					for (ui32 x = 1; x < width; ++x) {
						data.readBytes((i8*)c1, 0, Channel);
						c = img->source.getBytes() + (ui32)((base + x) * Channel);
						for (ui8 i = 0; i < Channel; ++i) c0[i] = (c1[i] + ((c[i] + c0[i]) >> 1)) & 0xFF;
						img->source.writeBytes((i8*)c0, 0, Channel);
					}

					break;
				}
				case Filter::PAETH:
				{
					data.readBytes((i8*)c0, 0, Channel);
					ui32 base = (y - 1) * width;
					auto c2 = img->source.getBytes() + (ui32)(base * Channel);
					for (ui8 i = 0; i < Channel; ++i) c0[i] = (c0[i] + c2[i]) & 0xFF;
					img->source.writeBytes((i8*)c0, 0, Channel);

					for (ui32 x = 1; x < width; ++x) {
						data.readBytes((i8*)c1, 0, Channel);
						auto c3 = img->source.getBytes() + (ui32)((base + x) * Channel);
						for (ui8 i = 0; i < Channel; ++i) c0[i] = (c1[i] + _paethPredictor(c0[i], c3[i], c2[i])) & 0xFF;
						img->source.writeBytes((i8*)c0, 0, Channel);
						c2 = c3;
					}

					break;
				}
				default:
					data.setPosition(data.getPosition() + width * Channel);
					break;
				}
			}

			return img;
		}

		static Image* AE_CALL _parseIndexColor(ui32 width, ui32 height, ui8 bitDepth, ByteArray& data, ByteArray& source, ui32 plte, ui32 trns);

		/**
		 * c0 = left, c1 = above, c2 = upper left.
		 */
		static ui8 AE_CALL _paethPredictor(ui8 c0, ui8 c1, ui8 c2);
		static ui32 AE_CALL _readBits(ByteArray& data, ui32 length, ui8& bits, ui32& bitsLength);
	};
}