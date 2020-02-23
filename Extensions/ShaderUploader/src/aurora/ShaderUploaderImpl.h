#pragma once

#include "aurora/Shader.h"
#include "aurora/String.h"
#include <regex>

namespace aurora::extensions::shader_uploader {
	inline constexpr bool VALID_CHARS[] = {
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //  0-  9
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 10- 19
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 20- 29
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 30- 39
		0, 0, 0, 0, 0, 0, 0, 0, 1, 1,  // 40- 49 0 1
		1, 1, 1, 1, 1, 1, 1, 1, 0, 0,  // 50- 59 2-9
		0, 0, 0, 0, 0, 1, 1, 1, 1, 1,  // 60- 69 A-E
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 70- 79 F-O
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 80- 89 P-Y
		1, 0, 0, 0, 0, 1, 0, 1, 1, 1,  // 90- 99 Z _ a-c
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //100-109 d-m
		1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  //110-119 n-w
		1, 1, 1, 0, 0, 0, 0, 0, 0, 0,  //120-129 x-z
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //130-139
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //140-149
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //150-159
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //160-169
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //170-179
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //180-189
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //190-199
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //200-209
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //210-219
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //220-229
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //230-239
		0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //240-249
		0, 0, 0, 0, 0, 0			   //250-255
	};


	struct ParsedData {
		std::vector<ShaderDefine> staticDefines;
		std::vector<std::string_view> dynamicDefines;
		RefPtr<ProgramSource> vs;
		RefPtr<ProgramSource> ps;
	};


	template<char Start, char End>
	inline std::tuple<std::string::size_type, std::string::size_type> findNest(const std::string& data, size_t begin) {
		size_t nest = 0;
		for (size_t i = begin, n = data.size(); i < n; ++i) {
			switch (data[i]) {
			case Start:
			{
				if (nest == 0) begin = i;
				++nest;

				break;
			}
			case End:
			{
				if (nest == 0) {
					//error
				} else {
					--nest;
					if (nest == 0) return std::make_tuple(begin + 1, i - begin - 1);
				}

				break;
			}
			default:
				break;
			}
		}

		return std::make_tuple(std::string::npos, 0);
	}

	template<bool IsStatic, typename T>
	inline bool AE_CALL parseDefines(std::vector<T>& out, const std::string_view& content) {
		std::vector<std::string_view> defs;
		String::split(content, String::CharFlag::NEW_LINE, defs);
		std::vector<std::string_view> kvs;
		for (auto& def : defs) {
			auto fmtDef = String::trim(def, String::CharFlag::WHITE_SPACE);
			if (fmtDef.empty()) continue;

			String::split(fmtDef, String::CharFlag::WHITE_SPACE, kvs);
			if (auto n = kvs.size(); n) {
				if constexpr (IsStatic) {
					if (n == 1) {
						out.emplace_back(kvs[0], "");
					} else if (n > 1) {
						for (size_t i = 1, nn = n - 2; i < nn; ++i) {
							if (!kvs[i].empty()) {
								println("ShaderUploader::upload error : parse static defines error");
								return false;
							}
						}

						out.emplace_back(kvs[0], kvs[n - 1]);
					}
				} else {
					if (n == 1) {
						out.emplace_back(kvs[0]);
					} else {
						println("ShaderUploader::upload error : parse dynamic defines error");
						return false;
					}
				}
			}

			kvs.clear();
		}
		
		return true;
	}

	template<ProgramStage Stage>
	inline bool AE_CALL parseProgram(RefPtr<ProgramSource>& out, const std::string_view& content) {
		out = new ProgramSource();
		out->stage = Stage;
		out->language = ProgramLanguage::HLSL;

		auto fmtContent = String::trim(content, String::CharFlag::WHITE_SPACE);
		out->data = ByteArray((uint8_t*)fmtContent.data(), fmtContent.size(), ByteArray::Usage::COPY);

		return true;
	}

	inline bool AE_CALL parseBlock(ParsedData& data, const std::string_view& name, const std::string_view& args, const std::string_view& content) {
		if (name == "define") {
			auto fmtArgs = String::trim(args, String::CharFlag::WHITE_SPACE);
			if (fmtArgs == "static") {
				if (!parseDefines<true>(data.staticDefines, content)) return false;
			} else if (fmtArgs == "dynamic") {
				if (!parseDefines<false>(data.dynamicDefines, content)) return false;
			}
		} else if (name == "program") {
			auto fmtArgs = String::trim(args, String::CharFlag::WHITE_SPACE);
			if (fmtArgs == "vs") {
				if (!parseProgram<ProgramStage::VS>(data.vs, content)) return false;
			} else if (fmtArgs == "ps") {
				if (!parseProgram<ProgramStage::PS>(data.ps, content)) return false;
			}
		}

		return true;
	}

	inline bool AE_CALL upload(Shader* shader, modules::graphics::IGraphicsModule* graphics, const ByteArray& source, const Shader::IncludeHandler& handler) {
		if (shader) {
			shader->unload();

			auto src = std::regex_replace((const char*)source.getSource(), std::regex("//.*|/\\*[\\s\\S]*?\\*/|"), "");
			//for (auto& c : src) {
			//	if (c == '\r') c = '\n';
			//}
			//src = std::regex_replace(src, std::regex("\r\n"), "\n");
			//src = std::regex_replace(src, std::regex("\r"), "\n");

			ParsedData data;

			size_t i = 0;
			do {
				if (auto [begin1, size1] = findNest<'(', ')'>(src, i); size1) {
					if (auto [begin2, size2] = findNest<'{', '}'>(src, begin1 + size1 + 1); size2) {
						if (begin1 >= 2) {
							std::string::size_type begin0 = i;
							std::string::size_type end0 = std::string::npos;
							size_t j = begin1 - 2;
							do {
								if (VALID_CHARS[src[j]]) {
									if (end0 == std::string::npos) {
										end0 = j;
									}
								} else if (end0 == std::string::npos && (String::CHARS[src[j]] & String::CharFlag::WHITE_SPACE)) {
									continue;
								} else {
									begin0 = j + 1;
									break;
								}

							} while (j--);

							if (end0 != std::string::npos) {
								if (!parseBlock(data,
									std::string_view(src.data() + begin0, end0 - begin0 + 1),
									std::string_view(src.data() + begin1, size1),
									std::string_view(src.data() + begin2, size2))) return false;
							}
						}

						i = begin2 + size2 + 1;

					} else {
						break;
					}
				} else {
					break;
				}
			} while (true);

			shader->upload(graphics, data.vs, data.ps,
				data.staticDefines.data(), data.staticDefines.size(), data.dynamicDefines.data(), data.dynamicDefines.size(), handler);
		}

		return false;
	}
}