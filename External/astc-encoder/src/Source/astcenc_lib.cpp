#include "astcenccli_toplevel.cpp"
#include "astcenc_lib.h"

///*
int32_t astcenc_lib::encode(const std::string& cmd, const std::string& blocksize, const std::string& preset, const std::vector<std::string>& options, const astcenc_image& image_uncomp_in, uint8_t*& outData, size_t& outSize) {
	//double start_time = get_time();

#ifdef DEBUG_CAPTURE_NAN
	feenableexcept(FE_DIVBYZERO | FE_INVALID);
#endif

	//argc = 7;
	//char* zzz[] = { "", "-cl", "D:/Users/Sephiroth/Desktop/zzz.png", "D:/Users/Sephiroth/Desktop/zzz.astc", "4x4", "-medium", "-yflip" };
	//argv = zzz;

	//if (argc < 2) {
	//	astcenc_print_shorthelp();
	//	return 0;
	//}

	std::vector<char*> argv(options.size() + 6);
	argv[1] = (char*)cmd.data();
	argv[4] = (char*)blocksize.data();
	argv[5] = (char*)preset.data();
	for (size_t i = 0, n = options.size(); i < n; ++i) argv[i + 6] = (char*)options[i].data();

	astcenc_operation operation;
	astcenc_profile profile;
	int error = parse_commandline_options(argv.size(), argv.data(), operation, profile);
	if (error) {
		return 1;
	}

	//switch (operation) {
	//case ASTCENC_OP_HELP:
	//	astcenc_print_longhelp();
	//	return 0;
	//case ASTCENC_OP_VERSION:
	//	astcenc_print_header();
	//	return 0;
	//default:
	//	break;
	//}


	//std::string input_filename = argc >= 3 ? argv[2] : "";
	//std::string output_filename = argc >= 4 ? argv[3] : "";

	//if (input_filename.empty()) {
	//	printf("ERROR: Input file not specified\n");
	//	return 1;
	//}

	//if (output_filename.empty()) {
	//	printf("ERROR: Output file not specified\n");
	//	return 1;
	//}

	// TODO: Handle RAII resources so they get freed when out of scope
	// Load the compressed input file if needed

	// This has to come first, as the block size is in the file header
	astc_compressed_image image_comp{};
	//if (operation & ASTCENC_STAGE_LD_COMP) {
	//	if (ends_with(input_filename, ".astc")) {
	//		// TODO: Just pass on a std::string
	//		error = load_cimage(input_filename.c_str(), image_comp);
	//		if (error) {
	//			return 1;
	//		}
	//	} else if (ends_with(input_filename, ".ktx")) {
	//		// TODO: Just pass on a std::string
	//		bool is_srgb;
	//		error = load_ktx_compressed_image(input_filename.c_str(), is_srgb, image_comp);
	//		if (error) {
	//			return 1;
	//		}

	//		if (is_srgb && (profile != ASTCENC_PRF_LDR_SRGB)) {
	//			printf("WARNING: Input file is sRGB, but decompressing as linear\n");
	//		}

	//		if (!is_srgb && (profile == ASTCENC_PRF_LDR_SRGB)) {
	//			printf("WARNING: Input file is linear, but decompressing as sRGB\n");
	//		}
	//	} else {
	//		printf("ERROR: Unknown compressed input file type\n");
	//	}
	//}

	astcenc_config config{};
	astcenc_preprocess preprocess;
	error = init_astcenc_config(argv.size(), argv.data(), profile, operation, image_comp, preprocess, config);
	if (error) {
		return 1;
	}

	// Initialize cli_config_options with default values
	cli_config_options cli_config{ 0, 1, false, false, -10, 10,
		{ ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A },
		{ ASTCENC_SWZ_R, ASTCENC_SWZ_G, ASTCENC_SWZ_B, ASTCENC_SWZ_A } };

	error = edit_astcenc_config(argv.size(), argv.data(), operation, cli_config, config);
	if (error) {
		return 1;
	}

	//astcenc_image* image_uncomp_in = nullptr;
	//unsigned int image_uncomp_in_num_chan = 0;
	//bool image_uncomp_in_is_hdr = false;
	//astcenc_image* image_decomp_out = nullptr;

	// TODO: Handle RAII resources so they get freed when out of scope
	astcenc_error    codec_status;
	astcenc_context* codec_context;

	codec_status = astcenc_context_alloc(config, cli_config.thread_count, &codec_context);
	if (codec_status != ASTCENC_SUCCESS) {
		printf("ERROR: Codec context alloc failed: %s\n", astcenc_get_error_string(codec_status));
		return 1;
	}

	// Load the uncompressed input file if needed
	//if (operation & ASTCENC_STAGE_LD_NCOMP) {
	//	image_uncomp_in = load_uncomp_file(input_filename.c_str(), cli_config.array_size,
	//		cli_config.y_flip, image_uncomp_in_is_hdr, image_uncomp_in_num_chan);
	//	if (!image_uncomp_in) {
	//		printf("ERROR: Failed to load uncompressed image file\n");
	//		return 1;
	//	}


	//	if (preprocess != ASTCENC_PP_NONE) {
	//		// Allocate a float image so we can avoid additional quantization,
	//		// as e.g. premultiplication can result in fractional color values
	//		astcenc_image* image_pp = alloc_image(32,
	//			image_uncomp_in->dim_x,
	//			image_uncomp_in->dim_y,
	//			image_uncomp_in->dim_z);
	//		if (!image_pp) {
	//			printf("ERROR: Failed to allocate preprocessed image\n");
	//			return 1;
	//		}

	//		if (preprocess == ASTCENC_PP_NORMALIZE) {
	//			image_preprocess_normalize(*image_uncomp_in, *image_pp);
	//		}

	//		if (preprocess == ASTCENC_PP_PREMULTIPLY) {
	//			image_preprocess_premultiply(*image_uncomp_in, *image_pp,
	//				config.profile);
	//		}

	//		// Delete the original as we no longer need it
	//		free_image(image_uncomp_in);
	//		image_uncomp_in = image_pp;
	//	}

	//	if (!cli_config.silentmode) {
	//		printf("Source image\n");
	//		printf("============\n\n");
	//		printf("    Source:                     %s\n", input_filename.c_str());
	//		printf("    Color profile:              %s\n", image_uncomp_in_is_hdr ? "HDR" : "LDR");
	//		if (image_uncomp_in->dim_z > 1) {
	//			printf("    Dimensions:                 3D, %ux%ux%u\n",
	//				image_uncomp_in->dim_x, image_uncomp_in->dim_y, image_uncomp_in->dim_z);
	//		} else {
	//			printf("    Dimensions:                 2D, %ux%u\n",
	//				image_uncomp_in->dim_x, image_uncomp_in->dim_y);
	//		}
	//		printf("    Channels:                   %d\n\n", image_uncomp_in_num_chan);
	//	}
	//}

	//double start_coding_time = get_time();

	double image_size = 0.0;
	//if (image_uncomp_in) {
		image_size = (double)image_uncomp_in.dim_x *
			(double)image_uncomp_in.dim_y *
			(double)image_uncomp_in.dim_z;
	//} else {
	//	image_size = (double)image_comp.dim_x *
	//		(double)image_comp.dim_y *
	//		(double)image_comp.dim_z;
	//}

	// Compress an image
	if (operation & ASTCENC_STAGE_COMPRESS) {
		constexpr uint32_t HEADER_LEN = 16;

		unsigned int blocks_x = (image_uncomp_in.dim_x + config.block_x - 1) / config.block_x;
		unsigned int blocks_y = (image_uncomp_in.dim_y + config.block_y - 1) / config.block_y;
		unsigned int blocks_z = (image_uncomp_in.dim_z + config.block_z - 1) / config.block_z;
		size_t buffer_size = blocks_x * blocks_y * blocks_z * 16;
		uint8_t* buffer = new uint8_t[buffer_size + HEADER_LEN];
		{
			outData = buffer;
			outSize = buffer_size + HEADER_LEN;

			const uint32_t ASTC_MAGIC_ID = 0x5CA1AB13;

			buffer[0] = ASTC_MAGIC_ID & 0xFF;
			buffer[1] = (ASTC_MAGIC_ID >> 8) & 0xFF;
			buffer[2] = (ASTC_MAGIC_ID >> 16) & 0xFF;
			buffer[3] = (ASTC_MAGIC_ID >> 24) & 0xFF;

			buffer[4] = config.block_x;
			buffer[5] = config.block_y;
			buffer[6] = config.block_z;

			buffer[7] = image_uncomp_in.dim_x & 0xFF;
			buffer[8] = (image_uncomp_in.dim_x >> 8) & 0xFF;
			buffer[9] = (image_uncomp_in.dim_x >> 16) & 0xFF;

			buffer[10] = image_uncomp_in.dim_y & 0xFF;
			buffer[11] = (image_uncomp_in.dim_y >> 8) & 0xFF;
			buffer[12] = (image_uncomp_in.dim_y >> 16) & 0xFF;

			buffer[13] = image_uncomp_in.dim_z & 0xFF;
			buffer[14] = (image_uncomp_in.dim_z >> 8) & 0xFF;
			buffer[15] = (image_uncomp_in.dim_z >> 16) & 0xFF;
		}
		
		compression_workload work;
		work.context = codec_context;
		work.image = (decltype(work.image))&image_uncomp_in;
		work.swizzle = cli_config.swz_encode;
		work.data_out = buffer + HEADER_LEN;
		work.data_len = buffer_size;
		work.error = ASTCENC_SUCCESS;

		// Only launch worker threads for multi-threaded use - it makes basic
		// single-threaded profiling and debugging a little less convoluted
		if (cli_config.thread_count > 1) {
			launch_threads(cli_config.thread_count, compression_workload_runner, &work);
		} else {
			work.error = astcenc_compress_image(
				work.context, *work.image, work.swizzle,
				work.data_out, work.data_len, 0);
		}

		if (work.error != ASTCENC_SUCCESS) {
			printf("ERROR: Codec compress failed: %s\n", astcenc_get_error_string(work.error));
			return 1;
		}

		//image_comp.block_x = config.block_x;
		//image_comp.block_y = config.block_y;
		//image_comp.block_z = config.block_z;
		//image_comp.dim_x = image_uncomp_in.dim_x;
		//image_comp.dim_y = image_uncomp_in.dim_y;
		//image_comp.dim_z = image_uncomp_in.dim_z;
		//image_comp.data = buffer;
		//image_comp.data_len = buffer_size;
	}

	// Decompress an image
	//if (operation & ASTCENC_STAGE_DECOMPRESS) {
	//	int out_bitness = get_output_filename_enforced_bitness(output_filename.c_str());
	//	if (out_bitness == -1) {
	//		bool is_hdr = (config.profile == ASTCENC_PRF_HDR) || (config.profile == ASTCENC_PRF_HDR_RGB_LDR_A);
	//		// TODO: Make this 32 to use direct passthrough as float
	//		out_bitness = is_hdr ? 16 : 8;
	//	}

	//	image_decomp_out = alloc_image(
	//		out_bitness, image_comp.dim_x, image_comp.dim_y, image_comp.dim_z);

	//	codec_status = astcenc_decompress_image(codec_context, image_comp.data, image_comp.data_len,
	//		*image_decomp_out, cli_config.swz_decode);
	//	if (codec_status != ASTCENC_SUCCESS) {
	//		printf("ERROR: Codec decompress failed: %s\n", astcenc_get_error_string(codec_status));
	//		return 1;
	//	}
	//}

	//double end_coding_time = get_time();

	// Print metrics in comparison mode
	//if (operation & ASTCENC_STAGE_COMPARE) {
	////	compute_error_metrics(image_uncomp_in_is_hdr, image_uncomp_in_num_chan, &image_uncomp_in,
	//		image_decomp_out, cli_config.low_fstop, cli_config.high_fstop);
	//}

	// Store compressed image
//	if (operation & ASTCENC_STAGE_ST_COMP) {
//		if (ends_with(output_filename, ".astc")) {
//			error = store_cimage(image_comp, output_filename.c_str());
//			if (error) {
//				printf("ERROR: Failed to store compressed image\n");
//				return 1;
//			}
//		} else if (ends_with(output_filename, ".ktx")) {
//			bool srgb = profile == ASTCENC_PRF_LDR_SRGB;
//			error = store_ktx_compressed_image(image_comp, output_filename.c_str(), srgb);
//			if (error) {
//				printf("ERROR: Failed to store compressed image\n");
//				return 1;
//			}
//		} else {
//#if defined(_WIN32)
//			bool is_null = output_filename == "NUL" || output_filename == "nul";
//#else
//			bool is_null = output_filename == "/dev/null";
//#endif
//			if (!is_null) {
//				printf("ERROR: Unknown compressed output file type\n");
//				return 1;
//			}
//		}
//	}

	// Store decompressed image
//	if (operation & ASTCENC_STAGE_ST_NCOMP) {
//		int store_result = -1;
//		const char* format_string = "";
//
//#if defined(_WIN32)
//		bool is_null = output_filename == "NUL" || output_filename == "nul";
//#else
//		bool is_null = output_filename == "/dev/null";
//#endif
//
//		if (!is_null) {
//			store_result = store_ncimage(image_decomp_out, output_filename.c_str(),
//				&format_string, cli_config.y_flip);
//			if (store_result < 0) {
//				printf("ERROR: Failed to write output image %s\n", output_filename.c_str());
//				return 1;
//			}
//		}
//	}

	//free_image(image_uncomp_in);
	//free_image(image_decomp_out);
	astcenc_context_free(codec_context);

	//delete[] image_comp.data;

	//if ((operation & ASTCENC_STAGE_COMPARE) || (!cli_config.silentmode)) {
	//	double end_time = get_time();
	//	double tex_rate = image_size / (end_coding_time - start_coding_time);
	//	tex_rate = tex_rate / 1000000.0;

	//	printf("Performance metrics\n");
	//	printf("===================\n\n");
	//	printf("    Total time:                %8.4f s\n", end_time - start_time);
	//	printf("    Coding time:               %8.4f s\n", end_coding_time - start_coding_time);
	//	printf("    Coding rate:               %8.4f MT/s\n", tex_rate);
	//}

	return 0;
}
//*/