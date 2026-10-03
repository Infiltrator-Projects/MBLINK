// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * MBLINK compatibility compilation shim. The CoreBluetooth implementation is
 * owned by LINK and compiled into this product target from the pinned gitlink.
 */
#import "MBLinkBLETransport+MBLINK.h"
#include "../../src/link/platform/apple/LinkBLETransport.m"

/*
 * MBLinkDiagnosticsEvidence.inc is an Objective-C implementation fragment,
 * not a public header. Give its internal declarations the same nullability
 * default used by the surrounding Apple controller headers so current Xcode
 * does not treat otherwise-correct private pointer declarations as incomplete.
 */
NS_ASSUME_NONNULL_BEGIN
#include "MBLinkDiagnosticsEvidence.inc"
NS_ASSUME_NONNULL_END
