// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Product-owned evidence rows for Mercedes manufacturer live data.
 *
 * LINK remains the owner of the shared CSV/session recorder. MBLINK owns the
 * Mercedes decoders, so the product appends presentation-ready manufacturer
 * rows after each selected live refresh and combines them with LINK's lossless
 * transcript at export time. This keeps the raw command/response evidence and
 * adds one named row per Table value without teaching LINK Mercedes semantics.
 */
#import "MBLinkDiagnosticsController.h"

#import <objc/runtime.h>

static const void *MBLinkEvidenceRowsKey = &MBLinkEvidenceRowsKey;
static const void *MBLinkEvidenceLiveModuleKey = &MBLinkEvidenceLiveModuleKey;

static NSMutableData *MBLinkEvidenceRows(MBLinkDiagnosticsController *controller)
{
    NSMutableData *rows = objc_getAssociatedObject(controller, MBLinkEvidenceRowsKey);
    if (rows == nil) {
        rows = [[NSMutableData alloc] init];
        objc_setAssociatedObject(
            controller, MBLinkEvidenceRowsKey, rows,
            OBJC_ASSOCIATION_RETAIN_NONATOMIC);
    }
    return rows;
}

static NSString *MBLinkEvidenceCSVField(NSString *value)
{
    NSString *safe = value != nil ? value : @"";
    safe = [safe stringByReplacingOccurrencesOfString:@"\""
                                           withString:@"\"\""];
    return [NSString stringWithFormat:@"\"%@\"", safe];
}

/*
 * LINK 0.15.90 emits one surplus empty field in transcript stream rows:
 * the schema has 12 columns, while transcript rows contain 13. Preserve the
 * lossless transcript and repair only that exact prefix at product export.
 * The expression is deliberately narrow so a corrected future LINK row is
 * left untouched.
 */
static NSData *MBLinkEvidenceNormalizeBaseCSV(NSData *base)
{
    if (base.length == 0U) return base;
    NSString *text = [[NSString alloc] initWithData:base
                                           encoding:NSUTF8StringEncoding];
    if (text.length == 0U) return base;

    NSError *error = nil;
    NSRegularExpression *pattern = [NSRegularExpression
        regularExpressionWithPattern:@"(?m)^transcript,,([0-9]+),,,,,,,,"
                               options:0
                                 error:&error];
    if (pattern == nil || error != nil) return base;

    NSString *normalized = [pattern
        stringByReplacingMatchesInString:text
                                  options:0
                                    range:NSMakeRange(0, text.length)
                             withTemplate:@"transcript,,$1,,,,,,,"];
    if ([normalized isEqualToString:text]) return base;
    NSData *bytes = [normalized dataUsingEncoding:NSUTF8StringEncoding];
    return bytes != nil ? bytes : base;
}

static uint64_t MBLinkEvidenceLatestTranscriptTimestamp(NSData *base)
{
    if (base.length == 0U) return 0U;
    NSString *text = [[NSString alloc] initWithData:base
                                           encoding:NSUTF8StringEncoding];
    if (text.length == 0U) return 0U;

    NSRange marker = [text rangeOfString:@"\ntranscript,,"
                                 options:NSBackwardsSearch];
    NSUInteger start = NSNotFound;
    if (marker.location != NSNotFound) {
        start = NSMaxRange(marker);
    } else if ([text hasPrefix:@"transcript,,"]) {
        start = @"transcript,,".length;
    }
    if (start == NSNotFound || start >= text.length) return 0U;

    NSRange comma = [text rangeOfString:@","
                               options:0
                                 range:NSMakeRange(start, text.length - start)];
    if (comma.location == NSNotFound || comma.location == start) return 0U;
    NSString *number = [text substringWithRange:
        NSMakeRange(start, comma.location - start)];
    return (uint64_t)number.longLongValue;
}

static NSString *MBLinkEvidenceCANAddress(
    uint32_t identifier,
    BOOL extended)
{
    return extended
        ? [NSString stringWithFormat:@"0x%08X", (unsigned int)identifier]
        : [NSString stringWithFormat:@"0x%03X", (unsigned int)identifier];
}

static MBLinkMercedesModuleSnapshot *MBLinkEvidenceModule(
    MBLinkDiagnosticsController *controller,
    NSString *identifier)
{
    for (MBLinkMercedesModuleSnapshot *module in
         controller.mercedesModuleSnapshots ?: @[]) {
        if ([module.identifier isEqualToString:identifier]) return module;
    }
    return nil;
}

static NSString *MBLinkEvidenceCatalogueTitle(
    NSArray<MBLinkManufacturerPIDDefinitionSnapshot *> *definitions,
    MBLinkMercedesDataSnapshot *snapshot)
{
    for (MBLinkManufacturerPIDDefinitionSnapshot *definition in definitions) {
        if (definition.service == snapshot.service &&
            definition.identifier == snapshot.identifier &&
            definition.title.length != 0U) {
            return definition.title;
        }
    }
    if (snapshot.name.length != 0U) return snapshot.name;
    return [NSString stringWithFormat:@"Mercedes %@", snapshot.codeText];
}

static void MBLinkEvidenceAppendRow(
    MBLinkDiagnosticsController *controller,
    uint64_t timestamp,
    MBLinkMercedesModuleSnapshot *module,
    NSString *code,
    NSString *title,
    NSString *value,
    NSString *unit,
    NSString *result,
    NSString *rawHex)
{
    if (controller == nil || module == nil || title.length == 0U) return;

    NSString *moduleName = module.name.length != 0U
        ? module.name
        : (module.designation.length != 0U ? module.designation : module.identifier);
    NSString *prettyName = moduleName.length != 0U
        ? [NSString stringWithFormat:@"%@ · %@", moduleName, title]
        : title;
    NSString *responder = MBLinkEvidenceCANAddress(
        module.responseCANIdentifier, module.isExtendedID);

    NSString *row = [NSString stringWithFormat:
        @"manufacturer,,%llu,%@,%@,%@,%@,%@,%@,%@,%@,%@\n",
        (unsigned long long)timestamp,
        MBLinkEvidenceCSVField(code),
        MBLinkEvidenceCSVField(prettyName),
        MBLinkEvidenceCSVField(value),
        MBLinkEvidenceCSVField(unit),
        MBLinkEvidenceCSVField(responder),
        module.isExtendedID ? @"1" : @"0",
        MBLinkEvidenceCSVField(code),
        MBLinkEvidenceCSVField(result),
        MBLinkEvidenceCSVField(rawHex)];
    NSData *bytes = [row dataUsingEncoding:NSUTF8StringEncoding];
    if (bytes.length == 0U) return;

    @synchronized (controller) {
        [MBLinkEvidenceRows(controller) appendData:bytes];
    }
}

@interface MBLinkDiagnosticsController (MBLinkEvidencePrivate)
- (void)beginManufacturerDataOperationForModuleIdentifier:(NSString *)identifier
                                            forceFullScan:(BOOL)forceFullScan
                                                 liveOnly:(BOOL)liveOnly
                                        candidateCommands:
                                            (NSArray<NSNumber *> * _Nullable)candidateCommands;
- (void)finishManufacturerDataScanWithStatus:(NSString *)status;
@end

@implementation MBLinkDiagnosticsController (MBLinkNamedEvidence)

+ (void)load
{
    Method beginOriginal = class_getInstanceMethod(
        self,
        @selector(beginManufacturerDataOperationForModuleIdentifier:
            forceFullScan:liveOnly:candidateCommands:));
    Method beginReplacement = class_getInstanceMethod(
        self,
        @selector(mblink_evidence_beginManufacturerDataOperationForModuleIdentifier:
            forceFullScan:liveOnly:candidateCommands:));
    if (beginOriginal != NULL && beginReplacement != NULL) {
        method_exchangeImplementations(beginOriginal, beginReplacement);
    }

    Method finishOriginal = class_getInstanceMethod(
        self, @selector(finishManufacturerDataScanWithStatus:));
    Method finishReplacement = class_getInstanceMethod(
        self, @selector(mblink_evidence_finishManufacturerDataScanWithStatus:));
    if (finishOriginal != NULL && finishReplacement != NULL) {
        method_exchangeImplementations(finishOriginal, finishReplacement);
    }
}

- (void)mblink_evidence_beginManufacturerDataOperationForModuleIdentifier:
            (NSString *)identifier
                                            forceFullScan:(BOOL)forceFullScan
                                                 liveOnly:(BOOL)liveOnly
                                        candidateCommands:
            (NSArray<NSNumber *> * _Nullable)candidateCommands
{
    [self mblink_evidence_beginManufacturerDataOperationForModuleIdentifier:
        identifier
        forceFullScan:forceFullScan
        liveOnly:liveOnly
        candidateCommands:candidateCommands];

    NSString *liveModule = nil;
    if (liveOnly && identifier.length != 0U &&
        [self.manufacturerDataScanModuleIdentifier isEqualToString:identifier]) {
        liveModule = [identifier copy];
    }
    objc_setAssociatedObject(
        self, MBLinkEvidenceLiveModuleKey, liveModule,
        OBJC_ASSOCIATION_COPY_NONATOMIC);
}

- (void)mblink_evidence_finishManufacturerDataScanWithStatus:(NSString *)status
{
    NSString *moduleIdentifier = objc_getAssociatedObject(
        self, MBLinkEvidenceLiveModuleKey);
    const BOOL selectedLiveRefresh = moduleIdentifier.length != 0U &&
        [self.manufacturerDataScanModuleIdentifier
            isEqualToString:moduleIdentifier];
    NSArray<NSNumber *> *selectedCommands = selectedLiveRefresh
        ? [[self manufacturerLivePollingCommandsForModuleIdentifier:
            moduleIdentifier] copy]
        : @[];

    [self mblink_evidence_finishManufacturerDataScanWithStatus:status];

    NSString *currentLiveModule = objc_getAssociatedObject(
        self, MBLinkEvidenceLiveModuleKey);
    if ([currentLiveModule isEqualToString:moduleIdentifier]) {
        objc_setAssociatedObject(
            self, MBLinkEvidenceLiveModuleKey, nil,
            OBJC_ASSOCIATION_COPY_NONATOMIC);
    }
    if (!selectedLiveRefresh || selectedCommands.count == 0U) return;

    NSData *base = [super csvDataSnapshot];
    const uint64_t timestamp = MBLinkEvidenceLatestTranscriptTimestamp(base);
    MBLinkMercedesModuleSnapshot *module =
        MBLinkEvidenceModule(self, moduleIdentifier);
    if (module == nil) return;

    NSSet<NSNumber *> *selected = [NSSet setWithArray:selectedCommands];
    NSArray<MBLinkMercedesDataSnapshot *> *snapshots =
        [self manufacturerDataSnapshotsForModuleIdentifier:moduleIdentifier];
    NSArray<MBLinkManufacturerPIDDefinitionSnapshot *> *definitions =
        [self documentedDataDefinitionsForModuleIdentifier:moduleIdentifier];

    const uint32_t transmissionRLI30Token =
        ((uint32_t)UINT8_C(0x21) << 16U) | UINT16_C(0x0030);
    MBLinkMercedesDataSnapshot *rli30 = nil;
    if ([selected containsObject:@(transmissionRLI30Token)]) {
        for (MBLinkMercedesDataSnapshot *snapshot in snapshots) {
            if (snapshot.service == UINT8_C(0x21) &&
                snapshot.identifier == UINT16_C(0x0030) &&
                !snapshot.isStale) {
                rli30 = snapshot;
                break;
            }
        }
    }

    NSArray<MBLinkTransmissionLiveValueSnapshot *> *transmissionValues = @[];
    if (rli30 != nil) {
        NSArray<MBLinkTransmissionLiveValueSnapshot *> *decoded =
            [self transmissionLiveValueSnapshots];
        NSMutableArray<MBLinkTransmissionLiveValueSnapshot *> *matching =
            [[NSMutableArray alloc] init];
        for (MBLinkTransmissionLiveValueSnapshot *item in decoded) {
            if ([item.formattedValue hasPrefix:@"Stale · "]) continue;
            if (rli30.rawHex.length != 0U &&
                ![item.rawHex isEqualToString:rli30.rawHex]) {
                continue;
            }
            [matching addObject:item];
            NSString *unit = [item.suffix
                stringByTrimmingCharactersInSet:
                    NSCharacterSet.whitespaceAndNewlineCharacterSet];
            const BOOL raw = [unit containsString:@"raw"];
            MBLinkEvidenceAppendRow(
                self, timestamp, module, @"21 30", item.title,
                item.formattedValue,
                unit,
                raw ? @"raw" : @"decoded",
                item.rawHex);
        }
        transmissionValues = [matching copy];
    }

    for (MBLinkMercedesDataSnapshot *snapshot in snapshots) {
        const uint32_t token =
            ((uint32_t)snapshot.service << 16U) |
            (uint32_t)snapshot.identifier;
        if (![selected containsObject:@(token)] || snapshot.isStale) continue;
        if (token == transmissionRLI30Token && transmissionValues.count != 0U)
            continue;

        MBLinkEvidenceAppendRow(
            self, timestamp, module, snapshot.codeText,
            MBLinkEvidenceCatalogueTitle(definitions, snapshot),
            snapshot.formattedValue,
            snapshot.unit ?: @"",
            snapshot.isMapped ? @"decoded" : @"raw",
            snapshot.rawHex);
    }
}

- (NSData *)csvDataSnapshot
{
    NSData *base = MBLinkEvidenceNormalizeBaseCSV([super csvDataSnapshot]);
    if (base == nil) return nil;

    NSData *extra;
    @synchronized (self) {
        extra = [MBLinkEvidenceRows(self) copy];
    }
    if (extra.length == 0U) return base;

    NSMutableData *combined = [base mutableCopy];
    if (combined.length != 0U) {
        const uint8_t *bytes = combined.bytes;
        if (bytes[combined.length - 1U] != (uint8_t)'\n') {
            static const char newline = '\n';
            [combined appendBytes:&newline length:1U];
        }
    }
    [combined appendData:extra];
    return [combined copy];
}

- (NSString *)csvSnapshot
{
    NSData *data = [self csvDataSnapshot];
    if (data == nil) return nil;
    return [[NSString alloc] initWithData:data encoding:NSUTF8StringEncoding];
}

@end
