static void append_graphs(
    GtkWidget *body,
    const MblinkLinuxContext *context)
{
    GtkWidget *summary = link_gtk_card_new(
        "GRAPH", "Instrument-panel telemetry history");
    GtkWidget *flow = gtk_flow_box_new();
    size_t index;
    size_t rendered = 0U;

    gtk_widget_add_css_class(flow, "mblink-trace-flow");
    gtk_flow_box_set_selection_mode(
        GTK_FLOW_BOX(flow), GTK_SELECTION_NONE);
    gtk_flow_box_set_min_children_per_line(GTK_FLOW_BOX(flow), 1);
    gtk_flow_box_set_max_children_per_line(GTK_FLOW_BOX(flow), 3);
    gtk_flow_box_set_row_spacing(GTK_FLOW_BOX(flow), 12);
    gtk_flow_box_set_column_spacing(GTK_FLOW_BOX(flow), 12);

    link_gtk_card_append_status(
        summary, diagnostic_text(context),
        context->diagnostic_ready ? "state-success" : "state-warning");

    for (index = 0U; index < MBLINK_LINUX_GRAPH_TRACE_COUNT; ++index) {
        const uint8_t pid = mblink_linux_graph_pids[index];
        const size_t count =
            context->session_trace.graph_history_count[index];
        const MblinkObd2PidDefinition *definition;
        GtkWidget *trace_card;
        char kicker[48];
        char trace[192];
        char current[96];
        char minimum_text[96];
        char maximum_text[96];
        char range[224];
        char responder[48];
        double minimum;
        double maximum;
        size_t start;
        size_t sample_index;
        LinkObd2Sample boundary_sample;

        if (count == 0U || !context->sample_valid[pid]) continue;
        definition = mblink_obd2_pid_definition(UINT8_C(0x01), pid);
        format_sample(
            &context->samples[pid], context,
            current, sizeof(current));
        mblink_linux_trace_format_sparkline(
            context->session_trace.graph_history[index], count,
            context->session_trace.graph_history_next[index],
            trace, sizeof(trace));

        start = count <
                G_N_ELEMENTS(
                    context->session_trace.graph_history[index])
            ? 0U
            : context->session_trace.graph_history_next[index];
        minimum =
            context->session_trace.graph_history[index][start];
        maximum = minimum;
        for (sample_index = 1U; sample_index < count; ++sample_index) {
            const double sample_value =
                context->session_trace.graph_history[index][
                    (start + sample_index) %
                    G_N_ELEMENTS(
                        context->session_trace.graph_history[index])];
            if (sample_value < minimum) minimum = sample_value;
            if (sample_value > maximum) maximum = sample_value;
        }

        boundary_sample = context->samples[pid];
        boundary_sample.value = minimum;
        format_sample(
            &boundary_sample, context,
            minimum_text, sizeof(minimum_text));
        boundary_sample.value = maximum;
        format_sample(
            &boundary_sample, context,
            maximum_text, sizeof(maximum_text));

        if (context->sample_responder_valid[pid]) {
            (void)snprintf(
                responder, sizeof(responder),
                context->sample_responder_extended[pid]
                    ? "ECU 0x%08X" : "ECU 0x%03X",
                (unsigned int)context->sample_responder[pid]);
        } else {
            (void)snprintf(
                responder, sizeof(responder), "ECU unknown");
        }

        (void)snprintf(
            kicker, sizeof(kicker), "PID 0x%02X · %zu sample%s",
            (unsigned int)pid, count, count == 1U ? "" : "s");
        trace_card = link_gtk_card_new(
            kicker,
            definition != NULL && definition->name != NULL
                ? definition->name : "SAE parameter");
        gtk_widget_add_css_class(trace_card, "mblink-trace-card");
        (void)snprintf(
            range, sizeof(range), "%s — %s",
            minimum_text, maximum_text);
        link_gtk_card_append_detail(
            trace_card, "Current", current);
        link_gtk_card_append_detail(
            trace_card, "Session range", range);
        link_gtk_card_append_detail(
            trace_card, "Responder", responder);
        link_gtk_card_append_note(trace_card, trace);
        gtk_flow_box_append(GTK_FLOW_BOX(flow), trace_card);
        ++rendered;
    }

    if (rendered == 0U) {
        link_gtk_card_append_note(
            summary,
            "Connect and collect live samples to build rolling 48-sample traces for the primary dashboard signals.");
    } else {
        link_gtk_card_append_note(
            summary,
            "Each instrument panel shows the measured current value, session range, selected physical responder and rolling trace. No interpolation is used.");
    }
    gtk_box_append(GTK_BOX(body), summary);
    if (rendered != 0U)
        gtk_box_append(GTK_BOX(body), flow);
}
