#pragma once

#include "srk/Intrusive.h"

namespace srk {
	class SRK_FW_DLL Application : public Ref {
	public:
		Application(const std::string_view& appId, uint32_t pid = 0, const std::filesystem::path& appPath = std::filesystem::path());
		Application(const std::u8string_view& appId, uint32_t pid = 0, const std::filesystem::path& appPath = std::filesystem::path()) : Application((const std::string_view&)appId, pid, appPath) {}
		virtual ~Application() {};

		inline void* SRK_CALL getNative() const {
			return _native;
		}

		void SRK_CALL terminate(int32_t code = 0) const;

		inline std::string_view SRK_CALL getApplicationId() const {
			return _appId;
		}

		inline uint32_t SRK_CALL getProcessId() const {
			return _pid;
		}

		inline const std::filesystem::path& SRK_CALL getPath() const {
			return _path;
		}

	private:
		std::string _appId;
		void* _native;
		uint32_t _pid;
		std::filesystem::path _path;
	};
}