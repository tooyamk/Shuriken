#include "BaseBuffer.h"
#include "Graphics.h"
#include <algorithm>

namespace aurora::modules::graphics::win_d3d11 {
	BaseBuffer::BaseBuffer(Graphics& graphics, UINT bufferType) :
		_grap(&graphics),
		_bufferType(bufferType),
		_handle(nullptr),
		_bufferUsage(0),
		_mapUsage(0),
		_mapData(nullptr) {
	}

	BaseBuffer::~BaseBuffer() {
		_delBuffer();
	}

	bool BaseBuffer::_stroage(ui32 size, ui32 bufferUsage, const void* data) {
		_delBuffer();

		_size = size;

		D3D11_USAGE dxUsage;
		UINT cpuFlags = 0;
		if (bufferUsage & BufferUsage::CPU_READ) cpuFlags |= D3D11_CPU_ACCESS_READ;
		if (bufferUsage & BufferUsage::CPU_WRITE) cpuFlags |= D3D11_CPU_ACCESS_WRITE;

		if ((bufferUsage & BufferUsage::CPU_READ) || ((bufferUsage & BufferUsage::CPU_GPU_WRITE) == BufferUsage::CPU_GPU_WRITE)) {
			dxUsage = D3D11_USAGE_STAGING;
			_bufferUsage = BufferUsage::CPU_READ_WRITE | BufferUsage::GPU_WRITE;
		} else if (bufferUsage & BufferUsage::CPU_WRITE) {
			dxUsage = D3D11_USAGE_DYNAMIC;
			_bufferUsage = BufferUsage::CPU_WRITE;
		} else if (bufferUsage & BufferUsage::GPU_WRITE || !data) {
			dxUsage = D3D11_USAGE_DEFAULT;
			_bufferUsage = BufferUsage::GPU_WRITE;
		} else {
			dxUsage = D3D11_USAGE_IMMUTABLE;
		}

		D3D11_BUFFER_DESC desc;
		memset(&desc, 0, sizeof(desc));
		desc.CPUAccessFlags = cpuFlags;
		desc.Usage = dxUsage;
		desc.BindFlags = _bufferType;
		desc.ByteWidth = size;

		D3D11_SUBRESOURCE_DATA res;
		if (data) {
			memset(&res, 0, sizeof(res));
			res.pSysMem = data;
		}

		auto hr = _grap->getDevice()->CreateBuffer(&desc, data ? &res : nullptr, &_handle);
		if (FAILED(hr)) {
			_delBuffer();
			return false;
		}

		return true;
	}

	bool BaseBuffer::_map(ui32 mapUsage) {
		mapUsage &= BufferMapUsage::READ | BufferMapUsage::WRITE;
		if (_handle && mapUsage) {
			_unmap();

			D3D11_MAPPED_SUBRESOURCE mappedResource;
			memset(&mappedResource, 0, sizeof(D3D11_MAPPED_SUBRESOURCE));

			UINT mapType = 0;
			if ((mapUsage & BufferMapUsage::READ) && (_bufferUsage & BufferUsage::CPU_READ)) {
				mapType |= D3D11_MAP_READ;
			} else {
				mapUsage &= ~BufferMapUsage::READ;
			}
			if ((mapUsage & BufferMapUsage::WRITE) && (_bufferUsage & BufferUsage::CPU_WRITE)) {
				if (_bufferType == D3D11_BIND_CONSTANT_BUFFER) {
					mapType |= _grap->getFeatureOptions().MapNoOverwriteOnDynamicConstantBuffer ? D3D11_MAP_WRITE_NO_OVERWRITE : D3D11_MAP_WRITE_DISCARD;
				} else {
					mapType |= D3D11_MAP_WRITE_NO_OVERWRITE;
				}				
			} else {
				mapUsage &= ~BufferMapUsage::WRITE;
			}
			if (!mapType || FAILED(_grap->getContext()->Map(_handle, 0, (D3D11_MAP)mapType, 0, &mappedResource))) return false;

			_mapUsage = mapUsage;
			_mapData = mappedResource.pData;

			return true;
		}

		return false;
	}

	void BaseBuffer::_unmap() {
		if (_mapUsage) {
			_grap->getContext()->Unmap(_handle, 0);
			_mapData = nullptr;
			_mapUsage = 0;
		}
	}

	i32 BaseBuffer::_read(ui32 offset, void* dst, ui32 dstLen, i32 readLen) {
		if (_mapUsage & BufferMapUsage::READ) {
			if (dstLen == 0 || readLen == 0 || offset >= _size) return 0;
			if (dst) {
				if (readLen < 0) readLen = _size - offset;
				if ((ui32)readLen > dstLen) readLen = dstLen;
				memcpy(dst, (i8*)_mapData + offset, readLen);
				return readLen;
			}
		}
		return -1;
	}

	i32 BaseBuffer::_write(ui32 offset, const void* data, ui32 length) {
		if (_mapUsage & BufferMapUsage::WRITE) {
			if (data && length && offset < _size) {
				length = std::min<ui32>(length, _size - offset);
				memcpy((i8*)_mapData + offset, data, length);
				return length;
			}
			return 0;
		}
		return -1;
	}

	void BaseBuffer::_flush() {
		if (_handle && (_bufferUsage & BufferUsage::GPU_WRITE)) {

		}
	}

	void BaseBuffer::_delBuffer() {
		if (_handle) {
			_unmap();

			_handle->Release();
			_handle = nullptr;
		}
		_size = 0;
		_bufferUsage = 0;
	}

	void BaseBuffer::_waitServerSync() {
	}

	void BaseBuffer::_delSync() {
	}
}