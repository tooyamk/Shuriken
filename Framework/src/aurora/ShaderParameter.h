#pragma once

#include "aurora/Ref.h"
#include "aurora/math/Matrix.h"
#include "aurora/math/Vector.h"
#include <unordered_map>
#include <vector>

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


	enum class ShaderParameterUpdateBehavior : uint8_t {
		NOT,
		CHECK,
		FORCE
	};


	class AE_FW_DLL ShaderParameter : public Ref {
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
		inline ShaderParameter& AE_CALL set(arithmetic_t<T> value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(arithmetic_t<T>* value, uint32_t size, uint16_t perElementSize, bool copy, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(value, size, perElementSize, ShaderParameterType::DATA, copy, updateBehavior);
			return *this;
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(const Vec2<T>& value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(const Vec3<T>& value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		template<typename T>
		inline ShaderParameter& AE_CALL set(const Vec4<T>& value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		inline ShaderParameter& AE_CALL set(const Matrix34& value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		inline ShaderParameter& AE_CALL set(const Matrix44& value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		inline ShaderParameter& AE_CALL set(const modules::graphics::ISampler* value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(value, sizeof(value), sizeof(value), ShaderParameterType::SAMPLER, false, updateBehavior);
			return *this;
		}
		inline ShaderParameter& AE_CALL set(const modules::graphics::ITextureView* value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(value, sizeof(value), sizeof(value), ShaderParameterType::TEXTURE, false, updateBehavior);
			return *this;
		}
		inline ShaderParameter& AE_CALL set(const IShaderParameterGetter* value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(value, sizeof(value), sizeof(value), ShaderParameterType::GETTER, false, updateBehavior);
			return *this;
		}
		inline ShaderParameter& AE_CALL set(const void* data, uint32_t size, uint16_t perElementSize, ShaderParameterType type, bool copy, ShaderParameterUpdateBehavior updateBehavior) {
			_set(data, size, perElementSize, type, copy, updateBehavior);
			return *this;
		}

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
				const void* externalData;
				bool externalRef;
			};
		} _data;
		static constexpr uint32_t DEFAULT_DATA_SIZE = sizeof(Data);

		HandlerNode _handlers;

		void AE_CALL _set(const void* data, uint32_t size, uint16_t perElementSize, ShaderParameterType type, bool copy, ShaderParameterUpdateBehavior updateBehavior);
	};


	class AE_FW_DLL IShaderParameterGetter : public Ref {
	public:
		virtual ~IShaderParameterGetter() {}

		virtual RefPtr<ShaderParameter> AE_CALL get(const query_string& name) const = 0;
		virtual RefPtr<ShaderParameter> AE_CALL get(const query_string& name, ShaderParameterType type) const = 0;
	};


	class AE_FW_DLL ShaderParameterCollection : public IShaderParameterGetter {
	public:
		virtual ~ShaderParameterCollection() {}

		virtual RefPtr<ShaderParameter> AE_CALL get(const query_string& name) const override;
		virtual RefPtr<ShaderParameter> AE_CALL get(const query_string& name, ShaderParameterType type) const override;

		ShaderParameter* AE_CALL set(const query_string& name, ShaderParameter* parameter);
		inline RefPtr<ShaderParameter> AE_CALL remove(const query_string& name) {
			return _remove(name);
		}
		inline bool AE_CALL isEmpty() const {
			return _parameters.empty();
		}
		inline void AE_CALL clear() {
			_parameters.clear();
		}

	protected:
		string_unordered_map<RefPtr<ShaderParameter>> _parameters;

		ShaderParameter* AE_CALL _remove(const query_string& name);
	};


	class AE_FW_DLL ShaderParameterGetterStack : public IShaderParameterGetter {
	public:
		virtual ~ShaderParameterGetterStack() {}

		virtual RefPtr<ShaderParameter> AE_CALL get(const query_string& name) const override;
		virtual RefPtr<ShaderParameter> AE_CALL get(const query_string& name, ShaderParameterType type) const override;

		inline bool AE_CALL push(IShaderParameterGetter* getter) {
			if (getter) {
				_stack.emplace_back(getter);
				return true;
			}
			return false;
		}
		template<typename... Args, typename = std::enable_if_t<std::conjunction_v<std::is_base_of<IShaderParameterGetter, Args>...>>>
		inline size_t AE_CALL push(Args*... args) {
			size_t n = 0;
			((_push(n, args)), ...);
			return n;
		}
		inline bool AE_CALL push(IShaderParameterGetter& getter) {
			_stack.emplace_back(&getter);
			return true;
		}
		template<typename... Args, typename = std::enable_if_t<std::conjunction_v<std::is_base_of<IShaderParameterGetter, Args>...>>>
		inline size_t AE_CALL push(Args&&... args) {
			((_stack.emplace_back(args)), ...);
			return sizeof...(args);
		}

		inline void AE_CALL pop() {
			_stack.pop_back();
		}
		inline void AE_CALL pop(size_t count) {
			if (count) _stack.erase(_stack.end() - count, _stack.end());
		}

		inline void AE_CALL clear() {
			_stack.clear();
		}

	protected:
		std::vector<RefPtr<IShaderParameterGetter>> _stack;

		inline void AE_CALL _push(size_t& n, IShaderParameterGetter* getter) {
			if (getter) {
				_stack.emplace_back(getter);
				++n;
			}
		}
	};
}