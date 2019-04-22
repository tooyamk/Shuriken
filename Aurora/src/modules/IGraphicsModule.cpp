#include "IGraphicsModule.h"
#include "base/String.h"
#include "math/Vector2.h"
#include "math/Vector3.h"
#include "math/Vector4.h"

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


	IBuffer::IBuffer(IGraphicsModule& graphics) : IObject(graphics) {
	}

	IBuffer::~IBuffer() {
	}


	IVertexBuffer::IVertexBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {
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


	IIndexBuffer::IIndexBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {
	}

	IIndexBuffer::~IIndexBuffer() {
	}


	SamplerFilter::SamplerFilter() :
		operation(SamplerFilterOperation::NORMAL),
		minification(SamplerFilterMode::LINEAR),
		magnification(SamplerFilterMode::LINEAR),
		mipmap(SamplerFilterMode::LINEAR) {
	}


	SamplerAddress::SamplerAddress(SamplerAddressMode u, SamplerAddressMode v, SamplerAddressMode w) :
		u(u),
		v(v),
		w(w) {
	}


	ISampler::ISampler(IGraphicsModule& graphics) : IObject(graphics) {
	}

	ISampler::~ISampler() {
	}


	ITexture::ITexture(IGraphicsModule& graphics) : IObject(graphics) {
	}

	ITexture::~ITexture() {
	}


	ITexture2D::ITexture2D(IGraphicsModule& graphics) : ITexture(graphics) {
	}

	ITexture2D::~ITexture2D() {
	}


	IConstantBuffer::IConstantBuffer(IGraphicsModule& graphics) : IBuffer(graphics) {
	}

	IConstantBuffer::~IConstantBuffer() {
	}


	ShaderParameter::ShaderParameter(ShaderParameterUsage usage) :
		_usage(usage),
		_type(Type::DEFAULT),
		_updateId(0),
		_data(),
		_size(0),
		_exclusiveRc(0),
		_exclusiveFnTarget(nullptr),
		_exclusiveFn(nullptr){
		memset(&_data, 0, sizeof(_data));
	}

	ShaderParameter::~ShaderParameter() {
		releaseExclusiveBuffers();
		if (_type == Type::INTERNAL) {
			delete[] _data.internalData;
		} else if (_type == Type::EXTERNAL && _data.externalRef && _data.externalData) {
			((Ref*)_data.externalData)->unref();
		}
	}

	void ShaderParameter::releaseExclusiveBuffers() {
		auto t = _exclusiveFnTarget;
		auto f = _exclusiveFn;
		_exclusiveRc = 0;
		_exclusiveFnTarget = nullptr;
		_exclusiveFn = nullptr;

		if (f) f(t, *this);
	}

	void ShaderParameter::setUsage(ShaderParameterUsage usage) {
		if (_usage != usage) {
			if (usage == ShaderParameterUsage::SHARE) releaseExclusiveBuffers();
			_usage = usage;
		}
	}

	void ShaderParameter::__setExclusive(void* callTarget, EXCLUSIVE_FN callback) {
		if (_exclusiveFnTarget != callTarget || _exclusiveFn != callback) {
			releaseExclusiveBuffers();

			_exclusiveFnTarget = callTarget;
			_exclusiveFn = callback;
		}
		++_exclusiveRc;
	}

	void ShaderParameter::__releaseExclusive(void* callTarget, EXCLUSIVE_FN callback) {
		if (_exclusiveFnTarget == callTarget && _exclusiveFn == callback && !--_exclusiveRc) {
			_exclusiveFnTarget = nullptr;
			_exclusiveFn = nullptr;
		}
	}

	ShaderParameter& ShaderParameter::set(const ISampler* value) {
		set(value, sizeof(value), sizeof(value), false, true);
		return *this;
	}

	ShaderParameter& ShaderParameter::set(const ITexture* value) {
		set(value, sizeof(value), sizeof(value), false, true);
		return *this;
	}

	ShaderParameter& ShaderParameter::set(f32 value) {
		set(&value, sizeof(value), sizeof(value), true, false);
		return *this;
	}

	ShaderParameter& ShaderParameter::set(const Vector2& value) {
		set(&value, sizeof(value), sizeof(value), true, false);
		return *this;
	}

	ShaderParameter& ShaderParameter::set(const Vector3& value) {
		set(&value, sizeof(value), sizeof(value), true, false);
		return *this;
	}

	ShaderParameter& ShaderParameter::set(const Vector4& value) {
		set(&value, sizeof(value), sizeof(value), true, false);
		return *this;
	}

	ShaderParameter& ShaderParameter::set(const void* data, ui32 size, ui16 perElementSize, bool copy, bool ref) {
		switch (_type) {
		case Type::DEFAULT:
		{
			if (copy) {
				if (size <= DEFAULT_DATA_SIZE) {
					memcpy(&_data, data, size);
				} else {
					_type = Type::INTERNAL;
					_data.internalData = new i8[size];
					_data.internalSize = size;
					memcpy(&_data.internalData, data, size);
				}
			} else {
				_type = Type::EXTERNAL;
				_data.externalData = data;
				if (ref && data) ((Ref*)data)->ref();
			}

			break;
		}
		case Type::INTERNAL:
		{
			if (copy) {
				if (size <= DEFAULT_DATA_SIZE) {
					delete[] _data.internalData;
					_type = Type::DEFAULT;
					memcpy(&_data, data, size);
				} else {
					if (_data.internalSize < size) {
						delete[] _data.internalData;
						_data.internalData = new i8[size];
						_data.internalSize = size;
					}
					memcpy(&_data.internalData, data, size);
				}
			} else {
				delete[] _data.internalData;
				_type = Type::EXTERNAL;
				_data.externalData = data;
				if (ref && data) ((Ref*)data)->ref();
			}

			break;
		}
		case Type::EXTERNAL:
		{
			if (copy) {
				if (_data.externalRef && _data.externalData) ((Ref*)data)->unref();

				if (size <= DEFAULT_DATA_SIZE) {
					_type = Type::DEFAULT;
					memcpy(&_data, data, size);
				} else {
					_type = Type::INTERNAL;
					_data.internalData = new i8[size];
					_data.internalSize = size;
					memcpy(&_data.internalData, data, size);
				}
			} else {
				if (_data.externalData != data) {
					if (ref && data) ((Ref*)data)->ref();
					if (_data.externalRef && _data.externalData) ((Ref*)data)->unref();
					_data.externalData = data;
					_data.externalRef = ref;
				}
			}

			break;
		}
		default:
			break;
		}
		_size = size;
		_perElementSize = perElementSize;

		return *this;
	}


	ShaderParameterFactory::~ShaderParameterFactory() {
		clear();
	}

	ShaderParameter* ShaderParameterFactory::get(const std::string& name) const {
		auto itr = _parameters.find(name);
		return itr == _parameters.end() ? nullptr : itr->second;
	}

	ShaderParameter* ShaderParameterFactory::add(const std::string& name, ShaderParameter* buffer) {
		auto itr = _parameters.find(name);
		if (buffer) {
			if (itr == _parameters.end()) {
				buffer->ref();
				_parameters.emplace(name, buffer);
			} else if (itr->second != buffer) {
				buffer->ref();
				itr->second->unref();
				itr->second = buffer;
			}
		} else {
			if (itr != _parameters.end()) {
				itr->second->unref();
				_parameters.erase(itr);
			}
		}

		return buffer;
	}

	void ShaderParameterFactory::remove(const std::string& name) {
		auto itr = _parameters.find(name);
		if (itr != _parameters.end()) {
			itr->second->unref();
			_parameters.erase(itr);
		}
	}

	void ShaderParameterFactory::clear() {
		for (auto& itr : _parameters) itr.second->unref();
		_parameters.clear();
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