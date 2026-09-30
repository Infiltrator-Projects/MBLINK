// SPDX-License-Identifier: GPL-3.0-or-later
#import <Foundation/Foundation.h>
#import "../../src/link/platform/apple/LinkDiagnosticsController.h"

NS_ASSUME_NONNULL_BEGIN

@class MBLinkDiagnosticsController;

@interface MBLinkStandardDataSnapshot : NSObject
@property(nonatomic, readonly) uint8_t pid;
@property(nonatomic, readonly) uint32_t responderCANIdentifier;
@property(nonatomic, readonly, getter=isExtendedID) BOOL extendedID;
@property(nonatomic, readonly) NSUInteger valueKind;
@property(nonatomic, readonly) NSUInteger signalCount;
@property(nonatomic, copy, readonly) NSString *formattedValue;
@property(nonatomic, copy, readonly) NSString *rawHex;
@end

@interface MBLinkMercedesDataSnapshot : NSObject

@property(nonatomic, readonly) uint16_t identifier;
@property(nonatomic, readonly) uint8_t service;
@property(nonatomic, copy, readonly) NSString *codeText;
@property(nonatomic, copy, readonly, nullable) NSString *name;
@property(nonatomic, copy, readonly, nullable) NSString *unit;
@property(nonatomic, copy, readonly) NSString *formattedValue;
@property(nonatomic, copy, readonly) NSString *rawHex;
@property(nonatomic, copy, readonly) NSData *rawData;
@property(nonatomic, readonly, getter=isMapped) BOOL mapped;
@property(nonatomic, readonly, getter=isNumericValueAvailable)
    BOOL numericValueAvailable;
@property(nonatomic, readonly) double numericValue;

@end

@interface MBLinkTransmissionLiveValueSnapshot : NSObject
@property(nonatomic, copy, readonly) NSString *identifier;
@property(nonatomic, readonly) uint16_t localIdentifier;
@property(nonatomic, copy, readonly) NSString *shortName;
@property(nonatomic, copy, readonly) NSString *title;
@property(nonatomic, copy, readonly) NSString *suffix;
@property(nonatomic, copy, readonly) NSString *formattedValue;
@property(nonatomic, readonly, getter=isNumericValueAvailable) BOOL numericValueAvailable;
@property(nonatomic, readonly) double numericValue;
@property(nonatomic, copy, readonly) NSString *rawHex;
@property(nonatomic, copy, readonly) NSString *qualityNote;
@end

/**
 * One source-backed Mercedes live-data choice for a discovered controller.
 *
 * These definitions come from MBLINK's controller/transmission catalogues or
 * exact-route evidence. Merely discovering the ECU does not poll this value;
 * the iPhone UI must explicitly opt it in.
 */
@interface MBLinkManufacturerPIDDefinitionSnapshot : NSObject
@property(nonatomic, copy, readonly) NSString *stableKey;
@property(nonatomic, readonly) uint16_t identifier;
@property(nonatomic, readonly) uint8_t service;
@property(nonatomic, copy, readonly) NSString *shortName;
@property(nonatomic, copy, readonly) NSString *title;
@property(nonatomic, copy, readonly) NSString *provenance;
@property(nonatomic, readonly, getter=isLive) BOOL live;
@end

@interface MBLinkMercedesModuleSnapshot : NSObject

@property(nonatomic, copy, readonly) NSString *identifier;
@property(nonatomic, copy, readonly) NSString *name;
@property(nonatomic, copy, readonly) NSString *designation;
@property(nonatomic, copy, readonly) NSString *network;
@property(nonatomic, copy, readonly) NSString *kind;
@property(nonatomic, copy, readonly) NSString *protocolName;
@property(nonatomic, readonly) uint32_t requestCANIdentifier;
@property(nonatomic, readonly) uint32_t responseCANIdentifier;
@property(nonatomic, readonly, getter=isExtendedID) BOOL extendedID;
@property(nonatomic, copy, readonly, nullable) NSString *identityText;
@property(nonatomic, copy, readonly, nullable) NSString *partNumber;
@property(nonatomic, copy, readonly, nullable) NSString *softwareNumber;
@property(nonatomic, copy, readonly, nullable) NSString *hardwareNumber;
@property(nonatomic, copy, readonly) NSString *faultStatus;
@property(nonatomic, readonly) NSUInteger faultCount;
@property(nonatomic, copy, readonly) NSArray<NSString *> *faults;
@property(nonatomic, copy, readonly) NSArray<NSString *> *evidenceDetails;

@end

@protocol MBLinkDiagnosticsControllerDelegate <NSObject>
- (void)diagnosticsControllerDidUpdate:(MBLinkDiagnosticsController *)controller;
@end

@interface MBLinkDiagnosticsController : LinkProductDiagnosticsController

- (instancetype)init;

@property(nonatomic, weak, nullable) id<MBLinkDiagnosticsControllerDelegate> delegate;
@property(nonatomic, copy, readonly) NSString *mercedesProbeStatusText;
@property(nonatomic, copy, readonly, nullable) NSString *mercedesProbeEndpointText;
@property(nonatomic, copy, readonly, nullable) NSString *mercedesVINText;
@property(nonatomic, copy, readonly) NSString *mercedesIdentitySummaryText;
@property(nonatomic, copy, readonly) NSArray<NSString *> *mercedesIdentityResults;
@property(nonatomic, copy, readonly) NSString *mercedesCrd3SummaryText;
@property(nonatomic, copy, readonly) NSString *mercedesUDSFaultStatusText;
@property(nonatomic, copy, readonly) NSArray<NSString *> *mercedesUDSFaults;
@property(nonatomic, copy, readonly)
    NSArray<MBLinkMercedesModuleSnapshot *> *mercedesModuleSnapshots;
@property(nonatomic, copy, readonly) NSString *vehicleProfileStatusText;
@property(nonatomic, readonly, getter=isManufacturerDataScanActive)
    BOOL manufacturerDataScanActive;
@property(nonatomic, copy, readonly) NSString *manufacturerDataScanStatusText;
@property(nonatomic, copy, readonly, nullable)
    NSString *manufacturerDataScanModuleIdentifier;

/*
 * Resolve saved/offline Mercedes ECU identity with the same catalogue used by
 * live discovery. This performs no vehicle I/O.
 */
- (NSString *)resolvedMercedesModuleNameForRequestCANIdentifier:
        (uint32_t)requestCANIdentifier
    responseCANIdentifier:(uint32_t)responseCANIdentifier
    extendedID:(BOOL)extendedID
    protocol:(NSUInteger)protocol
    identityText:(nullable NSString *)identityText
    partNumber:(nullable NSString *)partNumber
    softwareNumber:(nullable NSString *)softwareNumber
    hardwareNumber:(nullable NSString *)hardwareNumber
    NS_SWIFT_NAME(resolvedMercedesModuleName(requestCANIdentifier:responseCANIdentifier:extendedID:protocol:identityText:partNumber:softwareNumber:hardwareNumber:));

/* Preserve MBLINK's established Swift spellings for inherited unit helpers. */
- (double)displayValueForPID:(uint8_t)pid canonicalValue:(double)value
    NS_SWIFT_NAME(displayValue(pid:canonicalValue:));
- (NSString *)displayUnitForPID:(uint8_t)pid
    NS_SWIFT_NAME(displayUnit(pid:));
- (double)displayTemperatureCelsius:(double)celsius
    NS_SWIFT_NAME(displayTemperature(celsius:));
- (NSString *)displayTemperatureUnit
    NS_SWIFT_NAME(displayTemperatureUnit());

/**
 * Discover read-only Mercedes manufacturer data identifiers on one exact ECU
 * route. Positive UDS DIDs / KWP local identifiers are retained per module;
 * unknown identifiers remain raw instead of being mislabeled as SAE OBD-II.
 */
- (void)discoverManufacturerDataForModuleIdentifier:(NSString *)identifier;

/**
 * Force a complete bounded manufacturer-data discovery pass on one exact ECU
 * route. This is intentionally distinct from refresh: refresh re-reads the
 * identifiers already proven positive, while rescan searches the full safe
 * range again for newly responding identifiers.
 */
- (void)rescanManufacturerDataForModuleIdentifier:(NSString *)identifier;

- (NSArray<MBLinkMercedesDataSnapshot *> *)
    manufacturerDataSnapshotsForModuleIdentifier:(NSString *)identifier;

/**
 * Return only one-time startup/module-card values for this resolved ECU.
 * This is the startup half of the ECU pack; it never includes user polling.
 */
- (NSArray<MBLinkMercedesDataSnapshot *> *)
    startupDataSnapshotsForModuleIdentifier:(NSString *)identifier;

/**
 * Presentation-ready live GS 21 30 values decoded only by the portable
 * Mercedes transmission layer. Swift must not reinterpret raw KWP bytes.
 */
- (NSArray<MBLinkTransmissionLiveValueSnapshot *> *)transmissionLiveValueSnapshots;
/**
 * Decode only the selected logical values from the shared GS 21 30 record.
 * All identifiers still share one scheduled 21 30 request.
 */
- (NSArray<MBLinkTransmissionLiveValueSnapshot *> *)
    transmissionLiveValueSnapshotsForIdentifiers:(NSArray<NSString *> *)identifiers
    NS_SWIFT_NAME(transmissionLiveValueSnapshots(identifiers:));

/**
 * Return the documentation-backed live-data catalogue for one discovered ECU.
 * This is a metadata lookup only: it never probes the vehicle.
 */
- (NSArray<MBLinkManufacturerPIDDefinitionSnapshot *> *)
    documentedDataDefinitionsForModuleIdentifier:(NSString *)identifier;

/**
 * Load one saved VIN's Mercedes module evidence for offline PID setup.
 * This changes only the metadata source used by the chooser; it never starts
 * a transport session or sends a diagnostic request.
 */
- (void)loadSavedVehicleProfileForPIDConfiguration:(NSString *)vin
    NS_SWIFT_NAME(loadSavedVehicleProfileForPIDConfiguration(vin:));

/**
 * Select exact manufacturer read commands for periodic polling.
 *
 * Each NSNumber packs service in bits 16..23 and identifier in bits 0..15.
 * Keeping both fields is required for KWP controllers that mix 0x1A and 0x21
 * reads. Empty is the default and means no manufacturer polling for this
 * module.
 */
- (NSArray<NSNumber *> *)
    manufacturerLivePollingCommandsForModuleIdentifier:(NSString *)identifier;
- (void)setManufacturerLivePollingCommands:(NSArray<NSNumber *> *)commands
                        forModuleIdentifier:(NSString *)identifier;

- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid
                     responderCANIdentifier:(uint32_t)responderCANIdentifier
                                  extendedID:(BOOL)extendedID
                                       limit:(NSUInteger)limit;
- (NSArray<NSNumber *> *)observedPIDsForResponderCANIdentifier:
    (uint32_t)responderCANIdentifier
                                                      extendedID:(BOOL)extendedID;
- (nullable MBLinkStandardDataSnapshot *)standardDataSnapshotForPID:(uint8_t)pid;
- (nullable MBLinkStandardDataSnapshot *)standardDataSnapshotForPID:(uint8_t)pid
                     responderCANIdentifier:(uint32_t)responderCANIdentifier
                                  extendedID:(BOOL)extendedID;

@end

NS_ASSUME_NONNULL_END
