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

/* Exercise the production evidence methods with a deterministic LINK recorder
 * and scan completion. No transport, clock or vehicle is required. */
@interface EvidenceRecorder : NSObject
@property(nonatomic, copy) NSData *baseCSV;
- (NSData *)csvDataSnapshot;
@end
@implementation EvidenceRecorder
- (NSData *)csvDataSnapshot { return self.baseCSV; }
@end

@interface EvidenceController : EvidenceRecorder
@property(nonatomic, copy) NSString *manufacturerDataScanModuleIdentifier;
@property(nonatomic, copy) NSArray<MBLinkMercedesModuleSnapshot *> *mercedesModuleSnapshots;
@property(nonatomic, copy) NSArray<MBLinkMercedesDataSnapshot *> *snapshots;
@property(nonatomic, copy) NSArray<MBLinkManufacturerPIDDefinitionSnapshot *> *definitions;
@property(nonatomic, copy) NSArray<MBLinkTransmissionLiveValueSnapshot *> *transmission;
@property(nonatomic, copy) NSArray<NSNumber *> *selected;
@property(nonatomic) BOOL rejectBegin;
@property(nonatomic, copy) NSString *nextLiveModule;
- (void)beginManufacturerDataOperationForModuleIdentifier:(NSString *)identifier
    liveOnly:(BOOL)liveOnly
    candidateCommands:(NSArray<NSNumber *> *)candidateCommands;
- (void)startManufacturerDataOperationForModuleIdentifier:(NSString *)identifier
    liveOnly:(BOOL)liveOnly
    candidateCommands:(NSArray<NSNumber *> *)candidateCommands;
- (void)finishManufacturerDataScanWithStatus:(NSString *)status;
- (void)completeManufacturerDataScanWithStatus:(NSString *)status;
- (NSArray<NSNumber *> *)manufacturerLivePollingCommandsForModuleIdentifier:(NSString *)identifier;
- (NSArray<MBLinkMercedesDataSnapshot *> *)manufacturerDataSnapshotsForModuleIdentifier:(NSString *)identifier;
- (NSArray<MBLinkManufacturerPIDDefinitionSnapshot *> *)documentedDataDefinitionsForModuleIdentifier:(NSString *)identifier;
- (NSArray<MBLinkTransmissionLiveValueSnapshot *> *)transmissionLiveValueSnapshots;
- (NSString *)csvSnapshot;
@end

@implementation EvidenceController {
    NSMutableData *_manufacturerEvidenceRows;
    NSString *_manufacturerEvidenceLiveModuleIdentifier;
}
// Compile the same ordinary controller methods as the product.
#define MBLinkDiagnosticsController EvidenceController
#include "../../platform/apple/MBLinkDiagnosticsEvidence.inc"
#undef MBLinkDiagnosticsController

- (void)startManufacturerDataOperationForModuleIdentifier:(NSString *)identifier
    liveOnly:(BOOL)liveOnly
    candidateCommands:(NSArray<NSNumber *> *)candidateCommands
{
    if (!self.rejectBegin) self.manufacturerDataScanModuleIdentifier = identifier;
}
- (void)completeManufacturerDataScanWithStatus:(NSString *)status
{
    self.manufacturerDataScanModuleIdentifier = nil;
    if (self.nextLiveModule.length != 0) {
        NSString *next = self.nextLiveModule;
        self.nextLiveModule = nil;
        [self beginManufacturerDataOperationForModuleIdentifier:next
            liveOnly:YES candidateCommands:nil];
    }
}
- (NSArray<NSNumber *> *)manufacturerLivePollingCommandsForModuleIdentifier:(NSString *)identifier
{ return self.selected ?: @[]; }
- (NSArray<MBLinkMercedesDataSnapshot *> *)manufacturerDataSnapshotsForModuleIdentifier:(NSString *)identifier
{ return self.snapshots ?: @[]; }
- (NSArray<MBLinkManufacturerPIDDefinitionSnapshot *> *)documentedDataDefinitionsForModuleIdentifier:(NSString *)identifier
{ return self.definitions ?: @[]; }
- (NSArray<MBLinkTransmissionLiveValueSnapshot *> *)transmissionLiveValueSnapshots
{ return self.transmission ?: @[]; }
@end

static void refresh(EvidenceController *controller, BOOL live)
{
    [controller beginManufacturerDataOperationForModuleIdentifier:@"GS"
        liveOnly:live candidateCommands:nil];
    [controller finishManufacturerDataScanWithStatus:@"Complete"];
}

static int manufacturerEvidenceRegression(void)
{
    EvidenceController *controller = [[EvidenceController alloc] init];
    CHECK(controller.csvDataSnapshot == nil && controller.csvSnapshot == nil);
    NSString *base = @"header\ntranscript,,123,,,,,,,,\"TX\",\"21B1\"\n";
    NSString *normalized = @"header\ntranscript,,123,,,,,,,\"TX\",\"21B1\"\n";
    controller.baseCSV = [base dataUsingEncoding:NSUTF8StringEncoding];
    CHECK([controller.csvSnapshot isEqualToString:normalized]);
    controller.baseCSV = [normalized dataUsingEncoding:NSUTF8StringEncoding];
    CHECK([controller.csvSnapshot isEqualToString:normalized]);

    MBLinkMercedesModuleSnapshot *module = [[MBLinkMercedesModuleSnapshot alloc] init];
    module.identifier = @"GS";
    module.name = @"Gearbox, \"GS\"";
    module.responseCANIdentifier = 0x7E9;
    controller.mercedesModuleSnapshots = @[module];
    MBLinkMercedesDataSnapshot *live = sample(0x21, 0xB1);
    live.codeText = @"21 B1";
    live.mapped = YES;
    live.unit = @"°C";
    MBLinkMercedesDataSnapshot *stale = sample(0x22, 0x1234);
    stale.stale = YES;
    controller.snapshots = @[live, stale, sample(0x22, 0x4321)];
    controller.selected = @[@0x2100B1, @0x221234];
    MBLinkManufacturerPIDDefinitionSnapshot *definition =
        [[MBLinkManufacturerPIDDefinitionSnapshot alloc] init];
    definition.service = 0x21;
    definition.identifier = 0xB1;
    definition.title = @"ATF \"temperature\"";
    controller.definitions = @[definition];

    refresh(controller, NO); // Startup/manual reads produce no named live rows.
    controller.rejectBegin = YES;
    refresh(controller, YES); // Rejected requests produce no named live rows.
    CHECK([controller.csvSnapshot isEqualToString:normalized]);
    controller.rejectBegin = NO;
    refresh(controller, YES);
    NSString *namedRow = @"manufacturer,,123,\"21 B1\",\"Gearbox, \"\"GS\"\" · ATF \"\"temperature\"\"\",\"71 °C\",\"°C\",\"0x7E9\",0,\"21 B1\",\"decoded\",\"6F\"\n";
    NSString *expected = [normalized stringByAppendingString:namedRow];
    CHECK([controller.csvSnapshot isEqualToString:expected]);
    CHECK([controller.csvDataSnapshot isEqualToData:
        [expected dataUsingEncoding:NSUTF8StringEncoding]]);
    [controller finishManufacturerDataScanWithStatus:@"Repeated completion"];
    CHECK([controller.csvSnapshot isEqualToString:expected]);
    controller.selected = @[];
    refresh(controller, YES);
    CHECK([controller.csvSnapshot isEqualToString:expected]);

    // A raw value uses its snapshot title and the extended CAN address. A base
    // without a final newline gets exactly one separator before evidence.
    controller = [[EvidenceController alloc] init];
    controller.baseCSV = [@"transcript,,456,,,,,,,\"RX\",\"6130AABB\""
        dataUsingEncoding:NSUTF8StringEncoding];
    module.name = @"";
    module.designation = @"GS";
    module.extendedID = YES;
    module.responseCANIdentifier = 0x18DAF110;
    controller.mercedesModuleSnapshots = @[module];
    live.mapped = NO;
    live.name = @"Raw temperature";
    live.unit = nil;
    controller.snapshots = @[live];
    controller.selected = @[@0x2100B1];
    refresh(controller, YES);
    NSString *rawRow = @"manufacturer,,456,\"21 B1\",\"GS · Raw temperature\",\"71 °C\",\"\",\"0x18DAF110\",1,\"21 B1\",\"raw\",\"6F\"\n";
    expected = [[@"transcript,,456,,,,,,,\"RX\",\"6130AABB\"\n"
        stringByAppendingString:rawRow] copy];
    CHECK([controller.csvSnapshot isEqualToString:expected]);

    // RLI30 expands into named values once, excluding stale and unrelated
    // decoded samples. Without matching values, retain the original raw row.
    controller = [[EvidenceController alloc] init];
    controller.baseCSV = [normalized dataUsingEncoding:NSUTF8StringEncoding];
    controller.mercedesModuleSnapshots = @[module];
    MBLinkMercedesDataSnapshot *rli30 = sample(0x21, 0x30);
    rli30.codeText = @"21 30";
    rli30.rawHex = @"AABB";
    controller.snapshots = @[rli30];
    controller.selected = @[@0x210030];
    MBLinkTransmissionLiveValueSnapshot *gear = [[MBLinkTransmissionLiveValueSnapshot alloc] init];
    gear.title = @"Actual gear";
    gear.formattedValue = @"3";
    gear.suffix = @" raw ";
    gear.rawHex = @"AABB";
    MBLinkTransmissionLiveValueSnapshot *old = [[MBLinkTransmissionLiveValueSnapshot alloc] init];
    old.formattedValue = @"Stale · 2";
    old.rawHex = @"AABB";
    MBLinkTransmissionLiveValueSnapshot *other = [[MBLinkTransmissionLiveValueSnapshot alloc] init];
    other.formattedValue = @"4";
    other.rawHex = @"CCDD";
    controller.transmission = @[gear, old, other];
    refresh(controller, YES);
    NSString *gearRow = @"manufacturer,,123,\"21 30\",\"GS · Actual gear\",\"3\",\"raw\",\"0x18DAF110\",1,\"21 30\",\"raw\",\"AABB\"\n";
    expected = [normalized stringByAppendingString:gearRow];
    CHECK([controller.csvSnapshot isEqualToString:expected]);
    controller.transmission = @[];
    refresh(controller, YES);
    NSString *fallbackRow = @"manufacturer,,123,\"21 30\",\"GS · Mercedes 21 30\",\"71 °C\",\"\",\"0x18DAF110\",1,\"21 30\",\"raw\",\"AABB\"\n";
    expected = [expected stringByAppendingString:fallbackRow];
    CHECK([controller.csvSnapshot isEqualToString:expected]);
    rli30.stale = YES;
    refresh(controller, YES);
    CHECK([controller.csvSnapshot isEqualToString:expected]);

    // Completion can start another module before returning. Its recording
    // context survives while the finished module's row is still appended.
    rli30.stale = NO;
    MBLinkMercedesModuleSnapshot *second = [[MBLinkMercedesModuleSnapshot alloc] init];
    second.identifier = @"NEXT";
    second.name = @"Next";
    second.responseCANIdentifier = 0x7E8;
    controller.mercedesModuleSnapshots = @[module, second];
    controller.nextLiveModule = @"NEXT";
    refresh(controller, YES);
    expected = [expected stringByAppendingString:fallbackRow];
    CHECK([controller.csvSnapshot isEqualToString:expected]);
    [controller finishManufacturerDataScanWithStatus:@"Next complete"];
    NSString *nextRow = @"manufacturer,,123,\"21 30\",\"Next · Mercedes 21 30\",\"71 °C\",\"\",\"0x7E8\",0,\"21 30\",\"raw\",\"AABB\"\n";
    CHECK([controller.csvSnapshot isEqualToString:[expected stringByAppendingString:nextRow]]);
    puts("Manufacturer CSV evidence regression passed");
    return 0;
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
        CHECK(manufacturerEvidenceRegression() == 0);
        puts("Manufacturer snapshot freshness regression passed");
    }
    return 0;
}
