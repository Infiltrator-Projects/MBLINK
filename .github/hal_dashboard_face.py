from pathlib import Path
import subprocess

LINK_REV = "bc5ebe79c972b632e26292f888d02cae3440a13f"
OLD_LINK_REV = "0ec3d2614c2aebd706d23655d993496acd7fc69f"


def replace(path, old, new, count=1):
    p = Path(path)
    text = p.read_text()
    if old not in text:
        raise SystemExit(f"expected text not found in {path}: {old[:140]!r}")
    p.write_text(text.replace(old, new, count))


subprocess.run(["git", "-C", "src/link", "fetch", "origin", LINK_REV], check=True)
subprocess.run(["git", "-C", "src/link", "checkout", "--detach", LINK_REV], check=True)

Path("VERSION").write_text("0.7.181\n")
replace(
    "include/mblink/version.h",
    '#define MBLINK_VERSION_STRING "0.7.180"',
    '#define MBLINK_VERSION_STRING "0.7.181"')

pbx = Path("app/ios/MBLINK.xcodeproj/project.pbxproj")
text = pbx.read_text()
if text.count("MARKETING_VERSION = 0.7.180;") != 4:
    raise SystemExit("expected four 0.7.180 Xcode marketing-version entries")
text = text.replace("MARKETING_VERSION = 0.7.180;", "MARKETING_VERSION = 0.7.181;")
if OLD_LINK_REV not in text:
    raise SystemExit("old LINK source revision not present in Xcode project")
text = text.replace(OLD_LINK_REV, LINK_REV)
pbx.write_text(text)

bridge = Path("app/ios/MBLINK/MBLINK-Bridging-Header.h")
text = bridge.read_text()
if '#include "link/dashboard.h"\n' not in text:
    text += '#include "link/dashboard.h"\n'
bridge.write_text(text)

vm = Path("app/ios/MBLINK/ConnectionViewModel.swift")
text = vm.read_text()
quality_anchor = '''            result.append(DiagnosticParameter(\n                id: stableKey,'''
range_block = '''            let dashboardRange: (Double, Double)? = {\n                guard let scalarDefinition else { return nil }\n                var range = LinkDashboardGaugeRange()\n                guard link_dashboard_gauge_range_for_parameter(\n                    scalarDefinition, &range) else { return nil }\n                return (\n                    displayScalar(pid: pid, rawValue: range.minimum),\n                    displayScalar(pid: pid, rawValue: range.maximum))\n            }()\n\n            result.append(DiagnosticParameter(\n                id: stableKey,'''
if quality_anchor not in text:
    raise SystemExit("standard diagnostic parameter anchor not found")
text = text.replace(quality_anchor, range_block, 1)

standard_tail = '''                history: history,\n                sourceLabel: sourceLabel,\n                qualityNote: qualityNote))'''
standard_new = '''                history: history,\n                sourceLabel: sourceLabel,\n                qualityNote: qualityNote,\n                dashboardMinimum: dashboardRange?.0,\n                dashboardMaximum: dashboardRange?.1))'''
if standard_tail not in text:
    raise SystemExit("standard diagnostic parameter tail not found")
text = text.replace(standard_tail, standard_new, 1)

module_tail = '''                    history: parameter.history,\n                    sourceLabel: parameter.sourceLabel,\n                    qualityNote: parameter.qualityNote)'''
module_new = '''                    history: parameter.history,\n                    sourceLabel: parameter.sourceLabel,\n                    qualityNote: parameter.qualityNote,\n                    dashboardMinimum: parameter.dashboardMinimum,\n                    dashboardMaximum: parameter.dashboardMaximum)'''
if module_tail not in text:
    raise SystemExit("module parameter mapping tail not found")
text = text.replace(module_tail, module_new, 1)
vm.write_text(text)

app = Path("app/ios/MBLINK/MBLINKApp.swift")
text = app.read_text()
view_anchor = '''private struct MBDashboardView: View {\n    @EnvironmentObject private var connection: ConnectionViewModel\n\n    private let defaultKeys = ['''
view_new = '''private struct MBDashboardView: View {\n    @EnvironmentObject private var connection: ConnectionViewModel\n    @AppStorage("link.dashboard.presentationMode.v1")\n    private var dashboardModeRaw = LinkDashboardPresentationMode.combined.rawValue\n\n    private var dashboardMode: LinkDashboardPresentationMode {\n        LinkDashboardPresentationMode(rawValue: dashboardModeRaw) ?? .combined\n    }\n\n    private var dashboardModeBinding: Binding<LinkDashboardPresentationMode> {\n        Binding(\n            get: { dashboardMode },\n            set: { dashboardModeRaw = $0.rawValue })\n    }\n\n    private let defaultKeys = ['''
if view_anchor not in text:
    raise SystemExit("MBDashboardView header anchor not found")
text = text.replace(view_anchor, view_new, 1)

empty_anchor = '''                    if displayed.isEmpty {\n                        MBPanel {'''
picker_block = '''                    MBPanel {\n                        VStack(alignment: .leading, spacing: 10) {\n                            Text("Dashboard display")\n                                .font(MBTypography.captionBold)\n                                .foregroundStyle(MBBrand.silver)\n                            LinkDashboardModePicker(selection: dashboardModeBinding)\n                        }\n                    }\n                    if displayed.isEmpty {\n                        MBPanel {'''
if empty_anchor not in text:
    raise SystemExit("Dashboard empty-state anchor not found")
text = text.replace(empty_anchor, picker_block, 1)

metric_anchor = '''                            ForEach(displayed) { parameter in\n                                MBMetricTile(parameter: parameter)\n                            }'''
metric_new = '''                            ForEach(displayed) { parameter in\n                                LinkDashboardMetric(\n                                    parameter: parameter,\n                                    mode: dashboardMode)\n                            }'''
if metric_anchor not in text:
    raise SystemExit("Dashboard metric-grid anchor not found")
text = text.replace(metric_anchor, metric_new, 1)
app.write_text(text)

linux = Path("app/linux/main.c")
text = linux.read_text()
include_anchor = '#include "link/workspace.h"\n'
if include_anchor not in text:
    raise SystemExit("Linux LINK include anchor not found")
text = text.replace(include_anchor, include_anchor + '#include "link/dashboard.h"\n', 1)

context_anchor = '''    bool polling_enabled[256];\n    MblinkTemperatureUnit temperature_unit;'''
context_new = '''    bool polling_enabled[256];\n    LinkDashboardPresentationMode dashboard_mode;\n    MblinkTemperatureUnit temperature_unit;'''
if context_anchor not in text:
    raise SystemExit("Linux context anchor not found")
text = text.replace(context_anchor, context_new, 1)

init_anchor = '''    context->air_mass_unit = MBLINK_AIR_MASS_G_PER_SECOND;\n    context->presentation_revision = 1U;'''
init_new = '''    context->air_mass_unit = MBLINK_AIR_MASS_G_PER_SECOND;\n    /* Combined preserves the established Linux cockpit appearance by default. */\n    context->dashboard_mode = LINK_DASHBOARD_PRESENTATION_COMBINED;\n    context->presentation_revision = 1U;'''
if init_anchor not in text:
    raise SystemExit("Linux preference defaults anchor not found")
text = text.replace(init_anchor, init_new, 1)

load_anchor = '''        context->air_mass_unit = (MblinkAirMassUnit)\n            key_file_integer_or_default(\n                key_file, "units", "air_mass",\n                MBLINK_AIR_MASS_G_PER_SECOND,\n                MBLINK_AIR_MASS_LB_PER_MINUTE);\n\n        /*'''
load_new = '''        context->air_mass_unit = (MblinkAirMassUnit)\n            key_file_integer_or_default(\n                key_file, "units", "air_mass",\n                MBLINK_AIR_MASS_G_PER_SECOND,\n                MBLINK_AIR_MASS_LB_PER_MINUTE);\n        context->dashboard_mode = (LinkDashboardPresentationMode)\n            key_file_integer_or_default(\n                key_file, "display", "dashboard_mode",\n                LINK_DASHBOARD_PRESENTATION_COMBINED,\n                LINK_DASHBOARD_PRESENTATION_COMBINED);\n\n        /*'''
if load_anchor not in text:
    raise SystemExit("Linux preference load anchor not found")
text = text.replace(load_anchor, load_new, 1)

save_anchor = '''    g_key_file_set_integer(\n        key_file, "units", "air_mass", context->air_mass_unit);\n    for (unsigned int pid = 1U;'''
save_new = '''    g_key_file_set_integer(\n        key_file, "units", "air_mass", context->air_mass_unit);\n    g_key_file_set_integer(\n        key_file, "display", "dashboard_mode", context->dashboard_mode);\n    for (unsigned int pid = 1U;'''
if save_anchor not in text:
    raise SystemExit("Linux preference save anchor not found")
text = text.replace(save_anchor, save_new, 1)

callback_anchor = '''static void mblink_link_unit_preferences(\n    const MblinkLinuxContext *context,'''
callback = '''static void dashboard_mode_changed(\n    GtkDropDown *dropdown,\n    GParamSpec *spec,\n    gpointer opaque)\n{\n    MblinkLinuxContext *context = opaque;\n    const guint selected = gtk_drop_down_get_selected(dropdown);\n    (void)spec;\n    if (context == NULL ||\n        selected > LINK_DASHBOARD_PRESENTATION_COMBINED ||\n        context->dashboard_mode == (LinkDashboardPresentationMode)selected) {\n        return;\n    }\n    context->dashboard_mode = (LinkDashboardPresentationMode)selected;\n    ++context->presentation_revision;\n    save_display_preferences(context);\n}\n\nstatic void mblink_link_unit_preferences(\n    const MblinkLinuxContext *context,'''
if callback_anchor not in text:
    raise SystemExit("Linux callback anchor not found")
text = text.replace(callback_anchor, callback, 1)

old_fraction = '''static double mblink_cockpit_fraction(uint8_t pid, double value)\n{\n    double fraction;\n    switch (pid) {\n    case UINT8_C(0x0c):\n        fraction = value / 7000.0;\n        break;\n    case UINT8_C(0x0d):\n        fraction = value / 260.0;\n        break;\n    case UINT8_C(0x05):\n        fraction = (value + 40.0) / 190.0;\n        break;\n    case UINT8_C(0x23):\n        fraction = value / 200000.0;\n        break;\n    case UINT8_C(0x2f):\n        fraction = value / 100.0;\n        break;\n    default:\n        fraction = 0.0;\n        break;\n    }\n    if (fraction < 0.0) return 0.0;\n    if (fraction > 1.0) return 1.0;\n    return fraction;\n}\n'''
new_fraction = '''static double mblink_cockpit_fraction(\n    const MblinkParameterDefinition *definition,\n    double value)\n{\n    LinkDashboardGaugeRange range;\n    double fraction = 0.0;\n    if (!link_dashboard_gauge_range_for_parameter(definition, &range) ||\n        !link_dashboard_gauge_fraction(&range, value, &fraction)) {\n        return 0.0;\n    }\n    return fraction;\n}\n'''
if old_fraction not in text:
    raise SystemExit("Linux hard-coded cockpit fraction function not found")
text = text.replace(old_fraction, new_fraction, 1)

call_anchor = '''            fraction = mblink_cockpit_fraction(\n                pid, context->samples[pid].value);'''
call_new = '''            fraction = mblink_cockpit_fraction(\n                definition, context->samples[pid].value);'''
if call_anchor not in text:
    raise SystemExit("Linux cockpit fraction call not found")
text = text.replace(call_anchor, call_new, 1)

sig_anchor = '''static void append_dashboard(GtkWidget *body, const MblinkLinuxContext *context)'''
if sig_anchor not in text:
    raise SystemExit("Linux dashboard signature not found")
text = text.replace(sig_anchor, 'static void append_dashboard(GtkWidget *body, MblinkLinuxContext *context)', 1)

status_anchor = '''    link_gtk_card_append_status(\n        cockpit,\n        context->diagnostic_ready ? "LIVE COCKPIT" : diagnostic_text(context),\n        context->diagnostic_ready ? "state-success" : "state-warning");\n\n    for (index = 0U;'''
status_new = '''    link_gtk_card_append_status(\n        cockpit,\n        context->diagnostic_ready ? "LIVE COCKPIT" : diagnostic_text(context),\n        context->diagnostic_ready ? "state-success" : "state-warning");\n\n    {\n        static const char *mode_names[] = {\n            "Numbers", "Dials", "Combined", NULL\n        };\n        GtkWidget *mode_row = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 10);\n        GtkWidget *mode_label = gtk_label_new("DISPLAY");\n        GtkWidget *mode = gtk_drop_down_new_from_strings(mode_names);\n        gtk_widget_set_hexpand(mode_label, TRUE);\n        gtk_widget_set_halign(mode_label, GTK_ALIGN_START);\n        gtk_drop_down_set_selected(\n            GTK_DROP_DOWN(mode), (guint)context->dashboard_mode);\n        g_signal_connect(\n            mode, "notify::selected",\n            G_CALLBACK(dashboard_mode_changed), context);\n        gtk_box_append(GTK_BOX(mode_row), mode_label);\n        gtk_box_append(GTK_BOX(mode_row), mode);\n        gtk_box_append(GTK_BOX(cockpit), mode_row);\n    }\n\n    for (index = 0U;'''
if status_anchor not in text:
    raise SystemExit("Linux dashboard status anchor not found")
text = text.replace(status_anchor, status_new, 1)

append_gauge = '''        gtk_flow_box_append(\n            GTK_FLOW_BOX(flow),\n            mblink_cockpit_gauge_new(\n                gauges[index].title, pid_text, value,\n                available, fraction));'''
append_mode = '''        if (context->dashboard_mode == LINK_DASHBOARD_PRESENTATION_NUMBERS) {\n            link_gtk_card_append_detail(cockpit, gauges[index].title, value);\n        } else {\n            gtk_flow_box_append(\n                GTK_FLOW_BOX(flow),\n                mblink_cockpit_gauge_new(\n                    gauges[index].title, pid_text, value,\n                    available, fraction));\n        }'''
if append_gauge not in text:
    raise SystemExit("Linux gauge append anchor not found")
text = text.replace(append_gauge, append_mode, 1)

flow_anchor = '''    gtk_box_append(GTK_BOX(cockpit), flow);\n    link_gtk_card_append_note('''
flow_new = '''    if (context->dashboard_mode != LINK_DASHBOARD_PRESENTATION_NUMBERS)\n        gtk_box_append(GTK_BOX(cockpit), flow);\n    link_gtk_card_append_note('''
if flow_anchor not in text:
    raise SystemExit("Linux cockpit flow append anchor not found")
text = text.replace(flow_anchor, flow_new, 1)

support_anchor = '''    gtk_box_append(GTK_BOX(body), support);\n    append_fuel_economy(body, context);'''
support_new = '''    if (context->dashboard_mode != LINK_DASHBOARD_PRESENTATION_DIALS)\n        gtk_box_append(GTK_BOX(body), support);\n    append_fuel_economy(body, context);'''
if support_anchor not in text:
    raise SystemExit("Linux supporting-data append anchor not found")
text = text.replace(support_anchor, support_new, 1)
linux.write_text(text)

readme = Path("README.md")
text = readme.read_text()
needle = "Dashboard"
if needle in text and "Numbers / Dials / Combined" not in text:
    text += "\n\nDashboard presentation is shared through LINK 0.15.4. Linux and iPhone expose the same persistent Numbers / Dials / Combined modes; numeric gauges use LINK-owned ranges while text/state values remain text-only.\n"
readme.write_text(text)

Path('.github/hal_dashboard_face.py').unlink()
Path('.github/workflows/hal-dashboard-face.yml').unlink()
