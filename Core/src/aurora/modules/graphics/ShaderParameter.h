#pragma once

#include "aurora/Ref.h"
#include "aurora/math/Vector.h"

namespace aurora::modules::graphics {
	class ISampler;
	class ITextureView;
	class ShaderParameterFactory;


	enum class ShaderParameterUsage : uint8_t {
		AUTO,
		SHARE,
		EXCLUSIVE
	};


	enum class ShaderParameterType : uint8_t {
		DATA,
		SAMPLER,
		TEXTURE,
		FACTORY
	};


	class AE_DLL ShaderParameter : public Ref {
	public:
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
		inline ShaderParameter& AE_CALL set(const ISampler* value) {
			return set(value, sizeof(value), sizeof(value), ShaderParameterType::SAMPLER, false);
		}
		inline ShaderParameter& AE_CALL set(const ITextureView* value) {
			return set(value, sizeof(value), sizeof(value), ShaderParameterType::TEXTURE, false);
		}
		inline ShaderParameter& AE_CALL set(const ShaderParameterFactory* value) {
			return set(value, sizeof(value), sizeof(value), ShaderParameterType::FACTORY, false);
		}
		ShaderParameter& AE_CALL set(const void* data, uint32_t size, uint16_t perElementSize, ShaderParameterType type, bool copy);

		void AE_CALL clear();

	ae_internal_public:
		using EXCLUSIVE_FN = void(*)(void* callTarget, const ShaderParameter& param);
		void AE_CALL __setExclusive(void* callTarget, EXCLUSIVE_FN callback);
		void AE_CALL __releaseExclusive(void* callTarget, EXCLUSIVE_FN callback);

	private:
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
			uint8_t data[sizeof(f32) << 2];

			struct {
				uint8_t* internalData;
				uint32_t internalSize;
			};

			struct {
				bool externalRef;
				const void* externalData;
			};
		} _data;
		static const uint32_t DEFAULT_DATA_SIZE = sizeof(Data);

		uint32_t _exclusiveRc;
		void* _exclusiveFnTarget;
		EXCLUSIVE_FN _exclusiveFn;
	};
}