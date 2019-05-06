#include "ConstantBufferManager.h"
#include "modules/graphics/IGraphicsModule.h"
#include "modules/graphics/ShaderParameter.h"
#include "utils/hash/CRC.h"

namespace aurora::modules::graphics {
	ConstantBufferLayout::ConstantBufferLayout() :
		bindPoint(0),
		size(0),
		featureCode(0) {
	}

	void ConstantBufferLayout::calcFeatureCode() {
		featureCode = hash::CRC::CRC64StreamBegin();
		hash::CRC::CRC64StreamIteration(featureCode, (i8*)&size, sizeof(size));

		ui16 numValidVars = 0;
		for (auto& var : variables) _calcFeatureCode(var, numValidVars);

		hash::CRC::CRC64StreamIteration(featureCode, (i8*)&numValidVars, sizeof(numValidVars));
		hash::CRC::CRC64StreamEnd(featureCode);
	}

	void ConstantBufferLayout::_calcFeatureCode(const Variables& var, ui16& numValidVars) {
		if (var.structMembers.empty()) {
			auto nameLen = var.name.size();
			hash::CRC::CRC64StreamIteration(featureCode, (i8*)&nameLen, sizeof(nameLen));
			hash::CRC::CRC64StreamIteration(featureCode, var.name.c_str(), var.name.size());
			hash::CRC::CRC64StreamIteration(featureCode, (i8*)&var.offset, sizeof(var.offset));
			hash::CRC::CRC64StreamIteration(featureCode, (i8*)&var.size, sizeof(var.size));

			++numValidVars;
		} else {
			for (auto& mvar : var.structMembers) _calcFeatureCode(mvar, numValidVars);
		}
	}


	ConstantBufferManager::ConstantBufferManager(IGraphicsModule* graphics) :
		_graphics(graphics) {
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

	void ConstantBufferManager::_registerShareConstantLayout(ui32 size) {
		auto itr = _shareConstBufferPool.find(size);
		if (itr == _shareConstBufferPool.end()) {
			auto& pool = _shareConstBufferPool.emplace(std::piecewise_construct, std::forward_as_tuple(size), std::forward_as_tuple()).first->second;
			pool.rc = 1;
			pool.idleIndex = 0;
		} else {
			++itr->second.rc;
		}
	}

	void ConstantBufferManager::_unregisterShareConstantLayout(ui32 size) {
		auto itr = _shareConstBufferPool.find(size);
		if (itr != _shareConstBufferPool.end()) {
			if (itr->second.rc == 1) {
				//_graphics->ref();

				for (auto cb : itr->second.buffers) cb->unref();
				_shareConstBufferPool.erase(itr);

				//_graphics->unref();
			}
		}
	}

	IConstantBuffer* ConstantBufferManager::popShareConstantBuffer(ui32 size) {
		auto itr = _shareConstBufferPool.find(size);
		if (itr != _shareConstBufferPool.end()) {
			auto& pool = itr->second;
			auto& buffers = pool.buffers;
			auto len = buffers.size();
			IConstantBuffer* cb;
			if (pool.idleIndex == len) {
				cb = createShareConstantBufferCallback();
				cb->ref();
				cb->create(size, Usage::CPU_WRITE);
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
			auto itr = _shareConstBufferPool.find(size);
			if (itr != _shareConstBufferPool.end()) itr->second.idleIndex = 0;
		}
		_usedShareConstBufferPool.clear();
	}

	IConstantBuffer* ConstantBufferManager::getExclusiveConstantBuffer(const std::vector<ShaderParameter*>& parameters, const ConstantBufferLayout& layout) {
		return _getExclusiveConstantBuffer(layout, parameters, 0, parameters.size() - 1, nullptr, _exclusiveConstRoots);
	}

	void ConstantBufferManager::_registerExclusiveConstantLayout(ConstantBufferLayout& layout) {
		auto itr = _exclusiveConstPool.find(layout.featureCode);
		if (itr == _exclusiveConstPool.end()) {
			_exclusiveConstPool.emplace(std::piecewise_construct, std::forward_as_tuple(layout.featureCode), std::forward_as_tuple()).first->second.rc = 1;
		} else {
			++itr->second.rc;
		}
	}

	void ConstantBufferManager::_unregisterExclusiveConstantLayout(ConstantBufferLayout& layout) {
		auto itr = _exclusiveConstPool.find(layout.featureCode);
		if (itr != _exclusiveConstPool.end() && !--itr->second.rc) {
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
		ui32 cur, ui32 max, ExclusiveConstNode* parent, std::unordered_map<const ShaderParameter*, ExclusiveConstNode>& chindrenContainer) {
		auto param = parameters[cur];
		ExclusiveConstNode* node = nullptr;

		if (param) {
			auto itr = chindrenContainer.find(param);
			if (itr == chindrenContainer.end()) {
				node = &chindrenContainer.emplace(std::piecewise_construct, std::forward_as_tuple(param), std::forward_as_tuple()).first->second;
				node->parameter = param;
				node->parent = parent;

				auto itr2 = _exclusiveConstNodes.find(param);
				if (itr2 == _exclusiveConstNodes.end()) {
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

			auto itr = node->buffers.find(layout.featureCode);
			IConstantBuffer* cb = nullptr;
			if (itr == node->buffers.end()) {
				cb = node->buffers.emplace(layout.featureCode, createExclusiveConstantBufferCallback(cur + 1)).first->second;
				cb->ref();
				cb->create(layout.size, Usage::CPU_WRITE);

				_exclusiveConstPool.find(layout.featureCode)->second.nodes.emplace(node);

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
		auto itr = _exclusiveConstNodes.find(&param);
		if (itr != _exclusiveConstNodes.end()) {
			auto& nodes = itr->second;
			for (auto node : nodes) {
				_releaseExclusiveConstantToLeaf(*node, false);
				_releaseExclusiveConstantToRoot(node->parent, nullptr, node->numAssociativeBuffers, false);
			}

			_exclusiveConstNodes.erase(itr);
		}
	}

	void ConstantBufferManager::_releaseExclusiveConstantToRoot(ExclusiveConstNode* parent, ExclusiveConstNode* releaseChild, ui32 releaseNumAssociativeBuffers, bool releaseParam) {
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
		} else {
			if (releaseChild) {
				_releaseExclusiveConstantSelf(*releaseChild, releaseParam);
				_exclusiveConstRoots.erase(_exclusiveConstRoots.find(releaseChild->parameter));
			}
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
}