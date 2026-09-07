#import <Cocoa/Cocoa.h>

bool isFocused() {
	NSOpenGLContext* ctx = [NSOpenGLContext currentContext];
	if (!ctx) return false;

	NSView* view = [ctx view];
	if (!view) return false;

	NSWindow* glWindow = [view window];
	if (!glWindow) return false;

	NSWindow* activeWindow = [NSApp keyWindow];
	if (!activeWindow) activeWindow = [NSApp mainWindow];
	if (!activeWindow) return false;

	return glWindow == activeWindow;
}
