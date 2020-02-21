#include "FBXConverter.h"
#include "FBXConverterImpl.h"

namespace aurora::extensions {
	FBXConverter::Result FBXConverter::parse(const ByteArray& source) {
		return fbx_converter::parse(source);
	}
}