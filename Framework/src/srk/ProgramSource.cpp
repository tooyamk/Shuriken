#include "ProgramSource.h"
#include "srk/String.h"
#include "srk/modules/graphics/IGraphicsModule.h"

namespace srk {
	ProgramSource::ProgramSource() :
		language(ProgramLanguage::UNKNOWN),
		stage(ProgramStage::UNKNOWN),
		data() {
	}

	ProgramSource::ProgramSource(ProgramSource&& value) :
		language(value.language),
		stage(value.stage),
		entryPoint(std::move(value.entryPoint)),
		version(std::move(value.version)),
		data(std::move(value.data)) {
	}

	ProgramSource& ProgramSource::operator=(ProgramSource&& value) noexcept {
		language = value.language;
		stage = value.stage;
		entryPoint = std::move(value.entryPoint);
		version = std::move(value.version);
		data = std::move(value.data);

		return *this;
	}

	bool ProgramSource::isValid() const {
		return language != ProgramLanguage::UNKNOWN &&
			stage != ProgramStage::UNKNOWN &&
			data.isValid();
	}

	std::string_view ProgramSource::toHLSLShaderStage(ProgramStage stage) {
		using namespace std::literals;

		switch (stage) {
		case ProgramStage::VS:
			return "vs"sv;
		case ProgramStage::PS:
			return "ps"sv;
		case ProgramStage::GS:
			return "gs"sv;
		case ProgramStage::CS:
			return "cs"sv;
		case ProgramStage::HS:
			return "hs"sv;
		case ProgramStage::DS:
			return "ds"sv;
		default:
			return ""sv;
		}
	}

	std::string ProgramSource::toHLSLShaderModel(ProgramStage stage, const std::string_view& version) {
		std::string sm(toHLSLShaderStage(stage));

		sm.push_back('_');

		if (version.empty()) {
			sm += "5_0";
		} else {
			std::vector<std::string_view> vers;
			String::split(version, ".", [&vers](const std::string_view& data) {
				vers.emplace_back(data);
			});
			uint32_t n = vers.size();
			for (uint32_t i = 0; i < n; ++i) {
				if (i != 0) sm.push_back('_');
				sm += vers[i];
			}
		}

		return std::move(sm);
	}
}