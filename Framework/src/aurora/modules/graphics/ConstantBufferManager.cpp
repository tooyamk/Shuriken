#include "ConstantBufferManager.h"
#include "aurora/ShaderParameter.h"
#include "aurora/modules/graphics/IGraphicsModule.h"
#include <algorithm>

namespace aurora::modules::graphics {
	ConstantBufferLayout::ConstantBufferLayout() :
		bindPoint(0),
		size(0),
		featureValue(0) {
	}

	void ConstantBufferLayout::calcFeatureValue() {
		featureValue = hash::CRC::update<64, false>(0, &size, sizeof(size), _crcTable);

		uint16_t numValidVars = 0;
		for (auto& var : variables) _calcFeatureValue(var, numValidVars);

		featureValue = hash::CRC::update<64, false>(featureValue, &numValidVars, sizeof(numValidVars), _crcTable);
		featureValue = hash::CRC::finish<64, false>(featureValue, 0);
	}

	void ConstantBufferLayout::_calcFeatureValue(const Variables& var, uint16_t& numValidVars) {
		if (var.structMembers.empty()) {
			auto nameLen = var.name.size();
			featureValue = hash::CRC::update<64, false>(featureValue, &nameLen, sizeof(nameLen), _crcTable);
			featureValue = hash::CRC::update<64, false>(featureValue, var.name.data(), var.name.size(), _crcTable);
			featureValue = hash::CRC::update<64, false>(featureValue, &var.offset, sizeof(var.offset), _crcTable);
			featureValue = hash::CRC::update<64, false>(featureValue, &var.size, sizeof(var.size), _crcTable);

			++numValidVars;
		} else {
			for (auto& mvar : var.structMembers) _calcFeatureValue(mvar, numValidVars);
		}
	}

	void ConstantBufferLayout::collectUsingInfo(const IShaderParameterGetter& getter, ShaderParameterUsageStatistics& statistics,
		std::vector<const ShaderParameter*>& usingParams, std::vector<const ConstantBufferLayout::Variables*>& usingVars) const {
		for (auto& var : variables) _collectUsingInfo(var, getter, statistics, usingParams, usingVars);
	}

	void ConstantBufferLayout::_collectUsingInfo(const ConstantBufferLayout::Variables& var, const IShaderParameterGetter& getter,
		ShaderParameterUsageStatistics& statistics, std::vector<const ShaderParameter*>& usingParams, std::vector<const ConstantBufferLayout::Variables*>& usingVars) const {
		if (auto p = getter.get(var.name); var.structMembers.size()) {
			if (p && p->getType() == ShaderParameterType::GETTER) {
				if (auto data = p->getData(); data) {
					for (auto& memVar : var.structMembers) _collectUsingInfo(memVar, *(const IShaderParameterGetter*)data, statistics, usingParams, usingVars);
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
		auto rst = _shareConstBufferPool.emplace(std::piecewise_construct, std::forward_as_tuple(size), std::forward_as_tuple());
		auto& pool = rst.first->second;
		if (rst.second) {
			pool.rc = 1;
			pool.idleIndex = 0;
		} else {
			++pool.rc;
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
		auto rst = _exclusiveConstPool.emplace(std::piecewise_construct, std::forward_as_tuple(layout.featureValue), std::forward_as_tuple());
		auto& pool = rst.first->second;
		if (rst.second) {
			pool.rc = 1;
		} else {
			++pool.rc;
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
		uint32_t cur, uint32_t max, ExclusiveConstNode* parent, std::unordered_map<const ShaderParameter*, RefPtr<ExclusiveConstNode>>& childContainer) {
		auto param = parameters[cur];
		RefPtr<ExclusiveConstNode> node;

		if (param) {
			auto rst = childContainer.emplace(std::piecewise_construct, std::forward_as_tuple(param), std::forward_as_tuple(nullptr));
			if (rst.second) {
				node = new ExclusiveConstNode();
				rst.first->second = node;
				node->parameter = param;
				node->parent = parent;

				_exclusiveConstNodes.emplace(std::piecewise_construct, std::forward_as_tuple(param), std::forward_as_tuple()).first->second.emplace(node);

				param->addReleaseExclusiveHandler(this, &ConstantBufferManager::_releaseExclusiveConstant);
			} else {
				node = rst.first->second;
			}
		}

		if (cur == max) {
			if (!param) node = parent;
			if (!node) return nullptr;

			IConstantBuffer* cb = nullptr;
			if (auto rst = node->buffers.emplace(layout.featureValue, nullptr); rst.second) {
				cb = createExclusiveConstantBufferCallback(cur + 1);
				rst.first->second = cb;
				if (cb) {
					cb->ref();
					cb->create(layout.size, Usage::MAP_WRITE);

					_exclusiveConstPool.find(layout.featureValue)->second.nodes.emplace(node);

					do {
						++node->numAssociativeBuffers;
						node = node->parent;
					} while (node);
				}
			} else {
				cb = rst.first->second;
			}

			return cb;
		} else {
			if (param) {
				return _getExclusiveConstantBuffer(layout, parameters, cur + 1, max, node, node->children);
			} else {
				return _getExclusiveConstantBuffer(layout, parameters, cur + 1, max, parent, childContainer);
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
					parent->children.erase(releaseChild->parameter);
				}
				_releaseExclusiveConstantToRoot(parent->parent, nullptr, releaseNumAssociativeBuffers, releaseParam);
			}
		} else if (releaseChild) {
			_releaseExclusiveConstantSelf(*releaseChild, releaseParam);
			_exclusiveConstRoots.erase(releaseChild->parameter);
		}
	}

	void ConstantBufferManager::_releaseExclusiveConstantToLeaf(ExclusiveConstNode& node, bool releaseParam) {
		_releaseExclusiveConstantSelf(node, releaseParam);
		for (auto& itr : node.children) _releaseExclusiveConstantToLeaf(*itr.second, releaseParam);
	}

	void ConstantBufferManager::_releaseExclusiveConstantSelf(ExclusiveConstNode& node, bool releaseParam) {
		for (auto& itr : node.buffers) itr.second->unref();
		if (releaseParam) node.parameter->removeReleaseExclusiveHandler(this, &ConstantBufferManager::_releaseExclusiveConstant);
	}

	void ConstantBufferManager::updateConstantBuffer(IConstantBuffer* cb, const ShaderParameter& param, const ConstantBufferLayout::Variables& var) {
		uint32_t size = param.getSize();
		if (!size) return;

		if (auto pes = param.getPerElementSize(); pes < size) {
			auto stride = var.stride & 0x7FFFFFFFU;
			if (auto remainder = var.stride >= 0x80000000U ? (pes & (stride - 1)) : pes % var.stride; remainder) {
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