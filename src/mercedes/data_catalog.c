// SPDX-License-Identifier: GPL-3.0-or-later
/**
 * @file data_catalog.c
 * @brief Mercedes documented ECU data, controller profiles and route evidence.
 *
 * This file owns static diagnostic knowledge only. The transport/state-machine
 * implementation remains in data_scan.c.
 */
#include "mblink/mercedes_data_scan.h"
#include "mblink/mercedes_documented_ecus.h"
#include "mblink/mercedes_module_catalog.h"

#include "infiltratr/core.h"

#include <stdio.h>
#include <string.h>

#include "mercedes_documented_ecus.inc"
static const char k_documented_route_source[] =
    "panda-zhao/panda-zhao.github.io Foxwell/Xentry-derived Mercedes metadata";
static const char k_documented_field_source[] =
    "laravelcompany/ecudocs.com GPL-3.0 public ECU JSON definitions";

static const MblinkMercedesDocumentedField mblink_documented_fields[] = {
    {0x22,0xf100,"ECU software mode",4U,7,1U,NULL,k_documented_field_source},
    {0x22,0xf100,"Gateway flag",4U,6,1U,NULL,k_documented_field_source},
    {0x22,0xf100,"Identification",4U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf100,"Variant",5U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf100,"Version",6U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf100,"Session type",7U,1,1U,NULL,k_documented_field_source},
    {0x22,0xf111,"Mercedes Car Group hardware part number",4U,-1,0U,NULL,k_documented_field_source},
    {0x22,0xf121,"Mercedes Car Group software part number",4U,-1,0U,NULL,k_documented_field_source},
    {0x22,0xf150,"Hardware version year",4U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf150,"Hardware version week",5U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf150,"Hardware version patch level",6U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf151,"Software version year",4U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf151,"Software version week",5U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf151,"Software version patch level",6U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf153,"Boot software version year",4U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf153,"Boot software version week",5U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf153,"Boot software version patch level",6U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf154,"Hardware supplier",4U,-1,0U,NULL,k_documented_field_source},
    {0x22,0xf155,"Software supplier",4U,-1,0U,NULL,k_documented_field_source},
    {0x22,0xf15b,"Software programmed and valid",4U,7,1U,NULL,k_documented_field_source},
    {0x22,0xf15b,"Software mismatch",4U,6,1U,NULL,k_documented_field_source},
    {0x22,0xf15b,"Hardware mismatch",4U,5,1U,NULL,k_documented_field_source},
    {0x22,0xf15b,"Supplier identification",5U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf15b,"Programming date year",7U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf15b,"Programming date month",8U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf15b,"Programming date day",9U,-1,8U,NULL,k_documented_field_source},
    {0x22,0xf15b,"Programming tool serial number",10U,-1,0U,NULL,k_documented_field_source},
    {0x22,0xf18c,"ECU serial number",4U,-1,0U,NULL,k_documented_field_source},
    {0x22,0xf187,"Vehicle manufacturer spare part number",4U,-1,0U,NULL,"ISO 14229"},
    {0x22,0xf188,"Vehicle manufacturer ECU software number",4U,-1,0U,NULL,"ISO 14229"},
    {0x22,0xf190,"VIN original",4U,-1,0U,NULL,k_documented_field_source},
    {0x22,0xf191,"Vehicle manufacturer ECU hardware number",4U,-1,0U,NULL,"ISO 14229"},
    {0x22,0xf197,"System name",4U,-1,0U,NULL,"ISO 14229; vendor definitions can vary"},
    {0x22,0xf1a0,"VIN current",4U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x86,"DCS ECU identification record",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x87,"Vehicle manufacturer ECU identification / spare-part record",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x88,"Vehicle manufacturer ECU software number",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x89,"ECU software version / diagnostic variant",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x8a,"System supplier identifier",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x8b,"ECU manufacturing date",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x8c,"ECU serial number",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x90,"Vehicle identification number",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x97,"System name or engine type",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x98,"Repair shop code / tester serial number",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x99,"Programming date",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x9a,"Calibration repair-shop / equipment serial",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x9b,"ECU installation date",3U,-1,0U,NULL,k_documented_field_source},
    {0x1a,0x9c,"Calibration equipment software number",3U,-1,0U,NULL,k_documented_field_source}
};
typedef struct MblinkMercedesControllerProfileAlias { const char *family; const char *name; } MblinkMercedesControllerProfileAlias;
static const MblinkMercedesControllerProfileAlias mblink_documented_controller_aliases[] = {
    {"engine-crd3","CRD3"},
    {"transmission-egs51","EGS51"},{"transmission-egs52","EGS52"},{"transmission-egs53","EGS53"},{"transmission-vgs-nag2","VGSNAG2"},
    {"esp-abr2xt","ABR2XT_X"},{"esp-esp212","ESP212_X"},{"restraints-orc204","ORC_204"},{"restraints-orc212","ORC_212_X"},
    {"pretensioner-rbtmfl204","RBTMFL_204"},{"pretensioner-rbtmfr204","RBTMFR_204"},{"camera-mfk","MPC212"},
    {"fuel-pump-fscu","FSCM212"},{"cluster-ic204","IC_204"},{"cluster-ic212","IC_212"},{"headunit-hu204","HU_204"},
    {"audio-ctrlc204","CTRLC_204"},{"display-dispc204","DISPC_204"},{"gateway-cgw204","CGW_204"},{"gateway-cgw212","CGW_212_X"},
    {"eis-ezs204","EIS_204"},{"eis-ezs212","EIS_212_X"},{"steering-mrm","MRM221"},{"steering-sccm204","SCCM_204_X"},{"steering-sccm212","SCCM_212_X"},{"steering-scm","SCCM_212_X"},
    {"sam-front-212","SAMF_212"},{"sam-rear-212","SAMR_212"},{"climate-212","HVAC_212"},
    {"airmatic-ads212","ADS212"},{"distronic-dtr","DTR_212"},{"seat-driver-212","SEATD_212"},{"seat-passenger-204","SEATP_204"}
};
size_t mblink_mercedes_documented_ecu_profile_count(void){return INFILTRATR_ARRAY_LENGTH(mblink_documented_profiles);}
const MblinkMercedesDocumentedEcuProfile *mblink_mercedes_documented_ecu_profile_at(size_t i){return i<mblink_mercedes_documented_ecu_profile_count()?&mblink_documented_profiles[i]:NULL;}
size_t mblink_mercedes_documented_ecu_profile_count_for_route(uint32_t tx,uint32_t rx,bool ext){size_t n=0U;for(size_t i=0U;i<mblink_mercedes_documented_ecu_profile_count();++i){const MblinkMercedesDocumentedEcuProfile*p=&mblink_documented_profiles[i];if(p->route_available&&p->tx_can_id==tx&&p->rx_can_id==rx&&p->extended_id==ext)++n;}return n;}
const MblinkMercedesDocumentedEcuProfile *mblink_mercedes_documented_ecu_profile_at_for_route(uint32_t tx,uint32_t rx,bool ext,size_t wanted){size_t n=0U;for(size_t i=0U;i<mblink_mercedes_documented_ecu_profile_count();++i){const MblinkMercedesDocumentedEcuProfile*p=&mblink_documented_profiles[i];if(!p->route_available||p->tx_can_id!=tx||p->rx_can_id!=rx||p->extended_id!=ext)continue;if(n++==wanted)return p;}return NULL;}
const MblinkMercedesDocumentedEcuProfile *mblink_mercedes_documented_ecu_profile_for_name_and_route(const char*name,uint32_t tx,uint32_t rx,bool ext){if(name==NULL||name[0]=='\0')return NULL;for(size_t i=0U;i<mblink_mercedes_documented_ecu_profile_count();++i){const MblinkMercedesDocumentedEcuProfile*p=&mblink_documented_profiles[i];if(strcmp(p->name,name)==0&&(!p->route_available||(p->tx_can_id==tx&&p->rx_can_id==rx&&p->extended_id==ext)))return p;}return NULL;}
const MblinkMercedesDocumentedEcuProfile *mblink_mercedes_documented_ecu_profile_for_controller_family(const char*family,uint32_t tx,uint32_t rx,bool ext,MblinkMercedesDiagnosticProtocol protocol){if(family==NULL||family[0]=='\0')return NULL;for(size_t a=0U;a<INFILTRATR_ARRAY_LENGTH(mblink_documented_controller_aliases);++a){if(strcmp(mblink_documented_controller_aliases[a].family,family)!=0)continue;for(size_t i=0U;i<mblink_mercedes_documented_ecu_profile_count();++i){const MblinkMercedesDocumentedEcuProfile*p=&mblink_documented_profiles[i];if(strcmp(p->name,mblink_documented_controller_aliases[a].name)!=0)continue;if(p->route_available&&(p->tx_can_id!=tx||p->rx_can_id!=rx||p->extended_id!=ext))continue;if(p->protocol_known&&p->protocol!=protocol)continue;return p;}}return NULL;}
size_t mblink_mercedes_documented_ecu_read_count(const MblinkMercedesDocumentedEcuProfile*p){return p!=NULL?p->read_count:0U;}
const MblinkMercedesDocumentedRead *mblink_mercedes_documented_ecu_read_at(const MblinkMercedesDocumentedEcuProfile*p,size_t i){if(p==NULL||i>=p->read_count||p->read_offset+i>=INFILTRATR_ARRAY_LENGTH(mblink_documented_reads))return NULL;return &mblink_documented_reads[p->read_offset+i];}

static bool mblink_mercedes_documented_route_read_seen_before(
    uint32_t tx, uint32_t rx, bool ext,
    MblinkMercedesDiagnosticProtocol protocol,
    size_t profile_limit, size_t read_limit,
    uint8_t service, uint16_t identifier)
{
    size_t profile_index;
    for (profile_index = 0U; profile_index <= profile_limit; ++profile_index) {
        const MblinkMercedesDocumentedEcuProfile *profile =
            &mblink_documented_profiles[profile_index];
        size_t max_read;
        size_t read_index;

        if (!profile->route_available ||
            profile->tx_can_id != tx || profile->rx_can_id != rx ||
            profile->extended_id != ext ||
            (profile->protocol_known && profile->protocol != protocol)) {
            continue;
        }
        max_read = profile_index == profile_limit
            ? read_limit : profile->read_count;
        if (max_read > profile->read_count) max_read = profile->read_count;
        for (read_index = 0U; read_index < max_read; ++read_index) {
            const MblinkMercedesDocumentedRead *read =
                mblink_mercedes_documented_ecu_read_at(profile, read_index);
            if (read != NULL &&
                read->service == service &&
                read->identifier == identifier) {
                return true;
            }
        }
    }
    return false;
}

size_t mblink_mercedes_documented_route_read_count(
    uint32_t tx, uint32_t rx, bool ext,
    MblinkMercedesDiagnosticProtocol protocol)
{
    size_t count = 0U;
    size_t profile_index;

    for (profile_index = 0U;
         profile_index < mblink_mercedes_documented_ecu_profile_count();
         ++profile_index) {
        const MblinkMercedesDocumentedEcuProfile *profile =
            &mblink_documented_profiles[profile_index];
        size_t read_index;

        if (!profile->route_available ||
            profile->tx_can_id != tx || profile->rx_can_id != rx ||
            profile->extended_id != ext ||
            (profile->protocol_known && profile->protocol != protocol)) {
            continue;
        }
        for (read_index = 0U; read_index < profile->read_count; ++read_index) {
            const MblinkMercedesDocumentedRead *read =
                mblink_mercedes_documented_ecu_read_at(profile, read_index);
            if (read == NULL ||
                !mblink_mercedes_documented_read_is_safe(
                    read->service, read->identifier)) {
                continue;
            }
            if (!mblink_mercedes_documented_route_read_seen_before(
                    tx, rx, ext, protocol,
                    profile_index, read_index,
                    read->service, read->identifier)) {
                ++count;
            }
        }
    }
    return count;
}

const MblinkMercedesDocumentedRead *mblink_mercedes_documented_route_read_at(
    uint32_t tx, uint32_t rx, bool ext,
    MblinkMercedesDiagnosticProtocol protocol, size_t wanted)
{
    size_t match = 0U;
    size_t profile_index;

    for (profile_index = 0U;
         profile_index < mblink_mercedes_documented_ecu_profile_count();
         ++profile_index) {
        const MblinkMercedesDocumentedEcuProfile *profile =
            &mblink_documented_profiles[profile_index];
        size_t read_index;

        if (!profile->route_available ||
            profile->tx_can_id != tx || profile->rx_can_id != rx ||
            profile->extended_id != ext ||
            (profile->protocol_known && profile->protocol != protocol)) {
            continue;
        }
        for (read_index = 0U; read_index < profile->read_count; ++read_index) {
            const MblinkMercedesDocumentedRead *read =
                mblink_mercedes_documented_ecu_read_at(profile, read_index);
            if (read == NULL ||
                !mblink_mercedes_documented_read_is_safe(
                    read->service, read->identifier) ||
                mblink_mercedes_documented_route_read_seen_before(
                    tx, rx, ext, protocol,
                    profile_index, read_index,
                    read->service, read->identifier)) {
                continue;
            }
            if (match++ == wanted) return read;
        }
    }
    return NULL;
}

static bool mblink_mercedes_documented_normalize_control_command(
    const char *source,
    char command[5])
{
    const char *cursor = source;
    size_t index;

    if (command == NULL) return false;
    command[0] = '\0';
    if (cursor == NULL) return false;
    if (cursor[0] == '0' && (cursor[1] == 'x' || cursor[1] == 'X'))
        cursor += 2;
    if (strlen(cursor) != 4U) return false;

    for (index = 0U; index < 4U; ++index) {
        const char value = cursor[index];
        const bool digit = value >= '0' && value <= '9';
        const bool upper = value >= 'A' && value <= 'F';
        const bool lower = value >= 'a' && value <= 'f';
        if (!digit && !upper && !lower) return false;
        command[index] = lower ? (char)(value - 'a' + 'A') : value;
    }
    command[4] = '\0';
    return true;
}

bool mblink_mercedes_documented_route_control_command(
    uint32_t tx, uint32_t rx, bool ext,
    MblinkMercedesDiagnosticProtocol protocol, bool quit_session,
    char *buffer, size_t buffer_size)
{
    const size_t count =
        mblink_mercedes_documented_ecu_profile_count_for_route(tx, rx, ext);
    bool found = false;
    char resolved[5] = {0};

    if (buffer == NULL || buffer_size < sizeof(resolved)) {
        if (buffer != NULL && buffer_size != 0U) buffer[0] = '\0';
        return false;
    }
    buffer[0] = '\0';

    /*
     * Physical Mercedes routes are reused by several controller generations.
     * Session control is automatic only when all matching profiles for the
     * observed protocol agree. This prevents a route coincidence from choosing
     * another generation's diagnostic state machine.
     */
    for (size_t index = 0U; index < count; ++index) {
        const MblinkMercedesDocumentedEcuProfile *profile =
            mblink_mercedes_documented_ecu_profile_at_for_route(
                tx, rx, ext, index);
        const char *source;
        char candidate[5];

        if (profile == NULL ||
            (profile->protocol_known && profile->protocol != protocol)) {
            continue;
        }
        source = quit_session
            ? profile->quit_command : profile->session_command;
        if (!mblink_mercedes_documented_normalize_control_command(
                source, candidate)) {
            continue;
        }
        if (!found) {
            memcpy(resolved, candidate, sizeof(resolved));
            found = true;
        } else if (strcmp(resolved, candidate) != 0) {
            return false;
        }
    }

    if (!found) return false;
    memcpy(buffer, resolved, sizeof(resolved));
    return true;
}

MblinkMercedesDocumentedReadLayout mblink_mercedes_documented_read_layout(
    uint8_t service, uint16_t identifier)
{
    static const struct {
        uint8_t service;
        uint16_t identifier;
        MblinkMercedesDocumentedReadLayout layout;
    } layouts[] = {
        { UINT8_C(0x22), UINT16_C(0xf150),
          MBLINK_MERCEDES_DOCUMENTED_READ_LAYOUT_YEAR_WEEK_PATCH },
        { UINT8_C(0x22), UINT16_C(0xf151),
          MBLINK_MERCEDES_DOCUMENTED_READ_LAYOUT_YEAR_WEEK_PATCH },
        { UINT8_C(0x22), UINT16_C(0xf153),
          MBLINK_MERCEDES_DOCUMENTED_READ_LAYOUT_YEAR_WEEK_PATCH }
    };

    for (size_t index = 0U;
         index < INFILTRATR_ARRAY_LENGTH(layouts); ++index) {
        if (layouts[index].service == service &&
            layouts[index].identifier == identifier) {
            return layouts[index].layout;
        }
    }
    return MBLINK_MERCEDES_DOCUMENTED_READ_LAYOUT_DEFAULT;
}

const char *mblink_mercedes_documented_read_name(uint8_t s,uint16_t id){
 if(s==0x22){switch(id){case 0xf100:return"Active diagnostic information";case 0xf111:return"Mercedes hardware part number";case 0xf121:return"Mercedes software part number";case 0xf150:return"Hardware version";case 0xf151:return"Software version";case 0xf153:return"Boot software version";case 0xf154:return"Hardware supplier";case 0xf155:return"Software supplier";case 0xf15b:return"Programming fingerprint";case 0xf18c:return"ECU serial number";case 0xf187:return"Vehicle manufacturer spare part number";case 0xf188:return"Vehicle manufacturer ECU software number";case 0xf190:return"VIN original";case 0xf191:return"Vehicle manufacturer ECU hardware number";case 0xf197:return"System name";case 0xf1a0:return"VIN current";default:return NULL;}}
 if(s==0x1a){switch(id){case 0x86:return"DCS ECU identification";case 0x87:return"Vehicle manufacturer ECU identification";case 0x88:return"Vehicle manufacturer ECU software number";case 0x89:return"ECU software version / diagnostic variant";case 0x8a:return"System supplier identifier";case 0x8b:return"ECU manufacturing date";case 0x8c:return"ECU serial number";case 0x90:return"Vehicle identification number";case 0x97:return"System name or engine type";case 0x98:return"Repair shop / tester serial";case 0x99:return"Programming date";case 0x9a:return"Calibration repair-shop / equipment serial";case 0x9b:return"ECU installation date";case 0x9c:return"Calibration equipment software number";default:return NULL;}}
 return NULL;
}
bool mblink_mercedes_documented_read_is_safe(uint8_t s,uint16_t id){if(s==0x22)return true;if((s==0x21||s==0x1a)&&id!=0U&&id<=0xffU)return true;return false;}
bool mblink_mercedes_documented_read_is_module_metadata(
    uint8_t service, uint16_t identifier)
{
    if (service == UINT8_C(0x22)) {
        switch (identifier) {
        case UINT16_C(0xf111):
        case UINT16_C(0xf121):
        case UINT16_C(0xf150):
        case UINT16_C(0xf151):
        case UINT16_C(0xf153):
        case UINT16_C(0xf187):
        case UINT16_C(0xf188):
        case UINT16_C(0xf189):
        case UINT16_C(0xf191):
        case UINT16_C(0xf197):
        case UINT16_C(0xf192):
        case UINT16_C(0xf193):
        case UINT16_C(0xf194):
        case UINT16_C(0xf195):
            return true;
        default:
            return false;
        }
    }
    /* KWP identification also includes serial/date/calibration records above
     * 0x89. Keep the whole identification block out of recurring polling. */
    return service == UINT8_C(0x1a) &&
           identifier >= UINT16_C(0x0086) &&
           identifier <= UINT16_C(0x009f);
}
size_t mblink_mercedes_documented_field_count(uint8_t s,uint16_t id){size_t n=0U;for(size_t i=0U;i<INFILTRATR_ARRAY_LENGTH(mblink_documented_fields);++i)if(mblink_documented_fields[i].service==s&&mblink_documented_fields[i].identifier==id)++n;return n;}
const MblinkMercedesDocumentedField *mblink_mercedes_documented_field_at(uint8_t s,uint16_t id,size_t wanted){size_t n=0U;for(size_t i=0U;i<INFILTRATR_ARRAY_LENGTH(mblink_documented_fields);++i){const MblinkMercedesDocumentedField*f=&mblink_documented_fields[i];if(f->service!=s||f->identifier!=id)continue;if(n++==wanted)return f;}return NULL;}
const char *mblink_mercedes_documented_route_source(void){return k_documented_route_source;}
const char *mblink_mercedes_documented_field_source(void){return k_documented_field_source;}

/*
 * Controller-scoped read-only data profiles.
 *
 * These are deliberately keyed by ECU family rather than CAN route. The same
 * address or broad module kind can host different controller generations with
 * different diagnostic namespaces.
 *
 * The three raw records below are vehicle-verified only as positive identifiers
 * on the named C207 controller families; their semantics remain unknown.
 */
static const char k_20260903_field_evidence_provenance[] =
    "De-identified 2026-09-03 MBLINK vehicle captures: exact read-only "
    "request returned positively; semantic meaning remains unasserted unless "
    "independently documented.";

/*
 * Controller-scoped read-only data profiles.
 *
 * Family profiles own semantic applicability. Unknown records stay RAW; a
 * positive response proves only that the selected controller supports the
 * read-only identifier, not what the payload means.
 */
static const MblinkMercedesControllerDataProfileEntry
    controller_data_profile[] = {
#include "cbf_controller_data.inc"
    /*
     * Public Mercedes Vediamo CBF data services.
     *
     * These are controller-scoped documented Data services, not observations
     * from a particular vehicle. PID Setup is the user's monitoring selector,
     * so every source-backed controller Data service belongs in that catalogue
     * whether its value changes rapidly, rarely, or appears static in a given
     * capture. Vehicle response never creates or removes catalogue membership.
     * Identification/programming/session records remain separate categories.
     */{
        "engine-crd3", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0x1001), true,
        "SCN / calibration identification",
        MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
        "CaesarSuite CRD3.CBF analysis · DT_SCN_Lesen · UDS 22 10 01"
    },
    {
        "engine-crd3", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0x1002), true,
        "Explicit variant-coding data (30 bytes)",
        MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
        "CaesarSuite CRD3.CBF J2534 trace · "
        "DT_RVC_CRD3_explizit_restricted_30Byte · positive 62 10 02 response"
    },
    {
        "engine-crd3", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0xF804), true,
        "Calibration identification",
        MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
        "CaesarSuite CRD3.CBF analysis · "
        "DT_STO_ID_Calibration_Identification · UDS 22 F8 04"
    },
    {
        "engine-crd3", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0xF806), true,
        "Calibration verification number",
        MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
        "CaesarSuite CRD3.CBF analysis · "
        "DT_STO_ID_Calibration_Verification_Number · UDS 22 F8 06"
    },
    {
        "engine-crd3", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0x2007), true, "Battery voltage",
        MBLINK_MERCEDES_DEFINITION_SOURCE_CORROBORATED,
        "CaesarSuite CRD3 DT_2007 documents DID 0x2007 and its scaling; "
        "field capture independently proves the positive response."
    },{ "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0x200F), false,
        "Observed raw DID 0x200F",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },{ "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0x2017), false,
        "Observed raw DID 0x2017",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0x2023), false,
        "Observed raw DID 0x2023",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0x2043), false,
        "Observed raw DID 0x2043",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },{ "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0x2070), false,
        "Observed raw DID 0x2070",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0x20C0), false,
        "Observed raw DID 0x20C0",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "esp-abr2xt", MBLINK_MERCEDES_DIAGNOSTIC_UDS,
        UINT8_C(0x22), UINT16_C(0x20DF), false,
        "Observed raw DID 0x20DF",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x01), false,
        "Observed raw local record 0x01",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x02), false,
        "Observed raw local record 0x02",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x07), false,
        "Observed raw local record 0x07",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x0D), true,
        "Observed raw local record 0x0D",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x0F), true,
        "Observed raw local record 0x0F",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x11), true,
        "Observed raw local record 0x11",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x13), true,
        "Observed raw local record 0x13",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x18), false,
        "Observed raw local record 0x18",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x23), false,
        "Observed raw local record 0x23",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x24), false,
        "Observed raw local record 0x24",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x2B), false,
        "Observed raw local record 0x2B",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x2D), false,
        "Observed raw local record 0x2D",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x51), false,
        "Observed raw local record 0x51",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x52), false,
        "Observed raw local record 0x52",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x0058), false,
        "Observed static raw local record 0x58",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x59), false,
        "Observed raw local record 0x59",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x60), false,
        "Observed raw local record 0x60",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x61), false,
        "Observed raw local record 0x61",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x62), false,
        "Observed raw local record 0x62",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x63), false,
        "Observed raw local record 0x63",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x64), false,
        "Observed raw local record 0x64",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x65), false,
        "Observed raw local record 0x65",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x69), false,
        "Observed raw local record 0x69",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x70), false,
        "Observed raw local record 0x70",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x71), false,
        "Observed raw local record 0x71",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x72), false,
        "Observed raw local record 0x72",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x77), false,
        "Observed raw local record 0x77",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0xE0), false,
        "Observed raw local record 0xE0",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
    { "restraints-orc212", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0xE4), false,
        "Observed raw local record 0xE4",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
        { "headunit-hu204", MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
        UINT8_C(0x21), UINT16_C(0x02), false,
        "Observed raw local record 0x02",
        MBLINK_MERCEDES_DEFINITION_VEHICLE_VERIFIED,
        k_20260903_field_evidence_provenance },
        };

/*
 * Exact-route fallback from the same field captures.
 *
 * This exists specifically so stricter family classification cannot make a
 * previously proven ECU go silent while identity is still unresolved. Route
 * evidence never supplies semantic decoding and never authorises writes.
 */
static const MblinkMercedesRouteEvidenceEntry route_evidence[] = {
    { UINT32_C(0x7e1), UINT32_C(0x7e9),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_TRANSMISSION, UINT16_C(0x30),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x7e1), UINT32_C(0x7e9),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_TRANSMISSION, UINT16_C(0x31),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x7e1), UINT32_C(0x7e9),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_TRANSMISSION, UINT16_C(0x32),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x7e1), UINT32_C(0x7e9),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_TRANSMISSION, UINT16_C(0x33),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2001),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2003),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2004),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2007),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2009),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x200A),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x200D),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x200F),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2010),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2014),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2017),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2023),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2043),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2046),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2047),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x2070),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x20C0),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x632), UINT32_C(0x486),
      false, MBLINK_MERCEDES_DIAGNOSTIC_UDS,
      MBLINK_MERCEDES_MODULE_ABS_ESP, UINT16_C(0x20DF),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x01),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x02),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x07),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x0D),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x0F),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x11),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x13),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x18),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x23),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x24),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x2B),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x2D),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x51),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x52),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x0058),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x59),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x60),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x61),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x62),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x63),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x64),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x65),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x69),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x70),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x71),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x72),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0x77),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0xE0),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x64a), UINT32_C(0x489),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_RESTRAINTS, UINT16_C(0xE4),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x652), UINT32_C(0x48a),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_BODY, UINT16_C(0x01),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x652), UINT32_C(0x48a),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_BODY, UINT16_C(0x02),
      false, k_20260903_field_evidence_provenance },
    { UINT32_C(0x652), UINT32_C(0x48a),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_BODY, UINT16_C(0x05),
      true, k_20260903_field_evidence_provenance },
    { UINT32_C(0x652), UINT32_C(0x48a),
      false, MBLINK_MERCEDES_DIAGNOSTIC_KWP2000,
      MBLINK_MERCEDES_MODULE_BODY, UINT16_C(0x06),
      true, k_20260903_field_evidence_provenance },
};

const char *mblink_mercedes_data_profile_key_for_controller(
    const char *module_key,
    const char *identity,
    const char *software_number,
    const char *hardware_number)
{
    const MblinkMercedesControllerFamilyDefinition *family =
        mblink_mercedes_controller_family_definition_for_evidence(
            module_key, identity, software_number, hardware_number);
    size_t index;

    if (family == NULL || family->key == NULL) return NULL;

    /*
     * A controller family has a data profile exactly when the profile table
     * contains rows under that family key. Keep that ownership data-driven so
     * adding a sourced ECU family never requires another hard-coded if-chain.
     */
    for (index = 0U;
         index < INFILTRATR_ARRAY_LENGTH(controller_data_profile);
         ++index) {
        if (strcmp(controller_data_profile[index].profile_key, family->key) == 0)
            return family->key;
    }
    return NULL;
}

size_t mblink_mercedes_controller_data_profile_identifier_count(
    const char *profile_key,
    MblinkMercedesDiagnosticProtocol protocol)
{
    size_t count = 0U;
    size_t index;

    if (profile_key == NULL || profile_key[0] == '\0') return 0U;
    for (index = 0U;
         index < INFILTRATR_ARRAY_LENGTH(controller_data_profile);
         ++index) {
        const MblinkMercedesControllerDataProfileEntry *entry =
            &controller_data_profile[index];
        if (entry->protocol != protocol ||
            strcmp(entry->profile_key, profile_key) != 0) {
            continue;
        }
        ++count;
    }
    return count;
}

const MblinkMercedesControllerDataProfileEntry *
mblink_mercedes_controller_data_profile_identifier_at(
    const char *profile_key,
    MblinkMercedesDiagnosticProtocol protocol,
    size_t requested_index)
{
    size_t match_index = 0U;
    size_t index;

    if (profile_key == NULL || profile_key[0] == '\0') return NULL;
    for (index = 0U;
         index < INFILTRATR_ARRAY_LENGTH(controller_data_profile);
         ++index) {
        const MblinkMercedesControllerDataProfileEntry *entry =
            &controller_data_profile[index];
        if (entry->protocol != protocol ||
            strcmp(entry->profile_key, profile_key) != 0) {
            continue;
        }
        if (match_index == requested_index) return entry;
        ++match_index;
    }
    return NULL;
}

const MblinkMercedesControllerDataProfileEntry *
mblink_mercedes_controller_data_profile_find_service(
    const char *profile_key,
    MblinkMercedesDiagnosticProtocol protocol,
    uint8_t service,
    uint16_t identifier)
{
    size_t index;

    if (profile_key == NULL || profile_key[0] == '\0') return NULL;
    for (index = 0U;
         index < INFILTRATR_ARRAY_LENGTH(controller_data_profile);
         ++index) {
        const MblinkMercedesControllerDataProfileEntry *entry =
            &controller_data_profile[index];
        if (entry->protocol == protocol &&
            entry->service == service &&
            entry->identifier == identifier &&
            strcmp(entry->profile_key, profile_key) == 0) {
            return entry;
        }
    }
    return NULL;
}

const MblinkMercedesControllerDataProfileEntry *
mblink_mercedes_controller_data_profile_find(
    const char *profile_key,
    MblinkMercedesDiagnosticProtocol protocol,
    uint16_t identifier)
{
    size_t index;

    if (profile_key == NULL || profile_key[0] == '\0') return NULL;
    for (index = 0U;
         index < INFILTRATR_ARRAY_LENGTH(controller_data_profile);
         ++index) {
        const MblinkMercedesControllerDataProfileEntry *entry =
            &controller_data_profile[index];
        if (entry->protocol == protocol &&
            entry->identifier == identifier &&
            strcmp(entry->profile_key, profile_key) == 0) {
            return entry;
        }
    }
    return NULL;
}

static bool route_evidence_matches(
    const MblinkMercedesRouteEvidenceEntry *entry,
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol,
    MblinkMercedesModuleKind module_kind)
{
    return entry != NULL &&
           entry->tx_can_id == tx_can_id &&
           entry->rx_can_id == rx_can_id &&
           entry->extended_id == extended_id &&
           entry->protocol == protocol &&
           entry->module_kind == module_kind;
}

size_t mblink_mercedes_route_evidence_identifier_count(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol,
    MblinkMercedesModuleKind module_kind)
{
    size_t count = 0U;
    size_t index;
    for (index = 0U;
         index < INFILTRATR_ARRAY_LENGTH(route_evidence);
         ++index) {
        if (route_evidence_matches(
                &route_evidence[index], tx_can_id, rx_can_id,
                extended_id, protocol, module_kind)) {
            ++count;
        }
    }
    return count;
}

const MblinkMercedesRouteEvidenceEntry *
mblink_mercedes_route_evidence_identifier_at(
    uint32_t tx_can_id,
    uint32_t rx_can_id,
    bool extended_id,
    MblinkMercedesDiagnosticProtocol protocol,
    MblinkMercedesModuleKind module_kind,
    size_t requested_index)
{
    size_t match_index = 0U;
    size_t index;
    for (index = 0U;
         index < INFILTRATR_ARRAY_LENGTH(route_evidence);
         ++index) {
        if (!route_evidence_matches(
                &route_evidence[index], tx_can_id, rx_can_id,
                extended_id, protocol, module_kind)) {
            continue;
        }
        if (match_index == requested_index) return &route_evidence[index];
        ++match_index;
    }
    return NULL;
}
