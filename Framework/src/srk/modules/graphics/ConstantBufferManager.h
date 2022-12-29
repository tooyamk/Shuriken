#pragma once

#include "srk/Intrusive.h"
#include "srk/hash/xxHash.h"
#include <functional>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace srk {
	class ShaderParameter;
	class IShaderParameterGetter;
}

namespace srk::modules::graphics {
	class IConstantBuffer;
	
	
	struct SRK_FW_DLL ShaderParameterUsageStatistics {
		uint16_t exclusiveCount = 0;
		uint16_t autoCount = 0;
		uint16_t shareCount = 0;
		uint16_t unknownCount = 0;
	};


	class SRK_FW_DLL ConstantBufferLayout {
	public:
		struct SRK_FW_DLL Variables {
			std::string name;
			uint32_t offset = 0;
			uint32_t size = 0;
			uint32_t stride = 0;
			std::vector<Variables> members;
		};


		ConstantBufferLayout();

		std::string name;
		uint32_t bindPoint = 0;
		uint32_t size = 0;
		std::vector<Variables> variables;
		uint64_t featureValue = 0;

		void SRK_CALL calcFeatureValue();

		void SRK_CALL collectUsingInfo(const IShaderParameterGetter& getter, ShaderParameterUsageStatistics& statistics,
			std::vector<const ShaderParameter*>& usingParams, std::vector<const ConstantBufferLayout::Variables*>& usingVars) const;

	private:
		void SRK_CALL _calcFeatureValue(const Variables& var, uint16_t& numValidVars, hash::xxHash<64>& hasher);

		void SRK_CALL _collectUsingInfo(const ConstantBufferLayout::Variables& var, const IShaderParameterGetter& getter, ShaderParameterUsageStatistics& statistics,
			std::vector<const ShaderParameter*>& usingParams, std::vector<const ConstantBufferLayout::Variables*>& usingVars) const;
	};


	class SRK_FW_DLL ConstantBufferManager {
	public:
		ConstantBufferManager();
		ConstantBufferManager(const ConstantBufferManager& manager) = delete;
		ConstantBufferManager(ConstantBufferManager&& manager) = delete;
		~ConstantBufferManager();

		std::function<IConstantBuffer*()> createShareConstantBufferCallback = nullptr;
		std::function<IConstantBuffer*(uint32_t numParameters)> createExclusiveConstantBufferCallback = nullptr;

		void SRK_CALL registerConstantLayout(ConstantBufferLayout& layout);
		void SRK_CALL unregisterConstantLayout(ConstantBufferLayout& layout);

		IConstantBuffer* SRK_CALL popShareConstantBuffer(uint32_t size);
		void SRK_CALL resetUsedShareConstantBuffers();

		IConstantBuffer* SRK_CALL getExclusiveConstantBuffer(const std::vector<ShaderParameter*>& parameters, const ConstantBufferLayout& layout);

		static void SRK_CALL updateConstantBuffer(IConstantBuffer* cb, const ShaderParameter& param, const ConstantBufferLayout::Variables& var);

	private:
		struct ShareConstBuffers {
			uint32_t rc;
			uint32_t idleIndex;
			std::vector<IConstantBuffer*> buffers;
		};


		std::unordered_map<uint32_t, ShareConstBuffers> _shareConstBufferPool;
		std::unordered_set<uint32_t> _usedShareConstBufferPool;

		void SRK_CALL _registerShareConstantLayout(uint32_t size);
		void SRK_CALL _unregisterShareConstantLayout(uint32_t size);


		class ExclusiveConstNode {
			SRK_REF_OBJECT(ExclusiveConstNode)
		public:
			ExclusiveConstNode() :
				numAssociativeBuffers(0),
				parameter(nullptr),
				parent(nullptr) {
			}

			uint32_t numAssociativeBuffers;
			std::unordered_map<const ShaderParameter*, IntrusivePtr<ExclusiveConstNode>> children;
			std::unordered_map<uint64_t, IConstantBuffer*> buffers;
			ShaderParameter* parameter;
			IntrusivePtr<ExclusiveConstNode> parent;
		};


		std::unordered_map<const ShaderParameter*, IntrusivePtr<ExclusiveConstNode>> _exclusiveConstRoots;
		std::unordered_map<const ShaderParameter*, std::unordered_set<ExclusiveConstNode*>> _exclusiveConstNodes;


		struct ExclusiveConsts {
			uint32_t rc;
			std::unordered_set<ExclusiveConstNode*> nodes;
		};


		std::unordered_map<uint64_t, ExclusiveConsts> _exclusiveConstPool;


		IConstantBuffer* SRK_CALL _getExclusiveConstantBuffer(const ConstantBufferLayout& layout, const std::vector<ShaderParameter*>& parameters,
			uint32_t cur, uint32_t max, ExclusiveConstNode* parent, std::unordered_map<const ShaderParameter*, IntrusivePtr<ExclusiveConstNode>>& childContainer);
		void SRK_CALL _registerExclusiveConstantLayout(ConstantBufferLayout& layout);
		void SRK_CALL _unregisterExclusiveConstantLayout(ConstantBufferLayout& layout);
		static void _releaseExclusiveConstant(void* target, const ShaderParameter& param);
		void _releaseExclusiveConstant(const ShaderParameter& param);
		void _releaseExclusiveConstantToRoot(ExclusiveConstNode* parent, ExclusiveConstNode* releaseChild, uint32_t releaseNumAssociativeBuffers, bool releaseParam);
		void _releaseExclusiveConstantToLeaf(ExclusiveConstNode& node, bool releaseParam);
		void _releaseExclusiveConstantSelf(ExclusiveConstNode& node, bool releaseParam);
	};
}