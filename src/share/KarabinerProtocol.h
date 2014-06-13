// -*- Mode: objc; Coding: utf-8; indent-tabs-mode: nil; -*-

#import <Cocoa/Cocoa.h>

@protocol KarabinerProtocol
- (int) value:(NSString*)name;
- (int) defaultValue:(NSString*)name;
- (void) setValue:(int)newval forName:(NSString*)name;

- (NSDictionary*) changed;

- (NSInteger)     configlist_selectedIndex;
- (NSArray*)      configlist_getConfigList;
- (void)          configlist_select:(NSInteger)newindex;

- (void) configxml_reload;
- (NSString*) symbolMapName:(NSString*)type value:(NSInteger)value;

- (void) relaunch;

// for AXNotifier
- (void) updateFocusedUIElementInformation:(NSDictionary*)information;

// For EventViewer.
- (NSArray*) device_information:(NSInteger)type;
- (NSDictionary*) focused_uielement_information;
- (NSDictionary*) inputsource_information;

@end
