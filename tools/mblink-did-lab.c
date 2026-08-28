// SPDX-License-Identifier: GPL-3.0-or-later
#include "mblink/mercedes_did_lab.h"

#include <ctype.h>
#include <errno.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void usage(const char *program)
{
    fprintf(stderr,
        "Usage:\n"
        "  %s catalog\n"
        "  %s decode DID_HEX RESPONSE_HEX\n"
        "  %s correlate REFERENCE.csv CANDIDATE.csv [max_lag_ms [pair_tolerance_ms [lag_step_ms]]]\n"
        "Series CSV format: timestamp_ms,value (header optional).\n",
        program, program, program);
}

static int hex_nibble(int value)
{
    if (value >= '0' && value <= '9') return value - '0';
    if (value >= 'a' && value <= 'f') return value - 'a' + 10;
    if (value >= 'A' && value <= 'F') return value - 'A' + 10;
    return -1;
}

static bool parse_hex_bytes(const char *text, uint8_t *buffer,
                            size_t capacity, size_t *written)
{
    int high = -1;
    size_t count = 0U;
    size_t index;
    if (text == NULL || buffer == NULL || written == NULL) return false;
    for (index = 0U; text[index] != '\0'; ++index) {
        const int nibble = hex_nibble((unsigned char)text[index]);
        if (nibble < 0) {
            if (isspace((unsigned char)text[index]) ||
                text[index] == ':' || text[index] == '-') continue;
            return false;
        }
        if (high < 0) high = nibble;
        else {
            if (count >= capacity) return false;
            buffer[count++] = (uint8_t)((high << 4) | nibble);
            high = -1;
        }
    }
    if (high >= 0 || count == 0U) return false;
    *written = count;
    return true;
}

static bool parse_u64(const char *text, uint64_t *value)
{
    char *end = NULL;
    unsigned long long parsed;
    if (text == NULL || value == NULL || text[0] == '\0') return false;
    errno = 0;
    parsed = strtoull(text, &end, 10);
    if (errno != 0 || end == text || *end != '\0') return false;
    *value = (uint64_t)parsed;
    return true;
}

static bool load_series(const char *path, MblinkSignalPoint **points,
                        size_t *count)
{
    FILE *file;
    MblinkSignalPoint *items = NULL;
    size_t used = 0U, capacity = 0U;
    char line[256];

    if (path == NULL || points == NULL || count == NULL) return false;
    file = fopen(path, "r");
    if (file == NULL) return false;

    while (fgets(line, sizeof(line), file) != NULL) {
        char *comma = strchr(line, ',');
        char *end_time = NULL, *end_value = NULL;
        unsigned long long timestamp;
        double value;
        if (comma == NULL) continue;
        *comma = '\0';
        errno = 0;
        timestamp = strtoull(line, &end_time, 10);
        if (errno != 0 || end_time == line) continue;
        while (*end_time != '\0' && isspace((unsigned char)*end_time))
            ++end_time;
        if (*end_time != '\0') continue;
        errno = 0;
        value = strtod(comma + 1, &end_value);
        if (errno != 0 || end_value == comma + 1) continue;
        while (*end_value != '\0' && isspace((unsigned char)*end_value))
            ++end_value;
        if (*end_value != '\0') continue;

        if (used == capacity) {
            size_t next = capacity == 0U ? 256U : capacity * 2U;
            MblinkSignalPoint *grown;
            if (next < capacity || next > SIZE_MAX / sizeof(*items)) {
                free(items); fclose(file); return false;
            }
            grown = realloc(items, next * sizeof(*items));
            if (grown == NULL) { free(items); fclose(file); return false; }
            items = grown; capacity = next;
        }
        items[used].timestamp_ms = (uint64_t)timestamp;
        items[used].value = value;
        ++used;
    }
    fclose(file);
    if (used < 2U) { free(items); return false; }
    *points = items; *count = used;
    return true;
}

static int command_catalog(void)
{
    size_t index;
    puts("status\tdid\tkey\tname\tunit\treference\tsource");
    for (index = 0U; index < mblink_mercedes_did_lab_count(); ++index) {
        const MblinkMercedesDidLabDefinition *d =
            mblink_mercedes_did_lab_at(index);
        char did[16];
        if (d == NULL) continue;
        if (d->identifier_known)
            (void)snprintf(did, sizeof(did), "0x%04X",
                           (unsigned int)d->identifier);
        else (void)snprintf(did, sizeof(did), "unmapped");
        printf("%s\t%s\t%s\t%s\t%s\t%s\t%s\n",
               mblink_mercedes_did_lab_status_name(d->status), did,
               d->stable_key, d->name, d->unit,
               d->correlation_reference_key != NULL
                   ? d->correlation_reference_key : "-",
               d->source_locator);
    }
    return 0;
}

static int command_decode(const char *did_text, const char *response_text)
{
    unsigned long did_value;
    char *end = NULL;
    uint8_t pdu[128];
    size_t pdu_length = 0U;
    const MblinkMercedesDidLabDefinition *d;
    MblinkMercedesDidLabDecodeResult r;
    double value = 0.0;

    errno = 0;
    did_value = strtoul(did_text, &end, 16);
    if (errno != 0 || end == did_text || *end != '\0' ||
        did_value > UINT16_MAX) return 2;
    d = mblink_mercedes_did_lab_find_identifier((uint16_t)did_value);
    if (d == NULL) return 3;
    if (!parse_hex_bytes(response_text, pdu, sizeof(pdu), &pdu_length))
        return 2;
    r = mblink_mercedes_did_lab_decode_response(d, pdu, pdu_length, &value);
    if (r != MBLINK_MERCEDES_DID_LAB_DECODE_OK) {
        fprintf(stderr, "decode failed: %s\n",
                mblink_mercedes_did_lab_decode_result_name(r));
        return 4;
    }
    printf("%s = %.6f %s\n", d->name, value, d->unit);
    printf("status: %s; automatic polling: %s\n",
           mblink_mercedes_did_lab_status_name(d->status),
           mblink_mercedes_did_lab_can_auto_poll(d) ? "yes" : "no");
    return 0;
}

static int command_correlate(int argc, char **argv)
{
    MblinkSignalPoint *reference = NULL, *candidate = NULL;
    size_t reference_count = 0U, candidate_count = 0U;
    uint64_t max_lag = 2000U, tolerance = 100U, step = 100U;
    MblinkSignalCorrelationResult result;
    int status = 1;

    if (argc >= 5 && !parse_u64(argv[4], &max_lag)) return 2;
    if (argc >= 6 && !parse_u64(argv[5], &tolerance)) return 2;
    if (argc >= 7 && !parse_u64(argv[6], &step)) return 2;
    if (!load_series(argv[2], &reference, &reference_count) ||
        !load_series(argv[3], &candidate, &candidate_count)) goto cleanup;
    if (!mblink_signal_correlation_best_linear(
            reference, reference_count, candidate, candidate_count,
            max_lag, step, tolerance, &result)) goto cleanup;

    printf("pairs=%zu\nlag_ms=%" PRId64 "\npearson_r=%.9f\n",
           result.pair_count, result.lag_ms, result.pearson_r);
    printf("slope=%.9f\nintercept=%.9f\nrmse=%.9f\n",
           result.slope, result.intercept, result.rmse);
    printf("normalized_rmse=%.9f\nscore=%.9f\nstrength=%s\n",
           result.normalized_rmse, result.score,
           mblink_signal_correlation_strength(&result));
    status = 0;
cleanup:
    free(reference); free(candidate);
    return status;
}

int main(int argc, char **argv)
{
    if (argc == 2 && strcmp(argv[1], "catalog") == 0)
        return command_catalog();
    if (argc == 4 && strcmp(argv[1], "decode") == 0)
        return command_decode(argv[2], argv[3]);
    if (argc >= 4 && argc <= 7 && strcmp(argv[1], "correlate") == 0)
        return command_correlate(argc, argv);
    usage(argv[0]);
    return 2;
}
