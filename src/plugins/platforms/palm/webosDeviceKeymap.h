
// Copyright 2008-2009 Palm Inc.

#ifndef __webosDeviceKeymap_h__
#define __webosDeviceKeymap_h__

typedef struct {
	int devicekey;
	int normal;			// keycode if no modifiers
	int shift;			// keycode if shift
	int opt;			// keycode if opt/sym
	int qtKey;
	int qtKeyOpt;
	int virtualkeycode; // VK_
} KeyMapType;

#define LAST_KEY			(0xffffffff)

typedef enum KeyLayoutType {
	KeyLayoutTypeQwerty,
	KeyLayoutTypeAzerty,
	KeyLayoutTypeQwertz,
	KeyLayoutTypeQwertzDe,
	KeyLayoutTypeAzertyFr,
	KeyLayoutTypeQwertyEvt1,
} KeyLayoutType;

enum KeyMapDevice {
	DeviceCastle,
	DevicePixie,
	DeviceWindsor,
	DeviceBroadway,
	DeviceGtk,
	DeviceQemu,
	DeviceMantaray,

	DeviceLast
};


#endif // __webosDeviceKeymap_h__

