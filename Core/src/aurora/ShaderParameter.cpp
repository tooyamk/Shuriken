#include "ShaderParameter.h"

namespace aurora {
	ShaderParameter::ShaderParameter(ShaderParameterUsage usage) :
		_usage(usage),
		_type(ShaderParameterType::DATA),
		_storageType(StorageType::DEFAULT),
		_updateId(0),
		_data(),
		_size(0) {
		memset(&_data, 0, sizeof(_data));
	}

	ShaderParameter::~ShaderParameter() {
		releaseExclusiveBuffers();

		if (_storageType == StorageType::INTERNAL) {
			delete[] _data.internalData;
		} else if (_storageType == StorageType::EXTERNAL && _data.externalRef && _data.externalData) {
			((Ref*)_data.externalData)->unref();
		}
	}

	void ShaderParameter::releaseExclusiveBuffers() {
		if (_handlers.callback) {
			auto head = _handlers;
			_handlers.callback = nullptr;
			auto n = &head;
			do {
				n->callback(n->data, *this);
				auto next = n->next;
				if (n != &head) delete n;
				n = next;
			} while (n);
		}
	}

	void ShaderParameter::setUsage(ShaderParameterUsage usage) {
		if (_usage != usage) {
			if (usage == ShaderParameterUsage::SHARE) releaseExclusiveBuffers();
			_usage = usage;
		}
	}

	void ShaderParameter::addReleaseExclusiveHandler(void* data, EXCLUSIVE_FN callback) {
		if (callback) {
			if (_handlers.callback) {
				auto n = &_handlers;
				do {
					if (n->data == data && n->callback == callback) return;
					if (n->next) {
						n = n->next;
					} else {
						break;
					}
				} while (true);

				n->next = new HandlerNode(data, callback);
			} else {
				_handlers.data = data;
				_handlers.callback = callback;
			}
		}
	}

	bool ShaderParameter::removeReleaseExclusiveHandler(void* data, EXCLUSIVE_FN callback) {
		if (_handlers.callback && callback) {
			HandlerNode* prev = nullptr;
			auto cur = &_handlers;
			do {
				if (cur->data == data && cur->callback == callback) {
					if (prev) {
						prev->next = cur->next;
						delete cur;
					} else {
						if (cur->next) {
							_handlers.set(*cur->next);
							delete cur->next;
						} else {
							_handlers.callback = nullptr;
						}
					}

					return true;
				}
				
				prev = cur;
				cur = cur->next;
			} while (cur);
		}

		return false;
	}

	ShaderParameter& ShaderParameter::set(const void* data, uint32_t size, uint16_t perElementSize, ShaderParameterType type, bool copy) {
		if (type >= ShaderParameterType::SAMPLER) copy = false;
		bool isRefObj = type >= ShaderParameterType::SAMPLER;

		switch (_storageType) {
		case StorageType::DEFAULT:
		{
			if (copy) {
				if (size <= DEFAULT_DATA_SIZE) {
					memcpy(&_data, data, size);
				} else {
					_storageType = StorageType::INTERNAL;
					_data.internalData = new uint8_t[size];
					_data.internalSize = size;
					memcpy(&_data.internalData, data, size);
				}
			} else {
				_storageType = StorageType::EXTERNAL;
				_data.externalData = data;
				if (isRefObj && data) ((Ref*)data)->ref();
			}

			break;
		}
		case StorageType::INTERNAL:
		{
			if (copy) {
				if (size <= DEFAULT_DATA_SIZE) {
					delete[] _data.internalData;
					_storageType = StorageType::DEFAULT;
					memcpy(&_data, data, size);
				} else {
					if (_data.internalSize < size) {
						delete[] _data.internalData;
						_data.internalData = new uint8_t[size];
						_data.internalSize = size;
					}
					memcpy(&_data.internalData, data, size);
				}
			} else {
				delete[] _data.internalData;
				_storageType = StorageType::EXTERNAL;
				_data.externalData = data;
				if (isRefObj && data) ((Ref*)data)->ref();
			}

			break;
		}
		case StorageType::EXTERNAL:
		{
			if (copy) {
				if (_data.externalRef && _data.externalData) ((Ref*)data)->unref();

				if (size <= DEFAULT_DATA_SIZE) {
					_storageType = StorageType::DEFAULT;
					memcpy(&_data, data, size);
				} else {
					_storageType = StorageType::INTERNAL;
					_data.internalData = new uint8_t[size];
					_data.internalSize = size;
					memcpy(&_data.internalData, data, size);
				}
			} else {
				if (_data.externalData != data) {
					if (isRefObj && data) ((Ref*)data)->ref();
					if (_data.externalRef && _data.externalData) ((Ref*)_data.externalData)->unref();
					_data.externalData = data;
					_data.externalRef = isRefObj;
				}
			}

			break;
		}
		default:
			break;
		}

		_type = type;
		_size = size;
		_perElementSize = perElementSize;

		return *this;
	}

	void ShaderParameter::clear() {
		if (_storageType == StorageType::EXTERNAL && _data.externalData) {
			if (_data.externalRef) ((Ref*)_data.externalData)->unref();
			_data.externalData = nullptr;
			_data.externalRef = false;
		}

		_type = ShaderParameterType::DATA;
		_size = 0;
		_perElementSize = 0;
	}


	ShaderParameter* ShaderParameterCollection::get(const std::string& name) const {
		auto itr = _parameters.find(name);
		return itr == _parameters.end() ? nullptr : itr->second;
	}

	ShaderParameter* ShaderParameterCollection::get(const std::string& name, ShaderParameterType type) const {
		auto itr = _parameters.find(name);
		return itr == _parameters.end() ? nullptr : (itr->second->getType() == type ? itr->second : nullptr);
	}

	ShaderParameter* ShaderParameterCollection::add(const std::string& name, ShaderParameter* parameter) {
		if (auto itr = _parameters.find(name); parameter) {
			if (itr == _parameters.end()) {
				_parameters.emplace(name, parameter);
			} else if (itr->second != parameter) {
				itr->second = parameter;
			}
		} else if (itr != _parameters.end()) {
			_parameters.erase(itr);
		}

		return parameter;
	}


	ShaderParameter* ShaderParameterGetterStack::get(const std::string& name) const {
		auto i = _stack.size();
		while (i--) {
			if (_stack[i]) {
				if (auto rst = _stack[i]->get(name); rst) return rst;
			}
		}
		return nullptr;
	}

	ShaderParameter* ShaderParameterGetterStack::get(const std::string& name, ShaderParameterType type) const {
		auto i = _stack.size();
		while (i--) {
			if (_stack[i]) {
				if (auto rst = _stack[i]->get(name, type); rst) return rst;
			}
		}
		return nullptr;
	}
}