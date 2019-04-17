#include "IGraphicsModule.h"
#include "base/String.h"

#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
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
#if AE_TARGET_OS_PLATFORM == AE_OS_PLATFORM_WIN
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

	void GraphicsAdapter::autoSort(const std::vector<GraphicsAdapter>& adapters, std::vector<ui32>& dst) {
		std::vector<f64> scores;
		dst.clear();
		for (ui32 i = 0, n = adapters.size(); i < n; ++i) {
			scores.emplace_back(_calcScore(adapters[i]));
			dst.emplace_back(i);
		}

		std::sort(dst.begin(), dst.end(), [&scores](const ui32 idx1, const ui32 idx2) {
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


	IGraphicsModule::~IGraphicsModule() {
	}


	IObject::IObject(IGraphicsModule& graphics) :
		_graphics(graphics.ref<IGraphicsModule>()) {
	}

	IObject::~IObject() {
		Ref::setNull(_graphics);
	}


	IVertexBuffer::IVertexBuffer(IGraphicsModule& graphics) : IObject(graphics) {
	}

	IVertexBuffer::~IVertexBuffer() {
	}


	VertexBufferFactory::~VertexBufferFactory() {
		clear();
	}

	IVertexBuffer* VertexBufferFactory::get(const std::string& name) const {
		auto itr = _buffers.find(name);
		return itr == _buffers.end() ? nullptr : itr->second;
	}

	void VertexBufferFactory::add(const std::string& name, IVertexBuffer* buffer) {
		auto itr = _buffers.find(name);
		if (buffer) {
			if (itr == _buffers.end()) {
				buffer->ref();
				_buffers.emplace(name, buffer);
			} else if (itr->second != buffer) {
				buffer->ref();
				itr->second->unref();
				itr->second = buffer;
			}
		} else {
			if (itr != _buffers.end()) {
				itr->second->unref();
				_buffers.erase(itr);
			}
		}
	}

	void VertexBufferFactory::remove(const std::string& name) {
		auto itr = _buffers.find(name);
		if (itr != _buffers.end()) {
			itr->second->unref();
			_buffers.erase(itr);
		}
	}

	void VertexBufferFactory::clear() {
		for (auto& itr : _buffers) itr.second->unref();
		_buffers.clear();
	}


	IIndexBuffer::IIndexBuffer(IGraphicsModule& graphics) : IObject(graphics) {
	}

	IIndexBuffer::~IIndexBuffer() {
	}


	IConstantBuffer::IConstantBuffer(IGraphicsModule& graphics) : IObject(graphics) {
	}

	IConstantBuffer::~IConstantBuffer() {
	}


	ConstantFactory::~ConstantFactory() {
		clear();
	}

	Constant* ConstantFactory::get(const std::string& name) const {
		auto itr = _buffers.find(name);
		return itr == _buffers.end() ? nullptr : itr->second;
	}

	void ConstantFactory::add(const std::string& name, Constant* buffer) {
		auto itr = _buffers.find(name);
		if (buffer) {
			if (itr == _buffers.end()) {
				buffer->ref();
				_buffers.emplace(name, buffer);
			} else if (itr->second != buffer) {
				buffer->ref();
				itr->second->unref();
				itr->second = buffer;
			}
		} else {
			if (itr != _buffers.end()) {
				itr->second->unref();
				_buffers.erase(itr);
			}
		}
	}

	void ConstantFactory::remove(const std::string& name) {
		auto itr = _buffers.find(name);
		if (itr != _buffers.end()) {
			itr->second->unref();
			_buffers.erase(itr);
		}
	}

	void ConstantFactory::clear() {
		for (auto& itr : _buffers) itr.second->unref();
		_buffers.clear();
	}


	ProgramSource::ProgramSource() :
		language(ProgramLanguage::UNKNOWN),
		stage(ProgramStage::UNKNOWN),
		data() {
	}

	ProgramSource::ProgramSource(ProgramSource&& value) :
		language(value.language),
		stage(value.stage),
		version(std::move(value.version)),
		data(std::move(value.data)) {
	}

	ProgramSource& ProgramSource::operator=(ProgramSource&& value) {
		language = value.language;
		stage = value.stage;
		version = std::move(value.version);
		data = std::move(value.data);

		return *this;
	}

	bool ProgramSource::isValid() const {
		return language != ProgramLanguage::UNKNOWN &&
			stage != ProgramStage::UNKNOWN &&
			data.isValid();
	}

	std::string ProgramSource::toHLSLShaderModel(ProgramStage stage, const std::string& version) {
		std::string sm;
		switch (stage) {
		case ProgramStage::VS:
			sm = "vs";
			break;
		case ProgramStage::PS:
			sm = "ps";
			break;
		case ProgramStage::GS:
			sm = "gs";
			break;
		case ProgramStage::CS:
			sm = "cs";
			break;
		case ProgramStage::HS:
			sm = "hs";
			break;
		case ProgramStage::DS:
			sm = "ds";
			break;
		default:
			return "";
		}
		sm.push_back('_');

		if (version.empty()) {
			sm += "5_0";
		} else {
			std::vector<std::string> vers;
			String::split(version, ".", vers);
			ui32 n = vers.size();
			for (ui32 i = 0; i < n; ++i) {
				if (i != 0) sm.push_back('_');
				sm += vers[i];
			}
		}

		return std::move(sm);
	}


	IProgram::IProgram(IGraphicsModule& graphics) : IObject(graphics) {
	}

	IProgram::~IProgram() {
	}
}