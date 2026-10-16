import Foundation

struct MBStandardPIDSelectionModule {
    let id: String
    let responseCANIdentifier: UInt32
    let extendedID: Bool
}

struct MBStandardPIDSelectionMigrator {
    static let legacyVehicleWideControllerIdentifier = "standard-obd"

    private static let legacyPollingDefaultsKey =
        "mblink.polling.enabledStableKeys.v1"
    private static let globalMigrationDefaultsKey =
        "mblink.standard.pidSelectionsGlobalMigrated.v1"
    private static let explicitSelectionDefaultsKey =
        "mblink.standard.pidSelectionsExplicitByVehicle.v1"
    private static let controllerScopedMigrationDefaultsKey =
        "mblink.standard.pidSelectionsControllerScopedByVehicle.v1"
    private static let legacyAutomaticPollingStableKeys: Set<String> = [
        "obd2.engine.rpm", "obd2.vehicle.speed", "obd2.engine.coolant",
        "obd2.diesel.rail_pressure", "obd2.engine.throttle",
        "obd2.driver.accelerator_pedal_d",
        "obd2.driver.accelerator_pedal_e",
        "obd2.environment.ambient_air",
        "obd2.fuel.tank_level"
    ]

    private let store: LinkPIDSelectionStore
    private let modules: [MBStandardPIDSelectionModule]
    private let savedProfileCount: Int
    private let excludedStableKeys: Set<String>
    private let defaults: UserDefaults

    init(
        store: LinkPIDSelectionStore,
        modules: [MBStandardPIDSelectionModule],
        savedProfileCount: Int,
        excludedStableKeys: Set<String>,
        defaults: UserDefaults = .standard
    ) {
        self.store = store
        self.modules = modules
        self.savedProfileCount = savedProfileCount
        self.excludedStableKeys = excludedStableKeys
        self.defaults = defaults
    }

    func selection(forVIN vin: String, moduleID: String) -> Set<String> {
        guard modules.contains(where: { $0.id == moduleID }) else { return [] }
        ensureControllerScopedMigration(vin: vin)
        guard store.hasSelection(
            forVIN: vin, controllerIdentifier: moduleID)
        else { return [] }

        let stored = Set(store.stableKeys(
            forVIN: vin, controllerIdentifier: moduleID))
        let sanitized = sanitize(stored)
        if sanitized != stored {
            store.setStableKeys(
                Array(sanitized).sorted(),
                forVIN: vin,
                controllerIdentifier: moduleID)
        }
        return sanitized
    }

    func storeSelection(
        _ selection: Set<String>,
        forVIN vin: String,
        moduleID: String
    ) {
        guard modules.contains(where: { $0.id == moduleID }) else { return }
        store.setStableKeys(
            Array(sanitize(selection)).sorted(),
            forVIN: vin,
            controllerIdentifier: moduleID)
        markExplicitlyEdited(vin: vin)
        markMigrationComplete(vin: vin)
    }

    func aggregateSelection(forVIN vin: String) -> Set<String> {
        ensureControllerScopedMigration(vin: vin)
        return modules.reduce(into: Set<String>()) { result, module in
            guard store.hasSelection(
                forVIN: vin, controllerIdentifier: module.id)
            else { return }
            result.formUnion(sanitize(Set(store.stableKeys(
                forVIN: vin, controllerIdentifier: module.id))))
        }
    }

    func resetSelections(forVIN vin: String) {
        for module in modules {
            store.setStableKeys(
                [], forVIN: vin, controllerIdentifier: module.id)
        }
        if store.hasSelection(
            forVIN: vin,
            controllerIdentifier: Self.legacyVehicleWideControllerIdentifier
        ) {
            store.setStableKeys(
                [],
                forVIN: vin,
                controllerIdentifier:
                    Self.legacyVehicleWideControllerIdentifier)
        }
        markExplicitlyEdited(vin: vin)
        markMigrationComplete(vin: vin)
    }

    func markExplicitlyEdited(vin: String) {
        guard vin.count == 17 else { return }
        var values = defaults.dictionary(
            forKey: Self.explicitSelectionDefaultsKey) ?? [:]
        values[vin] = true
        defaults.set(values, forKey: Self.explicitSelectionDefaultsKey)
    }

    func migrationComplete(vin: String) -> Bool {
        let values = defaults.dictionary(
            forKey: Self.controllerScopedMigrationDefaultsKey) ?? [:]
        if let value = values[vin] as? Bool { return value }
        return (values[vin] as? NSNumber)?.boolValue ?? false
    }

    private func explicitlyEdited(vin: String) -> Bool {
        let values = defaults.dictionary(
            forKey: Self.explicitSelectionDefaultsKey) ?? [:]
        if let value = values[vin] as? Bool { return value }
        return (values[vin] as? NSNumber)?.boolValue ?? false
    }

    private func markMigrationComplete(vin: String) {
        guard vin.count == 17 else { return }
        var values = defaults.dictionary(
            forKey: Self.controllerScopedMigrationDefaultsKey) ?? [:]
        values[vin] = true
        defaults.set(
            values, forKey: Self.controllerScopedMigrationDefaultsKey)
    }

    private func sanitize(_ selection: Set<String>) -> Set<String> {
        selection.subtracting(excludedStableKeys)
    }

    private func preferredControllerID() -> String? {
        modules.first(where: {
            !$0.extendedID && $0.responseCANIdentifier == 0x7E8
        })?.id ?? modules.first?.id
    }

    private func pid(forStableKey stableKey: String) -> UInt8? {
        if stableKey.hasPrefix("sae.obd2.mode01."),
           let value = UInt8(stableKey.suffix(2), radix: 16) {
            return value
        }
        return stableKey.withCString { key in
            guard let definition =
                mblink_parameter_obd2_definition_for_stable_key(key)
            else { return nil }
            return UInt8(exactly: definition.pointee.key.identifier)
        }
    }

    private func isDocumentedPollingKey(_ stableKey: String) -> Bool {
        guard let pid = pid(forStableKey: stableKey),
              (pid & 0x1F) != 0,
              pid != 0x01
        else { return false }
        return mblink_obd2_pid_definition(0x01, pid) != nil
    }

    private func legacyGlobalPollingKeys() -> Set<String> {
        if store.hasGlobalSelection {
            return Set(store.globalStableKeys)
        }

        var initial = Set<String>()
        if let legacyValues =
            defaults.array(
                forKey: Self.legacyPollingDefaultsKey) as? [String] {
            let legacy = Set(legacyValues)
            if legacy != Self.legacyAutomaticPollingStableKeys {
                initial = legacy
            }
        }
        store.setGlobalStableKeys(Array(initial).sorted())
        return initial
    }

    private func recoverOlderPollingKeys(vin: String) -> Set<String> {
        var recovered = Set<String>()
        for module in modules {
            if store.hasSelection(
                forVIN: vin, controllerIdentifier: module.id) {
                recovered.formUnion(store.stableKeys(
                    forVIN: vin, controllerIdentifier: module.id))
            }
        }

        if recovered.isEmpty && savedProfileCount <= 1 {
            recovered.formUnion(legacyGlobalPollingKeys())
        }
        return sanitize(recovered)
    }

    private func legacyVehicleWideSelection(vin: String) -> Set<String>? {
        guard store.hasSelection(
            forVIN: vin,
            controllerIdentifier: Self.legacyVehicleWideControllerIdentifier)
        else { return nil }

        let existing = sanitize(Set(store.stableKeys(
            forVIN: vin,
            controllerIdentifier:
                Self.legacyVehicleWideControllerIdentifier)))
        if !existing.isEmpty || explicitlyEdited(vin: vin) {
            return existing
        }

        let recovered = recoverOlderPollingKeys(vin: vin)
        return recovered.isEmpty ? existing : recovered
    }

    private func migrationSourceSelection(vin: String) -> Set<String> {
        if let retired = legacyVehicleWideSelection(vin: vin) {
            return retired
        }

        if !defaults.bool(forKey: Self.globalMigrationDefaultsKey) {
            let initial = sanitize(legacyGlobalPollingKeys())
            defaults.set(true, forKey: Self.globalMigrationDefaultsKey)
            return initial
        }
        return []
    }

    private func ensureControllerScopedMigration(vin: String) {
        guard vin.count == 17,
              !modules.isEmpty,
              !migrationComplete(vin: vin)
        else { return }

        let existingPerModule = modules.contains {
            store.hasSelection(forVIN: vin, controllerIdentifier: $0.id)
        }
        guard existingPerModule || preferredControllerID() != nil else {
            return
        }

        if existingPerModule {
            for module in modules {
                if store.hasSelection(
                    forVIN: vin, controllerIdentifier: module.id) {
                    let stored = Set(store.stableKeys(
                        forVIN: vin, controllerIdentifier: module.id))
                    let sanitized = sanitize(stored)
                    if sanitized != stored {
                        store.setStableKeys(
                            Array(sanitized).sorted(),
                            forVIN: vin,
                            controllerIdentifier: module.id)
                    }
                } else {
                    store.setStableKeys(
                        [], forVIN: vin, controllerIdentifier: module.id)
                }
            }
            markMigrationComplete(vin: vin)
            return
        }

        let source = migrationSourceSelection(vin: vin)
        let primary = preferredControllerID()
        for module in modules {
            let migrated = module.id == primary
                ? Set(source.filter(isDocumentedPollingKey))
                : []
            store.setStableKeys(
                Array(migrated).sorted(),
                forVIN: vin,
                controllerIdentifier: module.id)
        }
        markMigrationComplete(vin: vin)
    }
}
