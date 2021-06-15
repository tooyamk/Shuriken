#pragma once

#include "aurora/Debug.h"
#include "aurora/Shader.h"
#include "aurora/String.h"
#include <regex>

namespace aurora::extensions::shader_script {
	struct Block {
		AE_REF_OBJECT(Block)
	public:
		std::string_view name;
		size_t contentBegin = 0;
		std::string_view content;
		std::vector<IntrusivePtr<Block>> chindren;
	};


	struct ProgramData {
		IntrusivePtr<ProgramSource> vs;
		IntrusivePtr<ProgramSource> ps;
	};


	struct VariantShader {
		VariantShader() {}
		VariantShader(const VariantShader& value) : 
			program(value.program),
			defines(value.defines) {
		}

		ProgramData program;
		ShaderDefineCollection defines;
	};


	struct ParsedData {
		std::vector<ShaderDefine> staticDefines;
		std::vector<std::string_view> dynamicDefines;
		ProgramData mainProgram;
		std::vector<VariantShader> variantShaders;
	};


	template<typename T>
	inline bool AE_CALL parseDefineBlock(T& out, const std::string_view& content) {
		using namespace std::literals;

		auto fmtContent = String::trim(content, String::CharFlag::WHITE_SPACE);
		std::vector<std::string_view> defs;
		String::split(content, String::CharFlag::NEW_LINE, [&defs](const std::string_view& sv) {
			if (!sv.empty()) defs.emplace_back(sv);
		});
		for (auto& def : defs) {
			auto fmtDef = String::trim(def, String::CharFlag::WHITE_SPACE);
			if (fmtDef.empty()) continue;

			auto pos = String::find(fmtDef, String::CharFlag::WHITE_SPACE);

			if constexpr (std::same_as<T, std::vector<ShaderDefine>> || std::same_as<T, ShaderDefineCollection>) {
				if (pos == std::string_view::npos) {
					if constexpr (std::same_as<T, std::vector<ShaderDefine>>) {
						out.emplace_back(fmtDef, nullptr);
					} else {
						out.set(std::string(fmtDef), "");
					}
				} else {
					std::string_view k(fmtDef.data(), pos);
					auto v = String::trim(std::string_view(fmtDef.data() + pos + 1, fmtDef.size() - pos - 1), String::CharFlag::WHITE_SPACE);
					if constexpr (std::same_as<T, std::vector<ShaderDefine>>) {
						out.emplace_back(k, v);
					} else {
						out.set(std::string(k), std::string(v));
					}
				}
			} else {
				if (pos == std::string_view::npos) {
					out.emplace_back(fmtDef);
				} else {
					printdln(L"ShaderScript::parseDefines error : parse dynamic defines error, value has white space"sv);
					return false;
				}
			}
		}
		
		return true;
	}

	template<ProgramStage Stage>
	inline bool AE_CALL parseProgram(IntrusivePtr<ProgramSource>& out, const std::string_view& content) {
		out = new ProgramSource();
		out->stage = Stage;
		out->language = ProgramLanguage::HLSL;

		auto fmtContent = String::trim(content, String::CharFlag::WHITE_SPACE);
		out->data = ByteArray((uint8_t*)fmtContent.data(), fmtContent.size(), ByteArray::Usage::COPY);

		return true;
	}

	inline bool AE_CALL parseMainDefineBlock(ParsedData& rst, const Block& block) {
		for (auto& c : block.chindren) {
			auto name = String::trim(c->name, String::CharFlag::WHITE_SPACE);
			if (name == "static") {
				if (!parseDefineBlock(rst.staticDefines, c->content)) return false;
			} else if (name == "dynamic") {
				if (!parseDefineBlock(rst.dynamicDefines, c->content)) return false;
			}
		}

		return true;
	}

	inline bool AE_CALL parseProgramBlock(ProgramData& out, const Block& block) {
		for (auto& c : block.chindren) {
			auto name = String::trim(c->name, String::CharFlag::WHITE_SPACE);
			if (name == "vs") {
				if (!parseProgram<ProgramStage::VS>(out.vs, c->content)) return false;
			} else if (name == "ps") {
				if (!parseProgram<ProgramStage::PS>(out.ps, c->content)) return false;
			}
		}

		return true;
	}

	inline bool AE_CALL parseVariantBlock(VariantShader& out, const Block& block) {
		for (auto& c : block.chindren) {
			auto name = String::trim(c->name, String::CharFlag::WHITE_SPACE);
			if (name == "define") {
				if (!parseDefineBlock(out.defines, c->content)) return false;
			} else if (name == "program") {
				if (!parseProgramBlock(out.program, *c)) return false;
			}
		}

		return true;
	}

	inline bool AE_CALL parseShaderBlock(ParsedData& rst, const Block& block) {
		auto name = String::trim(block.name, String::CharFlag::WHITE_SPACE);
		if (name == "shader") {
			for (auto& c : block.chindren) {
				name = String::trim(c->name, String::CharFlag::WHITE_SPACE);
				if (name == "define") {
					if (!parseMainDefineBlock(rst, *c)) return false;
				} else if (name == "program") {
					if (!parseProgramBlock(rst.mainProgram, *c)) return false;
				} else if (name == "variant") {
					if (!parseVariantBlock(rst.variantShaders.emplace_back(), *c)) return false;
				}
			}

			return true;
		} else {
			return false;
		}
	}

	inline std::vector<IntrusivePtr<Block>> AE_CALL parseBlocks(const std::string_view& data) {
		std::vector<IntrusivePtr<Block>> blocks;

		std::vector<IntrusivePtr<Block>> stack;

		IntrusivePtr<Block> curBlock;

		size_t begin = 0;
		for (size_t i = 0, n = data.size(); i < n; ++i) {
			switch (data[i]) {
			case '{':
			{
				IntrusivePtr block = new Block();

				if (curBlock) {
					curBlock->chindren.emplace_back(block);
					stack.emplace_back(curBlock);
				} else {
					blocks.emplace_back(block);
				}
				curBlock = block;

				curBlock->name = std::string_view(data.data() + begin, i - begin);
				begin = i + 1;
				curBlock->contentBegin = begin;

				break;
			}
			case '}':
			{
				if (curBlock) {
					curBlock->content = std::string_view(data.data() + curBlock->contentBegin, i - curBlock->contentBegin);
					begin = i + 1;

					if (stack.empty()) {
						curBlock = nullptr;
					} else {
						curBlock = stack[stack.size() - 1];
						stack.pop_back();
					}
				} else {
					//error
					return std::vector<IntrusivePtr<Block>>();
				}

				break;
			}
			default:
				break;
			}
		}

		return std::move(blocks);
	}

	inline bool AE_CALL set(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const Shader::IncludeHandler& includeHandler, const Shader::InputHandler& inputHandler) {
		if (shader) {
			shader->unset();

			if (!source.getLength()) return false;

			auto src = std::regex_replace((const char*)source.getSource(), std::regex("//.*|/\\*[\\s\\S]*?\\*/|"), "");
			//for (auto& c : src) {
			//	if (c == '\r') c = '\n';
			//}
			//src = std::regex_replace(src, std::regex("\r\n"), "\n");
			//src = std::regex_replace(src, std::regex("\r"), "\n");

			auto blocks = parseBlocks(src);
			if (auto size = blocks.size(); size) {
				ParsedData data;

				--size;
				do {
					if (parseShaderBlock(data, *blocks[size])) {
						shader->set(graphics, data.mainProgram.vs, data.mainProgram.ps, data.staticDefines.data(), data.staticDefines.size(), data.dynamicDefines.data(), data.dynamicDefines.size(), includeHandler, inputHandler);
						for (auto& v : data.variantShaders) shader->setVariant(v.program.vs, v.program.ps, &v.defines);

						return true;
					} else {
						if (size) {
							data = ParsedData();
							--size;
						} else {
							return false;
						}
					}
				} while (true);
			}
		}

		return false;
	}
}