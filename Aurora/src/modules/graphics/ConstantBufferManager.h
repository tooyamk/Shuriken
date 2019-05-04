#pragma once

#include "base/LowLevel.h"
#include <functional>
#include <unordered_map>
#include <unordered_set>

namespace aurora::modules::graphics {
	class IConstantBuffer;
	class IGraphicsModule;
	class ShaderParameter;


	struct AE_DLL ConstantBufferLayout {
		struct AE_DLL Variables {
			std::string name;
			ui32 offset;
			ui32 size;
			std::vector<Variables> structMembers;
		};

		std::string name;
		ui32 bindPoint;
		std::vector<Variables> variables;
		ui32 size;
		ui64 featureCode;
	};


	class AE_DLL ConstantBufferManager {
	public:
		ConstantBufferManager(IGraphicsModule* graphics);
		~ConstantBufferManager();

		std::function<void(IConstantBuffer* buffer, ui32 numParameters)> createdExclusiveConstantBufferCallback = nullptr;

		void AE_CALL registerConstantLayout(ConstantBufferLayout& layout);
		void AE_CALL unregisterConstantLayout(ConstantBufferLayout& layout);

		IConstantBuffer* AE_CALL popShareConstantBuffer(ui32 size);
		void AE_CALL resetUsedShareConstantBuffers();

		IConstantBuffer* AE_CALL getExclusiveConstantBuffer(const std::vector<ShaderParameter*>& parameters, const ConstantBufferLayout& layout);

	private:
		IGraphicsModule* _graphics;


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