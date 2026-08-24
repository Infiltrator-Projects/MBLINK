// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file MBLinkBLETransport.h
 * @brief Compatibility names for LINK's shared CoreBluetooth provider.
 */
#import "../../src/link/platform/apple/LinkBLETransport.h"

#define MBLinkBLETransport LinkBLETransport
#define MBLinkBLETransportDelegate LinkBLETransportDelegate

typedef LinkBLETransportState MBLinkBLETransportState;

#define MBLinkBLETransportStateIdle LinkBLETransportStateIdle
#define MBLinkBLETransportStateWaitingForBluetooth LinkBLETransportStateWaitingForBluetooth
#define MBLinkBLETransportStateScanning LinkBLETransportStateScanning
#define MBLinkBLETransportStateConnecting LinkBLETransportStateConnecting
#define MBLinkBLETransportStateDiscovering LinkBLETransportStateDiscovering
#define MBLinkBLETransportStateProbing LinkBLETransportStateProbing
#define MBLinkBLETransportStateReady LinkBLETransportStateReady
#define MBLinkBLETransportStateDisconnected LinkBLETransportStateDisconnected
#define MBLinkBLETransportStateFailed LinkBLETransportStateFailed
