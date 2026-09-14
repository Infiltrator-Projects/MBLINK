// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Private model implementations for the Mercedes diagnostics controller.
 * This file is included by MBLinkDiagnosticsController.m so the Xcode target
 * keeps one translation unit and no project-file wiring is required.
 */
@interface MBLinkStandardDataSnapshot ()
@property(nonatomic, readwrite) uint8_t pid;
@property(nonatomic, readwrite) uint32_t responderCANIdentifier;
@property(nonatomic, readwrite, getter=isExtendedID) BOOL extendedID;
@property(nonatomic, readwrite) NSUInteger valueKind;
@property(nonatomic, readwrite) NSUInteger signalCount;
@property(nonatomic, copy, readwrite) NSString *formattedValue;
@property(nonatomic, copy, readwrite) NSString *rawHex;
@end
@implementation MBLinkStandardDataSnapshot
@end

@interface MBLinkMercedesDataSnapshot ()
@property(nonatomic, readwrite) uint16_t identifier;
@property(nonatomic, readwrite) uint8_t service;
@property(nonatomic, copy, readwrite) NSString *codeText;
@property(nonatomic, copy, readwrite, nullable) NSString *name;
@property(nonatomic, copy, readwrite, nullable) NSString *unit;
@property(nonatomic, copy, readwrite) NSString *formattedValue;
@property(nonatomic, copy, readwrite) NSString *rawHex;
@property(nonatomic, copy, readwrite) NSData *rawData;
@property(nonatomic, readwrite, getter=isMapped) BOOL mapped;
@property(nonatomic, readwrite, getter=isNumericValueAvailable)
    BOOL numericValueAvailable;
@property(nonatomic, readwrite) double numericValue;
@end

@implementation MBLinkMercedesDataSnapshot
@end

@interface MBLinkTransmissionLiveValueSnapshot ()
@property(nonatomic, copy, readwrite) NSString *identifier;
@property(nonatomic, readwrite) uint16_t localIdentifier;
@property(nonatomic, copy, readwrite) NSString *shortName;
@property(nonatomic, copy, readwrite) NSString *title;
@property(nonatomic, copy, readwrite) NSString *suffix;
@property(nonatomic, copy, readwrite) NSString *formattedValue;
@property(nonatomic, readwrite, getter=isNumericValueAvailable) BOOL numericValueAvailable;
@property(nonatomic, readwrite) double numericValue;
@property(nonatomic, copy, readwrite) NSString *rawHex;
@property(nonatomic, readwrite, getter=isPollingEnabled) BOOL pollingEnabled;
@property(nonatomic, copy, readwrite) NSString *qualityNote;
@end
@implementation MBLinkTransmissionLiveValueSnapshot
@end

@interface MBLinkManufacturerPIDDefinitionSnapshot ()
@property(nonatomic, copy, readwrite) NSString *stableKey;
@property(nonatomic, readwrite) uint16_t identifier;
@property(nonatomic, readwrite) uint8_t service;
@property(nonatomic, copy, readwrite) NSString *shortName;
@property(nonatomic, copy, readwrite) NSString *title;
@property(nonatomic, copy, readwrite) NSString *provenance;
@property(nonatomic, readwrite, getter=isLive) BOOL live;
@end
@implementation MBLinkManufacturerPIDDefinitionSnapshot
@end

@interface MBLinkMercedesModuleSnapshot ()
@property(nonatomic, copy, readwrite) NSString *identifier;
@property(nonatomic, copy, readwrite) NSString *name;
@property(nonatomic, copy, readwrite) NSString *designation;
@property(nonatomic, copy, readwrite) NSString *network;
@property(nonatomic, copy, readwrite) NSString *kind;
@property(nonatomic, copy, readwrite) NSString *protocolName;
@property(nonatomic, readwrite) uint32_t requestCANIdentifier;
@property(nonatomic, readwrite) uint32_t responseCANIdentifier;
@property(nonatomic, readwrite, getter=isExtendedID) BOOL extendedID;
@property(nonatomic, copy, readwrite, nullable) NSString *identityText;
@property(nonatomic, copy, readwrite, nullable) NSString *partNumber;
@property(nonatomic, copy, readwrite, nullable) NSString *softwareNumber;
@property(nonatomic, copy, readwrite, nullable) NSString *hardwareNumber;
@property(nonatomic, copy, readwrite) NSString *faultStatus;
@property(nonatomic, readwrite) NSUInteger faultCount;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *faults;
@property(nonatomic, copy, readwrite) NSArray<NSString *> *evidenceDetails;
@end

@implementation MBLinkMercedesModuleSnapshot
@end

