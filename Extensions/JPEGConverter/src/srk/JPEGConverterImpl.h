#pragma once

#include "srk/Image.h"
#include "srk/modules/graphics/IGraphicsModule.h"
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
		ByteArray buf(stride * h);
		auto outBuf = buf.getSource();
		while (cinfo.output_scanline < cinfo.output_height) {
			jpeg_read_scanlines(&cinfo, &outBuf, 1);
			outBuf += stride;
		}

		jpeg_finish_decompress(&cinfo);
		jpeg_destroy_decompress(&cinfo);

		img->size.set(w, h);
		switch (c) {
		case 3:
			img->format = modules::graphics::TextureFormat::R8G8B8;
		}
		img->source = std::move(buf);

		return img;
	}
}