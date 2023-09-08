#pragma once

#include "srk/Image.h"
#include "srk/modules/graphics/GraphicsModule.h"
#include "jpeglib.h"

namespace srk::extensions::jpeg_converter {
	struct Err {
		jpeg_error_mgr errMsg;
		bool isErr;
	};

	inline void errorExit(j_common_ptr cinfo) {
		auto err = (Err*)cinfo->err;
		err->isErr = true;
	}

	inline Image* SRK_CALL decode(const ByteArray& source) {
		auto img = new Image();

		Err err;
		err.isErr = false;
		jpeg_decompress_struct cinfo;
		cinfo.err = jpeg_std_error(&err.errMsg);
		err.errMsg.error_exit = errorExit;

		jpeg_create_decompress(&cinfo);

		jpeg_mem_src(&cinfo, source.getSource(), source.getLength());
		if (err.isErr) {
			jpeg_destroy_decompress(&cinfo);
			return img;
		}

		jpeg_read_header(&cinfo, true);
		if (err.isErr) {
			jpeg_destroy_decompress(&cinfo);
			return img;
		}

		jpeg_start_decompress(&cinfo);
		if (err.isErr) {
			jpeg_destroy_decompress(&cinfo);
			return img;
		}

		auto w = cinfo.output_width;
		auto h = cinfo.output_height;
		auto c = cinfo.output_components;
		auto stride = w * c;
		auto bufLen = stride * h;
		ByteArray buf(bufLen, bufLen);
		auto outBuf = buf.getSource();
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines(&cinfo, &outBuf, 1);
			outBuf += stride;
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		img->dimensions.set(w, h);
		switch (c) {
		case 3:
			img->format = modules::graphics::TextureFormat::R8G8B8_TYPELESS;
			break;
		default:
			img->format = modules::graphics::TextureFormat::UNKNOWN;
			break;
		}
		img->source = std::move(buf);

		return img;
	}

	inline ByteArray SRK_CALL encode(const Image& img, uint32_t quality) {
		ByteArray out;

		if (img.format != modules::graphics::TextureFormat::R8G8B8_UNORM && img.format != modules::graphics::TextureFormat::R8G8B8_UNORM_SRGB) return out;

		jpeg_compress_struct cinfo;
		jpeg_error_mgr err;

		cinfo.err = jpeg_std_error(&err);
		jpeg_create_compress(&cinfo);

		unsigned char* buf = nullptr;
		unsigned long bufSize = 0;
		jpeg_mem_dest(&cinfo, &buf, &bufSize);

		cinfo.image_width = img.dimensions[0];
		cinfo.image_height = img.dimensions[1];
		cinfo.input_components = 3;
		cinfo.in_color_space = JCS_RGB;

		jpeg_set_defaults(&cinfo);
		jpeg_set_quality(&cinfo, quality, true);
		jpeg_start_compress(&cinfo, true);

		auto stride = img.dimensions[0] * 3;
		while (cinfo.next_scanline < cinfo.image_height) {
			auto rowPointer = (JSAMPROW*)(img.source.getSource() + cinfo.next_scanline * stride);
			jpeg_write_scanlines(&cinfo, rowPointer, 1);
		}

		jpeg_finish_compress(&cinfo);
		jpeg_destroy_compress(&cinfo);

		out = ByteArray(buf, bufSize, bufSize, ByteArray::Usage::EXCLUSIVE);

		return out;
	}
}