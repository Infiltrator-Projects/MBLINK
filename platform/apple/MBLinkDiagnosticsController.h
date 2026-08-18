// SPDX-License-Identifier: GPL-3.0-or-later
#import <Foundation/Foundation.h>

#include <stdint.h>

NS_ASSUME_NONNULL_BEGIN

@class MBLinkDiagnosticsController;

NS_SWIFT_UI_ACTOR
@protocol MBLinkDiagnosticsControllerDelegate <NSObject>
- (void)diagnosticsControllerDidUpdate:(MBLinkDiagnosticsController *)controller;
@end

/**
 * Thin Apple application bridge over libmblink.
 *
 * CoreBluetooth remains transport-only. Poll scheduling, sample history and
 * CSV formatting are owned by the portable C core.
 */
NS_SWIFT_UI_ACTOR
@interface MBLinkDiagnosticsController : NSObject

@property(nonatomic, weak, nullable) id<MBLinkDiagnosticsControllerDelegate> delegate;
@property(nonatomic, copy, readonly) NSString *statusText;
@property(nonatomic, copy, readonly, nullable) NSString *peripheralName;
@property(nonatomic, copy, readonly, nullable) NSString *adapterIdentifier;
@property(nonatomic, readonly, getter=isActive) BOOL active;
@property(nonatomic, readonly, getter=isReady) BOOL ready;

@property(nonatomic, readonly) BOOL hasEngineLoad;
@property(nonatomic, readonly) double engineLoadPercent;
@property(nonatomic, readonly) BOOL hasCoolantTemperature;
@property(nonatomic, readonly) double coolantTemperatureCelsius;
@property(nonatomic, readonly) BOOL hasManifoldPressure;
@property(nonatomic, readonly) double manifoldPressureKPa;
@property(nonatomic, readonly) BOOL hasRPM;
@property(nonatomic, readonly) double rpm;
@property(nonatomic, readonly) BOOL hasVehicleSpeed;
@property(nonatomic, readonly) double vehicleSpeedKmh;
@property(nonatomic, readonly) BOOL hasIntakeAirTemperature;
@property(nonatomic, readonly) double intakeAirTemperatureCelsius;
@property(nonatomic, readonly) BOOL hasMassAirFlow;
@property(nonatomic, readonly) double massAirFlowGramsPerSecond;
@property(nonatomic, readonly) BOOL hasThrottlePosition;
@property(nonatomic, readonly) double throttlePositionPercent;

@property(nonatomic, readonly) NSUInteger recordedSampleCount;

- (void)start;
- (void)disconnect;

- (NSArray<NSNumber *> *)recentValuesForPID:(uint8_t)pid
                                      limit:(NSUInteger)limit
    NS_SWIFT_NAME(recentValues(forPID:limit:));
- (BOOL)favouriteForPID:(uint8_t)pid
    NS_SWIFT_NAME(favourite(forPID:));
- (void)setFavourite:(BOOL)favourite
              forPID:(uint8_t)pid
    NS_SWIFT_NAME(setFavourite(_:forPID:));
- (nullable NSString *)csvSnapshot
    NS_SWIFT_NAME(csvSnapshot());

@end

NS_ASSUME_NONNULL_END
