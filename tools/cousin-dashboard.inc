typedef struct MblinkCockpitGaugeDraw {
    double fraction;
    bool available;
} MblinkCockpitGaugeDraw;

static void mblink_cockpit_gauge_draw(
    GtkDrawingArea *area,
    cairo_t *cr,
    int width,
    int height,
    gpointer opaque)
{
    const MblinkCockpitGaugeDraw *gauge = opaque;
    const double pi = 3.14159265358979323846;
    const double start = pi * 0.75;
    const double span = pi * 1.50;
    const double size = width < height ? (double)width : (double)height;
    const double cx = (double)width / 2.0;
    const double cy = (double)height / 2.0;
    const double radius = size * 0.39;
    cairo_pattern_t *face;
    double fraction;

    (void)area;
    if (cr == NULL || gauge == NULL || width <= 0 || height <= 0)
        return;

    face = cairo_pattern_create_radial(
        cx - radius * 0.20, cy - radius * 0.24, radius * 0.06,
        cx, cy, radius);
    cairo_pattern_add_color_stop_rgb(face, 0.0, 0.13, 0.15, 0.17);
    cairo_pattern_add_color_stop_rgb(face, 0.70, 0.055, 0.065, 0.075);
    cairo_pattern_add_color_stop_rgb(face, 1.0, 0.020, 0.024, 0.028);
    cairo_arc(cr, cx, cy, radius - 8.0, 0.0, pi * 2.0);
    cairo_set_source(cr, face);
    cairo_fill(cr);
    cairo_pattern_destroy(face);

    cairo_set_line_cap(cr, CAIRO_LINE_CAP_ROUND);
    cairo_set_line_width(cr, 7.0);
    cairo_set_source_rgb(cr, 0.20, 0.23, 0.25);
    cairo_arc(cr, cx, cy, radius, start, start + span);
    cairo_stroke(cr);

    cairo_set_line_width(cr, 1.0);
    cairo_set_source_rgba(cr, 0.75, 0.79, 0.82, 0.38);
    cairo_arc(cr, cx, cy, radius - 12.0, 0.0, pi * 2.0);
    cairo_stroke(cr);

    if (!gauge->available)
        return;
    fraction = gauge->fraction;
    if (fraction < 0.0) fraction = 0.0;
    if (fraction > 1.0) fraction = 1.0;
    cairo_set_line_width(cr, 7.0);
    cairo_set_source_rgb(cr, 0.0, 0.678, 0.937);
    cairo_arc(cr, cx, cy, radius, start, start + (span * fraction));
    cairo_stroke(cr);
}

static double mblink_cockpit_fraction(uint8_t pid, double value)
{
    double fraction;
    switch (pid) {
    case UINT8_C(0x0c):
        fraction = value / 7000.0;
        break;
    case UINT8_C(0x0d):
        fraction = value / 260.0;
        break;
    case UINT8_C(0x05):
        fraction = (value + 40.0) / 190.0;
        break;
    case UINT8_C(0x23):
        fraction = value / 200000.0;
        break;
    case UINT8_C(0x2f):
        fraction = value / 100.0;
        break;
    default:
        fraction = 0.0;
        break;
    }
    if (fraction < 0.0) return 0.0;
    if (fraction > 1.0) return 1.0;
    return fraction;
}

static GtkWidget *mblink_cockpit_gauge_new(
    const char *title,
    const char *pid_text,
    const char *value_text,
    bool available,
    double fraction)
{
    GtkWidget *gauge = gtk_box_new(GTK_ORIENTATION_VERTICAL, 4);
    GtkWidget *overlay = gtk_overlay_new();
    GtkWidget *drawing = gtk_drawing_area_new();
    GtkWidget *center = gtk_box_new(GTK_ORIENTATION_VERTICAL, 1);
    GtkWidget *value = gtk_label_new(value_text);
    GtkWidget *pid = gtk_label_new(pid_text);
    GtkWidget *title_label = gtk_label_new(title);
    MblinkCockpitGaugeDraw *draw = g_new0(MblinkCockpitGaugeDraw, 1);

    draw->fraction = fraction;
    draw->available = available;
    gtk_widget_add_css_class(gauge, "mblink-cockpit-gauge");
    gtk_widget_set_size_request(gauge, 174, -1);
    gtk_widget_set_size_request(drawing, 154, 154);
    gtk_drawing_area_set_draw_func(
        GTK_DRAWING_AREA(drawing),
        mblink_cockpit_gauge_draw, draw, g_free);

    gtk_widget_add_css_class(value, "mblink-gauge-value");
    gtk_widget_add_css_class(pid, "mblink-gauge-pid");
    gtk_widget_add_css_class(title_label, "mblink-gauge-title");
    gtk_label_set_xalign(GTK_LABEL(value), 0.5F);
    gtk_label_set_xalign(GTK_LABEL(pid), 0.5F);
    gtk_label_set_xalign(GTK_LABEL(title_label), 0.5F);

    gtk_box_append(GTK_BOX(center), value);
    gtk_box_append(GTK_BOX(center), pid);
    gtk_widget_set_halign(center, GTK_ALIGN_CENTER);
    gtk_widget_set_valign(center, GTK_ALIGN_CENTER);
    gtk_overlay_set_child(GTK_OVERLAY(overlay), drawing);
    gtk_overlay_add_overlay(GTK_OVERLAY(overlay), center);
    gtk_box_append(GTK_BOX(gauge), overlay);
    gtk_box_append(GTK_BOX(gauge), title_label);
    return gauge;
}

static void append_dashboard(GtkWidget *body, const MblinkLinuxContext *context)
{
    static const struct {
        const char *stable_key;
        const char *title;
    } gauges[] = {
        { "obd2.engine.rpm", "Engine speed" },
        { "obd2.vehicle.speed", "Road speed" },
        { "obd2.engine.coolant", "Coolant" },
        { "obd2.diesel.rail_pressure", "Fuel rail" },
        { "obd2.fuel.tank_level", "Fuel level" }
    };
    static const char *support_keys[] = {
        "obd2.dpf.bank1_delta_pressure",
        "obd2.aftertreatment.egt_b1s1"
    };
    GtkWidget *cockpit = link_gtk_card_new(
        "COCKPIT", "Live powertrain instruments");
    GtkWidget *flow = gtk_flow_box_new();
    GtkWidget *support = link_gtk_card_new(
        "SUPPORTING DATA", "Aftertreatment and pressure signals");
    size_t index;

    gtk_widget_add_css_class(cockpit, "mblink-cockpit-card");
    gtk_widget_add_css_class(flow, "mblink-cockpit-flow");
    gtk_flow_box_set_selection_mode(
        GTK_FLOW_BOX(flow), GTK_SELECTION_NONE);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flow), 2);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow), 5);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(flow), 10);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flow), 10);

    link_gtk_card_append_status(
        cockpit,
        context->diagnostic_ready ? "LIVE COCKPIT" : diagnostic_text(context),
        context->diagnostic_ready ? "state-success" : "state-warning");

    for (index = 0U; index < G_N_ELEMENTS(gauges); ++index) {
        const MblinkParameterDefinition *definition =
            mblink_parameter_obd2_definition_for_stable_key(
                gauges[index].stable_key);
        char value[96];
        char pid_text[24];
        uint8_t pid;
        bool available;
        double fraction = 0.0;

        if (definition == NULL) continue;
        pid = definition->key.identifier;
        available = context->sample_valid[pid];
        if (available) {
            format_sample(
                &context->samples[pid], context, value, sizeof(value));
            fraction = mblink_cockpit_fraction(
                pid, context->samples[pid].value);
        } else {
            (void)snprintf(value, sizeof(value), "Waiting");
        }
        (void)snprintf(
            pid_text, sizeof(pid_text), "PID 0x%02X",
            (unsigned int)pid);
        gtk_flow_box_append(
            GTK_FLOW_BOX(flow),
            mblink_cockpit_gauge_new(
                gauges[index].title, pid_text, value,
                available, fraction));
    }

    gtk_box_append(GTK_BOX(cockpit), flow);
    link_gtk_card_append_note(
        cockpit,
        "Mercedes-style instrument presentation uses only measured diagnostic samples; no interpolation or synthetic live values are introduced.");
    gtk_box_append(GTK_BOX(body), cockpit);

    for (index = 0U; index < G_N_ELEMENTS(support_keys); ++index) {
        const MblinkParameterDefinition *definition =
            mblink_parameter_obd2_definition_for_stable_key(
                support_keys[index]);
        char value[96];
        if (definition == NULL) continue;
        if (context->sample_valid[definition->key.identifier]) {
            format_sample(
                &context->samples[definition->key.identifier],
                context, value, sizeof(value));
        } else {
            (void)snprintf(value, sizeof(value), "Waiting");
        }
        link_gtk_card_append_detail(
            support, definition->name, value);
    }
    link_gtk_card_append_note(
        support,
        "Table remains the complete technical view; Dashboard intentionally concentrates the signals most useful at a glance.");
    gtk_box_append(GTK_BOX(body), support);
    append_fuel_economy(body, context);
}
