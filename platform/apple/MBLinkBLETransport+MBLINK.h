// SPDX-License-Identifier: GPL-3.0-or-later
#import "MBLinkBLETransport.h"
#import "mblink/transport.h"

NS_ASSUME_NONNULL_BEGIN

/**
 * Return a C ABI transport backed by the supplied CoreBluetooth provider.
 *
 * The provider must outlive every MblinkTransport copy created from it.
 */
MblinkTransport MBLinkBLETransportMakeCTransport(MBLinkBLETransport *transport);

NS_ASSUME_NONNULL_END
