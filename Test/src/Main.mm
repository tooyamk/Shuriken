#include "srk/predefine/OS.h"

#if SRK_OS == SRK_OS_MACOS
#   import <Cocoa/Cocoa.h>
#   include "Entry.h"

@interface AppDelegate : NSObject<NSApplicationDelegate>
@end

@implementation AppDelegate
@end

int32_t main() {
    [NSApplication sharedApplication];
    [NSApp setDelegate:[AppDelegate new]];
    [NSApp finishLaunching];
    
	Enttry e;
	return e.run();
}
#endif
