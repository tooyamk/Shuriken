#pragma once

#include "srk/Intrusive.h"

namespace srk {
	class SRK_FW_DLL Application : public Ref {
	public:
		Application(const std::string_view& appId, void* native = nullptr, const std::filesystem::path& appPath = std::filesystem::path());
		Application(const std::u8string_view& appId, void* native = nullptr, const std::filesystem::path& appPath = std::filesystem::path()) : Application((const std::string_view&)appId, native, appPath) {}
		virtual ~Application() {};

		inline void* SRK_CALL getNative() const {
			return _native;
		}

		inline void SRK_CALL shutdown() const {
			std::exit(0);
		}

		inline std::string_view SRK_CALL getIdentity() const {
			return _id;
		}

		inline const std::filesystem::path& SRK_CALL getPath() const {
			return _path;
		}

	private:
		std::string _id;
		void* _native;
		std::filesystem::path _path;
	};
}