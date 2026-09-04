from pathlib import Path

source = Path('platform/apple/MBLinkDiagnosticsController.m')
text = source.read_text()
old = '''- (BOOL)beginCachedVehicleProfileRefresh;
- (void)persistCapabilitiesFromFlowEvent:
    (const LinkDiagnosticFlowEvent *)event;
'''
new = '''- (BOOL)beginCachedVehicleProfileRefresh;
- (void)persistDiscoveredCapabilities;
- (void)persistCapabilitiesFromFlowEvent:
    (const LinkDiagnosticFlowEvent *)event;
'''
assert old in text
text = text.replace(old, new, 1)

old = '''    /*
     * Finish the standard OBD stored/pending/permanent DTC inventory first.
     * Mercedes module discovery can be lengthy and must not prevent ordinary
     * OBD evidence from reaching the UI.
     */
    flowConfig.manufacturer_extension_after_pid_discovery = false;
    flowConfig.manufacturer_extension_after_standard_dtcs = true;
    flowConfig.restore_adapter_after_manufacturer_extension = true;
'''
new = '''    /*
     * Resolve the physical vehicle before doing the broader diagnostic work.
     * LINK reads Mode 09 VIN immediately after ELM initialisation, MBLINK then
     * selects/creates the authoritative VIN profile and validates/learns the
     * Mercedes controller map. Full SAE capability/fault context follows.
     */
    flowConfig.manufacturer_extension_after_standard_vin = true;
    flowConfig.manufacturer_extension_after_pid_discovery = false;
    flowConfig.manufacturer_extension_after_standard_dtcs = false;
    flowConfig.restore_adapter_after_manufacturer_extension = true;
'''
assert old in text
text = text.replace(old, new, 1)

old = '''    if (event == NULL) return;
    if (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE ||
'''
new = '''    if (event == NULL) return;
    if (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_PID_DISCOVERY_COMPLETE) {
        [self persistDiscoveredCapabilities];
        [self notifyDelegate];
        return;
    }
    if (event->kind == LINK_DIAGNOSTIC_FLOW_EVENT_LIVE_SAMPLE ||
'''
assert old in text
text = text.replace(old, new, 1)

marker = '''- (void)persistCapabilitiesFromFlowEvent:
    (const LinkDiagnosticFlowEvent *)event
{
'''
assert marker in text
helper = '''- (void)persistDiscoveredCapabilities
{
    if (_shared.isSimulated || self.mercedesVINText.length == 0U) return;

    const LinkDiagnosticFlow *flow = [_shared diagnosticFlow];
    if (flow == NULL || flow->supported_pid_responders.count == 0U) return;

    NSDictionary *existing = _cachedVehicleProfile;
    if (existing == nil)
        existing = [self savedVehicleProfileForVIN:self.mercedesVINText];
    if (existing == nil) return;

    NSMutableDictionary *profile = [existing mutableCopy];
    NSMutableArray<NSMutableDictionary *> *responders =
        [[NSMutableArray alloc] init];
    NSArray *storedResponders = [profile[@"liveResponders"]
        isKindOfClass:[NSArray class]] ? profile[@"liveResponders"] : @[];
    for (id value in storedResponders) {
        if ([value isKindOfClass:[NSDictionary class]])
            [responders addObject:[(NSDictionary *)value mutableCopy]];
    }

    BOOL changed = NO;
    for (size_t index = 0U;
         index < flow->supported_pid_responders.count;
         ++index) {
        const LinkObd2ResponderPidSet *set =
            &flow->supported_pid_responders.entries[index];
        NSMutableDictionary *match = nil;
        for (NSMutableDictionary *candidate in responders) {
            NSNumber *rx = candidate[@"rx"];
            NSNumber *extended = candidate[@"extended"];
            if ([rx isKindOfClass:[NSNumber class]] &&
                [extended isKindOfClass:[NSNumber class]] &&
                rx.unsignedIntValue == set->responder_id &&
                extended.boolValue == set->extended_id) {
                match = candidate;
                break;
            }
        }
        if (match == nil) {
            match = [@{
                @"rx": @(set->responder_id),
                @"extended": @(set->extended_id),
                @"pids": @[]
            } mutableCopy];
            [responders addObject:match];
            changed = YES;
        }

        NSMutableArray<NSNumber *> *pids = [[NSMutableArray alloc] init];
        for (unsigned int pid = 0U; pid <= UINT8_MAX; ++pid) {
            if (link_obd2_pid_set_contains(
                    &set->supported_pids, (uint8_t)pid)) {
                [pids addObject:@(pid)];
            }
        }
        NSArray *previous = [match[@"pids"] isKindOfClass:[NSArray class]]
            ? match[@"pids"] : @[];
        if (![previous isEqualToArray:pids]) {
            match[@"pids"] = [pids copy];
            changed = YES;
        }
    }

    if (!changed) return;

    profile[@"schema"] = @(MBLinkVehicleProfileSchemaVersion);
    profile[@"updatedAt"] = @([[NSDate date] timeIntervalSince1970]);
    profile[@"liveResponders"] = [responders copy];
    NSUserDefaults *defaults = [NSUserDefaults standardUserDefaults];
    NSMutableDictionary *profiles =
        [[defaults dictionaryForKey:MBLinkVehicleProfilesDefaultsKey]
            mutableCopy] ?: [[NSMutableDictionary alloc] init];
    profiles[self.mercedesVINText] = [profile copy];
    [defaults setObject:[profiles copy]
                 forKey:MBLinkVehicleProfilesDefaultsKey];
    _cachedVehicleProfile = [profile copy];
}

'''
text = text.replace(marker, helper + marker, 1)
source.write_text(text)

apple = Path('docs/APPLE.md')
text = apple.read_text()
old = '''```text
ELM initialization
  → standard PID capability discovery
  → standard VIN
  → stored / pending / permanent standard DTC inventory
  → Mercedes read-only engine identification
  → first-VIN Mercedes mobile census (complete 11-bit/29-bit read-only target plan)
  → adapter restore
  → normal live-data polling
```
'''
new = '''```text
ELM initialization
  → standard VIN as the first vehicle request
  → select / create the authoritative VIN profile
  → validate saved Mercedes controller routes, or run the first-VIN identity-first census
  → adapter restore
  → standard PID capability discovery
  → stored / pending / permanent standard DTC inventory
  → readiness / freeze-frame context
  → normal live-data polling
```

The VIN/profile decision is deliberately ahead of the broader diagnostic inventory. A saved profile is still only cached evidence: known controller routes are validated, and a changed or invalid map is rebuilt. Standard OBD support is not skipped; it runs immediately after the vehicle-specific profile work and its responder-specific PID capability map is persisted back into the same VIN profile.
'''
assert old in text
text = text.replace(old, new, 1)
apple.write_text(text)

profiles = Path('docs/VEHICLE_PROFILES.md')
text = profiles.read_text()
start = text.index('### Remaining startup-order mismatch')
end = text.index('## Validation cases', start)
replacement = '''### Vehicle-first startup order

The normal iPhone startup sequence now follows the vehicle-first contract:

```text
adapter / ELM initialisation
  -> live Mode 09 VIN
  -> select / create the authoritative vehicle profile
  -> cached Mercedes controller validation or identity-first first-VIN census
  -> adapter restore
  -> responder-scoped SAE PID capability discovery
  -> stored / pending / permanent DTC inventory
  -> readiness / freeze-frame context
  -> normal live polling
```

This keeps the live VIN authoritative before expensive diagnostic work while retaining the complete generic OBD path. The later PID capability pass is written back to the already-selected VIN profile so reordering startup does not lose responder-specific capability data.

'''
text = text[:start] + replacement + text[end:]
profiles.write_text(text)
