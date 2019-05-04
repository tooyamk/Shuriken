#pragma once

#include "base/Ref.h"
#include <unordered_map>

namespace aurora::modules::graphics {
	class IVertexBuffer;


	class AE_DLL VertexBufferFactory : public Ref {
	public:
		~VertexBufferFactory();

		IVertexBuffer* AE_CALL get(const std::string& name) const;
		void AE_CALL add(const std::string& name, IVertexBuffer* buffer);
		void AE_CALL remove(const std::string& name);
		void AE_CALL clear();

	private:
		std::unordered_map<std::string, IVertexBuffer*> _buffers;
	};
}