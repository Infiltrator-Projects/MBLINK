// SPDX-License-Identifier: GPL-3.0-or-later
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class MBLinkDiagnosticsController;

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
@property(nonatomic, copy, readonly) NSString *faultScanStatusText;
@property(nonatomic, copy, readonly) NSArray<NSString *> *storedDTCs;
@property(nonatomic, copy, readonly) NSArray<NSString *> *pendingDTCs;
@property(nonatomic, copy, readonly) NSArray<NSString *> *permanentDTCs;
@property(nonatomic, copy, readonly) NSString *readinessStatusText;
@property(nonatomic, copy, readonly) NSArray<NSString *> *readinessMonitorStatus;
@property(nonatomic, copy, readonly) NSArray<NSString *> *freezeFrameContext;
@property(nonatomic, readonly, getter=isActive) BOOL active;
@property(nonatomic, readonly, getter=isReady) BOOL ready;
@property(nonatomic, readonly) NSUInteger recordedSampleCount;

- (void)start;
- (void)startSimulated;
- (void)disconnect;
- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid
                                      limit:(NSUInteger)limit;
- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid
                     responderCANIdentifier:(uint32_t)responderCANIdentifier
                                  extendedID:(BOOL)extendedID
                                       limit:(NSUInteger)limit;
- (NSArray<NSNumber *> *)observedPIDsForResponderCANIdentifier:
    (uint32_t)responderCANIdentifier
                                                      extendedID:(BOOL)extendedID;
- (BOOL)favouriteForPID:(uint8_t)pid;
- (void)setFavourite:(BOOL)favourite forPID:(uint8_t)pid;
- (BOOL)pollingEnabledForPID:(uint8_t)pid;
- (void)setPollingEnabled:(BOOL)enabled forPID:(uint8_t)pid;
- (nullable NSData *)csvDataSnapshot;
- (nullable NSString *)csvSnapshot;

@end

NS_ASSUME_NONNULL_END
