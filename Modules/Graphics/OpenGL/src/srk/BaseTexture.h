#pragma once

#include "Base.h"
#include "BaseBuffer.h"

namespace srk::modules::graphics::gl {
	class Graphics;

	class SRK_MODULE_DLL BaseTexture {
	private:
		class PixelBuffer {
		public:
			PixelBuffer();

			bool SRK_CALL create(Graphics& graphics, size_t size, Usage requiredUsage, Usage preferredUsage, const void* data = nullptr, size_t dataSize = 0);
			Usage SRK_CALL map(Usage expectMapUsage);
			void SRK_CALL unmap();
			size_t SRK_CALL read(void* dst, size_t dstLen, size_t offset);
			size_t SRK_CALL write(const void* data, size_t length, size_t offset);
			bool SRK_CALL copyFrom(uint32_t mipSlice, const BaseTexture* src);
			void SRK_CALL copyFrom(Graphics& graphics, const PixelBuffer* src);

			inline GLenum SRK_CALL getBufferType() const {
				return _baseBuffer.bufferType;
			}

			inline GLuint SRK_CALL getHandle() const {
				return _baseBuffer.handle;
			}

		protected:
			BaseBuffer _baseBuffer;
		};

	public:
		BaseTexture(TextureType texType);
		virtual ~BaseTexture();
		
		bool SRK_CALL create(Graphics& graphics, const Vec3uz& dim, size_t arraySize, size_t mipLevels, SampleCount sampleCount,
			TextureFormat format, Usage requiredUsage, Usage preferredUsage, const void*const* data = nullptr);
		Usage SRK_CALL map(size_t arraySlice, size_t mipSlice, Usage expectMapUsage);
		void SRK_CALL unmap(size_t arraySlice, size_t mipSlice);
		size_t SRK_CALL read(Graphics& graphics, size_t arraySlice, size_t mipSlice, size_t offset, void* dst, size_t dstLen);
		size_t SRK_CALL write(Graphics& graphics, size_t arraySlice, size_t mipSlice, size_t offset, const void* data, size_t length);
		bool SRK_CALL update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data);
		bool SRK_CALL copyFrom(Graphics& graphics, const Vec3uz& dstPos, size_t dstArraySlice, size_t dstMipSlice, const ITextureResource* src, size_t srcArraySlice, size_t srcMipSlice, const Box3uz& srcRange);
		void SRK_CALL flush();
		void SRK_CALL releaseTex();
		void SRK_CALL waitServerSync();
		void SRK_CALL releaseSync();

		inline static constexpr size_t SRK_CALL calcSubresource(size_t mipSlice, size_t arraySlice, size_t mipLevels) {
			return mipSlice + arraySlice * mipLevels;
		}

		bool dirty;
		SampleCount sampleCount;
		TextureType texType;
		TextureFormat format;
		Usage resUsage;
		Usage mapUsage;
		Vec3uz dim;
		size_t arraySize;
		size_t internalArraySize;
		size_t mipLevels;

		struct {
			GLenum target;
			GLenum internalFormat;
			GLenum format;
			GLenum type;
		} glTexInfo;

		size_t size;
		GLuint handle;

		//GLenum internalFormat;

		GLsync sync;

		struct MapData {
			Usage usage;
			bool inited;
			size_t size;
			size_t arraySlice;
			size_t mipSlice;
			Box3uz range;
			PixelBuffer* buffer;
		};

		std::vector<MapData> mapData;

	private:
		bool SRK_CALL _usageConflicted(Usage requiredUsage, Usage& preferredUsage, Usage conflictedUsage1, Usage conflictedUsage2);

		inline void SRK_CALL _unmap(size_t subresource, bool discard) {
			_unmap(mapData[subresource], discard);
		}
		void SRK_CALL _unmap(MapData& md, bool discard);
		bool SRK_CALL _update(size_t arraySlice, size_t mipSlice, const Box3uz& range, const void* data);
		bool SRK_CALL _createDone(Graphics& graphics, bool succeeded);
	};
}