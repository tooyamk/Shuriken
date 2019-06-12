#include "ShaderParameter.h"

namespace aurora::modules::graphics {
	ShaderParameter::ShaderParameter(ShaderParameterUsage usage) :
		_usage(usage),
		_type(ShaderParameterType::DATA),
		_storageType(StorageType::DEFAULT),
		_updateId(0),
		_data(),
		_size(0),
		_exclusiveRc(0),
		_exclusiveFnTarget(nullptr),
		_exclusiveFn(nullptr) {
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

	ShaderParameter& ShaderParameter::set(const void* data, ui32 size, ui16 perElementSize, ShaderParameterType type, bool copy) {
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
					_data.internalData = new i8[size];
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
						_data.internalData = new i8[size];
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
					_data.internalData = new i8[size];
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
}