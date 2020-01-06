#include "ConstantBufferManager.h"
#include "modules/graphics/IGraphicsModule.h"
#include "modules/graphics/ShaderParameterFactory.h"
#include "utils/hash/CRC.h"
#include <algorithm>

namespace aurora::modules::graphics {
	ConstantBufferLayout::ConstantBufferLayout() :
		bindPoint(0),
		size(0),
		featureValue(0) {
	}

	void ConstantBufferLayout::calcFeatureValue() {
		featureValue = hash::CRC::begin<64>();
		hash::CRC::update<64>(featureValue, (uint8_t*)&size, sizeof(size));

		uint16_t numValidVars = 0;
		for (auto& var : variables) _calcFeatureValue(var, numValidVars);

		hash::CRC::update<64>(featureValue, (uint8_t*)&numValidVars, sizeof(numValidVars));
		hash::CRC::end<64>(featureValue);
	}

	void ConstantBufferLayout::_calcFeatureValue(const Variables& var, uint16_t& numValidVars) {
		if (var.structMembers.empty()) {
			auto nameLen = var.name.size();
			hash::CRC::update<64>(featureValue, (uint8_t*)&nameLen, sizeof(nameLen));
			hash::CRC::update<64>(featureValue, (uint8_t*)var.name.data(), var.name.size());
			hash::CRC::update<64>(featureValue, (uint8_t*)&var.offset, sizeof(var.offset));
			hash::CRC::update<64>(featureValue, (uint8_t*)&var.size, sizeof(var.size));

			++numValidVars;
		} else {
			for (auto& mvar : var.structMembers) _calcFeatureValue(mvar, numValidVars);
		}
	}

	void ConstantBufferLayout::collectUsingInfo(const ShaderParameterFactory& factory, ShaderParameterUsageStatistics& statistics,
		std::vector<const ShaderParameter*>& usingParams, std::vector<const ConstantBufferLayout::Variables*>& usingVars) const {
		for (auto& var : variables) _collectUsingInfo(var, factory, statistics, usingParams, usingVars);
	}

	void ConstantBufferLayout::_collectUsingInfo(const ConstantBufferLayout::Variables& var, const ShaderParameterFactory& factory, 
		ShaderParameterUsageStatistics& statistics, std::vector<const ShaderParameter*>& usingParams, std::vector<const ConstantBufferLayout::Variables*>& usingVars) const {
		if (auto p = factory.get(var.name); var.structMembers.size()) {
			if (p && p->getType() == ShaderParameterType::FACTORY) {
				if (auto data = p->getData(); data) {
					for (auto& memVar : var.structMembers) _collectUsingInfo(memVar, *(const ShaderParameterFactory*)data, statistics, usingParams, usingVars);
				}
			}
		} else {
			if (p) {
				if (p->getUsage() == ShaderParameterUsage::EXCLUSIVE) {
					++statistics.exclusiveCount;
				} else if (p->getUsage() == ShaderParameterUsage::AUTO) {
					++statistics.autoCount;
				} else {
					++statistics.shareCount;
				}
			} else {
				++statistics.unknownCount;
			}

			usingParams.emplace_back(p);
			usingVars.emplace_back(&var);
		}
	}


	ConstantBufferManager::ConstantBufferManager() {
	}

	ConstantBufferManager::~ConstantBufferManager() {

	}

	void ConstantBufferManager::registerConstantLayout(ConstantBufferLayout& layout) {
		_registerShareConstantLayout(layout.size);
		_registerExclusiveConstantLayout(layout);
	}

	void ConstantBufferManager::unregisterConstantLayout(ConstantBufferLayout& layout) {
		_unregisterShareConstantLayout(layout.size);
		_unregisterExclusiveConstantLayout(layout);
	}

	void ConstantBufferManager::_registerShareConstantLayout(uint32_t size) {
		if (auto itr = _shareConstBufferPool.find(size); itr == _shareConstBufferPool.end()) {
			auto& pool = _shareConstBufferPool.emplace(std::piecewise_construct, std::forward_as_tuple(size), std::forward_as_tuple()).first->second;
			pool.rc = 1;
			pool.idleIndex = 0;
		} else {
			++itr->second.rc;
		}
	}

	void ConstantBufferManager::_unregisterShareConstantLayout(uint32_t size) {
		if (auto itr = _shareConstBufferPool.find(size); itr != _shareConstBufferPool.end() && !--itr->second.rc) {
			//_graphics->ref();

			for (auto cb : itr->second.buffers) cb->unref();
			_shareConstBufferPool.erase(itr);

			//_graphics->unref();
		}
	}

	IConstantBuffer* ConstantBufferManager::popShareConstantBuffer(uint32_t size) {
		if (auto itr = _shareConstBufferPool.find(size); itr != _shareConstBufferPool.end()) {
			auto& pool = itr->second;
			IConstantBuffer* cb;
			if (auto& buffers = pool.buffers; pool.idleIndex == buffers.size()) {
				cb = createShareConstantBufferCallback();
				cb->ref();
				cb->create(size, Usage::MAP_WRITE);
				buffers.emplace_back(cb);
			} else {
				cb = buffers[pool.idleIndex];
			}

			_usedShareConstBufferPool.emplace(size);
			++pool.idleIndex;
			return cb;
		}
		return nullptr;
	}

	void ConstantBufferManager::resetUsedShareConstantBuffers() {
		for (auto size : _usedShareConstBufferPool) {
			if (auto itr = _shareConstBufferPool.find(size); itr != _shareConstBufferPool.end()) itr->second.idleIndex = 0;
		}
		_usedShareConstBufferPool.clear();
	}

	IConstantBuffer* ConstantBufferManager::getExclusiveConstantBuffer(const std::vector<ShaderParameter*>& parameters, const ConstantBufferLayout& layout) {
		return _getExclusiveConstantBuffer(layout, parameters, 0, parameters.size() - 1, nullptr, _exclusiveConstRoots);
	}

	void ConstantBufferManager::_registerExclusiveConstantLayout(ConstantBufferLayout& layout) {
		if (auto itr = _exclusiveConstPool.find(layout.featureValue); itr == _exclusiveConstPool.end()) {
			_exclusiveConstPool.emplace(std::piecewise_construct, std::forward_as_tuple(layout.featureValue), std::forward_as_tuple()).first->second.rc = 1;
		} else {
			++itr->second.rc;
		}
	}

	void ConstantBufferManager::_unregisterExclusiveConstantLayout(ConstantBufferLayout& layout) {
		if (auto itr = _exclusiveConstPool.find(layout.featureValue); itr != _exclusiveConstPool.end() && !--itr->second.rc) {
			//_graphics->ref();

			for (auto node : itr->second.nodes) {
				_releaseExclusiveConstantToLeaf(*node, true);
				_releaseExclusiveConstantToRoot(node->parent, nullptr, node->numAssociativeBuffers, true);
			}

			_exclusiveConstPool.erase(itr);

			//_graphics->unref();
		}
	}

	IConstantBuffer* ConstantBufferManager::_getExclusiveConstantBuffer(const ConstantBufferLayout& layout, const std::vector<ShaderParameter*>& parameters,
		uint32_t cur, uint32_t max, ExclusiveConstNode* parent, std::unordered_map<const ShaderParameter*, ExclusiveConstNode>& chindrenContainer) {
		auto param = parameters[cur];
		ExclusiveConstNode* node = nullptr;

		if (param) {
			if (auto itr = chindrenContainer.find(param); itr == chindrenContainer.end()) {
				node = &chindrenContainer.emplace(std::piecewise_construct, std::forward_as_tuple(param), std::forward_as_tuple()).first->second;
				node->parameter = param;
				node->parent = parent;

				if (auto itr2 = _exclusiveConstNodes.find(param); itr2 == _exclusiveConstNodes.end()) {
					_exclusiveConstNodes.emplace(std::piecewise_construct, std::forward_as_tuple(param), std::forward_as_tuple()).first->second.emplace(node);
				} else {
					itr2->second.emplace(node);
				}

				param->__setExclusive(this, &ConstantBufferManager::_releaseExclusiveConstant);
			} else {
				node = &itr->second;
			}
		}

		if (cur == max) {
			if (!param) node = parent;
			if (!node) return nullptr;

			IConstantBuffer* cb = nullptr;
			if (auto itr = node->buffers.find(layout.featureValue); itr == node->buffers.end()) {
				cb = node->buffers.emplace(layout.featureValue, createExclusiveConstantBufferCallback(cur + 1)).first->second;
				cb->ref();
				cb->create(layout.size, Usage::MAP_WRITE);

				_exclusiveConstPool.find(layout.featureValue)->second.nodes.emplace(node);

				do {
					++node->numAssociativeBuffers;
					node = node->parent;
				} while (node);
			} else {
				cb = itr->second;
			}

			return cb;
		} else {
			if (param) {
				return _getExclusiveConstantBuffer(layout, parameters, cur + 1, max, node, node->children);
			} else {
				return _getExclusiveConstantBuffer(layout, parameters, cur + 1, max, parent, chindrenContainer);
			}
		}
	}

	void ConstantBufferManager::_releaseExclusiveConstant(void* target, const ShaderParameter& param) {
		if (target) ((ConstantBufferManager*)target)->_releaseExclusiveConstant(param);
	}

	void ConstantBufferManager::_releaseExclusiveConstant(const ShaderParameter& param) {
		if (auto itr = _exclusiveConstNodes.find(&param); itr != _exclusiveConstNodes.end()) {
			for (auto node : itr->second) {
				_releaseExclusiveConstantToLeaf(*node, false);
				_releaseExclusiveConstantToRoot(node->parent, nullptr, node->numAssociativeBuffers, false);
			}

			_exclusiveConstNodes.erase(itr);
		}
	}

	void ConstantBufferManager::_releaseExclusiveConstantToRoot(ExclusiveConstNode* parent, ExclusiveConstNode* releaseChild, uint32_t releaseNumAssociativeBuffers, bool releaseParam) {
		if (parent) {
			if (parent->numAssociativeBuffers <= releaseNumAssociativeBuffers) {
				if (releaseChild) _releaseExclusiveConstantSelf(*releaseChild, releaseParam);
				_releaseExclusiveConstantToRoot(parent->parent, parent, releaseNumAssociativeBuffers, releaseParam);
			} else {
				parent->numAssociativeBuffers -= releaseNumAssociativeBuffers;
				if (releaseChild) {
					_releaseExclusiveConstantSelf(*releaseChild, releaseParam);
					parent->children.erase(parent->children.find(releaseChild->parameter));
				}
				_releaseExclusiveConstantToRoot(parent->parent, nullptr, releaseNumAssociativeBuffers, releaseParam);
			}
		} else if (releaseChild) {
			_releaseExclusiveConstantSelf(*releaseChild, releaseParam);
			_exclusiveConstRoots.erase(_exclusiveConstRoots.find(releaseChild->parameter));
		}
	}

	void ConstantBufferManager::_releaseExclusiveConstantToLeaf(ExclusiveConstNode& node, bool releaseParam) {
		_releaseExclusiveConstantSelf(node, releaseParam);
		for (auto& itr : node.children) _releaseExclusiveConstantToLeaf(itr.second, releaseParam);
	}

	void ConstantBufferManager::_releaseExclusiveConstantSelf(ExclusiveConstNode& node, bool releaseParam) {
		for (auto& itr : node.buffers) itr.second->unref();
		if (releaseParam) node.parameter->__releaseExclusive(this, &ConstantBufferManager::_releaseExclusiveConstant);
	}

	void ConstantBufferManager::updateConstantBuffer(IConstantBuffer* cb, const ShaderParameter& param, const ConstantBufferLayout::Variables& var) {
		uint32_t size = param.getSize();
		if (!size) return;

		if (uint16_t pes = param.getPerElementSize(); pes < size) {
			auto stride = var.stride & 0x7FFFFFFFui32;
			if (auto remainder = var.stride >= 0x80000000ui32 ? (pes & (stride - 1)) : pes % var.stride; remainder) {
				auto offset = pes + stride - remainder;
				auto max = std::min<uint32_t>(size, var.size);
				uint32_t cur = 0, fillSize = 0;
				auto data = (const uint8_t*)param.getData();
				do {
					cb->write(var.offset + fillSize, data + cur, pes);
					cur += pes;
					fillSize += offset;
				} while (cur < max && fillSize < var.size);
			} else {
				cb->write(var.offset, param.getData(), std::min<uint32_t>(size, var.size));
			}
		} else {
			cb->write(var.offset, param.getData(), std::min<uint32_t>(size, var.size));
		}
	}
}