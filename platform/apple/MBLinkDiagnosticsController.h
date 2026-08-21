// SPDX-License-Identifier: GPL-3.0-or-later
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class MBLinkDiagnosticsController;

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
@property(nonatomic, copy, readonly) NSString *faultScanStatusText;
@property(nonatomic, copy, readonly) NSArray<NSString *> *storedDTCs;
@property(nonatomic, copy, readonly) NSArray<NSString *> *pendingDTCs;
@property(nonatomic, copy, readonly) NSArray<NSString *> *permanentDTCs;
@property(nonatomic, readonly, getter=isActive) BOOL active;
@property(nonatomic, readonly, getter=isReady) BOOL ready;
@property(nonatomic, readonly) NSUInteger recordedSampleCount;

- (void)start;
- (void)disconnect;
- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid
                                      limit:(NSUInteger)limit;
- (BOOL)favouriteForPID:(uint8_t)pid;
- (void)setFavourite:(BOOL)favourite forPID:(uint8_t)pid;
- (nullable NSString *)csvSnapshot;

@end

NS_ASSUME_NONNULL_END
