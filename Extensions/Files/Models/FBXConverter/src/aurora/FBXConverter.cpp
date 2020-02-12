#include "PNGConverter.h"
#include "png.h"
#include "modules/graphics/IGraphicsModule.h"

namespace aurora::extensions::file {
	namespace png_private {
		struct ImageSource {
			uint8_t* data;
			int32_t size;
			int32_t offset;
		};

		static void _readDataCallback(png_structp png_ptr, png_bytep data, png_size_t length) {
			auto isource = (ImageSource*)png_get_io_ptr(png_ptr);

			if ((int)(isource->offset + length) <= isource->size) {
				memcpy(data, isource->data + isource->offset, length);
				isource->offset += length;
			} else {
				png_error(png_ptr, "PNG readDataCallback failed");
			}
		}
	}

	Image* PNGConverter::parse(const ByteArray& source) {
		if (source.getLength() < 8 || png_sig_cmp((png_byte*)source.getSource(), 0, 8)) return nullptr;

		auto png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		auto info = png_create_info_struct(png);

		png_private::ImageSource is;
		is.data = (uint8_t*)source.getSource();
		is.size = source.getLength();
		is.offset = 0;
		
		png_set_read_fn(png, &is, png_private::_readDataCallback);
		png_read_info(png, info);
		auto width = png_get_image_width(png, info);
		auto height = png_get_image_height(png, info);
		auto bitDepth = png_get_bit_depth(png, info);
		auto colorType = png_get_color_type(png, info);

		if (colorType == PNG_COLOR_TYPE_PALETTE) {
			png_set_palette_to_rgb(png);
		} else if (colorType == PNG_COLOR_TYPE_GRAY && bitDepth < 8) {
			bitDepth = 8;
			png_set_expand_gray_1_2_4_to_8(png);
		}
		
		if (png_get_valid(png, info, PNG_INFO_tRNS)) png_set_tRNS_to_alpha(png);
		
		if (bitDepth == 16) {
			png_set_strip_16(png);
		} else if (bitDepth < 8) {
			png_set_packing(png);
		}

		png_read_update_info(png, info);
		bitDepth = png_get_bit_depth(png, info);
		colorType = png_get_color_type(png, info);

		auto rowBytes = png_get_rowbytes(png, info);
		auto rowData = new png_bytep[height];

		auto dataLen = rowBytes * height;
		auto data = new uint8_t[dataLen];
		
		for (uint32_t i = 0; i < height; ++i) rowData[i] = data + i * rowBytes;
		png_read_image(png, rowData);
		png_read_end(png, nullptr);

		if (rowData != nullptr) delete[] rowData;

		png_destroy_read_struct(&png, &info, 0);

		auto img = new Image();
		img->size.set(width, height);
		img->source = ByteArray(data, dataLen, ByteArray::Usage::EXCLUSIVE);

		switch (colorType) {
			/*
		case PNG_COLOR_TYPE_GRAY:
			_renderFormat = Texture2D::PixelFormat::I8;
			break;
		case PNG_COLOR_TYPE_GRAY_ALPHA:
			_renderFormat = Texture2D::PixelFormat::AI88;
			break;
			*/
		case PNG_COLOR_TYPE_RGB:
			img->format = modules::graphics::TextureFormat::R8G8B8;
			break;
		case PNG_COLOR_TYPE_RGB_ALPHA:
			img->format = modules::graphics::TextureFormat::R8G8B8A8;
			break;
		default:
			img->format = modules::graphics::TextureFormat::UNKNOWN;
			break;
		}

		return img;
	}
}