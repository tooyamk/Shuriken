#pragma once

#include "srk/Intrusive.h"
#include "srk/math/Matrix.h"
#include "srk/math/Vector.h"
#include <unordered_map>
#include <vector>

namespace srk::modules::graphics {
	class ISampler;
	class ITextureView;
}

namespace srk {
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


	class SRK_FW_DLL ShaderParameter {
		SRK_REF_OBJECT(ShaderParameter)
	public:
		using EXCLUSIVE_FN = void(*)(void* data, const ShaderParameter& param);

		ShaderParameter(ShaderParameterUsage usage = ShaderParameterUsage::AUTO);
		~ShaderParameter();

		void SRK_CALL releaseExclusiveBuffers();

		inline ShaderParameterUsage SRK_CALL getUsage() const {
			return _usage;
		}

		void SRK_CALL setUsage(ShaderParameterUsage usage);

		inline ShaderParameterType SRK_CALL getType() const {
			return _type;
		}

		inline const void* SRK_CALL getData() const {
			return _storageType == StorageType::DEFAULT ? &_data : _data.externalData;
		}

		inline uint16_t SRK_CALL getPerElementSize() const {
			return _perElementSize;
		}

		inline uint32_t SRK_CALL getSize() const {
			return _size;
		}

		inline uint32_t SRK_CALL getUpdateId() const {
			return _updateId;
		}

		inline ShaderParameter& SRK_CALL setUpdated() {
			++_updateId;
			return *this;
		}

		template<typename T>
		inline ShaderParameter& SRK_CALL set(ArithmeticType<T> value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		template<typename T>
		inline ShaderParameter& SRK_CALL set(ArithmeticType<T>* value, uint32_t size, uint16_t perElementSize, bool copy, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(value, size, perElementSize, ShaderParameterType::DATA, copy, updateBehavior);
			return *this;
		}
		template<typename T>
		inline ShaderParameter& SRK_CALL set(const Vec2<T>& value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		template<typename T>
		inline ShaderParameter& SRK_CALL set(const Vec3<T>& value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		template<typename T>
		inline ShaderParameter& SRK_CALL set(const Vec4<T>& value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		inline ShaderParameter& SRK_CALL set(const Matrix34& value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		inline ShaderParameter& SRK_CALL set(const Matrix44& value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(&value, sizeof(value), sizeof(value), ShaderParameterType::DATA, true, updateBehavior);
			return *this;
		}
		inline ShaderParameter& SRK_CALL set(const modules::graphics::ISampler* value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(value, sizeof(value), sizeof(value), ShaderParameterType::SAMPLER, false, updateBehavior);
			return *this;
		}
		inline ShaderParameter& SRK_CALL set(const modules::graphics::ITextureView* value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(value, sizeof(value), sizeof(value), ShaderParameterType::TEXTURE, false, updateBehavior);
			return *this;
		}
		inline ShaderParameter& SRK_CALL set(const IShaderParameterGetter* value, ShaderParameterUpdateBehavior updateBehavior = ShaderParameterUpdateBehavior::CHECK) {
			_set(value, sizeof(value), sizeof(value), ShaderParameterType::GETTER, false, updateBehavior);
			return *this;
		}
		inline ShaderParameter& SRK_CALL set(const void* data, uint32_t size, uint16_t perElementSize, ShaderParameterType type, bool copy, ShaderParameterUpdateBehavior updateBehavior) {
			_set(data, size, perElementSize, type, copy, updateBehavior);
			return *this;
		}

		void SRK_CALL addReleaseExclusiveHandler(void* data, EXCLUSIVE_FN callback);
		bool SRK_CALL removeReleaseExclusiveHandler(void* data, EXCLUSIVE_FN callback);

		void SRK_CALL clear();

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

			inline void SRK_CALL set(HandlerNode& node) {
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
			uint8_t data[sizeof(float32_t) * 4];

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

		void SRK_CALL _set(const void* data, uint32_t size, uint16_t perElementSize, ShaderParameterType type, bool copy, ShaderParameterUpdateBehavior updateBehavior);
	};


	class SRK_FW_DLL IShaderParameterGetter : public Ref {
	public:
		virtual ~IShaderParameterGetter() {}

		virtual IntrusivePtr<ShaderParameter> SRK_CALL get(const QueryString& name) const = 0;
		virtual IntrusivePtr<ShaderParameter> SRK_CALL get(const QueryString& name, ShaderParameterType type) const = 0;
	};


	class SRK_FW_DLL ShaderParameterCollection : public IShaderParameterGetter {
	public:
		virtual ~ShaderParameterCollection() {}

		virtual IntrusivePtr<ShaderParameter> SRK_CALL get(const QueryString& name) const override;
		virtual IntrusivePtr<ShaderParameter> SRK_CALL get(const QueryString& name, ShaderParameterType type) const override;

		ShaderParameter* SRK_CALL set(const QueryString& name, ShaderParameter* parameter);
		inline IntrusivePtr<ShaderParameter> SRK_CALL remove(const QueryString& name) {
			return _remove(name);
		}
		inline bool SRK_CALL isEmpty() const {
			return _parameters.empty();
		}
		inline void SRK_CALL clear() {
			_parameters.clear();
		}

	protected:
		StringUnorderedMap<IntrusivePtr<ShaderParameter>> _parameters;

		ShaderParameter* SRK_CALL _remove(const QueryString& name);
	};


	class SRK_FW_DLL ShaderParameterGetterStack : public IShaderParameterGetter {
	public:
		virtual ~ShaderParameterGetterStack() {}

		virtual IntrusivePtr<ShaderParameter> SRK_CALL get(const QueryString& name) const override;
		virtual IntrusivePtr<ShaderParameter> SRK_CALL get(const QueryString& name, ShaderParameterType type) const override;

		inline bool SRK_CALL push(IShaderParameterGetter* getter) {
			if (getter) {
				_stack.emplace_back(getter);
				return true;
			}
			return false;
		}
		template<std::derived_from<IShaderParameterGetter>... Args>
		inline size_t SRK_CALL push(Args*... args) {
			size_t n = 0;
			((_push(n, args)), ...);
			return n;
		}
		inline bool SRK_CALL push(IShaderParameterGetter& getter) {
			_stack.emplace_back(&getter);
			return true;
		}
		template<std::derived_from<IShaderParameterGetter>... Args>
		inline size_t SRK_CALL push(Args&&... args) {
			((_stack.emplace_back(args)), ...);
			return sizeof...(args);
		}

		inline void SRK_CALL pop() {
			_stack.pop_back();
		}
		inline void SRK_CALL pop(size_t count) {
			if (count) _stack.erase(_stack.end() - count, _stack.end());
		}

		inline void SRK_CALL clear() {
			_stack.clear();
		}

	protected:
		std::vector<IntrusivePtr<IShaderParameterGetter>> _stack;

		inline void SRK_CALL _push(size_t& n, IShaderParameterGetter* getter) {
			if (getter) {
				_stack.emplace_back(getter);
				++n;
			}
		}
	};
}