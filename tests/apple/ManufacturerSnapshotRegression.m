// SPDX-License-Identifier: GPL-3.0-or-later
#import "../../platform/apple/MBLinkDiagnosticsController.h"
#import "../../platform/apple/MBLinkDiagnosticsModels.inc"
#include <stdio.h>

#define CHECK(condition) do { if (!(condition)) { \
    fprintf(stderr, "Failed: %s at line %d\n", #condition, __LINE__); return 1; \
} } while (0)

static MBLinkMercedesDataSnapshot *sample(uint8_t service, uint16_t identifier)
{
    MBLinkMercedesDataSnapshot *value = [[MBLinkMercedesDataSnapshot alloc] init];
    value.service = service;
    value.identifier = identifier;
    value.formattedValue = @"71 °C";
    value.rawHex = @"6F";
    value.rawData = [NSData dataWithBytes:"o" length:1];
    value.numericValueAvailable = YES;
    value.numericValue = 71.0;
    return value;
}

int main(void)
{
    @autoreleasepool {
        MBLinkMercedesDataSnapshot *startup = sample(0x21, 0xB1);
        MBLinkMercedesDataSnapshot *live = sample(0x21, 0x30);
        MBLinkMercedesDataSnapshot *unrequested = sample(0x22, 0x1234);
        NSArray *initial = MBLinkMergeMercedesDataSnapshots(
            @[], @[startup, live, unrequested], [NSSet set]);
        CHECK(initial.count == 3 && !live.isStale && !startup.isStale);
        // A completed refresh with NO DATA/NRC/timeout publishes no positive.
        NSArray *failed = MBLinkMergeMercedesDataSnapshots(initial, @[],
            [NSSet setWithObject:@0x210030]);
        CHECK(failed.count == 3 && live.isStale);
        CHECK(!startup.isStale && !unrequested.isStale);
        CHECK([live.rawHex isEqualToString:@"6F"] && live.rawData.length == 1);
        CHECK([startup.formattedValue isEqualToString:@"71 °C"]);
        // Other successful reads cannot freshen the failed source.
        (void)MBLinkMergeMercedesDataSnapshots(failed, @[sample(0x22, 0x1234)],
            [NSSet setWithObject:@0x221234]);
        CHECK(live.isStale && !startup.isStale);
        MBLinkMercedesDataSnapshot *recovered = sample(0x21, 0x30);
        NSArray *fresh = MBLinkMergeMercedesDataSnapshots(failed, @[recovered],
            [NSSet setWithObject:@0x210030]);
        CHECK(fresh.count == 3 && !recovered.isStale);
        CHECK([fresh containsObject:recovered] && ![fresh containsObject:live]);
        puts("Manufacturer snapshot freshness regression passed");
    }
    return 0;
}
