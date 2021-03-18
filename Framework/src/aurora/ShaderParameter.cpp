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
			Ref::unref(*(Ref*)_data.externalData);
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

	void ShaderParameter::clear() {
		if (_storageType == StorageType::EXTERNAL && _data.externalData) {
			if (_data.externalRef) Ref::unref(*(Ref*)_data.externalData);
			_data.externalData = nullptr;
			_data.externalRef = false;
		}

		_type = ShaderParameterType::DATA;
		_size = 0;
		_perElementSize = 0;
	}

	void ShaderParameter::_set(const void* data, uint32_t size, uint16_t perElementSize, ShaderParameterType type, bool copy, ShaderParameterUpdateBehavior updateBehavior) {
		if (type >= ShaderParameterType::SAMPLER) copy = false;
		bool isRefObj = type >= ShaderParameterType::SAMPLER;

		switch (_storageType) {
		case StorageType::DEFAULT:
		{
			if (copy) {
				if (size <= DEFAULT_DATA_SIZE) {
					if (updateBehavior == ShaderParameterUpdateBehavior::CHECK) {
						if (memcmp(_data.data, data, size)) {
							memcpy(_data.data, data, size);
							setUpdated();
						}
					} else {
						memcpy(_data.data, data, size);
						if (updateBehavior == ShaderParameterUpdateBehavior::FORCE) setUpdated();
					}
				} else {
					_storageType = StorageType::INTERNAL;
					_data.internalData = new uint8_t[size];
					_data.internalSize = size;
					memcpy(_data.internalData, data, size);
					if (updateBehavior != ShaderParameterUpdateBehavior::NOT) setUpdated();
				}
			} else {
				_storageType = StorageType::EXTERNAL;
				_data.externalData = data;
				if (isRefObj) {
					_data.externalRef = true;
					if (data) ((Ref*)data)->ref();
				} else {
					_data.externalRef = false;
				}

				if (updateBehavior != ShaderParameterUpdateBehavior::NOT) setUpdated();
			}

			break;
		}
		case StorageType::INTERNAL:
		{
			if (copy) {
				if (size <= DEFAULT_DATA_SIZE) {
					delete[] _data.internalData;
					_storageType = StorageType::DEFAULT;
					memcpy(_data.data, data, size);
					if (updateBehavior != ShaderParameterUpdateBehavior::NOT) setUpdated();
				} else {
					if (_data.internalSize < size) {
						delete[] _data.internalData;
						_data.internalData = new uint8_t[size];
						_data.internalSize = size;
						memcpy(_data.internalData, data, size);
						if (updateBehavior != ShaderParameterUpdateBehavior::NOT) setUpdated();
					} else {
						if (updateBehavior == ShaderParameterUpdateBehavior::CHECK) {
							if (memcmp(_data.internalData, data, size)) {
								memcpy(_data.internalData, data, size);
								setUpdated();
							}
						} else {
							memcpy(_data.internalData, data, size);
							if (updateBehavior == ShaderParameterUpdateBehavior::FORCE) setUpdated();
						}
					}
				}
			} else {
				delete[] _data.internalData;
				_storageType = StorageType::EXTERNAL;
				_data.externalData = data;
				if (isRefObj) {
					_data.externalRef = true;
					if (data) ((Ref*)data)->ref();
				} else {
					_data.externalRef = false;
				}

				if (updateBehavior != ShaderParameterUpdateBehavior::NOT) setUpdated();
			}

			break;
		}
		case StorageType::EXTERNAL:
		{
			if (copy) {
				if (_data.externalRef && _data.externalData) Ref::unref(*(Ref*)data);

				if (size <= DEFAULT_DATA_SIZE) {
					_storageType = StorageType::DEFAULT;
					memcpy(&_data, data, size);
				} else {
					_storageType = StorageType::INTERNAL;
					_data.internalData = new uint8_t[size];
					_data.internalSize = size;
					memcpy(&_data.internalData, data, size);
				}
				if (updateBehavior != ShaderParameterUpdateBehavior::NOT) setUpdated();
			} else {
				if (_data.externalData == data) {
					if (updateBehavior != ShaderParameterUpdateBehavior::NOT) {
						if (_size == size) {
							if (memcmp(_data.externalData, data, size)) setUpdated();
						} else {
							setUpdated();
						}
					}
				} else {
					if (isRefObj && data) ((Ref*)data)->ref();
					if (_data.externalRef && _data.externalData) Ref::unref(*(Ref*)_data.externalData);
					_data.externalData = data;
					_data.externalRef = isRefObj;
					if (updateBehavior != ShaderParameterUpdateBehavior::NOT) setUpdated();
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
	}


	IntrusivePtr<ShaderParameter> ShaderParameterCollection::get(const QueryString& name) const {
		auto itr = _parameters.find(name);
		return itr == _parameters.end() ? nullptr : itr->second;
	}

	IntrusivePtr<ShaderParameter> ShaderParameterCollection::get(const QueryString& name, ShaderParameterType type) const {
		auto itr = _parameters.find(name);
		return itr == _parameters.end() ? nullptr : (itr->second->getType() == type ? itr->second : nullptr);
	}

	ShaderParameter* AE_CALL ShaderParameterCollection::set(const QueryString& name, ShaderParameter* parameter) {
		if (parameter) {
			if (auto itr = _parameters.find(name); itr == _parameters.end()) {
				_parameters.emplace(name, parameter);
			} else {
				itr->second = parameter;
			}
		} else {
			remove(name);
		}

		return parameter;
	}

	ShaderParameter* ShaderParameterCollection::_remove(const QueryString& name) {
		if (auto itr = _parameters.find(name); itr == _parameters.end()) {
			auto val = itr->second;
			_parameters.erase(itr);
			return val;
		}

		return nullptr;
	}


	IntrusivePtr<ShaderParameter> ShaderParameterGetterStack::get(const QueryString& name) const {
		auto i = _stack.size();
		while (i--) {
			if (_stack[i]) {
				if (auto rst = _stack[i]->get(name); rst) return rst;
			}
		}
		return nullptr;
	}

	IntrusivePtr<ShaderParameter> ShaderParameterGetterStack::get(const QueryString& name, ShaderParameterType type) const {
		auto i = _stack.size();
		while (i--) {
			if (_stack[i]) {
				if (auto rst = _stack[i]->get(name, type); rst) return rst;
			}
		}
		return nullptr;
	}
}