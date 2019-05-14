#pragma once

#include "modules/IInputModule.h"
#include "base/Application.h"

#include <Xinput.h>
#pragma comment(lib, "Xinput.lib")

namespace aurora::modules::win_xinput {
	struct GUID {
		const i8 head[6] = { 'X', 'I', 'n', 'p', 'u', 't' };
		ui8 index = 0;
	};
}