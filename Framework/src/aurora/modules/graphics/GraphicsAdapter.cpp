#include "GraphicsAdapter.h"
#include "aurora/String.h"

#if AE_OS == AE_OS_WINDOWS
#	include <dxgi.h>
#	include <fstream>
#endif

namespace aurora::modules::graphics {
	GraphicsAdapter::GraphicsAdapter() :
		vendorId(0),
		deviceId(0),
		dedicatedSystemMemory(0),
		dedicatedVideoMemory(0),
		sharedSystemMemory(0) {
	}

	GraphicsAdapter::GraphicsAdapter(GraphicsAdapter&& other) noexcept :
		vendorId(other.vendorId),
		deviceId(other.deviceId),
		dedicatedSystemMemory(other.dedicatedSystemMemory),
		dedicatedVideoMemory(other.dedicatedVideoMemory),
		sharedSystemMemory(other.sharedSystemMemory),
		description(std::move(other.description)) {
	}

	GraphicsAdapter& GraphicsAdapter::operator=(GraphicsAdapter&& other) noexcept {
		vendorId = other.vendorId;
		deviceId = other.deviceId;
		dedicatedSystemMemory = other.dedicatedSystemMemory;
		dedicatedVideoMemory = other.dedicatedVideoMemory;
		sharedSystemMemory = other.sharedSystemMemory;
		description = std::move(other.description);

		return *this;
	}

	void GraphicsAdapter::query(std::vector<GraphicsAdapter>& dst) {
#if AE_OS == AE_OS_WINDOWS
		IDXGIFactory* factory = nullptr;
		if (FAILED(CreateDXGIFactory1(__uuidof(IDXGIFactory), (void**)&factory))) return;

		for (UINT i = 0;; ++i) {
			IDXGIAdapter* adapter = nullptr;
			if (factory->EnumAdapters(i, &adapter) == DXGI_ERROR_NOT_FOUND) break;

			DXGI_ADAPTER_DESC desc;
			memset(&desc, 0, sizeof(DXGI_ADAPTER_DESC));

			if (FAILED(adapter->GetDesc(&desc))) {
				adapter->Release();
				continue;
			}

			auto& ga = dst.emplace_back();
			ga.vendorId = desc.VendorId;
			ga.deviceId = desc.DeviceId;
			ga.dedicatedSystemMemory = desc.DedicatedSystemMemory;
			ga.dedicatedVideoMemory = desc.DedicatedVideoMemory;
			ga.sharedSystemMemory = desc.SharedSystemMemory;
			ga.description = String::UnicodeToUtf8<std::wstring_view, std::string>(std::wstring_view(desc.Description));

			adapter->Release();
		}

		factory->Release();
#elif AE_OS == AE_OS_LINUX
		std::filesystem::path devicesPath("/sys/bus/pci/devices");

		if (std::filesystem::exists(devicesPath) && std::filesystem::is_directory(devicesPath)) {
			std::filesystem::path className("/class");
			std::filesystem::path vendorName("/vendor");
			std::filesystem::path deviceName("/device");
			std::filesystem::path resourceName("/resource");
			std::filesystem::directory_iterator devicesItr(devicesPath);

			std::vector<char> buf;

			auto readFileToBuffer = [&buf](std::filesystem::path path, const std::filesystem::path& file) {
				buf.clear();
				path += file;
				std::ifstream stream(path, std::ios::in | std::ios::binary);
				auto good = stream.good();
				if (good) {
					stream.seekg(0, std::ios::end);
					buf.resize(stream.tellg());
					stream.seekg(0, std::ios::beg);
					stream.read(buf.data(), buf.size());
				}
				stream.close();

				return good;
			};

			for (auto& itr : devicesItr) {
				auto& dirPath = itr.path();
				if (std::filesystem::is_directory(dirPath)) {
					struct {
						uint32_t vid = 0;
						uint32_t did = 0;
						uint64_t dedicatedVideoMemory = 0;
					} info;

					if (readFileToBuffer(dirPath, className)) {
						std::string_view val;
						String::split(std::string_view(buf.data(), buf.size()), String::CharFlag::NEW_LINE, [&val](const std::string_view data) {
							val = data;
							return false;
						});

						if ((String::toNumber<uint32_t>(val.substr(2), 16) >> 16) != 3) continue;
					} else {
						continue;
					}

					if (readFileToBuffer(dirPath, vendorName)) {
						std::string_view val;
						String::split(std::string_view(buf.data(), buf.size()), String::CharFlag::NEW_LINE, [&val](const std::string_view data) {
							val = data;
							return false;
						});

						info.vid = String::toNumber<uint32_t>(val.substr(2), 16);
					} else {
						continue;
					}

					if (readFileToBuffer(dirPath, deviceName)) {
						std::string_view val;
						String::split(std::string_view(buf.data(), buf.size()), String::CharFlag::NEW_LINE, [&val](const std::string_view data) {
							val = data;
							return false;
						});

						info.did = String::toNumber<uint32_t>(val.substr(2), 16);
					} else {
						continue;
					}

					if (readFileToBuffer(dirPath, resourceName)) {
						auto sssss = buf.size();
						String::split(std::string_view(buf.data(), buf.size()), String::CharFlag::NEW_LINE, [&info](const std::string_view data) {
							std::string_view arr[3];
							size_t count = 0;
							String::split(data, String::CharFlag::WHITE_SPACE, [&arr, &count](const std::string_view data) {
								arr[count++] = data;
								return count < 3;
							});

							if (count == 3) {
								auto flags = String::toNumber<size_t>(arr[2].substr(2), 16);
								if (flags & 0x200) {//IORESOURCE_MEM
									auto prefetch = (flags & 0x2000) != 0;//IORESOURCE_PREFETCH
									auto sizealign = (flags & 0x40000) != 0;//IORESOURCE_SIZEALIGN

									if (prefetch) {
										info.dedicatedVideoMemory = String::toNumber<uint64_t>(arr[1].substr(2), 16) - String::toNumber<uint64_t>(arr[0].substr(2), 16);
										return false;
									}
								}
							}

							return true;
						});
					} else {
						continue;
					}

					auto& ga = dst.emplace_back();
					ga.vendorId = info.vid;
					ga.deviceId = info.did;
					ga.dedicatedVideoMemory = info.dedicatedVideoMemory;
				}
			}
		}
#endif
	}

	GraphicsAdapter* GraphicsAdapter::autoChoose(std::vector<GraphicsAdapter>& adapters) {
		GraphicsAdapter* p = nullptr;
		float64_t highestScore = -1.;
		for (auto& adapter : adapters) {
			float64_t score = _calcScore(adapter);

			if (highestScore < score) {
				highestScore = score;
				p = &adapter;
			}
		}
		return p;
	}

	void GraphicsAdapter::autoSort(const std::vector<GraphicsAdapter>& adapters, std::vector<uint32_t>& dst) {
		auto size = adapters.size();
		std::vector<float64_t> scores(size);
		dst.clear();
		for (decltype(size) i = 0; i < size; ++i) {
			scores[i] = _calcScore(adapters[i]);
			dst.emplace_back(i);
		}

		std::sort(dst.begin(), dst.end(), [&scores](const uint32_t idx1, const uint32_t idx2) {
			return scores[idx1] > scores[idx2];
		});
	}

	float64_t GraphicsAdapter::_calcScore(const GraphicsAdapter& adapter) {
		const auto K2G = float64_t(1024. * 1024. * 1024.);

		float64_t score = 0.;

		score += adapter.dedicatedVideoMemory / K2G * 0.2;
		score += adapter.dedicatedSystemMemory / K2G * 0.1;
		score += adapter.sharedSystemMemory / K2G * 0.075;

		switch (adapter.vendorId) {
		case 0x10DE://nvidia
			score += 1.0;
			break;
		case 0x1002://amd
			score += 1.0;
			break;
		case 0x8086://intel
			score += 0.5;
			break;
		default:
			break;
		}

		return score;
	}
}