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

	ShaderParameter* ShaderParameterCollection::set(const std::string& name, ShaderParameter* parameter) {
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