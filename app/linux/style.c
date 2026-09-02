// SPDX-License-Identifier: GPL-3.0-or-later
#include "style.h"

#include <fontconfig/fontconfig.h>
#include <glib.h>
#include <stddef.h>

static const char mblink_css[] =
    "window { background: #050608; color: #e8ecef; font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    "window *, popover, popover * { font-family: \"MB Corpo S Title WEB\"; }"
    "button, button *, .link-toolbar-button, .link-toolbar-button *, .link-link-button, .link-link-button *, .link-save-session-button, .link-save-session-button *, .link-about-button, .link-about-button * { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    "dropdown, dropdown *, .link-adapter-combo, .link-adapter-combo *, popover, popover * { font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    "entry, entry *, textview, textview *, textview text, .monospace, .monospace *, .link-terminal, .link-terminal *, .link-log, .link-log * { font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-connection-bar { background: #101318; border-color: #353a40; }"
    ".link-brand { color: #eef1f3; font-family: \"MB Corpo A Title Cond WEB\"; font-weight: 400; }"
    ".link-brand-subtitle { color: #aeb6bd; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-brand-version { font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-section-title { color: #e7ebee; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-section-summary { color: #899198; font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-content-title { color: #e7ebee; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-content-summary { font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-card { background: linear-gradient(135deg,#171b20,#0d1014); border-color: #353a40; }"
    ".link-card-kicker { color: #8c949b; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-card-title { color: #eef1f3; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-detail-label { color: #7e858c; font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-detail-value { color: #eef1f3; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-card-note { color: #9ca4ab; font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".link-status-chip { border-color: #3b4147; font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-toolbar-label { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-toolbar-button, .link-toolbar-button * { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-link-button { background: #d7dde2; color: #111418; }"
    ".link-save-session-button, .link-save-session-button * { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-connection-status { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-about-button, .link-about-button * { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-settings-title { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".link-settings-description { font-family: \"MB Corpo S Title WEB\"; font-weight: 400; }"
    ".state-warning { color: #d19e47; border-color: #72572f; }"
    ".state-success { color: #63ab7c; border-color: #365f45; }"
    ".mblink-settings-section { margin-top: 2px; }"
    ".mblink-settings-row { padding: 10px 0; }"
    ".mblink-settings-row dropdown { min-width: 210px; }"
    ".mblink-settings-note { color: #899198; font-family: \"MB Corpo S Title WEB\"; font-size: 11px; font-weight: 400; }";

static const char mblink_metrics_css[] =

    ".link-titlebar { background: #202125; border-bottom: 1px solid #353a40; }"
    ".link-titlebar-label { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"


    "entry, entry *, textview, textview *, textview text, .monospace, .monospace *, .link-terminal, .link-terminal *, .link-log, .link-log * { font-size: 13px; }"














    ".mblink-settings-note { font-size: 12px; }"
    ".mblink-about-dialog { background: #050608; }"
    ".mblink-about-dialog stackswitcher { margin: 8px 14px 12px 14px; }"
    ".mblink-about-dialog stackswitcher button, .mblink-about-dialog stackswitcher button * { font-family: \"MB Corpo S Title WEB\"; font-weight: 700; }"
    ".mblink-about-dialog label, .mblink-about-dialog textview, .mblink-about-dialog textview text { font-family: \"MB Corpo S Title WEB\"; font-size: 14px; }"
    ".mblink-about-dialog textview, .mblink-about-dialog textview text { font-weight: 400; }"
    ".mblink-about-dialog scrolledwindow { min-width: 500px; min-height: 300px; }";

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
    static const char *const fonts[] = {
        "mb_corpo_a_cond_regular.ttf",
        "mb_corpo_s_bold.ttf",
        "mb_corpo_s_regular.ttf"
    };
    FcConfig *config;
    size_t index;

    if (FcInit() == FcFalse) return false;
    config = FcConfigGetCurrent();
    if (config == NULL) return false;
    for (index = 0U; index < G_N_ELEMENTS(fonts); ++index) {
        if (!register_one_project_font(config, fonts[index]))
            return false;
    }
    return FcConfigBuildFonts(config) != FcFalse;
}


const char *mblink_linux_style_base_css(void)
{
    return mblink_css;
}

const char *mblink_linux_style_metrics_css(void)
{
    return mblink_metrics_css;
}
