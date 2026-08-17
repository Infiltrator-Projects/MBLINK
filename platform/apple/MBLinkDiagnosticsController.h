// SPDX-License-Identifier: GPL-3.0-or-later
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class MBLinkDiagnosticsController;

NS_SWIFT_UI_ACTOR
@protocol MBLinkDiagnosticsControllerDelegate <NSObject>
- (void)diagnosticsControllerDidUpdate:(MBLinkDiagnosticsController *)controller;
@end

/**
 * Thin Apple application bridge over libmblink.
 *
 * The controller coordinates the CoreBluetooth provider and the portable C
 * ELM327/OBD-II engines. Diagnostic parsing and formulas remain in C.
 *
 * The Apple transport, timers, and delegate delivery are main-queue bound.
 * Expose that contract to Swift as MainActor isolation rather than weakening
 * Swift 6 concurrency checking at the call site.
 */
NS_SWIFT_UI_ACTOR
@interface MBLinkDiagnosticsController : NSObject

@property(nonatomic, weak, nullable) id<MBLinkDiagnosticsControllerDelegate> delegate;
@property(nonatomic, copy, readonly) NSString *statusText;
@property(nonatomic, copy, readonly, nullable) NSString *peripheralName;
@property(nonatomic, copy, readonly, nullable) NSString *adapterIdentifier;
@property(nonatomic, readonly, getter=isActive) BOOL active;
@property(nonatomic, readonly, getter=isReady) BOOL ready;
@property(nonatomic, readonly) BOOL hasRPM;
@property(nonatomic, readonly) double rpm;
@property(nonatomic, readonly) BOOL hasCoolantTemperature;
@property(nonatomic, readonly) double coolantTemperatureCelsius;

- (void)start;
- (void)disconnect;

@end

NS_ASSUME_NONNULL_END
