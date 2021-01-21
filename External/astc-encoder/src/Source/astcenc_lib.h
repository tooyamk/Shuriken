#include <string>
#include <vector>

struct astcenc_image;

class astcenc_lib {
public:
	static int32_t encode(const std::string& cmd, const std::string& blocksize, const std::string& preset, const std::vector<std::string>& options, const astcenc_image& image_uncomp_in, uint8_t*& outData, size_t& outSize);
};