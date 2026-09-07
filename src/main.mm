#import <Cocoa/Cocoa.h>

bool isFocused() {
	return NSApp != nil && [NSApp isActive];
}
