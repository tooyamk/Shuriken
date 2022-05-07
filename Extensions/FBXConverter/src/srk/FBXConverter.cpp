#include "FBXConverter.h"
#include "FBXConverterImpl.h"

namespace srk::extensions {
	FBXConverter::Result FBXConverter::decode(const ByteArray& source) {
		return fbx_converter::decode(source);
	}
}