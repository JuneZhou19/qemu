#include "qemu/osdep.h"
#include "qapi/qmp/qerror.h"
#include "qemu/error-report.h"
#include "qapi/error.h"
#include "qemu/cutils.h"
#include "trace.h"
#include "sysemu/sysemu.h"
#include "hw/scsi/scsi.h"
#include "block/scsi.h"
#include "sysemu/block-backend.h"
#include "sysemu/blockdev.h"
#include "hw/loader.h"
#include "eses.h"
#include "scsi-disk.h"

#include "qmp-commands.h"
#ifdef __linux
#include <unistd.h>
#endif


/*************************************************************************************/
#define PAGE_NUMBER (sizeof(page_support)/sizeof(page_contain_t))
#define PAGE_SIZE (2048)
#define SUBPAGE_HEAD_SIZE 4
#define PARAMETER_HEAD_SIZE 4
typedef struct error_inject_descriptor {
    const char *page_type;
    uint32_t page_code;
    uint32_t subpage_code;
    uint32_t parameter_code;
    uint8_t parameter_length;
    char *value;
}error_inject_descriptor_t;

error_inject_descriptor_t error_inject_map[] = {
                    {"life_used", 0x11, 0x00, 0x0001, 4, 0},
                    {"erase_count", 0x31, 0x00, 0x8000, 8, 0}
                    };


typedef struct page_contain_s{
    uint8_t page_code;
    uint8_t subpage_code;
    uint8_t *data_buffer;
}page_contain_t;

page_contain_t page_support[] = {
    { 0, 0, NULL},
    { 0, 0xFF, NULL},
    { 0x11, 0x00, NULL},
    { 0x18, 0x00, NULL},
    { 0x2f, 0x00, NULL},
    { 0x31, 0x00, NULL}
};

// refer to SAS2r16.pdf, Chapter 10.2.8 SCSI log parameters
typedef struct sas_phy_log_descriptor
{
    uint8_t rsv0;
    uint8_t phy_id;
    uint8_t rsv1;
    uint8_t sas_phy_log_desc_len;
    uint8_t atta_reason:4;
    uint8_t attached_device_type:3;
    uint8_t rsv2:1;

    uint8_t negotiated_logic_link_rate:4;
    uint8_t reason:4;

    uint8_t rsv3:1;
    uint8_t atta_smp_i_port: 1;
    uint8_t atta_stp_i_port: 1;
    uint8_t atta_ssp_i_port: 1;
    uint8_t rsv4:4;

    uint8_t rsv5:1;
    uint8_t atta_smp_t_port:1;
    uint8_t atta_stp_t_port:1;
    uint8_t atta_ssp_t_port:1;
    uint8_t rsv6:4;

    uint64_t sas_addr;          // byte 8~15
    uint64_t attached_sas_addr; // byte 16~23
    uint8_t attached_phy_id;    // byte 24
    uint8_t rsv7[7];            // byte 25~31

    uint32_t invalid_dword_count;   // 32~35
    uint32_t running_disparity_error_count;  // 36~39
    uint32_t loss_dword_synchronization;  // 40~43
    uint32_t phy_reset_problem;   // 44~47
    uint16_t resv8;   //48~49

    uint8_t phy_event_descriptor_len;  // 50
    uint8_t number_phy_event_descriptors; // 51

} __attribute__((packed, aligned(4))) sas_phy_log_descriptor_t;

typedef struct phy_event_descriptor
{
    uint16_t reserved0;
    uint8_t  reserved1;
    uint8_t  phy_event_source;
    uint32_t phy_event;
    uint32_t peak_value_detector_threshold;
} __attribute__((packed)) phy_event_descriptor_t;

static error_inject_descriptor_t *find_map_by_type(const char *type)
{
    int i;
    for(i=0; i<(sizeof(error_inject_map)/sizeof(error_inject_descriptor_t)); i++)
    {
        if(!strcmp(error_inject_map[i].page_type, type))
            return &error_inject_map[i];
    }
    return NULL;
}
static uint8_t* fill_log_page_header(uint8_t *ptr, uint8_t pagecode, uint8_t subpage_code,
                                     uint16_t page_len, uint8_t ds)
{
    uint8_t spf = 0;
    if (subpage_code != 0) spf = 1;
    *ptr++ = (spf << 6) | (ds << 7) | pagecode;
    *ptr++ = subpage_code;
    *ptr++ = (uint8_t)(page_len >> 8);
    *ptr++ = (uint8_t)(page_len);
    return ptr;
}

static inline void fill_log_parameter_header(uint8_t **ptr, uint16_t parameter_code, uint8_t du, uint8_t tsd, uint8_t etc, uint8_t tmc, uint8_t fmt_lnk, uint8_t len)
{
        uint8_t* p = *ptr;

        *p++ = (uint8_t)(parameter_code >> 8);
        *p++ = (uint8_t)(parameter_code);
        *p++ = (du << 7) | (tsd << 5) | (etc << 4) | (tmc << 2) | (fmt_lnk);
        *p++ = len;

        *ptr = p;

}

static inline uint8_t* fill_log_parameter_protocol_specific_port_log_header(uint8_t* ptr, uint8_t protocol_id,
                                                              uint8_t generation_code, uint8_t number_of_phy)
{
    *ptr++ = protocol_id & 0xf;
    *ptr++ = 0; // reserved.
    *ptr++ = generation_code;
    *ptr++ = number_of_phy;
    return ptr;
}

#define PROTOCOL_STANDARD_SPL 0x06

static void fill_log_sense_page(SCSIDiskState *s, uint8_t *buffer, uint8_t page_code, uint8_t subpage_code)
{
    uint8_t *ptr = buffer + 4;
    uint32_t i;
#define FILL_UL(p,a,b,c,d)  *((uint32_t*)p) = (d << 24) | (c << 16) | (b << 8) | a; p+=4

    switch (page_code)
    {
        case 0: // page 0, a page code list.
            if (subpage_code == 0x0)
            {
                for(i=0; i<PAGE_NUMBER; i++) {
                    /*Only when page code is different, ptr would increase */
                    if((i > 0) && (page_support[i].page_code == page_support[i-1].page_code)) {
                        continue;
                    }
                    *ptr++ = page_support[i].page_code;
                }

                break;
            }
            if (subpage_code == 0xff)
            {
                for(i=0; i<PAGE_NUMBER; i++) {
                    *ptr++ = page_support[i].page_code;
                    *ptr++ = page_support[i].subpage_code;
                }
                break;
            }
            break;
        case 0x11:
            if (subpage_code == 0)
            {
                fill_log_parameter_header(&ptr,  1, 0,0,0,0,3,4);
                FILL_UL(ptr, 0,0,0,0);
                break;
            }
            if (subpage_code == 0xff)
            {
                FILL_UL(ptr, 0x11, 0, 0x11, 0xff);
                break;
            }
            break;

        case 0x18:
            if (subpage_code == 0)
            {
                SCSIDiskState *another_port = NULL;
                sas_phy_log_descriptor_t* header = NULL;
                phy_event_descriptor_t* ptr_event_descriptor = NULL;
                fill_log_parameter_header(&ptr,  1, 0, 0, 0, 0, 3, 0x68);
                ptr = fill_log_parameter_protocol_specific_port_log_header(ptr, PROTOCOL_STANDARD_SPL, 0xa, 1);
                header = (sas_phy_log_descriptor_t*) ptr;
                header->phy_id = (uint8_t)(s->port_index - 1);
                header->sas_phy_log_desc_len = 0x60;
                header->attached_device_type = 0x2;
                header->atta_reason = 0;
                header->negotiated_logic_link_rate = 0xb;
                header->reason = 0;
                header->atta_smp_i_port = 1;
                header->atta_stp_i_port = 0;
                header->atta_ssp_i_port = 0;

                header->atta_smp_t_port = 1;
                header->atta_stp_t_port = 0;
                header->atta_ssp_t_port = 0;

                header->sas_addr = bswap64(s->qdev.port_wwn);
                header->attached_sas_addr = bswap64(s->attached_wwn);
                header->attached_phy_id = s->attached_phy_id;
                header->invalid_dword_count = bswap32(12);
                header->running_disparity_error_count = bswap32(12);
                header->loss_dword_synchronization = bswap32(27);
                header->phy_reset_problem = bswap32(0);
                header->phy_event_descriptor_len = 12;
                header->number_phy_event_descriptors = 4;
                // fill with fake events.
                ptr_event_descriptor = (phy_event_descriptor_t*) (header + 1);
                ptr_event_descriptor->phy_event_source = 1;
                ptr_event_descriptor->phy_event = bswap32(0xc);
                ptr_event_descriptor->peak_value_detector_threshold = bswap32(0);
                ptr_event_descriptor ++;
                ptr_event_descriptor->phy_event_source = 2;
                ptr_event_descriptor->phy_event = bswap32(0xc);
                ptr_event_descriptor->peak_value_detector_threshold = bswap32(0);
                ptr_event_descriptor ++;

                ptr_event_descriptor->phy_event_source = 3;
                ptr_event_descriptor->phy_event = bswap32(0x1b);
                ptr_event_descriptor->peak_value_detector_threshold = bswap32(0);
                ptr_event_descriptor ++;
                ptr_event_descriptor->phy_event_source = 4;
                ptr_event_descriptor->phy_event = bswap32(0x0);
                ptr_event_descriptor->peak_value_detector_threshold = bswap32(0);
                ptr_event_descriptor ++;

                ptr = (uint8_t*) ptr_event_descriptor;

                another_port = get_drive_peer_port(s);
                if (another_port == NULL) {
                    //FIXME
                    // break if it can't find information of another port.
                    break;
                }
                fill_log_parameter_header(&ptr,  2, 0, 0, 0, 0, 3,
                        sizeof(sas_phy_log_descriptor_t) + 4 * sizeof(phy_event_descriptor_t) + 8 - 4);  //0x68
                ptr = fill_log_parameter_protocol_specific_port_log_header(ptr, PROTOCOL_STANDARD_SPL, 0xa, 1);

                header = (sas_phy_log_descriptor_t*) ptr;
                header->phy_id = (uint8_t)(another_port->port_index - 1);
                header->sas_phy_log_desc_len = sizeof(sas_phy_log_descriptor_t) + 4 * sizeof(phy_event_descriptor_t) - 4; //0x60;
                header->attached_device_type = 0x2;
                header->atta_reason = 0;
                header->negotiated_logic_link_rate = 0xb;
                header->reason = 0;
                header->atta_smp_i_port = 1;
                header->atta_stp_i_port = 0;
                header->atta_ssp_i_port = 0;

                header->atta_smp_t_port = 1;
                header->atta_stp_t_port = 0;
                header->atta_ssp_t_port = 0;


                header->sas_addr = bswap64(another_port->qdev.wwn);
                header->attached_sas_addr = bswap64(another_port->attached_wwn);
                header->attached_phy_id = another_port->attached_phy_id;
                header->invalid_dword_count = bswap32(8);
                header->running_disparity_error_count = bswap32(8);
                header->loss_dword_synchronization = bswap32(8);
                header->phy_reset_problem = bswap32(0);
                header->phy_event_descriptor_len = 12;
                header->number_phy_event_descriptors = 4;
                // fill with fake events.
                ptr_event_descriptor = (phy_event_descriptor_t*) (header + 1);
                ptr_event_descriptor->phy_event_source = 1;
                ptr_event_descriptor->phy_event = bswap32(8);
                ptr_event_descriptor->peak_value_detector_threshold = bswap32(0);
                ptr_event_descriptor ++;
                ptr_event_descriptor->phy_event_source = 2;
                ptr_event_descriptor->phy_event = bswap32(8);
                ptr_event_descriptor->peak_value_detector_threshold = bswap32(0);
                ptr_event_descriptor ++;

                ptr_event_descriptor->phy_event_source = 3;
                ptr_event_descriptor->phy_event = bswap32(8);
                ptr_event_descriptor->peak_value_detector_threshold = bswap32(0);
                ptr_event_descriptor ++;
                ptr_event_descriptor->phy_event_source = 4;
                ptr_event_descriptor->phy_event = bswap32(0x0);
                ptr_event_descriptor->peak_value_detector_threshold = bswap32(0);
                ptr_event_descriptor ++;

                ptr = (uint8_t*) ptr_event_descriptor;
                break;
            }
            if (subpage_code == 0xff)
            {
                FILL_UL(ptr, 0x18, 0, 0x18, 0xff);
                break;
            }
            break;
        case 0x2f:
            if (subpage_code == 0)
            {
                // fill ADDITIONAL SENSE CODE, ADDITIONAL SENSE CODE QUALIFIER, MOST RECENT TEMPERATURE READING, Vendor specific 0
#define FILL_EXECPTION_HEADER(p, sense_code, qualifier, temperature, vendor_specific) \
               *p++ = sense_code; *p++ = qualifier; *p++ = temperature; *p++ = vendor_specific;

                fill_log_parameter_header(&ptr,  0, 0, 0, 0, 0, 3, 8);
                FILL_EXECPTION_HEADER(ptr, 0, 0, 0x1f, 0x46);
                FILL_UL(ptr, 0x25, 0, 0, 0);

                fill_log_parameter_header(&ptr,  1, 0, 0, 0, 0, 3, 4);
                FILL_EXECPTION_HEADER(ptr, 0x5d, 0x53, 0, 0);

                fill_log_parameter_header(&ptr,  2, 0, 0, 0, 0, 3, 4);
                FILL_EXECPTION_HEADER(ptr, 0x5d, 0x54, 0, 0);

                fill_log_parameter_header(&ptr,  3, 0, 0, 0, 0, 3, 4);
                FILL_EXECPTION_HEADER(ptr, 0x5d, 0x57, 0, 0);

                fill_log_parameter_header(&ptr,  4, 0, 0, 0, 0, 3, 4);
                FILL_EXECPTION_HEADER(ptr, 0x5d, 0x28, 0, 0);

                fill_log_parameter_header(&ptr,  5, 0, 0, 0, 0, 3, 4);
                FILL_EXECPTION_HEADER(ptr, 0x0b, 0x06, 0, 0);

                fill_log_parameter_header(&ptr,  6, 0, 0, 0, 0, 3, 4);
                FILL_EXECPTION_HEADER(ptr, 0x5d, 0x56, 0x3, 0);

                fill_log_parameter_header(&ptr,  7, 0, 0, 0, 0, 3, 4);
                FILL_EXECPTION_HEADER(ptr, 0x5d, 0x55, 0, 0);

                fill_log_parameter_header(&ptr,  8, 0, 0, 0, 0, 3, 4);
                FILL_EXECPTION_HEADER(ptr, 0x5d, 0x20, 0, 0);
#undef FILL_EXECPTION_HEADER
                break;
            }

            if (subpage_code == 0xff)
            {
                FILL_UL(ptr, 0x2f, 0, 0x2f, 0xff);
                break;
            }
            break;
        default:
            break;
    }

    fill_log_page_header(buffer, page_code, subpage_code, ptr - buffer - 4, 0);
}

uint8_t *prepare_log_sense_page(SCSIDiskState *s)
{
    int i;
    uint8_t *buffer = g_malloc(PAGE_SIZE * PAGE_NUMBER);
    uint8_t *ptr = buffer;

    // fill in log sense data for each page
    for(i=0; i<PAGE_NUMBER; i++)
    {
        fill_log_sense_page(s, ptr, page_support[i].page_code, page_support[i].subpage_code);
        ptr += PAGE_SIZE;
    }
    return buffer;
}
static bool find_page_subpage_index(uint32_t *index, uint8_t page_code, uint8_t subpage_code)
{
    uint32_t i;
    uint32_t length=sizeof(page_support)/sizeof(page_contain_t);
    for(i=0; i<length; i++){
        if((page_support[i].page_code == page_code) && (page_support[i].subpage_code == subpage_code)){
            *index = i; //get index
            return true;
        }
    }
    return false;
    

}
static bool find_parameter_offset_in_subpage(uint8_t *log_page, uint16_t parameter_code, uint8_t **parameter_offset)
{
    uint32_t parameter_length;
    uint32_t length = (log_page[2]<<8) + log_page[3];
    if(!length)
        return false;
    log_page += SUBPAGE_HEAD_SIZE; //skip 4 bytes page head
    uint8_t *start_log_page = log_page;

    while(1){
        if(((log_page[0]<<8) + log_page[1]) == parameter_code){
            *parameter_offset = log_page;
            return true;
        }
        parameter_length = log_page[3];
        log_page += parameter_length + PARAMETER_HEAD_SIZE;
        if ((log_page - start_log_page) >= length)
            break;
    }

    return false; 
}

/*Change log page by qmp*/
static bool modify_log_parameters(uint8_t *log_subpage, uint32_t parameter_code, uint64_t val)
{
    uint8_t *parameter_offset;
    if (!find_parameter_offset_in_subpage(log_subpage, parameter_code, &parameter_offset))
        return false;
    uint16_t parameter_length = parameter_offset[3];
    parameter_offset +=parameter_length + PARAMETER_HEAD_SIZE -1; //point to the end of parameter.
    while(parameter_length--) {
        *parameter_offset-- = val & 0xff;
        val >>= 8;
    }
    return true;
}

static bool add_log_parameters(uint8_t *log_subpage, uint16_t parameter_code, uint8_t parameter_length, uint64_t val)
{
    uint32_t length = (log_subpage[2]<<8) + log_subpage[3];
    log_subpage[2] = ((length + parameter_length + PARAMETER_HEAD_SIZE) << 8) & 0xff;
    log_subpage[3] = (length + parameter_length + PARAMETER_HEAD_SIZE) & 0xff;

    uint8_t *p_parameter = &log_subpage[SUBPAGE_HEAD_SIZE + length]; //point to the end+1 of parameters in subpage.
    //Fill the parameter head : 4 bytes
    fill_log_parameter_header(&p_parameter, parameter_code, 0,0,0,0,3,parameter_length);
    p_parameter = p_parameter + parameter_length - 1; //point to the End of parameter

    //Fill the parameters body of length : parameter_length
    uint8_t i = parameter_length;
    while(i--) {
        *p_parameter-- = val & 0xff;
        val >>= 8;
    }
    return true;
}

static void remove_log_parameters(uint8_t *log_subpage)
{

    uint32_t length = (log_subpage[2]<<8) + log_subpage[3];
    // reset all solid state media parameter filed to 0.
    memset((log_subpage + SUBPAGE_HEAD_SIZE), 0, length); // 4 bytes head
    // reset page length to 0, which means there is no valid solid state media parameter.
    log_subpage[2] = 0;
    log_subpage[3] = 0;
}

static uint8_t *find_start_of_subpage(uint8_t *s_log_page, uint8_t page_code, uint8_t subpage_code)
{
    uint32_t index;
    if (!find_page_subpage_index(&index, page_code, subpage_code)){
        return NULL;
    }
    return &s_log_page[index*PAGE_SIZE];

}
void dispatch_error_inject_request(uint8_t *s_log_page, const char *type, ActionMode action, bool has_parameter, uint16_t parameter, bool has_parameter_length, uint8_t parameter_length, bool has_val, uint64_t val, Error **errp)
{
    error_inject_descriptor_t *current_d = NULL;
    current_d = find_map_by_type(type);
    if (!current_d) goto error_out;

    uint16_t parameter_code = has_parameter ? parameter : current_d->parameter_code;
    uint8_t parameter_length_inuse = has_parameter_length ? parameter_length : current_d->parameter_length;

    /* Percentage Used Endurance Indicator log parameter format is
     defined in sbc4r14 Table225 */

    uint8_t *log_page = find_start_of_subpage(s_log_page, current_d->page_code, current_d->subpage_code);
    if(!log_page) return;

    switch(action){
        case ACTION_MODE_ADD:
            if(!has_val) {
                 error_setg(errp, "please set the val\n");
                 return;
            }
            add_log_parameters(log_page, parameter_code, parameter_length_inuse, val);
            break;
        case ACTION_MODE_REMOVE:
            remove_log_parameters(log_page);
            break;
        case ACTION_MODE_MODIFY:
            if(!has_val) {
                 error_setg(errp, "please set the val\n");
                 return;
            }
            if(!modify_log_parameters(log_page, parameter_code, val))
                error_setg(errp, "failed to find parameter code %x\n", parameter_code);
            break;
        default:
            goto error_out;
    }
    return;

error_out:
    error_setg(errp, "failed to find error type %s\n", type);
    return;

}

int get_page_data(uint8_t *s_log_page, uint8_t *buffer, uint8_t page_code, uint8_t subpage_code, uint16_t parameter_pointer)
{
    uint8_t *log_page = find_start_of_subpage(s_log_page, page_code, subpage_code);
    if(!log_page) return -1;

    uint16_t subpage_length = (log_page[2] << 8) + log_page[3];
    if(!subpage_length) {
        // fill subpage head    
        memcpy(buffer, log_page, 4);
        return 4;
    }

    uint8_t *ptr = log_page + 4;  // skip 4 bytes head for subpage
    uint8_t parameter_length = ptr[3];
    uint16_t skip_count = 0;

    while(((ptr[0] << 8) + ptr[1]) < parameter_pointer) {
        skip_count++;
        ptr += parameter_length;
        if (ptr - log_page - 4 > subpage_length) {
            //could not find parameter
            return -1;
        }
    }

    uint16_t actual_length = subpage_length - skip_count * parameter_length;
    // fill subpage head
    memcpy(buffer, log_page, 4);
    // modify page length
    buffer[2] = (actual_length >> 8) & 0xff;
    buffer[3] = actual_length & 0xff;

    // fill subpage parameters from start of parameter_pointer    
    memcpy(buffer + 4, ptr, actual_length);

    return actual_length + 4;
}
