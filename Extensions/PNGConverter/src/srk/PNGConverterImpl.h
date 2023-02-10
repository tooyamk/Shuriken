#pragma once

#include "srk/Image.h"
#include "srk/modules/graphics/IGraphicsModule.h"
#include "png.h"

namespace srk::extensions::png_converter {
	struct ImageSource {
		uint8_t* data;
		int32_t size;
		int32_t offset;
	};

	inline void SRK_CALL readDataCallback(png_structp png_ptr, png_bytep data, png_size_t length) {
		auto isource = (ImageSource*)png_get_io_ptr(png_ptr);

		if ((int)(isource->offset + length) <= isource->size) {
			memcpy(data, isource->data + isource->offset, length);
			isource->offset += length;
		} else {
			png_error(png_ptr, "PNG readDataCallback failed");
		}
	}

	inline Image* SRK_CALL decode(const ByteArray& source) {
		ByteArray src = source.slice();
		if (src.getLength() < 8 || png_sig_cmp((png_byte*)src.getSource(), 0, 8)) return nullptr;

		auto png = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		auto info = png_create_info_struct(png);

		ImageSource is;
		is.data = (uint8_t*)src.getSource();
		is.size = src.getLength();
		is.offset = 0;

		png_set_read_fn(png, &is, readDataCallback);
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
		img->dimensions.set(width, height);
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
			img->format = modules::graphics::TextureFormat::R8G8B8_TYPELESS;
			break;
		case PNG_COLOR_TYPE_RGBA:
			img->format = modules::graphics::TextureFormat::R8G8B8A8_TYPELESS;
			break;
		default:
			img->format = modules::graphics::TextureFormat::UNKNOWN;
			break;
		}

		return img;
	}

	inline void SRK_CALL writeDataCallback(png_structp png_ptr, png_bytep data, png_size_t length) {
		auto isource = (ByteArray*)png_get_io_ptr(png_ptr);
		isource->write<ba_vt::BYTE>(data, length);
	}

	inline ByteArray SRK_CALL encode(const Image& img) {
		ByteArray out;

		if (img.format != modules::graphics::TextureFormat::R8G8B8A8_UNORM && img.format != modules::graphics::TextureFormat::R8G8B8A8_UNORM_SRGB) return out;

		auto png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, nullptr, nullptr, nullptr);
		if (!png_ptr) return out;

		auto info_ptr = png_create_info_struct(png_ptr);
		if (!info_ptr) {
			png_destroy_write_struct(&png_ptr, nullptr);
			return out;
		}

		if (setjmp(png_jmpbuf(png_ptr))) {
			png_destroy_write_struct(&png_ptr, &info_ptr);
			return out;
		}

		png_set_write_fn(png_ptr, &out, writeDataCallback, nullptr);

		auto w = img.dimensions[0], h = img.dimensions[1];

		png_set_IHDR(png_ptr, info_ptr, w, h, 8, PNG_COLOR_TYPE_RGBA, PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

		auto palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));
		png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);

		png_write_info(png_ptr, info_ptr);

		png_set_packing(png_ptr);

		auto row_pointers = (png_bytep*)malloc(h * sizeof(png_bytep));
		if (!row_pointers) {
			png_destroy_write_struct(&png_ptr, &info_ptr);
			return out;
		}

		auto raw = img.source.getSource();
		for (uint32_t i = 0; i < h; ++i) row_pointers[i] = (png_bytep)raw + i * w * 4;
		png_write_image(png_ptr, row_pointers);
		free(row_pointers);
		row_pointers = nullptr;

		png_write_end(png_ptr, info_ptr);

		png_free(png_ptr, palette);
		palette = nullptr;

		png_destroy_write_struct(&png_ptr, &info_ptr);

		return out;
	}
}