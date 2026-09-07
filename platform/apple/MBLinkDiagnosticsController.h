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
@property(nonatomic, readonly, getter=isPollingEnabled) BOOL pollingEnabled;
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
 * Presentation-ready live GS 21 30 values decoded only by the portable
 * Mercedes transmission layer. Swift must not reinterpret raw KWP bytes.
 */
- (NSArray<MBLinkTransmissionLiveValueSnapshot *> *)transmissionLiveValueSnapshots;

/**
 * Return the documentation-backed live-data catalogue for one discovered ECU.
 * This is a metadata lookup only: it never probes the vehicle.
 */
- (NSArray<MBLinkManufacturerPIDDefinitionSnapshot *> *)
    documentedDataDefinitionsForModuleIdentifier:(NSString *)identifier;

/**
 * Select exact manufacturer wire identifiers for periodic polling.
 * Empty is the default and means no manufacturer live polling for this module.
 */
- (NSArray<NSNumber *> *)
    manufacturerLivePollingIdentifiersForModuleIdentifier:(NSString *)identifier;
- (void)setManufacturerLivePollingIdentifiers:(NSArray<NSNumber *> *)identifiers
                           forModuleIdentifier:(NSString *)identifier;

- (BOOL)manufacturerLivePollingSupportedForModuleIdentifier:(NSString *)identifier;
- (BOOL)manufacturerLivePollingEnabledForModuleIdentifier:(NSString *)identifier;
- (void)setManufacturerLivePollingEnabled:(BOOL)enabled
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
