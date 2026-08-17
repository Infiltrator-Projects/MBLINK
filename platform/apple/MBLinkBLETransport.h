// SPDX-License-Identifier: GPL-3.0-or-later
#import <Foundation/Foundation.h>

NS_ASSUME_NONNULL_BEGIN

@class MBLinkBLETransport;

typedef NS_ENUM(NSInteger, MBLinkBLETransportState) {
    MBLinkBLETransportStateIdle = 0,
    MBLinkBLETransportStateWaitingForBluetooth,
    MBLinkBLETransportStateScanning,
    MBLinkBLETransportStateConnecting,
    MBLinkBLETransportStateDiscovering,
    MBLinkBLETransportStateProbing,
    MBLinkBLETransportStateReady,
    MBLinkBLETransportStateDisconnected,
    MBLinkBLETransportStateFailed
};

@protocol MBLinkBLETransportDelegate <NSObject>
- (void)bleTransportDidUpdate:(MBLinkBLETransport *)transport;
@end

/**
 * CoreBluetooth implementation of the MBLINK byte-stream transport boundary.
 *
 * The public Objective-C surface intentionally contains no OBD-II parsing.
 * BLE/GATT discovery remains a platform concern; diagnostic interpretation
 * remains in libmblink.
 */
@interface MBLinkBLETransport : NSObject

@property(nonatomic, weak, nullable) id<MBLinkBLETransportDelegate> delegate;
@property(nonatomic, readonly) MBLinkBLETransportState state;
@property(nonatomic, copy, readonly) NSString *statusText;
@property(nonatomic, copy, readonly, nullable) NSString *peripheralName;
@property(nonatomic, copy, readonly, nullable) NSString *adapterIdentifier;
@property(nonatomic, copy, readonly, nullable) NSString *serviceUUID;
@property(nonatomic, copy, readonly, nullable) NSString *writeCharacteristicUUID;
@property(nonatomic, copy, readonly, nullable) NSString *notifyCharacteristicUUID;
@property(nonatomic, readonly, getter=isReady) BOOL ready;

- (void)start;
- (void)disconnect;

@end

NS_ASSUME_NONNULL_END
