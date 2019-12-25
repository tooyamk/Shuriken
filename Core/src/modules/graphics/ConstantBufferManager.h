#pragma once

#include "base/Global.h"
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace aurora::modules::graphics {
	class IConstantBuffer;
	class ShaderParameter;
	class ShaderParameterFactory;


	struct AE_DLL ShaderParameterUsageStatistics {
		uint16_t exclusiveCount = 0;
		uint16_t autoCount = 0;
		uint16_t shareCount = 0;
		uint16_t unknownCount = 0;
	};


	class AE_DLL ConstantBufferLayout {
	public:
		struct AE_DLL Variables {
			std::string name;
			uint32_t offset = 0;
			uint32_t size = 0;
			uint32_t stride = 0;
			std::vector<Variables> structMembers;
		};


		ConstantBufferLayout();

		std::string name;
		uint32_t bindPoint;
		std::vector<Variables> variables;
		uint32_t size;
		uint64_t featureCode;

		void AE_CALL calcFeatureCode();

		void AE_CALL collectUsingInfo(const ShaderParameterFactory& factory, ShaderParameterUsageStatistics& statistics,
			std::vector<const ShaderParameter*>& usingParams, std::vector<const ConstantBufferLayout::Variables*>& usingVars) const;

	private:
		void AE_CALL _calcFeatureCode(const Variables& var, uint16_t& numValidVars);

		void AE_CALL _collectUsingInfo(const ConstantBufferLayout::Variables& var, const ShaderParameterFactory& factory, ShaderParameterUsageStatistics& statistics,
			std::vector<const ShaderParameter*>& usingParams, std::vector<const ConstantBufferLayout::Variables*>& usingVars) const;
	};


	class AE_DLL ConstantBufferManager {
	public:
		ConstantBufferManager();
		ConstantBufferManager(const ConstantBufferManager& manager) = delete;
		ConstantBufferManager(ConstantBufferManager&& manager) = delete;
		~ConstantBufferManager();

		std::function<IConstantBuffer*()> createShareConstantBufferCallback = nullptr;
		std::function<IConstantBuffer* (uint32_t numParameters)> createExclusiveConstantBufferCallback = nullptr;

		void AE_CALL registerConstantLayout(ConstantBufferLayout& layout);
		void AE_CALL unregisterConstantLayout(ConstantBufferLayout& layout);

		IConstantBuffer* AE_CALL popShareConstantBuffer (uint32_t size);
		void AE_CALL resetUsedShareConstantBuffers();

		IConstantBuffer* AE_CALL getExclusiveConstantBuffer(const std::vector<ShaderParameter*>& parameters, const ConstantBufferLayout& layout);

		static void AE_CALL updateConstantBuffer(IConstantBuffer* cb, const ShaderParameter& param, const ConstantBufferLayout::Variables& var);

	private:
		struct ShareConstBuffers {
			uint32_t rc;
			uint32_t idleIndex;
			std::vector<IConstantBuffer*> buffers;
		};


		std::unordered_map<uint32_t, ShareConstBuffers> _shareConstBufferPool;
		std::unordered_set<uint32_t> _usedShareConstBufferPool;

		void AE_CALL _registerShareConstantLayout (uint32_t size);
		void AE_CALL _unregisterShareConstantLayout (uint32_t size);


		struct ExclusiveConstNode {
			ExclusiveConstNode() :
				numAssociativeBuffers(0),
				parameter(nullptr),
				parent(nullptr) {
			}

			uint32_t numAssociativeBuffers;
			std::unordered_map<const ShaderParameter*, ExclusiveConstNode> children;
			std::unordered_map<uint64_t, IConstantBuffer*> buffers;
			ShaderParameter* parameter;
			ExclusiveConstNode* parent;
		};


		std::unordered_map<const ShaderParameter*, ExclusiveConstNode> _exclusiveConstRoots;
		std::unordered_map<const ShaderParameter*, std::unordered_set<ExclusiveConstNode*>> _exclusiveConstNodes;


		struct ExcLusiveConsts {
			uint32_t rc;
			std::unordered_set<ExclusiveConstNode*> nodes;
		};


		std::unordered_map<uint64_t, ExcLusiveConsts> _exclusiveConstPool;


		IConstantBuffer* AE_CALL _getExclusiveConstantBuffer(const ConstantBufferLayout& layout, const std::vector<ShaderParameter*>& parameters,
			uint32_t cur, uint32_t max, ExclusiveConstNode* parent, std::unordered_map <const ShaderParameter*, ExclusiveConstNode>& chindrenContainer);
		void AE_CALL _registerExclusiveConstantLayout(ConstantBufferLayout& layout);
		void AE_CALL _unregisterExclusiveConstantLayout(ConstantBufferLayout& layout);
		static void _releaseExclusiveConstant(void* target, const ShaderParameter& param);
		void _releaseExclusiveConstant(const ShaderParameter& param);
		void _releaseExclusiveConstantToRoot(ExclusiveConstNode* parent, ExclusiveConstNode* releaseChild, uint32_t releaseNumAssociativeBuffers, bool releaseParam);
		void _releaseExclusiveConstantToLeaf(ExclusiveConstNode& node, bool releaseParam);
		void _releaseExclusiveConstantSelf(ExclusiveConstNode& node, bool releaseParam);
	};
}