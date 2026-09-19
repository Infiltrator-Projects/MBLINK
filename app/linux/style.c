// SPDX-License-Identifier: GPL-3.0-or-later
#include "style.h"

#include "infiltratr/core.h"
#include "infiltratr/design.h"

#include <fontconfig/fontconfig.h>
#include <glib.h>
#include <stddef.h>
#include <stdint.h>

static gchar *mblink_css;
static gchar *mblink_metrics_css;
static gsize mblink_style_initialised;

static unsigned int css_rgb(uint32_t rgb)
{
    return (unsigned int)(rgb & UINT32_C(0x00ffffff));
}

static unsigned int css_red(uint32_t rgb)
{
    return (unsigned int)((rgb >> 16U) & UINT32_C(0xff));
}

static unsigned int css_green(uint32_t rgb)
{
    return (unsigned int)((rgb >> 8U) & UINT32_C(0xff));
}

static unsigned int css_blue(uint32_t rgb)
{
    return (unsigned int)(rgb & UINT32_C(0xff));
}

static void build_base_css(
    GString *css,
    const InfiltratrThemePalette *palette,
    const InfiltratrTypography *type,
    const InfiltratrDesignMetrics *metrics)
{
    g_string_append_printf(
        css,
        "window { background: #%06x; color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->background_rgb), css_rgb(palette->text_rgb),
        type->ui_family, (unsigned int)type->ui_regular_weight);
    g_string_append_printf(
        css,
        "window *, popover, popover * { font-family: \"%s\"; }",
        type->ui_family);
    g_string_append_printf(
        css,
        "button, button *, .link-toolbar-button, .link-toolbar-button *, .link-link-button, .link-link-button *, .link-save-session-button, .link-save-session-button *, .link-about-button, .link-about-button * { font-family: \"%s\"; font-weight: %u; }",
        type->ui_family, (unsigned int)type->ui_bold_weight);
    g_string_append_printf(
        css,
        "dropdown, dropdown *, .link-adapter-combo, .link-adapter-combo *, popover, popover * { font-family: \"%s\"; font-weight: %u; }",
        type->ui_family, (unsigned int)type->ui_regular_weight);
    g_string_append_printf(
        css,
        "entry, entry *, textview, textview *, textview text, .monospace, .monospace *, .link-terminal, .link-terminal *, .link-log, .link-log * { font-family: \"%s\"; font-weight: %u; }",
        type->ui_family, (unsigned int)type->ui_regular_weight);
    g_string_append_printf(
        css,
        ".link-connection-bar { background: #%06x; border-color: #%06x; }",
        css_rgb(palette->connection_rgb),
        css_rgb(palette->connection_border_rgb));
    g_string_append_printf(
        css,
        ".link-brand { color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->title_rgb), type->brand_family,
        (unsigned int)type->brand_weight);
    g_string_append_printf(
        css,
        ".link-brand-subtitle { color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->muted_rgb), type->ui_family,
        (unsigned int)type->ui_bold_weight);
    g_string_append_printf(
        css,
        ".link-brand-version { font-family: \"%s\"; font-weight: %u; }",
        type->ui_family, (unsigned int)type->ui_regular_weight);
    g_string_append_printf(
        css,
        ".link-section-title { color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->heading_rgb), type->ui_family,
        (unsigned int)type->ui_bold_weight);
    g_string_append_printf(
        css,
        ".link-section-summary { color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->summary_rgb), type->ui_family,
        (unsigned int)type->ui_regular_weight);
    g_string_append_printf(
        css,
        ".link-content-title { color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->heading_rgb), type->ui_family,
        (unsigned int)type->ui_bold_weight);
    g_string_append_printf(
        css,
        ".link-content-summary { font-family: \"%s\"; font-weight: %u; }",
        type->ui_family, (unsigned int)type->ui_regular_weight);
    g_string_append_printf(
        css,
        ".link-card { background: linear-gradient(135deg,#%06x,#%06x); border-color: #%06x; }",
        css_rgb(palette->card_rgb), css_rgb(palette->surface_rgb),
        css_rgb(palette->border_rgb));
    g_string_append_printf(
        css,
        ".link-card-kicker { color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->kicker_rgb), type->ui_family,
        (unsigned int)type->ui_bold_weight);
    g_string_append_printf(
        css,
        ".link-card-title { color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->title_rgb), type->ui_family,
        (unsigned int)type->ui_bold_weight);
    g_string_append_printf(
        css,
        ".link-detail-label { color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->detail_label_rgb), type->ui_family,
        (unsigned int)type->ui_regular_weight);
    g_string_append_printf(
        css,
        ".link-detail-value { color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->title_rgb), type->ui_family,
        (unsigned int)type->ui_bold_weight);
    g_string_append_printf(
        css,
        ".link-card-note { color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->note_rgb), type->ui_family,
        (unsigned int)type->ui_regular_weight);
    g_string_append_printf(
        css,
        ".link-status-chip { border-color: #%06x; font-family: \"%s\"; font-weight: %u; }",
        css_rgb(palette->status_border_rgb), type->ui_family,
        (unsigned int)type->ui_bold_weight);
    g_string_append_printf(
        css,
        ".link-toolbar-label, .link-toolbar-button, .link-toolbar-button *, .link-save-session-button, .link-save-session-button *, .link-connection-status, .link-about-button, .link-about-button *, .link-settings-title { font-family: \"%s\"; font-weight: %u; }",
        type->ui_family, (unsigned int)type->ui_bold_weight);
    g_string_append_printf(
        css,
        ".link-link-button { background: #%06x; color: #%06x; border-color: #%06x; }",
        css_rgb(palette->neutral_accent_rgb),
        css_rgb(palette->accent_foreground_rgb),
        css_rgb(palette->neutral_accent_rgb));
    g_string_append_printf(
        css,
        ".link-link-button:hover { background: #%06x; border-color: #%06x; }",
        css_rgb(palette->accent_hover_rgb),
        css_rgb(palette->accent_hover_rgb));
    g_string_append_printf(
        css,
        ".link-settings-description { font-family: \"%s\"; font-weight: %u; }",
        type->ui_family, (unsigned int)type->ui_regular_weight);
    g_string_append(css, ".mblink-settings-section { margin-top: 2px; }");
    g_string_append_printf(
        css, ".mblink-settings-row { padding: %upx 0; }",
        (unsigned int)metrics->control_spacing);
    g_string_append(css, ".mblink-settings-row dropdown { min-width: 210px; }");
    g_string_append_printf(
        css,
        ".mblink-settings-note { color: #%06x; font-family: \"%s\"; font-size: 11px; font-weight: %u; }",
        css_rgb(palette->subtle_rgb), type->ui_family,
        (unsigned int)type->ui_regular_weight);
}

static void build_metrics_css(
    GString *css,
    const InfiltratrThemePalette *palette,
    const InfiltratrTypography *type,
    const InfiltratrDesignMetrics *metrics)
{
    const unsigned int accent_r = css_red(palette->neutral_accent_rgb);
    const unsigned int accent_g = css_green(palette->neutral_accent_rgb);
    const unsigned int accent_b = css_blue(palette->neutral_accent_rgb);
    const unsigned int warning_r = css_red(palette->warning_rgb);
    const unsigned int warning_g = css_green(palette->warning_rgb);
    const unsigned int warning_b = css_blue(palette->warning_rgb);

    g_string_append(css, ".link-nav-icon { opacity: 0.92; margin-right: 2px; }");
    g_string_append_printf(
        css,
        ".link-nav-row:selected { background: rgba(%u,%u,%u,0.075); border-color: rgba(%u,%u,%u,0.48); }",
        accent_r, accent_g, accent_b, accent_r, accent_g, accent_b);
    g_string_append(css, ".link-nav-row:selected .link-nav-icon { opacity: 1; }");
    g_string_append_printf(
        css, ".link-nav-row:selected .link-section-title { color: #%06x; }",
        css_rgb(palette->neutral_accent_rgb));
    g_string_append_printf(
        css, ".link-nav-row:selected .link-section-summary { color: #%06x; }",
        css_rgb(palette->selected_summary_rgb));
    g_string_append_printf(
        css, ".link-nav-row:nth-child(6) { margin-top: %upx; }",
        (unsigned int)metrics->section_spacing);
    g_string_append_printf(
        css,
        ".link-status-online { background: rgba(%u,%u,%u,0.10); border-color: rgba(%u,%u,%u,0.46); color: #%06x; }",
        accent_r, accent_g, accent_b, accent_r, accent_g, accent_b,
        css_rgb(palette->neutral_accent_rgb));
    g_string_append_printf(
        css,
        ".link-status-offline { background: rgba(%u,%u,%u,0.045); border-color: rgba(%u,%u,%u,0.22); color: #%06x; }",
        warning_r, warning_g, warning_b, warning_r, warning_g, warning_b,
        css_rgb(palette->warning_muted_rgb));
    g_string_append_printf(
        css,
        ".link-toolbar-button:hover, .link-save-session-button:hover, .link-about-button:hover { border-color: rgba(%u,%u,%u,0.36); }",
        accent_r, accent_g, accent_b);
    g_string_append_printf(
        css, "checkbutton:checked { color: #%06x; }",
        css_rgb(palette->neutral_accent_rgb));
    g_string_append_printf(
        css, ".state-warning { color: #%06x; border-color: #%06x; }",
        css_rgb(palette->warning_rgb),
        css_rgb(palette->warning_border_rgb));
    g_string_append_printf(
        css, ".state-success { color: #%06x; border-color: #%06x; }",
        css_rgb(palette->success_rgb),
        css_rgb(palette->success_border_rgb));
    g_string_append_printf(
        css,
        ".link-titlebar { background: #%06x; border-bottom: 1px solid #%06x; }",
        css_rgb(palette->titlebar_rgb), css_rgb(palette->border_rgb));
    g_string_append_printf(
        css,
        ".link-titlebar-label { font-family: \"%s\"; font-weight: %u; }",
        type->ui_family, (unsigned int)type->ui_bold_weight);
    g_string_append(
        css,
        "entry, entry *, textview, textview *, textview text, .monospace, .monospace *, .link-terminal, .link-terminal *, .link-log, .link-log * { font-size: 13px; }");

    /*
     * These cockpit/trace shades are deliberately MBLINK-specific composition,
     * not duplicate Common semantic roles. Preserve them exactly.
     */
    g_string_append(
        css,
        ".mblink-cockpit-card { background: linear-gradient(155deg,#20262c,#11161b 58%,#080a0d); border-color: #4a525b; }");
    g_string_append_printf(
        css, ".mblink-cockpit-flow, .mblink-trace-flow { margin-top: %upx; }",
        (unsigned int)metrics->compact_spacing);
    g_string_append_printf(
        css,
        ".mblink-cockpit-gauge { background: linear-gradient(155deg,#171c21,#0b0e11); border: 1px solid #394149; border-radius: %upx; padding: %upx %upx %upx %upx; }",
        (unsigned int)metrics->card_radius,
        (unsigned int)metrics->control_spacing,
        (unsigned int)metrics->control_spacing,
        (unsigned int)metrics->card_radius,
        (unsigned int)metrics->control_spacing);
    g_string_append_printf(
        css,
        ".mblink-gauge-value { color: #%06x; font-family: \"%s\"; font-size: 20px; font-weight: %u; }",
        css_rgb(palette->title_rgb), type->ui_family,
        (unsigned int)type->ui_bold_weight);
    g_string_append(css, ".mblink-gauge-pid { color: #7f8991; font-size: 10px; }");
    g_string_append_printf(
        css,
        ".mblink-gauge-title { color: #dce2e6; font-size: 13px; font-weight: %u; }",
        (unsigned int)type->ui_bold_weight);
    g_string_append(
        css,
        ".mblink-trace-card { background: linear-gradient(155deg,#171c21,#0b0e11); border-color: #3e464f; min-width: 300px; }");
    g_string_append_printf(
        css,
        ".mblink-trace-card .link-card-note { color: #%06x; font-size: 14px; }",
        css_rgb(palette->neutral_accent_rgb));
    g_string_append(css, ".mblink-settings-note { font-size: 12px; }");
    g_string_append_printf(
        css, ".link-about-dialog { background: #%06x; }",
        css_rgb(palette->background_rgb));
    g_string_append_printf(
        css,
        ".link-about-dialog stackswitcher button, .link-about-dialog stackswitcher button * { font-family: \"%s\"; font-weight: %u; }",
        type->ui_family, (unsigned int)type->ui_bold_weight);
    g_string_append_printf(
        css,
        ".link-about-dialog label, .link-about-dialog textview, .link-about-dialog textview text { font-family: \"%s\"; font-size: 14px; }",
        type->ui_family);
    g_string_append_printf(
        css,
        ".link-about-dialog textview, .link-about-dialog textview text { font-weight: %u; }",
        (unsigned int)type->ui_regular_weight);
}

static void ensure_style_css(void)
{
    if (g_once_init_enter(&mblink_style_initialised)) {
        const InfiltratrThemePalette *palette =
            infiltratr_theme_resolve(INFILTRATR_THEME_NIGHT, true);
        const InfiltratrTypography *type = infiltratr_typography();
        const InfiltratrDesignMetrics *metrics = infiltratr_design_metrics();
        GString *base = g_string_new(NULL);
        GString *metric_rules = g_string_new(NULL);

        build_base_css(base, palette, type, metrics);
        build_metrics_css(metric_rules, palette, type, metrics);
        mblink_css = g_string_free(base, FALSE);
        mblink_metrics_css = g_string_free(metric_rules, FALSE);
        g_once_init_leave(&mblink_style_initialised, 1U);
    }
}

static bool register_one_project_font(FcConfig *config, const char *filename)
{
    char *build_path;
    char *install_path;
    bool added = false;

    if (config == NULL || filename == NULL) return false;
    build_path = g_build_filename(MBLINK_FONT_BUILD_DIR, filename, NULL);
    install_path = g_build_filename(MBLINK_FONT_INSTALL_DIR, filename, NULL);
    if (build_path != NULL && g_file_test(build_path, G_FILE_TEST_IS_REGULAR))
        added = FcConfigAppFontAddFile(
            config, (const FcChar8 *)build_path) != FcFalse;
    if (!added && install_path != NULL &&
        g_file_test(install_path, G_FILE_TEST_IS_REGULAR))
        added = FcConfigAppFontAddFile(
            config, (const FcChar8 *)install_path) != FcFalse;
    g_free(build_path);
    g_free(install_path);
    return added;
}

bool mblink_linux_style_register_fonts(void)
{
    const InfiltratrTypography *type = infiltratr_typography();
    const char *fonts[3];
    FcConfig *config;
    size_t index;

    if (type == NULL) return false;
    fonts[0] = type->brand_regular_filename;
    fonts[1] = type->ui_bold_filename;
    fonts[2] = type->ui_regular_filename;

    if (FcInit() == FcFalse) return false;
    config = FcConfigGetCurrent();
    if (config == NULL) return false;
    for (index = 0U; index < INFILTRATR_ARRAY_LENGTH(fonts); ++index) {
        if (!register_one_project_font(config, fonts[index]))
            return false;
    }
    return FcConfigBuildFonts(config) != FcFalse;
}

const char *mblink_linux_style_base_css(void)
{
    ensure_style_css();
    return mblink_css;
}

const char *mblink_linux_style_metrics_css(void)
{
    ensure_style_css();
    return mblink_metrics_css;
}
