#pragma once

#include "../BaseTester.h"

class CompressTextureTester : public BaseTester {
public:
	virtual int32_t SRK_CALL run() override {
		auto srcDir = getAppPath().parent_path().u8string();
		auto src = extensions::PNGConverter::decode(readFile(srcDir + "/Resources/tex1.png"));
		if (!src) return 0;

		src->flipY();
		auto astc = extensions::ASTCConverter::encode(*src, Vec3ui32(6, 6, 1), extensions::ASTCConverter::Profile::LDR, extensions::ASTCConverter::Preset::MEDIUM, extensions::ASTCConverter::Flags::WRITE_HEADER, 12);
		writeFile(srcDir + "/img.astc", astc);
		return 0;
	}
};