// SPDX-License-Identifier: GPL-3.0-or-later
/*
 * Extract read-only Mercedes Vediamo CBF diagnostic facts.
 *
 * This tool intentionally emits only direct executable UDS ReadDataByIdentifier
 * Data services (22 xxxx). It does not import routines, IO control, coding,
 * security access, writes, or inferred semantics.
 */
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CBF_STUB_SIZE UINT32_C(0x410)

typedef struct {
    const uint8_t *data;
    size_t size;
    size_t pos;
    bool ok;
} Reader;

typedef struct {
    uint32_t cff_size;
    uint32_t base;
    int32_t caesar_version;
    int32_t ecu_count;
    int32_t ecu_offset;
    int32_t string_pool_size;
} CffHeader;

typedef struct {
    const char *qualifier;
    uint32_t diag_block;
    int32_t diag_count;
    int32_t diag_entry_size;
} EcuHeader;

typedef struct {
    const char *qualifier;
    uint16_t type;
    uint16_t executable;
    uint16_t client_access;
    uint16_t security_access;
    int16_t request_count;
    int32_t request_offset;
} DiagService;

static bool can_read(const Reader *reader, size_t count)
{
    return reader != NULL && reader->ok &&
        reader->pos <= reader->size && count <= reader->size - reader->pos;
}

static uint16_t read_u16(Reader *reader)
{
    uint16_t value = 0U;
    if (!can_read(reader, 2U)) {
        if (reader != NULL) reader->ok = false;
        return 0U;
    }
    value = (uint16_t)reader->data[reader->pos] |
        (uint16_t)((uint16_t)reader->data[reader->pos + 1U] << 8U);
    reader->pos += 2U;
    return value;
}

static int16_t read_i16(Reader *reader)
{
    return (int16_t)read_u16(reader);
}

static uint32_t read_u32(Reader *reader)
{
    uint32_t value = 0U;
    if (!can_read(reader, 4U)) {
        if (reader != NULL) reader->ok = false;
        return 0U;
    }
    value = (uint32_t)reader->data[reader->pos] |
        ((uint32_t)reader->data[reader->pos + 1U] << 8U) |
        ((uint32_t)reader->data[reader->pos + 2U] << 16U) |
        ((uint32_t)reader->data[reader->pos + 3U] << 24U);
    reader->pos += 4U;
    return value;
}

static int32_t read_i32(Reader *reader)
{
    return (int32_t)read_u32(reader);
}

static bool seek_to(Reader *reader, uint64_t offset)
{
    if (reader == NULL || offset > reader->size) {
        if (reader != NULL) reader->ok = false;
        return false;
    }
    reader->pos = (size_t)offset;
    return true;
}

static bool add_relative_offset(
    uint64_t base, int32_t relative, size_t limit, uint64_t *absolute)
{
    int64_t resolved;

    if (absolute == NULL || base > (uint64_t)INT64_MAX)
        return false;
    resolved = (int64_t)base + (int64_t)relative;
    if (resolved < 0 || (uint64_t)resolved > (uint64_t)limit)
        return false;
    *absolute = (uint64_t)resolved;
    return true;
}

static bool next_flag(uint32_t *flags)
{
    bool set;
    if (flags == NULL) return false;
    set = (*flags & UINT32_C(1)) != 0U;
    *flags >>= 1U;
    return set;
}

static int32_t read_flag_i32(Reader *reader, uint32_t *flags, int32_t fallback)
{
    return next_flag(flags) ? read_i32(reader) : fallback;
}

static int16_t read_flag_i16(Reader *reader, uint32_t *flags, int16_t fallback)
{
    return next_flag(flags) ? read_i16(reader) : fallback;
}

static uint16_t read_flag_u16(Reader *reader, uint32_t *flags, uint16_t fallback)
{
    return next_flag(flags) ? read_u16(reader) : fallback;
}

static const char *string_at(const Reader *reader, uint64_t offset)
{
    size_t index;
    if (reader == NULL || offset >= reader->size) return NULL;
    for (index = (size_t)offset; index < reader->size; ++index) {
        if (reader->data[index] == 0U)
            return (const char *)&reader->data[(size_t)offset];
    }
    return NULL;
}

static const char *read_flag_string(
    Reader *reader, uint32_t *flags, uint32_t base)
{
    int32_t relative;
    int64_t absolute;
    if (!next_flag(flags)) return NULL;
    relative = read_i32(reader);
    absolute = (int64_t)base + (int64_t)relative;
    if (!reader->ok || absolute < 0) {
        reader->ok = false;
        return NULL;
    }
    return string_at(reader, (uint64_t)absolute);
}

static bool parse_cff_header(Reader *reader, CffHeader *header)
{
    uint32_t flags;
    if (reader == NULL || header == NULL ||
        !seek_to(reader, CBF_STUB_SIZE)) return false;

    memset(header, 0, sizeof(*header));
    header->cff_size = read_u32(reader);
    header->base = (uint32_t)reader->pos;
    flags = (uint32_t)read_u16(reader);

    header->caesar_version = read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0); /* GPD version */
    header->ecu_count = read_flag_i32(reader, &flags, 0);
    header->ecu_offset = read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0); /* CTF offset */
    header->string_pool_size = read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0); /* DSC offset */
    (void)read_flag_i32(reader, &flags, 0); /* DSC count */
    (void)read_flag_i32(reader, &flags, 0); /* DSC entry size */
    (void)read_flag_string(reader, &flags, header->base);
    (void)read_flag_string(reader, &flags, header->base);

    return reader->ok && header->caesar_version >= 400 &&
        header->ecu_count > 0 && header->ecu_count < 4096 &&
        header->ecu_offset > 0 && header->string_pool_size > 0;
}

static bool skip_ecu_prefix(
    Reader *reader, uint32_t *flags, uint32_t base)
{
    (void)read_flag_i32(reader, flags, -1); /* name */
    (void)read_flag_i32(reader, flags, -1); /* description */
    (void)read_flag_string(reader, flags, base); /* XML version */
    (void)read_flag_i32(reader, flags, 0); /* interface count */
    (void)read_flag_i32(reader, flags, 0); /* interface offset */
    (void)read_flag_i32(reader, flags, 0); /* subtype count */
    (void)read_flag_i32(reader, flags, 0); /* subtype offset */
    (void)read_flag_string(reader, flags, base); /* class */
    (void)read_flag_string(reader, flags, base); /* unknown */
    (void)read_flag_string(reader, flags, base); /* unknown */
    (void)read_flag_i16(reader, flags, 0); /* ignition */
    (void)read_flag_i16(reader, flags, 0);
    (void)read_flag_i16(reader, flags, 0);
    (void)read_flag_i32(reader, flags, 0);
    (void)read_flag_i16(reader, flags, 0);
    (void)read_flag_i32(reader, flags, 0);
    return reader->ok;
}

static bool parse_ecu_header(
    Reader *reader, uint32_t base, const CffHeader *cff, EcuHeader *header)
{
    uint32_t flags;
    uint64_t data_base;
    int32_t diag_relative;

    if (reader == NULL || cff == NULL || header == NULL ||
        !seek_to(reader, base)) return false;

    memset(header, 0, sizeof(*header));
    flags = read_u32(reader);
    (void)read_u16(reader); /* extended flags begin after fields we need */
    (void)read_i32(reader); /* ECU header id */

    header->qualifier = read_flag_string(reader, &flags, base);
    if (!skip_ecu_prefix(reader, &flags, base)) return false;

    data_base = (uint64_t)(uint32_t)cff->string_pool_size +
        CBF_STUB_SIZE + (uint64_t)cff->cff_size + UINT64_C(4);
    if (data_base > UINT32_MAX) return false;

    /* Variant pool. */
    (void)read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0);

    /* Diagnostic-service pool. */
    diag_relative = read_flag_i32(reader, &flags, 0);
    header->diag_count = read_flag_i32(reader, &flags, 0);
    header->diag_entry_size = read_flag_i32(reader, &flags, 0);
    (void)read_flag_i32(reader, &flags, 0);

    if (!reader->ok || header->diag_count < 0 ||
        header->diag_entry_size < 14) return false;
    {
        uint64_t diag_block;
        if (!add_relative_offset(
                data_base, diag_relative, reader->size, &diag_block) ||
            diag_block > UINT32_MAX)
            return false;
        header->diag_block = (uint32_t)diag_block;
    }
    return header->qualifier != NULL;
}

static bool parse_service(
    Reader *reader, uint32_t base, DiagService *service)
{
    uint32_t flags;

    if (reader == NULL || service == NULL || !seek_to(reader, base))
        return false;

    memset(service, 0, sizeof(*service));
    flags = read_u32(reader);
    (void)read_u32(reader); /* extended flags not needed for request identity */

    service->qualifier = read_flag_string(reader, &flags, base);
    (void)read_flag_i32(reader, &flags, -1); /* name CTF */
    (void)read_flag_i32(reader, &flags, -1); /* description CTF */
    service->type = read_flag_u16(reader, &flags, 0U);
    service->executable = read_flag_u16(reader, &flags, 0U);
    service->client_access = read_flag_u16(reader, &flags, 0U);
    service->security_access = read_flag_u16(reader, &flags, 0U);

    (void)read_flag_i32(reader, &flags, 0); /* comparam count */
    (void)read_flag_i32(reader, &flags, 0); /* comparam offset */
    (void)read_flag_i32(reader, &flags, 0); /* Q count */
    (void)read_flag_i32(reader, &flags, 0); /* Q offset */
    (void)read_flag_i32(reader, &flags, 0); /* R count */
    (void)read_flag_i32(reader, &flags, 0); /* R offset */
    (void)read_flag_string(reader, &flags, base); /* input ref */
    (void)read_flag_i32(reader, &flags, 0); /* prep count */
    (void)read_flag_i32(reader, &flags, 0); /* prep offset */
    (void)read_flag_i32(reader, &flags, 0); /* V count */
    (void)read_flag_i32(reader, &flags, 0); /* V offset */
    service->request_count = read_flag_i16(reader, &flags, 0);
    service->request_offset = read_flag_i32(reader, &flags, 0);

    return reader->ok && service->qualifier != NULL;
}

static uint8_t *read_file(const char *path, size_t *size_out)
{
    FILE *file;
    long length;
    uint8_t *buffer;
    size_t read_count;

    if (path == NULL || size_out == NULL) return NULL;
    file = fopen(path, "rb");
    if (file == NULL) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0) {
        fclose(file);
        return NULL;
    }
    length = ftell(file);
    if (length <= 0 || (uint64_t)length > SIZE_MAX) {
        fclose(file);
        return NULL;
    }
    if (fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    buffer = malloc((size_t)length);
    if (buffer == NULL) {
        fclose(file);
        return NULL;
    }
    read_count = fread(buffer, 1U, (size_t)length, file);
    fclose(file);
    if (read_count != (size_t)length) {
        free(buffer);
        return NULL;
    }
    *size_out = (size_t)length;
    return buffer;
}

static int emit_direct_uds22(const uint8_t *data, size_t size)
{
    Reader reader = { data, size, 0U, true };
    CffHeader cff;
    uint64_t ecu_table;
    int32_t ecu_index;
    unsigned int emitted = 0U;

    if (!parse_cff_header(&reader, &cff)) {
        fprintf(stderr, "invalid or unsupported CBF header\n");
        return 2;
    }

    ecu_table = (uint64_t)cff.base + (uint64_t)(uint32_t)cff.ecu_offset;
    if (ecu_table > size) {
        fprintf(stderr, "invalid ECU table offset\n");
        return 2;
    }

    puts("ecu\tdid\tqualifier\tclient_access\tsecurity_access");

    for (ecu_index = 0; ecu_index < cff.ecu_count; ++ecu_index) {
        EcuHeader ecu;
        uint64_t table_entry = ecu_table + (uint64_t)(uint32_t)ecu_index * 4U;
        int32_t ecu_relative;
        int32_t service_index;

        if (!seek_to(&reader, table_entry)) return 2;
        ecu_relative = read_i32(&reader);
        if (!reader.ok || ecu_relative < 0 ||
            (uint64_t)(uint32_t)ecu_relative + ecu_table > UINT32_MAX)
            return 2;
        if (!parse_ecu_header(&reader,
                (uint32_t)(ecu_table + (uint64_t)(uint32_t)ecu_relative),
                &cff, &ecu)) {
            fprintf(stderr, "failed to parse ECU %" PRId32 "\n", ecu_index);
            return 2;
        }

        for (service_index = 0; service_index < ecu.diag_count;
             ++service_index) {
            uint64_t entry = (uint64_t)ecu.diag_block +
                (uint64_t)(uint32_t)service_index *
                (uint64_t)(uint32_t)ecu.diag_entry_size;
            int32_t service_relative;
            uint64_t service_base;
            DiagService service;
            uint64_t request_base;
            uint16_t did;

            if (entry + 14U > size || !seek_to(&reader, entry))
                return 2;
            service_relative = read_i32(&reader);
            (void)read_i32(&reader); /* entry size */
            (void)read_u32(&reader); /* CRC */
            (void)read_u16(&reader); /* config */
            if (!reader.ok ||
                !add_relative_offset(
                    (uint64_t)ecu.diag_block, service_relative,
                    size, &service_base) ||
                service_base > UINT32_MAX ||
                !parse_service(&reader, (uint32_t)service_base, &service))
                return 2;

            /* Caesar service class 5 is Data. Keep only direct UDS 0x22 reads. */
            if (service.type != 5U || service.executable == 0U ||
                service.request_count != 3)
                continue;

            if (!add_relative_offset(
                    service_base, service.request_offset,
                    size, &request_base) ||
                request_base > (uint64_t)size - 3U)
                return 2;
            if (data[(size_t)request_base] != UINT8_C(0x22))
                continue;

            did = (uint16_t)((uint16_t)data[(size_t)request_base + 1U] << 8U) |
                (uint16_t)data[(size_t)request_base + 2U];
            printf("%s\t%04" PRIX16 "\t%s\t%" PRIu16 "\t%" PRIu16 "\n",
                ecu.qualifier, did, service.qualifier,
                service.client_access, service.security_access);
            emitted++;
        }
    }

    fprintf(stderr, "direct_uds22_rows=%u\n", emitted);
    return 0;
}

int main(int argc, char **argv)
{
    uint8_t *data;
    size_t size = 0U;
    int result;

    if (argc != 2) {
        fprintf(stderr, "usage: %s FILE.cbf\n", argv[0]);
        return 64;
    }

    data = read_file(argv[1], &size);
    if (data == NULL) {
        fprintf(stderr, "failed to read %s: %s\n",
            argv[1], strerror(errno));
        return 66;
    }

    result = emit_direct_uds22(data, size);
    free(data);
    return result;
}
