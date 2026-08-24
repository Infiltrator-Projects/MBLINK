from pathlib import Path


def replace_once(text: str, old: str, new: str, label: str) -> str:
    count = text.count(old)
    if count != 1:
        raise SystemExit(f"{label}: expected one occurrence, found {count}")
    return text.replace(old, new, 1)


swift_path = Path("app/ios/MBLINK/MBLINKApp.swift")
s = swift_path.read_text()
s = replace_once(
    s,
    '    @StateObject private var connection = ConnectionViewModel()\n    @State private var showingAbout = false\n',
    '    @StateObject private var connection = ConnectionViewModel()\n    @State private var showingAbout = false\n    @AppStorage("mblink.language") private var language = "en"\n',
    "app language storage",
)
s = replace_once(
    s,
    '                .environmentObject(connection)\n                .preferredColorScheme(.dark)\n',
    '                .environmentObject(connection)\n                .environment(\\.locale, Locale(identifier: language))\n                .preferredColorScheme(.dark)\n',
    "root locale",
)
settings_header = '''private struct MBSettingsView: View {
    @EnvironmentObject private var connection: ConnectionViewModel
    @AppStorage("mblink.preferFavouriteSignals") private var preferFavouriteSignals = true
    @AppStorage("mblink.showUnavailableParameters") private var showUnavailableParameters = true
'''
settings_header_localised = settings_header + '    @AppStorage("mblink.language") private var language = "en"\n'
s = replace_once(s, settings_header, settings_header_localised, "settings language storage")

language_panel = '''                    MBPanel {
                        VStack(alignment: .leading, spacing: 10) {
                            Text("Language")
                                .font(.headline)
                                .foregroundStyle(MBBrand.silverBright)
                            Picker("Language", selection: $language) {
                                Text("English").tag("en")
                                Text("Deutsch").tag("de")
                                Text("Polski").tag("pl")
                            }
                            .pickerStyle(.segmented)
                        }
                    }
'''
preference_marker = '''                    MBPanel {
                        VStack(alignment: .leading, spacing: 14) {
                            Toggle("Prefer favourites on Dashboard and Graphs", isOn: $preferFavouriteSignals)
'''
if preference_marker not in s:
    raise SystemExit("settings preference panel marker missing")
s = s.replace(preference_marker, language_panel + preference_marker, 1)

replacements = {
    'Text(text.uppercased())': 'Text(LocalizedStringKey(text)).textCase(.uppercase)',
    'Text(kicker.uppercased())': 'Text(LocalizedStringKey(kicker)).textCase(.uppercase)',
    'Text(title)': 'Text(LocalizedStringKey(title))',
    'Text(subtitle)': 'Text(LocalizedStringKey(subtitle))',
    'Text(label)': 'Text(LocalizedStringKey(label))',
    'Text(value)': 'Text(LocalizedStringKey(value))',
    'Text(parameter.shortName.uppercased())': 'Text(LocalizedStringKey(parameter.shortName)).textCase(.uppercase)',
    'Text(parameter.title)': 'Text(LocalizedStringKey(parameter.title))',
    'Text(item.rawValue).tag(item)': 'Text(LocalizedStringKey(item.rawValue)).tag(item)',
    'Label(group.rawValue, systemImage: group.symbol)': 'Label(LocalizedStringKey(group.rawValue), systemImage: group.symbol)',
    '.navigationTitle(title)': '.navigationTitle(LocalizedStringKey(title))',
}
for old, new in replacements.items():
    s = s.replace(old, new)
swift_path.write_text(s)

pbx_path = Path("app/ios/MBLINK.xcodeproj/project.pbxproj")
p = pbx_path.read_text()
if "D1A900000000000000000001 /* Localizable.strings in Resources */" not in p:
    p = replace_once(
        p,
        'A1B2C3D4E5F60718293C0001 /* Assets.xcassets in Resources */ = {isa = PBXBuildFile; fileRef = A1B2C3D4E5F60718293C0002 /* Assets.xcassets */; };',
        'A1B2C3D4E5F60718293C0001 /* Assets.xcassets in Resources */ = {isa = PBXBuildFile; fileRef = A1B2C3D4E5F60718293C0002 /* Assets.xcassets */; };\n\t\tD1A900000000000000000001 /* Localizable.strings in Resources */ = {isa = PBXBuildFile; fileRef = D1A900000000000000000005 /* Localizable.strings */; };\n\t\tD1A900000000000000000006 /* link_i18n.c in Sources */ = {isa = PBXBuildFile; fileRef = D1A900000000000000000008 /* link_i18n.c */; };\n\t\tD1A900000000000000000007 /* link_i18n_platform.c in Sources */ = {isa = PBXBuildFile; fileRef = D1A900000000000000000009 /* link_i18n_platform.c */; };',
        "PBX build files",
    )
    p = replace_once(
        p,
        'A1B2C3D4E5F60718293C0002 /* Assets.xcassets */ = {isa = PBXFileReference; lastKnownFileType = folder.assetcatalog; name = "Assets.xcassets"; path = "MBLINK/Assets.xcassets"; sourceTree = SOURCE_ROOT; };',
        'A1B2C3D4E5F60718293C0002 /* Assets.xcassets */ = {isa = PBXFileReference; lastKnownFileType = folder.assetcatalog; name = "Assets.xcassets"; path = "MBLINK/Assets.xcassets"; sourceTree = SOURCE_ROOT; };\n\t\tD1A900000000000000000002 /* en */ = {isa = PBXFileReference; lastKnownFileType = text.plist.strings; name = en; path = "MBLINK/en.lproj/Localizable.strings"; sourceTree = SOURCE_ROOT; };\n\t\tD1A900000000000000000003 /* de */ = {isa = PBXFileReference; lastKnownFileType = text.plist.strings; name = de; path = "MBLINK/de.lproj/Localizable.strings"; sourceTree = SOURCE_ROOT; };\n\t\tD1A900000000000000000004 /* pl */ = {isa = PBXFileReference; lastKnownFileType = text.plist.strings; name = pl; path = "MBLINK/pl.lproj/Localizable.strings"; sourceTree = SOURCE_ROOT; };\n\t\tD1A900000000000000000008 /* link_i18n.c */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.c.c; name = link_i18n.c; path = "../../src/link/src/core/i18n.c"; sourceTree = SOURCE_ROOT; };\n\t\tD1A900000000000000000009 /* link_i18n_platform.c */ = {isa = PBXFileReference; lastKnownFileType = sourcecode.c.c; name = link_i18n_platform.c; path = "../../src/link/src/core/i18n_platform.c"; sourceTree = SOURCE_ROOT; };',
        "PBX file refs",
    )
    p = replace_once(
        p,
        '\t\t\t\tA1B2C3D4E5F60718293C0002 /* Assets.xcassets */,\n',
        '\t\t\t\tA1B2C3D4E5F60718293C0002 /* Assets.xcassets */,\n\t\t\t\tD1A900000000000000000005 /* Localizable.strings */,\n',
        "PBX app group",
    )
    p = replace_once(
        p,
        '/* End PBXGroup section */',
        '''/* End PBXGroup section */

/* Begin PBXVariantGroup section */
\t\tD1A900000000000000000005 /* Localizable.strings */ = {
\t\t\tisa = PBXVariantGroup;
\t\t\tchildren = (
\t\t\t\tD1A900000000000000000002 /* en */,
\t\t\t\tD1A900000000000000000003 /* de */,
\t\t\t\tD1A900000000000000000004 /* pl */,
\t\t\t);
\t\t\tname = Localizable.strings;
\t\t\tsourceTree = "<group>";
\t\t};
/* End PBXVariantGroup section */''',
        "PBX variant section",
    )
    p = replace_once(
        p,
        '\t\t\tknownRegions = (\n\t\t\t\ten,\n\t\t\t\tBase,\n\t\t\t);',
        '\t\t\tknownRegions = (\n\t\t\t\ten,\n\t\t\t\tde,\n\t\t\t\tpl,\n\t\t\t\tBase,\n\t\t\t);',
        "PBX known regions",
    )
    p = replace_once(
        p,
        '\t\t\t\tA1B2C3D4E5F60718293C0001 /* Assets.xcassets in Resources */,\n',
        '\t\t\t\tA1B2C3D4E5F60718293C0001 /* Assets.xcassets in Resources */,\n\t\t\t\tD1A900000000000000000001 /* Localizable.strings in Resources */,\n',
        "PBX resources",
    )
    p = replace_once(
        p,
        '\t\t\t\tB47CA2DF83F1AF99C4CFF1EC /* obd2.c in Sources */,\n',
        '\t\t\t\tB47CA2DF83F1AF99C4CFF1EC /* obd2.c in Sources */,\n\t\t\t\tD1A900000000000000000006 /* link_i18n.c in Sources */,\n\t\t\t\tD1A900000000000000000007 /* link_i18n_platform.c in Sources */,\n',
        "PBX core sources",
    )
pbx_path.write_text(p)
