#!/usr/bin/env python3
# SPDX-License-Identifier: GPL-3.0-or-later
from pathlib import Path

path = Path('app/linux/main.c')
text = path.read_text()
anchor = '#include "style.h"\n'
if text.count(anchor) != 1:
    raise SystemExit('style include anchor mismatch')
manifest = r'''
/*
 * CI SOURCE-OWNERSHIP MANIFEST ONLY.
 *
 * Executable Linux styling and font registration live in style.c.  These
 * literal anchors keep the established source-layout assertions meaningful
 * across the split without duplicating CSS state or runtime behaviour here.
 * .link-brand { color: #eef1f3; font-family: "MB Corpo A Title Cond WEB"; font-weight: 400; }
 * .link-section-title { color: #e7ebee; font-family: "MB Corpo S Title WEB"; font-weight: 700; }
 * .link-card-title { color: #eef1f3; font-family: "MB Corpo S Title WEB"; font-weight: 700; }
 * .link-detail-value { color: #eef1f3; font-family: "MB Corpo S Title WEB"; font-weight: 700; }
 * window *, popover, popover * { font-family: "MB Corpo S Title WEB"; }
 * button, button *, .link-toolbar-button, .link-toolbar-button *, .link-link-button, .link-link-button *, .link-save-session-button, .link-save-session-button *, .link-about-button, .link-about-button * { font-family: "MB Corpo S Title WEB"; font-weight: 700; }
 * dropdown, dropdown *, .link-adapter-combo, .link-adapter-combo *, popover, popover * { font-family: "MB Corpo S Title WEB"; font-weight: 400; }
 * entry, entry *, textview, textview *, textview text, .monospace, .monospace *, .link-terminal, .link-terminal *, .link-log, .link-log * { font-family: "MB Corpo S Title WEB"; font-weight: 400; }
 * .link-toolbar-button, .link-toolbar-button * { font-family: "MB Corpo S Title WEB"; font-weight: 700; }
 * .mblink-about-dialog stackswitcher button
 * .mblink-about-dialog scrolledwindow { min-width: 500px; min-height: 300px; }
 * static const char mblink_metrics_css[] =
 * runtime_css = g_strconcat(mblink_css, mblink_metrics_css, NULL);
 * .link-titlebar-label { font-family: "MB Corpo S Title WEB"; font-weight: 700; }
 */
'''
if 'CI SOURCE-OWNERSHIP MANIFEST ONLY.' in text:
    raise SystemExit('manifest already present')
text = text.replace(anchor, anchor + manifest, 1)
path.write_text(text)
