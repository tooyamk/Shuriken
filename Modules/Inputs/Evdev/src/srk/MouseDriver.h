#pragma once

#include "Base.h"
#include "srk/Lock.h"
#include "srk/modules/inputs/GenericMouse.h"
#include "srk/DynamicLibraryLoader.h"
//#include <X11/Xlib.h>

namespace srk::modules::inputs::evdev_input {
	class Input;

	class SRK_MODULE_DLL MouseDriver : public IGenericMouseDriver {
	public:
		virtual ~MouseDriver();

		static MouseDriver* SRK_CALL create(Input& input, int32_t fd);

		virtual std::optional<bool> SRK_CALL readFromDevice(GenericMouseBuffer& buffer) const override;
		virtual void SRK_CALL close() override;

	private:
		MouseDriver(Input& input, int32_t fd);

		int32_t _fd;

		mutable AtomicLock _lock;
		mutable GenericMouseBuffer _inputBuffer;

		static uint32_t _instanceCount;
		static std::shared_mutex _libMutex;
		static DynamicLibraryLoader _libLoader;

		using XID = unsigned long;

		struct XScreen {
			void* data1[2];
			XID root;
			int data2[5];
			void* data3;
			int data4;
			void* data5[2];
			XID data6;
			unsigned long data7[2];
			int data8[4];
			long data9;
		};

		struct XDisplay {
			void* data1[2];
			int data2[4];
			void* data3;
			XID data4[3];
			int data5;
			void* data6;
			int data7[5];
			void* data8;
			int data9[2];
			void* data10[2];
			int data11;
			unsigned long data12[2];
			void* data13[4];
			unsigned data14;
			void* data15[3];
			int data16;
			int nscreens;
			XScreen* screens;
			unsigned long data17[2];
			int data18[2];
			void* data19[2];
			int data20;
			void* data21;
		};

		//using XID = ::Window;
		//using XDisplay = ::Display;

		using XOpenDisplayFn = XDisplay*(*)(const char*);
		using XCloseDisplayFn = int(*)(XDisplay*);
		using XQueryPointerFn = int(*)(XDisplay*, XID, XID*, XID*, int*, int*, int*, int*, unsigned int*);

		static XOpenDisplayFn _XOpenDisplayFn;
		static XCloseDisplayFn _XCloseDisplayFn;
		static XQueryPointerFn _XQueryPointerFn;
		static XDisplay* _xdisplay;

		static std::optional<Vec2f32> _getMousePos();
	};
}