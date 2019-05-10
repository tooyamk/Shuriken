#pragma once

#include "base/LowLevel.h"
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace aurora::modules::graphics {
	class IConstantBuffer;
	class ShaderParameter;
	class ShaderParameterFactory;


	struct AE_DLL ShaderParameterUsageStatistics {
		ui16 exclusiveCount = 0;
		ui16 autoCount = 0;
		ui16 shareCount = 0;
		ui16 unknownCount = 0;
	};


	class AE_DLL ConstantBufferLayout {
	public:
		struct AE_DLL Variables {
			std::string name;
			ui32 offset = 0;
			ui32 size = 0;
			ui32 stride = 0;
			std::vector<Variables> structMembers;
		};


		ConstantBufferLayout();

		std::string name;
		ui32 bindPoint;
		std::vector<Variables> variables;
		ui32 size;
		ui64 featureCode;

		void AE_CALL calcFeatureCode();

		void AE_CALL collectUsingInfo(const ShaderParameterFactory& factory, ShaderParameterUsageStatistics& statistics,
			std::vector<const ShaderParameter*>& usingParams, std::vector<const ConstantBufferLayout::Variables*>& usingVars) const;

	private:
		void AE_CALL _calcFeatureCode(const Variables& var, ui16& numValidVars);

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
		std::function<IConstantBuffer*(ui32 numParameters)> createExclusiveConstantBufferCallback = nullptr;

		void AE_CALL registerConstantLayout(ConstantBufferLayout& layout);
		void AE_CALL unregisterConstantLayout(ConstantBufferLayout& layout);

		IConstantBuffer* AE_CALL popShareConstantBuffer(ui32 size);
		void AE_CALL resetUsedShareConstantBuffers();

		IConstantBuffer* AE_CALL getExclusiveConstantBuffer(const std::vector<ShaderParameter*>& parameters, const ConstantBufferLayout& layout);

		static void AE_CALL updateConstantBuffer(IConstantBuffer* cb, const ShaderParameter& param, const ConstantBufferLayout::Variables& var);

	private:
		struct ShareConstBuffers {
			ui32 rc;
			ui32 idleIndex;
			std::vector<IConstantBuffer*> buffers;
		};


		std::unordered_map<ui32, ShareConstBuffers> _shareConstBufferPool;
		std::unordered_set<ui32> _usedShareConstBufferPool;

		void AE_CALL _registerShareConstantLayout(ui32 size);
		void AE_CALL _unregisterShareConstantLayout(ui32 size);


		struct ExclusiveConstNode {
			ExclusiveConstNode() :
				numAssociativeBuffers(0),
				parameter(nullptr),
				parent(nullptr) {
			}

			ui32 numAssociativeBuffers;
			std::unordered_map<const ShaderParameter*, ExclusiveConstNode> children;
			std::unordered_map<ui64, IConstantBuffer*> buffers;
			ShaderParameter* parameter;
			ExclusiveConstNode* parent;
		};


		std::unordered_map<const ShaderParameter*, ExclusiveConstNode> _exclusiveConstRoots;
		std::unordered_map<const ShaderParameter*, std::unordered_set<ExclusiveConstNode*>> _exclusiveConstNodes;


		struct ExcLusiveConsts {
			ui32 rc;
			std::unordered_set<ExclusiveConstNode*> nodes;
		};


		std::unordered_map<ui64, ExcLusiveConsts> _exclusiveConstPool;


		IConstantBuffer* AE_CALL _getExclusiveConstantBuffer(const ConstantBufferLayout& layout, const std::vector<ShaderParameter*>& parameters,
			ui32 cur, ui32 max, ExclusiveConstNode* parent, std::unordered_map <const ShaderParameter*, ExclusiveConstNode>& chindrenContainer);
		void AE_CALL _registerExclusiveConstantLayout(ConstantBufferLayout& layout);
		void AE_CALL _unregisterExclusiveConstantLayout(ConstantBufferLayout& layout);
		static void _releaseExclusiveConstant(void* target, const ShaderParameter& param);
		void _releaseExclusiveConstant(const ShaderParameter& param);
		void _releaseExclusiveConstantToRoot(ExclusiveConstNode* parent, ExclusiveConstNode* releaseChild, ui32 releaseNumAssociativeBuffers, bool releaseParam);
		void _releaseExclusiveConstantToLeaf(ExclusiveConstNode& node, bool releaseParam);
		void _releaseExclusiveConstantSelf(ExclusiveConstNode& node, bool releaseParam);
	};
}