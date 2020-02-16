#pragma once

#include "aurora/Ref.h"
#include "aurora/math/Vector.h"
#include <unordered_map>

namespace aurora::modules::graphics {
	class ISampler;
	class ITextureView;
}

namespace aurora {
	class IShaderParameterGetter;


	enum class ShaderParameterUsage : uint8_t {
		AUTO,
		SHARE,
		EXCLUSIVE
	};


	enum class ShaderParameterType : uint8_t {
		DATA,
		SAMPLER,
		TEXTURE,
		GETTER
	};


	class AE_DLL ShaderParameter : public Ref {
	public:
		using EXCLUSIVE_FN = void(*)(void* data, const ShaderParameter& param);

		ShaderParameter(ShaderParameterUsage usage = ShaderParameterUsage::AUTO);
		~ShaderParameter();

		void AE_CALL releaseExclusiveBuffers();

		inline ShaderParameterUsage AE_CALL getUsage() const {
			return _usage;
		}

		void AE_CALL setUsage(ShaderParameterUsage usage);

		inline ShaderParameterType AE_CALL getType() const {
			return _type;
		}

		inline const void* AE_CALL getData() const {
			return _storageType == StorageType::DEFAULT ? &_data : _data.externalData;
		}

		inline uint16_t AE_CALL getPerElementSize() const {
			return _perElementSize;
		}

		inline uint32_t AE_CALL getSize() const {
			return _size;
		}

		inline uint32_t AE_CALL getUpdateId() const {
			return _updateId;
		}

		inline ShaderParameter& AE_CALL setUpdated() {
			++_updateId;
			return *this;
		}

		template<typename T>
		inline ShaderParameter& AE_CALL set(Math::NumberType<T> value) {
			return set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true);
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(Math::NumberType<T>* value, uint32_t size, uint16_t perElementSize, bool copy) {
			return set(value, size, perElementSize, ShaderParameterType::DATA, copy);
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(const Vec2<T>& value) {
			return set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true);
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(const Vec3<T>& value) {
			return set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true);
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(const Vec4<T>& value) {
			return set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true);
		}
		inline ShaderParameter& AE_CALL set(const modules::graphics::ISampler* value) {
			return set(value, sizeof(value), sizeof(value), ShaderParameterType::SAMPLER, false);
		}
		inline ShaderParameter& AE_CALL set(const modules::graphics::ITextureView* value) {
			return set(value, sizeof(value), sizeof(value), ShaderParameterType::TEXTURE, false);
		}
		inline ShaderParameter& AE_CALL set(const IShaderParameterGetter* value) {
			return set(value, sizeof(value), sizeof(value), ShaderParameterType::GETTER, false);
		}
		ShaderParameter& AE_CALL set(const void* data, uint32_t size, uint16_t perElementSize, ShaderParameterType type, bool copy);

		void AE_CALL addReleaseExclusiveHandler(void* data, EXCLUSIVE_FN callback);
		bool AE_CALL removeReleaseExclusiveHandler(void* data, EXCLUSIVE_FN callback);

		void AE_CALL clear();

	private:
		struct HandlerNode {
			HandlerNode() :
				next(nullptr),
				data(nullptr),
				callback(nullptr) {
			}
			HandlerNode(void* data, EXCLUSIVE_FN callback) :
				next(nullptr),
				data(data),
				callback(callback) {
			}

			inline void AE_CALL set(HandlerNode& node) {
				next = node.next;
				data = node.data;
				callback = node.callback;
			}

			HandlerNode* next;
			void* data;
			EXCLUSIVE_FN callback;
		};


		enum class StorageType : uint8_t {
			DEFAULT,
			INTERNAL,
			EXTERNAL
		};


		ShaderParameterUsage _usage;
		ShaderParameterType _type;
		StorageType _storageType;
		uint16_t _perElementSize;
		uint32_t _updateId;
		uint32_t _size;
		union Data {
			uint8_t data[sizeof(uintptr_t)];

			struct {
				uint8_t* internalData;
				uint32_t internalSize;
			};

			struct {
				bool externalRef;
				const void* externalData;
			};
		} _data;
		static constexpr uint32_t DEFAULT_DATA_SIZE = sizeof(Data);

		HandlerNode _handlers;
	};


	class AE_DLL IShaderParameterGetter : public Ref {
	public:
		virtual ~IShaderParameterGetter() {}

		virtual ShaderParameter* AE_CALL get(const std::string& name) const = 0;
		virtual ShaderParameter* AE_CALL get(const std::string& name, ShaderParameterType type) const = 0;
	};


	class AE_DLL ShaderParameterCollection : public IShaderParameterGetter {
	public:
		virtual ~ShaderParameterCollection() {}

		virtual ShaderParameter* AE_CALL get(const std::string& name) const override;
		virtual ShaderParameter* AE_CALL get(const std::string& name, ShaderParameterType type) const override;

		ShaderParameter* AE_CALL add(const std::string& name, ShaderParameter* parameter);
		inline void AE_CALL remove(const std::string& name) {
			if (auto itr = _parameters.find(name); itr != _parameters.end()) _parameters.erase(itr);
		}
		inline bool AE_CALL isEmpty() const {
			return _parameters.empty();
		}
		inline void AE_CALL clear() {
			_parameters.clear();
		}

	protected:
		std::unordered_map<std::string, RefPtr<ShaderParameter>> _parameters;
	};


	class AE_DLL ShaderParameterGetterStack : public IShaderParameterGetter {
	public:
		virtual ~ShaderParameterGetterStack() {}

		virtual ShaderParameter* AE_CALL get(const std::string& name) const override;
		virtual ShaderParameter* AE_CALL get(const std::string& name, ShaderParameterType type) const override;

		inline bool AE_CALL push(IShaderParameterGetter* getter) {
			if (getter) {
				_stack.emplace_back(getter);
				return true;
			}
			return false;
		}
		inline bool AE_CALL push(IShaderParameterGetter& getter) {
			_stack.emplace_back(&getter);
			return true;
		}
		inline void AE_CALL pop() {
			_stack.pop_back();
		}
		inline void AE_CALL pop(size_t count) {
			_stack.erase(_stack.end() - count, _stack.end());
		}
		inline void AE_CALL clear() {
			_stack.clear();
		}

	protected:
		std::vector<RefPtr<IShaderParameterGetter>> _stack;
	};
}