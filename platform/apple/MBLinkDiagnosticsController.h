// SPDX-License-Identifier: GPL-3.0-or-later
#import <Foundation/Foundation.h>

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

@interface MBLinkDiagnosticsController : NSObject

@property(nonatomic, weak, nullable) id<MBLinkDiagnosticsControllerDelegate> delegate;
@property(nonatomic, copy, readonly) NSString *statusText;
@property(nonatomic, copy, readonly, nullable) NSString *peripheralName;
@property(nonatomic, copy, readonly, nullable) NSString *adapterIdentifier;
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
@property(nonatomic, copy, readonly) NSString *faultScanStatusText;
@property(nonatomic, copy, readonly) NSArray<NSString *> *storedDTCs;
@property(nonatomic, copy, readonly) NSArray<NSString *> *pendingDTCs;
@property(nonatomic, copy, readonly) NSArray<NSString *> *permanentDTCs;
@property(nonatomic, copy, readonly) NSString *readinessStatusText;
@property(nonatomic, copy, readonly) NSArray<NSString *> *readinessMonitorStatus;
@property(nonatomic, copy, readonly) NSArray<NSString *> *freezeFrameContext;
@property(nonatomic, copy, readonly) NSString *diagnosticCapabilityText;
@property(nonatomic, copy, readonly) NSString *diagnosticCapabilityDetailText;
@property(nonatomic, copy, readonly) NSString *standardResponderSummary;
@property(nonatomic, copy, readonly) NSString *supportedPIDSummary;
@property(nonatomic, copy, readonly) NSString *standardVINText;
@property(nonatomic, copy, readonly) NSArray<NSString *> *standardLiveValueRows;
@property(nonatomic, readonly, getter=isActive) BOOL active;
@property(nonatomic, readonly, getter=isReady) BOOL ready;
@property(nonatomic, readonly) NSUInteger recordedSampleCount;
@property(nonatomic, copy, readonly) NSArray<NSString *> *availableLanguageTags;
@property(nonatomic, copy, readonly) NSArray<NSString *> *availableLanguageNames;
@property(nonatomic, copy, readonly) NSString *selectedLanguageTag;
@property(nonatomic, copy, readonly) NSArray<NSString *> *availableMeasurementSystemKeys;
@property(nonatomic, copy, readonly) NSArray<NSString *> *availableMeasurementSystemNames;
@property(nonatomic, copy, readonly) NSString *selectedMeasurementSystemKey;

- (void)start;
/** Start through one exact CoreBluetooth peripheral selected by the user. */
- (void)startWithPeripheralIdentifier:(NSString *)peripheralIdentifier;
- (void)startSimulated;
- (void)disconnect;
- (NSString *)localizedTextForKey:(NSString *)key;
- (void)setSelectedLanguageTag:(NSString *)tag;
- (void)setSelectedMeasurementSystemKey:(NSString *)key;

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

- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid
                                      limit:(NSUInteger)limit;
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
- (BOOL)favouriteForPID:(uint8_t)pid;
- (void)setFavourite:(BOOL)favourite forPID:(uint8_t)pid;
- (BOOL)pollingEnabledForPID:(uint8_t)pid;
- (void)setPollingEnabled:(BOOL)enabled forPID:(uint8_t)pid;
- (BOOL)supportsPID:(uint8_t)pid;
- (nullable NSData *)csvDataSnapshot;
- (nullable NSString *)csvSnapshot;

@end

NS_ASSUME_NONNULL_END
