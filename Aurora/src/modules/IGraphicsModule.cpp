#include "IGraphicsModule.h"
#include "base/String.h"

namespace aurora::modules::graphics {
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

	std::string ProgramSource::toHLSLShaderModeel(ProgramStage stage, const std::string& version) {
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
			sm += "6_0";
		} else {
			std::vector<std::string> vers;
			String::split(version, ".", vers);
			ui32 n = vers.size();
			for (ui32 i = 0; i < n; ++i) {
				if (i != 0) sm.push_back(L'_');
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