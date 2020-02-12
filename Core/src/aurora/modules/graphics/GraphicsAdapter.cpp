#include "GraphicsAdapter.h"
#include "aurora/String.h"

#if AE_OS == AE_OS_WIN
#include <dxgi.h>
#pragma comment(lib,"dxgi.lib")
#endif

namespace aurora::modules::graphics {
	GraphicsAdapter::GraphicsAdapter() :
		vendorId(0),
		deviceId(0),
		dedicatedSystemMemory(0),
		dedicatedVideoMemory(0),
		sharedSystemMemory(0) {
	}

	void GraphicsAdapter::query(std::vector<GraphicsAdapter>& dst) {
#if AE_OS == AE_OS_WIN
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
			ga.description = String::UnicodeToUtf8(desc.Description);

			adapter->Release();
		}

		factory->Release();
#endif
	}

	GraphicsAdapter* GraphicsAdapter::autoChoose(std::vector<GraphicsAdapter>& adapters) {
		GraphicsAdapter* p = nullptr;
		f64 highestScore = -1.;
		for (auto& adapter : adapters) {
			f64 score = _calcScore(adapter);

			if (highestScore < score) {
				highestScore = score;
				p = &adapter;
			}
		}
		return p;
	}

	void GraphicsAdapter::autoSort(const std::vector<GraphicsAdapter>& adapters, std::vector<uint32_t>& dst) {
		std::vector<f64> scores;
		dst.clear();
		for (uint32_t i = 0, n = adapters.size(); i < n; ++i) {
			scores.emplace_back(_calcScore(adapters[i]));
			dst.emplace_back(i);
		}

		std::sort(dst.begin(), dst.end(), [&scores](const uint32_t idx1, const uint32_t idx2) {
			return scores[idx1] > scores[idx2];
		});
	}

	f64 GraphicsAdapter::_calcScore(const GraphicsAdapter& adapter) {
		const auto K2G = f64(1024 * 1024 * 1024);

		f64 score = 0.;

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