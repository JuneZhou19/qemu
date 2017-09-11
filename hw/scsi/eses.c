#include <stddef.h>
#include <stdint.h>
#include <string.h>

#include "fbe_hook.h"
#include "eses.h"

#include "ses.h"
#include "hw/pci/pci.h"
#include "mpt3sas.h"
#include "trace-root.h"
#include "trace/control.h"

#include "hw/boards.h"  // for current_machine

#define DPRINTF(fmt, ...) \
    do { struct timeval _now; gettimeofday(&_now, NULL); qemu_log_mask(LOG_TRACE, "[%zd.%06zd] mpt3sas: " fmt, (size_t)_now.tv_sec, (size_t)_now.tv_usec, ##__VA_ARGS__); } while (0)

/* prototypes */
fbe_status_t enclosure_status_diagnostic_page_build_device_slot_status_elements(
    fbe_u8_t *device_slot_status_elements_start_ptr, 
    fbe_u8_t **device_slot_status_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *info);

fbe_status_t encl_stat_diag_page_exp_phy_elem_get_exp_index(
    terminator_sas_virtual_phy_info_t *info,
    fbe_u8_t position,
    fbe_u8_t phy_id,
    fbe_u8_t *exp_index);

fbe_status_t config_page_get_start_elem_index_in_stat_page(terminator_sas_virtual_phy_info_t *info,
                                                            ses_subencl_type_enum subencl_type,
                                                            terminator_eses_subencl_side side,
                                                            ses_elem_type_enum elem_type,
                                                            fbe_bool_t consider_num_possible_elems,
                                                            fbe_u8_t num_possible_elems,
                                                            fbe_bool_t consider_type_desc_text,
                                                            fbe_u8_t *type_desc_text,
                                                            fbe_u8_t *index);

static fbe_status_t terminator_map_position_max_conns_to_range_conn_id(
    fbe_sas_enclosure_type_t encl_type,
    fbe_u8_t position,
    fbe_u8_t max_conns,
    terminator_conn_map_range_t *return_range,
    fbe_u8_t *conn_id);

//static terminator_sp_id_t terminator_sp_id = TERMINATOR_SP_A;
//static fbe_bool_t conf_page_inited = FALSE;

//define tabasco a and b side config page
#define TABASCO_CONFIGURATION_PAGE_CONTENTS \
{ \
{ \
0x01, 0x04, 0x04, 0x33, 0x00, 0x00, 0x00, 0x09, 0x11, 0x00, 0x0a, 0xe0, 0x50, 0x06, 0x04, 0x80, \
0xf9, 0xf6, 0x65, 0x08, 0x45, 0x4d, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x54, 0x61, 0x62, 0x61, \
0x73, 0x63, 0x6f, 0x20, 0x4c, 0x43, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x31, 0x30, 0x31, \
0x07, 0x80, 0x41, 0x02, 0x00, 0x01, 0x01, 0x43, 0x46, 0x32, 0x44, 0x42, 0x31, 0x34, 0x30, 0x39, \
0x30, 0x30, 0x30, 0x38, 0x39, 0x00, 0x00, 0x04, 0x41, 0x20, 0x31, 0x2e, 0x31, 0x2e, 0x30, 0x2b, \
0x72, 0x63, 0x31, 0x2d, 0x33, 0x2d, 0x67, 0x32, 0x33, 0x32, 0x00, 0x00, 0x41, 0x21, 0x30, 0x2e, \
0x35, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x41, 0x22, 0x30, 0x2e, 0x37, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x41, 0x23, 0x30, 0x30, 0x2e, 0x31, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x02, 0x80, 0x00, 0x01, 0x03, 0x00, 0x02, \
0x01, 0x04, 0x80, 0x02, 0x00, 0x05, 0x00, 0x04, 0x01, 0x06, 0x00, 0x05, 0x01, 0x07, 0x01, 0x41, \
0x00, 0x08, 0x03, 0x41, 0x00, 0x09, 0x85, 0x00, 0x00, 0x0a, 0x86, 0x00, 0x00, 0x0b, 0x02, 0x00, \
0x00, 0x0c, 0x02, 0x01, 0x00, 0x0d, 0x02, 0x02, 0x00, 0x0e, 0x02, 0x03, 0x00, 0x0f, 0x02, 0x04, \
0x00, 0x10, 0x02, 0x05, 0x00, 0x11, 0x02, 0x06, 0x00, 0x12, 0x02, 0x07, 0x00, 0x13, 0x04, 0x00, \
0x00, 0x04, 0xc0, 0xc1, 0xc2, 0xc3, 0x05, 0x4c, 0x43, 0x43, 0x20, 0x41, 0x11, 0x01, 0x05, 0xa1, \
0x50, 0x06, 0x04, 0x80, 0x26, 0xf7, 0x65, 0x08, 0x45, 0x4d, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, \
0x54, 0x61, 0x62, 0x61, 0x73, 0x63, 0x6f, 0x20, 0x4c, 0x43, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, \
0x30, 0x31, 0x30, 0x31, 0x07, 0x81, 0x42, 0x02, 0x01, 0x00, 0x02, 0x43, 0x46, 0x32, 0x44, 0x42, \
0x31, 0x34, 0x30, 0x39, 0x30, 0x30, 0x31, 0x33, 0x34, 0x00, 0x00, 0x04, 0x5a, 0x00, 0x31, 0x2e, \
0x31, 0x2e, 0x30, 0x2b, 0x72, 0x63, 0x31, 0x2d, 0x33, 0x2d, 0x67, 0x32, 0x33, 0x32, 0x00, 0x00, \
0x5a, 0x01, 0x30, 0x2e, 0x35, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x5a, 0x02, 0x30, 0x2e, 0x37, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 0x03, 0x30, 0x30, 0x2e, 0x31, 0x39, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x00, 0x03, \
0x01, 0x15, 0x80, 0x03, 0x00, 0x16, 0x03, 0x5a, 0x00, 0x01, 0xc4, 0x05, 0x4c, 0x43, 0x43, 0x20, \
0x42, 0x11, 0x02, 0x03, 0x4f, 0x50, 0x06, 0x04, 0x80, 0x48, 0x7d, 0x67, 0x08, 0x45, 0x4d, 0x43, \
0x20, 0x20, 0x20, 0x20, 0x20, 0x54, 0x61, 0x62, 0x61, 0x73, 0x63, 0x6f, 0x20, 0x45, 0x6e, 0x63, \
0x6c, 0x20, 0x20, 0x20, 0x20, 0x30, 0x30, 0x30, 0x31, 0x0e, 0x1f, 0x4d, 0x02, 0x02, 0x02, 0x00, \
0x43, 0x46, 0x32, 0x43, 0x59, 0x31, 0x34, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x38, 0x00, 0x00, \
0x00, 0x02, 0x00, 0x00, 0x01, 0x01, 0x01, 0x80, 0x01, 0x00, 0x01, 0xc5, 0x07, 0x43, 0x68, 0x61, \
0x73, 0x73, 0x69, 0x73, 0x11, 0x03, 0x03, 0x66, 0x50, 0x06, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, \
0x45, 0x4d, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x30, 0x30, 0x42, 0x30, 0x30, 0x33, 0x35, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x39, 0x31, 0x36, 0x02, 0x80, 0x41, 0x02, \
0x00, 0x04, 0x35, 0x41, 0x43, 0x37, 0x48, 0x37, 0x31, 0x34, 0x30, 0x34, 0x30, 0x34, 0x33, 0x33, \
0x39, 0x20, 0x20, 0x01, 0x8e, 0x24, 0x30, 0x39, 0x2e, 0x31, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x17, 0x00, 0x06, 0x01, 0x01, 0xc6, 0x0e, \
0x50, 0x6f, 0x77, 0x65, 0x72, 0x20, 0x53, 0x75, 0x70, 0x70, 0x6c, 0x79, 0x20, 0x41, 0x11, 0x04, \
0x03, 0x66, 0x50, 0x06, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0x45, 0x4d, 0x43, 0x20, 0x20, 0x20, \
0x20, 0x20, 0x30, 0x30, 0x30, 0x42, 0x30, 0x30, 0x33, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x30, 0x39, 0x31, 0x36, 0x02, 0x81, 0x42, 0x02, 0x00, 0x03, 0x36, 0x41, 0x43, 0x37, \
0x48, 0x37, 0x31, 0x34, 0x30, 0x34, 0x30, 0x34, 0x33, 0x33, 0x38, 0x20, 0x20, 0x01, 0x95, 0x24, \
0x30, 0x39, 0x2e, 0x31, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x01, 0x18, 0x00, 0x07, 0x01, 0x01, 0xc7, 0x0e, 0x50, 0x6f, 0x77, 0x65, 0x72, 0x20, \
0x53, 0x75, 0x70, 0x70, 0x6c, 0x79, 0x20, 0x42, 0x17, 0x19, 0x00, 0x0c, 0x04, 0x02, 0x00, 0x0e, \
0x0e, 0x01, 0x00, 0x05, 0x81, 0x25, 0x00, 0x0c, 0x18, 0x01, 0x00, 0x0a, 0x07, 0x01, 0x00, 0x0c, \
0x19, 0x0a, 0x00, 0x0f, 0x0c, 0x02, 0x00, 0x0d, 0x0c, 0x01, 0x00, 0x0c, 0x10, 0x01, 0x00, 0x08, \
0x0e, 0x01, 0x01, 0x05, 0x81, 0x08, 0x01, 0x0c, 0x18, 0x01, 0x01, 0x0a, 0x07, 0x01, 0x01, 0x0c, \
0x19, 0x0a, 0x01, 0x0f, 0x0e, 0x01, 0x02, 0x09, 0x04, 0x07, 0x02, 0x0e, 0x19, 0x1a, 0x02, 0x0f, \
0x03, 0x04, 0x03, 0x0d, 0x04, 0x02, 0x03, 0x0e, 0x02, 0x01, 0x03, 0x0e, 0x03, 0x04, 0x04, 0x0d, \
0x04, 0x02, 0x04, 0x0e, 0x02, 0x01, 0x04, 0x0e, 0x41, 0x72, 0x72, 0x61, 0x79, 0x20, 0x44, 0x65, \
0x76, 0x69, 0x63, 0x65, 0x54, 0x65, 0x6d, 0x70, 0x2e, 0x20, 0x53, 0x65, 0x6e, 0x73, 0x6f, 0x72, \
0x20, 0x41, 0x4c, 0x43, 0x43, 0x20, 0x41, 0x45, 0x78, 0x70, 0x61, 0x6e, 0x64, 0x65, 0x72, 0x20, \
0x50, 0x68, 0x79, 0x45, 0x78, 0x70, 0x61, 0x6e, 0x64, 0x65, 0x72, 0x20, 0x41, 0x43, 0x6f, 0x6e, \
0x74, 0x72, 0x6f, 0x6c, 0x6c, 0x65, 0x72, 0x20, 0x41, 0x53, 0x41, 0x53, 0x20, 0x43, 0x6f, 0x6e, \
0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x20, 0x41, 0x44, 0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x20, \
0x47, 0x72, 0x65, 0x65, 0x6e, 0x44, 0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x20, 0x42, 0x6c, 0x75, \
0x65, 0x4c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x4c, 0x43, 0x43, 0x20, 0x42, 0x45, 0x78, \
0x70, 0x61, 0x6e, 0x64, 0x65, 0x72, 0x20, 0x50, 0x68, 0x79, 0x45, 0x78, 0x70, 0x61, 0x6e, 0x64, \
0x65, 0x72, 0x20, 0x42, 0x43, 0x6f, 0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x6c, 0x65, 0x72, 0x20, 0x42, \
0x53, 0x41, 0x53, 0x20, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x20, 0x42, 0x45, \
0x6e, 0x63, 0x6c, 0x6f, 0x73, 0x75, 0x72, 0x65, 0x54, 0x65, 0x6d, 0x70, 0x2e, 0x20, 0x53, 0x65, \
0x6e, 0x73, 0x6f, 0x72, 0x20, 0x4d, 0x53, 0x41, 0x53, 0x20, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, \
0x74, 0x6f, 0x72, 0x20, 0x4d, 0x43, 0x6f, 0x6f, 0x6c, 0x69, 0x6e, 0x67, 0x20, 0x46, 0x61, 0x6e, \
0x20, 0x41, 0x54, 0x65, 0x6d, 0x70, 0x2e, 0x20, 0x53, 0x65, 0x6e, 0x73, 0x6f, 0x72, 0x20, 0x41, \
0x50, 0x6f, 0x77, 0x65, 0x72, 0x20, 0x53, 0x75, 0x70, 0x70, 0x6c, 0x79, 0x20, 0x41, 0x43, 0x6f, \
0x6f, 0x6c, 0x69, 0x6e, 0x67, 0x20, 0x46, 0x61, 0x6e, 0x20, 0x42, 0x54, 0x65, 0x6d, 0x70, 0x2e, \
0x20, 0x53, 0x65, 0x6e, 0x73, 0x6f, 0x72, 0x20, 0x42, 0x50, 0x6f, 0x77, 0x65, 0x72, 0x20, 0x53, \
0x75, 0x70, 0x70, 0x6c, 0x79, 0x20, 0x42, \
}, \
{ \
0x01, 0x04, 0x04, 0x33, 0x00, 0x00, 0x00, 0x0d, 0x11, 0x00, 0x0a, 0xe0, 0x50, 0x06, 0x04, 0x80, \
0x26, 0xf7, 0x65, 0x08, 0x45, 0x4d, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x54, 0x61, 0x62, 0x61, \
0x73, 0x63, 0x6f, 0x20, 0x4c, 0x43, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x31, 0x30, 0x31, \
0x07, 0x81, 0x42, 0x02, 0x00, 0x01, 0x02, 0x43, 0x46, 0x32, 0x44, 0x42, 0x31, 0x34, 0x30, 0x39, \
0x30, 0x30, 0x31, 0x33, 0x34, 0x00, 0x00, 0x04, 0x41, 0x20, 0x31, 0x2e, 0x31, 0x2e, 0x30, 0x2b, \
0x72, 0x63, 0x31, 0x2d, 0x33, 0x2d, 0x67, 0x32, 0x33, 0x32, 0x00, 0x00, 0x41, 0x21, 0x30, 0x2e, \
0x35, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x41, 0x22, 0x30, 0x2e, 0x37, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x41, 0x23, 0x30, 0x30, 0x2e, 0x31, 0x39, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x12, 0x02, 0x80, 0x00, 0x01, 0x03, 0x00, 0x02, \
0x01, 0x04, 0x80, 0x02, 0x00, 0x05, 0x00, 0x04, 0x01, 0x06, 0x00, 0x05, 0x01, 0x07, 0x01, 0x41, \
0x00, 0x08, 0x03, 0x41, 0x00, 0x09, 0x85, 0x00, 0x00, 0x0a, 0x86, 0x00, 0x00, 0x0b, 0x02, 0x00, \
0x00, 0x0c, 0x02, 0x01, 0x00, 0x0d, 0x02, 0x02, 0x00, 0x0e, 0x02, 0x03, 0x00, 0x0f, 0x02, 0x04, \
0x00, 0x10, 0x02, 0x05, 0x00, 0x11, 0x02, 0x06, 0x00, 0x12, 0x02, 0x07, 0x00, 0x13, 0x04, 0x00, \
0x00, 0x04, 0xc0, 0xc1, 0xc2, 0xc3, 0x05, 0x4c, 0x43, 0x43, 0x20, 0x42, 0x11, 0x01, 0x05, 0xa1, \
0x50, 0x06, 0x04, 0x80, 0xf9, 0xf6, 0x65, 0x08, 0x45, 0x4d, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, \
0x54, 0x61, 0x62, 0x61, 0x73, 0x63, 0x6f, 0x20, 0x4c, 0x43, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, \
0x30, 0x31, 0x30, 0x31, 0x07, 0x80, 0x41, 0x02, 0x01, 0x00, 0x01, 0x43, 0x46, 0x32, 0x44, 0x42, \
0x31, 0x34, 0x30, 0x39, 0x30, 0x30, 0x30, 0x38, 0x39, 0x00, 0x00, 0x04, 0x5a, 0x00, 0x31, 0x2e, \
0x31, 0x2e, 0x30, 0x2b, 0x72, 0x63, 0x31, 0x2d, 0x33, 0x2d, 0x67, 0x32, 0x33, 0x32, 0x00, 0x00, \
0x5a, 0x01, 0x30, 0x2e, 0x35, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x5a, 0x02, 0x30, 0x2e, 0x37, 0x2e, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x5a, 0x03, 0x30, 0x30, 0x2e, 0x31, 0x39, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x03, 0x14, 0x00, 0x03, \
0x01, 0x15, 0x80, 0x03, 0x00, 0x16, 0x03, 0x5a, 0x00, 0x01, 0xc4, 0x05, 0x4c, 0x43, 0x43, 0x20, \
0x41, 0x11, 0x02, 0x03, 0x4f, 0x50, 0x06, 0x04, 0x80, 0x48, 0x7d, 0x67, 0x08, 0x45, 0x4d, 0x43, \
0x20, 0x20, 0x20, 0x20, 0x20, 0x54, 0x61, 0x62, 0x61, 0x73, 0x63, 0x6f, 0x20, 0x45, 0x6e, 0x63, \
0x6c, 0x20, 0x20, 0x20, 0x20, 0x30, 0x30, 0x30, 0x31, 0x0e, 0x1f, 0x4d, 0x02, 0x02, 0x02, 0x00, \
0x43, 0x46, 0x32, 0x43, 0x59, 0x31, 0x34, 0x31, 0x30, 0x30, 0x30, 0x30, 0x30, 0x38, 0x00, 0x00, \
0x00, 0x02, 0x00, 0x00, 0x01, 0x01, 0x01, 0x80, 0x01, 0x00, 0x01, 0xc5, 0x07, 0x43, 0x68, 0x61, \
0x73, 0x73, 0x69, 0x73, 0x11, 0x03, 0x03, 0x66, 0x50, 0x06, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, \
0x45, 0x4d, 0x43, 0x20, 0x20, 0x20, 0x20, 0x20, 0x30, 0x30, 0x30, 0x42, 0x30, 0x30, 0x33, 0x35, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x39, 0x31, 0x36, 0x02, 0x81, 0x42, 0x02, \
0x00, 0x04, 0x35, 0x41, 0x43, 0x37, 0x48, 0x37, 0x31, 0x34, 0x30, 0x34, 0x30, 0x34, 0x33, 0x33, \
0x38, 0x20, 0x20, 0x01, 0x8e, 0x24, 0x30, 0x39, 0x2e, 0x31, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01, 0x17, 0x00, 0x06, 0x01, 0x01, 0xc6, 0x0e, \
0x50, 0x6f, 0x77, 0x65, 0x72, 0x20, 0x53, 0x75, 0x70, 0x70, 0x6c, 0x79, 0x20, 0x42, 0x11, 0x04, \
0x03, 0x66, 0x50, 0x06, 0x04, 0x80, 0x00, 0x00, 0x00, 0x00, 0x45, 0x4d, 0x43, 0x20, 0x20, 0x20, \
0x20, 0x20, 0x30, 0x30, 0x30, 0x42, 0x30, 0x30, 0x33, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x30, 0x39, 0x31, 0x36, 0x02, 0x80, 0x41, 0x02, 0x00, 0x03, 0x36, 0x41, 0x43, 0x37, \
0x48, 0x37, 0x31, 0x34, 0x30, 0x34, 0x30, 0x34, 0x33, 0x33, 0x39, 0x20, 0x20, 0x01, 0x95, 0x24, \
0x30, 0x39, 0x2e, 0x31, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, \
0x00, 0x00, 0x01, 0x18, 0x00, 0x07, 0x01, 0x01, 0xc7, 0x0e, 0x50, 0x6f, 0x77, 0x65, 0x72, 0x20, \
0x53, 0x75, 0x70, 0x70, 0x6c, 0x79, 0x20, 0x41, 0x17, 0x19, 0x00, 0x0c, 0x04, 0x02, 0x00, 0x0e, \
0x0e, 0x01, 0x00, 0x05, 0x81, 0x25, 0x00, 0x0c, 0x18, 0x01, 0x00, 0x0a, 0x07, 0x01, 0x00, 0x0c, \
0x19, 0x0a, 0x00, 0x0f, 0x0c, 0x02, 0x00, 0x0d, 0x0c, 0x01, 0x00, 0x0c, 0x10, 0x01, 0x00, 0x08, \
0x0e, 0x01, 0x01, 0x05, 0x81, 0x08, 0x01, 0x0c, 0x18, 0x01, 0x01, 0x0a, 0x07, 0x01, 0x01, 0x0c, \
0x19, 0x0a, 0x01, 0x0f, 0x0e, 0x01, 0x02, 0x09, 0x04, 0x07, 0x02, 0x0e, 0x19, 0x1a, 0x02, 0x0f, \
0x03, 0x04, 0x03, 0x0d, 0x04, 0x02, 0x03, 0x0e, 0x02, 0x01, 0x03, 0x0e, 0x03, 0x04, 0x04, 0x0d, \
0x04, 0x02, 0x04, 0x0e, 0x02, 0x01, 0x04, 0x0e, 0x41, 0x72, 0x72, 0x61, 0x79, 0x20, 0x44, 0x65, \
0x76, 0x69, 0x63, 0x65, 0x54, 0x65, 0x6d, 0x70, 0x2e, 0x20, 0x53, 0x65, 0x6e, 0x73, 0x6f, 0x72, \
0x20, 0x42, 0x4c, 0x43, 0x43, 0x20, 0x42, 0x45, 0x78, 0x70, 0x61, 0x6e, 0x64, 0x65, 0x72, 0x20, \
0x50, 0x68, 0x79, 0x45, 0x78, 0x70, 0x61, 0x6e, 0x64, 0x65, 0x72, 0x20, 0x42, 0x43, 0x6f, 0x6e, \
0x74, 0x72, 0x6f, 0x6c, 0x6c, 0x65, 0x72, 0x20, 0x42, 0x53, 0x41, 0x53, 0x20, 0x43, 0x6f, 0x6e, \
0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x20, 0x42, 0x44, 0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x20, \
0x47, 0x72, 0x65, 0x65, 0x6e, 0x44, 0x69, 0x73, 0x70, 0x6c, 0x61, 0x79, 0x20, 0x42, 0x6c, 0x75, \
0x65, 0x4c, 0x61, 0x6e, 0x67, 0x75, 0x61, 0x67, 0x65, 0x4c, 0x43, 0x43, 0x20, 0x41, 0x45, 0x78, \
0x70, 0x61, 0x6e, 0x64, 0x65, 0x72, 0x20, 0x50, 0x68, 0x79, 0x45, 0x78, 0x70, 0x61, 0x6e, 0x64, \
0x65, 0x72, 0x20, 0x41, 0x43, 0x6f, 0x6e, 0x74, 0x72, 0x6f, 0x6c, 0x6c, 0x65, 0x72, 0x20, 0x41, \
0x53, 0x41, 0x53, 0x20, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, 0x74, 0x6f, 0x72, 0x20, 0x41, 0x45, \
0x6e, 0x63, 0x6c, 0x6f, 0x73, 0x75, 0x72, 0x65, 0x54, 0x65, 0x6d, 0x70, 0x2e, 0x20, 0x53, 0x65, \
0x6e, 0x73, 0x6f, 0x72, 0x20, 0x4d, 0x53, 0x41, 0x53, 0x20, 0x43, 0x6f, 0x6e, 0x6e, 0x65, 0x63, \
0x74, 0x6f, 0x72, 0x20, 0x4d, 0x43, 0x6f, 0x6f, 0x6c, 0x69, 0x6e, 0x67, 0x20, 0x46, 0x61, 0x6e, \
0x20, 0x42, 0x54, 0x65, 0x6d, 0x70, 0x2e, 0x20, 0x53, 0x65, 0x6e, 0x73, 0x6f, 0x72, 0x20, 0x42, \
0x50, 0x6f, 0x77, 0x65, 0x72, 0x20, 0x53, 0x75, 0x70, 0x70, 0x6c, 0x79, 0x20, 0x42, 0x43, 0x6f, \
0x6f, 0x6c, 0x69, 0x6e, 0x67, 0x20, 0x46, 0x61, 0x6e, 0x20, 0x41, 0x54, 0x65, 0x6d, 0x70, 0x2e, \
0x20, 0x53, 0x65, 0x6e, 0x73, 0x6f, 0x72, 0x20, 0x41, 0x50, 0x6f, 0x77, 0x65, 0x72, 0x20, 0x53, \
0x75, 0x70, 0x70, 0x6c, 0x79, 0x20, 0x41, \
} \
};

// Tabasco configuration info
fbe_u8_t tabasco_config_page_with_ps[][1200] = TABASCO_CONFIGURATION_PAGE_CONTENTS;
terminator_eses_config_page_info_t tabasco_config_page_info_with_ps[2];

/****************************************************************************/
/** Return a pointer to the number of \ref ses_buf_desc_struct "buffer descriptors" field
 *  in the specified \ref ses_subencl_desc_struct "subenclosure descriptor" in the
 *  \ref ses_pg_config_struct "Configuration diagnostic page".
 *
 * Use \ref fbe_eses_get_ses_buf_desc_p() "fbe_eses_get_ses_buf_desc_p(\p d)" to get a pointer to the array of descriptors.
 *
 * The subenclosure descriptor at \p d need not be within a SES page.  The
 * <tt>d-\></tt>\ref ses_subencl_desc_struct.num_ver_descs "num_ver_descs" field
 * in the descriptor must be filled in.
 ****************************************************************************/

static __inline fbe_u8_t *fbe_eses_get_num_buf_descs_p(ses_subencl_desc_struct *d) {
    return (fbe_u8_t *)(((fbe_u8_t *) d) + offsetof(ses_subencl_desc_struct, ver_desc) +
                d->num_ver_descs * sizeof(ses_ver_desc_struct));
}

/****************************************************************************/
/** Return a pointer to the \ref ses_buf_desc_struct "buffer descriptor" array
 *  in the specified \ref ses_subencl_desc_struct "subenclosure descriptor" in the
 *  \ref ses_pg_config_struct "Configuration diagnostic page".
 *
 * Use \ref fbe_eses_get_ses_num_buf_descs_p() "fbe_eses_get_ses_num_buf_descs_p(\p d)"
 * to get the number of descriptors in the array.
 *
 * The subenclosure descriptor at \p d need not be within a SES page.  The
 * <tt>d-\></tt>\ref ses_subencl_desc_struct.num_ver_descs "num_ver_descs" field
 * in the descriptor must be filled in.
 ****************************************************************************/

static __inline ses_buf_desc_struct *fbe_eses_get_first_buf_desc_p(ses_subencl_desc_struct *d) {
    return (ses_buf_desc_struct *) (fbe_eses_get_num_buf_descs_p(d) + 1);
}

/****************************************************************************/
/** Return a pointer to the number of VPD pages field in the VPD page list
 *  in the specified \ref ses_subencl_desc_struct "subenclosure descriptor" in the
 *  \ref ses_pg_config_struct "Configuration diagnostic page".
 *
 * Use fbe_eses_get_ses_vpd_pg_list_p(\p d) to get the list.
 *
 * The subenclosure descriptor at \p d need not be within a SES page.  The
 * <tt>d-\></tt>\ref ses_subencl_desc_struct.num_ver_descs "num_ver_descs" field and
 * the number of buffer descriptors field returned by \ref fbe_eses_get_ses_num_buf_descs_p()
 * "fbe_eses_get_ses_num_buf_descs_p(\p d)" in the descriptor must be filled in.
 ****************************************************************************/

static __inline fbe_u8_t *fbe_eses_get_ses_num_vpd_pgs_p(ses_subencl_desc_struct *d) {
    return (fbe_u8_t *)(((char *) fbe_eses_get_first_buf_desc_p(d)) +
            sizeof(ses_buf_desc_struct) * *fbe_eses_get_num_buf_descs_p(d));
}

/****************************************************************************/
/** Return a pointer to the VPD page list 
 *  in the specified \ref ses_subencl_desc_struct "subenclosure descriptor" in the
 *  \ref ses_pg_config_struct "Configuration diagnostic page".
 *
 * The VPD page list has no structure defined for it -- it's an array of bytes
 * containing VPD page codes.  Use
 * \ref fbe_eses_get_ses_num_vpd_pgs_p() "fbe_eses_get_ses_num_vpd_pgs_p(\p d)" to get its length.
 *
 * The subenclosure descriptor at \p d need not be within a SES page.  The
 * <tt>d-></tt>\ref ses_subencl_desc_struct.num_ver_descs "num_ver_descs" field and
 * the number of buffer descriptors field returned by \ref fbe_eses_get_ses_num_buf_descs_p()
 * "fbe_eses_get_ses_num_buf_descs_p(\p d)" in the descriptor must be filled in.
 ****************************************************************************/

static __inline fbe_u8_t *fbe_eses_get_ses_vpd_pg_list_p(ses_subencl_desc_struct *d) {
    return (fbe_u8_t *)(fbe_eses_get_ses_num_vpd_pgs_p(d) + 1);
}

/****************************************************************************/
/** Return a pointer to the subenclosure text length field
 *  in the specified \ref ses_subencl_desc_struct "subenclosure descriptor" in the
 *  \ref ses_pg_config_struct "Configuration diagnostic page".
 *
 *  Use \ref fbe_eses_get_ses_subencl_text_p() "fbe_eses_get_ses_subencl_test_p(\p d)" to get the text.
 *
 * The subenclosure descriptor at \p d need not be within a SES page.  The
 * <tt>d-></tt>\ref ses_subencl_desc_struct.num_ver_descs "num_ver_descs" field,
 * the number of buffer descriptors field returned by \ref fbe_eses_get_ses_num_buf_descs_p()
 * "fbe_eses_get_ses_num_buf_descs_p(\p d)", and the number of VPD pages returned by
 * \ref fbe_eses_get_ses_num_vpd_pgs_p() "fbe_eses_get_ses_num_vpd_pgs_p(\p d)" in the
 * descriptor must be filled in.
 ****************************************************************************/

static __inline fbe_u8_t *fbe_eses_get_ses_subencl_text_len_p(ses_subencl_desc_struct *d) {
    return fbe_eses_get_ses_vpd_pg_list_p(d) + *fbe_eses_get_ses_num_vpd_pgs_p(d);
}

/****************************************************************************/
/** Return a pointer to the subenclosure text field
 *  in the specified \ref ses_subencl_desc_struct "subenclosure descriptor" in the
 *  \ref ses_pg_config_struct "Configuration diagnostic page".
 *
 * The subenclosure text field is a string of bytes.  Use
 * \ref fbe_eses_get_ses_subencl_text_len_p() "fbe_eses_get_ses_subencl_text_len_p(\p d)" to get its length.
 *
 * The subenclosure descriptor at \p d need not be within a SES page.  The
 * <tt>d-></tt>\ref ses_subencl_desc_struct.num_ver_descs "num_ver_descs" field,
 * the number of buffer descriptors field returned by \ref fbe_eses_get_ses_num_buf_descs_p()
 * "fbe_eses_get_ses_num_buf_descs_p(\p d)", and the number of VPD pages returned by
 * \ref fbe_eses_get_ses_num_vpd_pgs_p() "fbe_eses_get_ses_num_vpd_pgs_p(\p d)" in the
 * descriptor must be filled in.
 ****************************************************************************/

static __inline CHAR *fbe_eses_get_ses_subencl_text_p(ses_subencl_desc_struct *d) {
    return (CHAR *)(fbe_eses_get_ses_subencl_text_len_p(d) + 1);
}

/****************************************************************************/
/** Return a pointer to the next
 *  \ref ses_subencl_desc_struct "subenclosure descriptor" in the
 *  \ref ses_pg_config_struct "Configuration diagnostic page" following the
 *  specified one at \p d.  If this is the last subenclosure descriptor in the page,
 *  the returned value points to the type descriptor header list, an array of
 *  \ref ses_type_desc_hdr_struct.  It is the caller's responsibility to
 *  count the number of subenclosure descriptors processed to determine whether the
 *  return value should be cast to a \ref ses_type_desc_hdr_struct.
 *
 * This function requires that the subenclosure descriptor at \p d be located
 * in a subenclosure descriptor list in a SES page, and that the
 * <tt>d-></tt>\ref ses_subencl_desc_struct.subencl_desc_len "subencl_desc_len" field be filled in.
 ****************************************************************************/

static __inline ses_subencl_desc_struct *fbe_eses_get_next_ses_subencl_desc_p(ses_subencl_desc_struct *d) 
{
    return (ses_subencl_desc_struct *)(((fbe_u8_t *) d) + d->subencl_desc_len + 4);
}

fbe_status_t fbe_terminator_api_get_sp_id(terminator_sas_virtual_phy_info_t *virtual_phy_handle, terminator_sp_id_t *sp_id)
{
    *sp_id = (terminator_sp_id_t)virtual_phy_handle->side;
    return FBE_STATUS_OK;
}

/*********************************************************************
*  config_page_get_subencl_slot ()
**********************************************************************
*
*  Description: This function gets the subenclosure slot number.
*
*  Params: subencl_desc_ptr
*          subencl_slot_ptr
* 
*  Return Value: success or failure
*
*  Notes:
*
*  History:
*    Jan-07-2014  PHE - created
*
*********************************************************************/
static fbe_status_t config_page_get_subencl_slot(ses_subencl_desc_struct * subencl_desc_ptr, 
                                          fbe_u8_t * subencl_slot_ptr)
{
    fbe_char_t              *subencl_text_p = NULL; 
    fbe_u8_t                *subencl_text_len_p = NULL;
    fbe_u8_t                 subencl_text_len = 0;

    subencl_text_p = fbe_eses_get_ses_subencl_text_p(subencl_desc_ptr);

    subencl_text_len_p = fbe_eses_get_ses_subencl_text_len_p(subencl_desc_ptr);
    subencl_text_len = *subencl_text_len_p;

    if(subencl_desc_ptr->subencl_type == SES_SUBENCL_TYPE_PS) 
    {
        if(strncmp((char *)subencl_text_p, 
                       FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_A, subencl_text_len) == 0)
        {
            *subencl_slot_ptr = 0;
        }
        else if(strncmp((char *)subencl_text_p, 
                               FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_B, subencl_text_len) == 0)
        {
            *subencl_slot_ptr = 1;
        }
        else if(strncmp((char *)subencl_text_p, 
                               FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_A1, subencl_text_len) == 0)
        {
            *subencl_slot_ptr = 0;
        }
        else if(strncmp((char *)subencl_text_p, 
                               FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_A0, subencl_text_len) == 0)
        {
            *subencl_slot_ptr = 1;
        }
        else if(strncmp((char *)subencl_text_p, 
                               FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_B1, subencl_text_len) == 0)
        {
            *subencl_slot_ptr = 2;
        }
        else if(strncmp((char *)subencl_text_p, 
                               FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_B0, subencl_text_len) == 0)
        {
            *subencl_slot_ptr = 3;
        }
        else
        {   
            *subencl_slot_ptr = 0;
        }
    }
    else
    {
        *subencl_slot_ptr = subencl_desc_ptr->side;
    }

    return FBE_STATUS_OK;
}


/*********************************************************************
*  config_page_parse_given_config_page ()
**********************************************************************
*
*  Description: This builds the structures containing SUBENCLOSURE
*       DESC information,  TYPE DESC HEADER information, VERSION
*       DESC information , BUFFER DESC information & others by parsing
*       the  given configuration page.
*
*  Inputs:
*   ses_subencl_desc_struct subencl_desc[] - subenclosure descriptor
*       array to fill
*   ses_type_desc_hdr_struct type_desc_hdr[] - type desc header array
*       to fill
*   offset_of_first_elem[] - Array where each element gives the offset
*       of first element of the given element type (each index in this
*       array corresponds to the index in type_desc_hdr[] array)
*   ses_ver_desc_struct ver_desc_array[]- The version descriptors of
*       of each of the subenclosures are stored consecutively in this
*       array. To know which version descriptors belong to which
*       subenclosure, we need the above "subencl_desc[]" array.
*   elem_index_of_first_elem[] - Array where each element gives index
*       of first element of the given element type (each index in this
*       array corresponds to the index in type_desc_hdr[] array)
*   terminator_eses_subencl_buf_desc_info_t subencl_buf_desc_info[] - Array
*       that contains the buffer descriptor info of each of the
*       subenclosures.
*   number of subenclosures, type descriptor headers, version descriptors
*   and buffer descriptors that are in the given configuration page.
*   encl_stat_diag_page_size - The size of the Enclosure status diagnostic
*       page( this is to be caluclated from the config data)
*
*  Return Value: success or failure
*
*  Notes:
*
*  History:
*    Sep08  Rajesh V created
*
*********************************************************************/
static fbe_status_t config_page_parse_given_config_page(fbe_u8_t *config_page,
                                                ses_subencl_desc_struct subencl_desc[],
                                                fbe_u8_t * subencl_slot,
                                                ses_type_desc_hdr_struct type_desc_hdr[],
                                                fbe_u8_t  elem_index_of_first_elem[],
                                                fbe_u16_t offset_of_first_elem[],
                                                ses_ver_desc_struct ver_desc_array[],
                                                terminator_eses_subencl_buf_desc_info_t subencl_buf_desc_info[],
                                                fbe_u8_t *num_subencls,
                                                fbe_u8_t *num_type_desc_headers,
                                                fbe_u8_t *num_ver_descs,
                                                fbe_u8_t *num_buf_descs,
                                                fbe_u16_t *encl_stat_diag_page_size)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    fbe_u8_t group_id = 0;
    fbe_u8_t elem_index = 0;
    fbe_u8_t element = 0;   // Loop through all the elements within the group.
    fbe_u8_t subencl = 0; // Loop through all the subenclosures.
    fbe_u8_t ver_desc = 0; // Loop through the version descriptors in a given SubEnclosure.
    fbe_u8_t buf_desc = 0; // Loop through the buffer descriptors in a given subenclosure.
    fbe_u16_t offset_in_ctrl_stat_pg = 0;
    fbe_u8_t total_num_subencls = 0;
    fbe_u8_t total_num_groups = 0;
    fbe_u8_t *ptr;
    ses_buf_desc_struct *buf_desc_ptr;


    ses_pg_config_struct * config_pg_ptr = NULL;
    ses_subencl_desc_struct * subencl_desc_ptr = NULL;
    ses_type_desc_hdr_struct * elem_type_desc_hdr_ptr = NULL;

    //terminator_sp_id_t spid;
    
    //status =  fbe_terminator_api_get_sp_id(&spid);

    *num_subencls = 0;
    *num_type_desc_headers = 0;
    *num_ver_descs = 0;
    *encl_stat_diag_page_size = 0;
    *num_buf_descs = 0;


    config_pg_ptr = (ses_pg_config_struct *)config_page;
    offset_in_ctrl_stat_pg = FBE_ESES_PAGE_HEADER_SIZE; // byte offset in the control/status page.

    // Get the pointer to the first subencl descriptor.
    subencl_desc_ptr = (ses_subencl_desc_struct *)((fbe_u8_t *)config_page + FBE_ESES_PAGE_HEADER_SIZE);
    // Get the total number of subenclosures.
    // (Adding 1 for the primary subenclosure).
    total_num_subencls = config_pg_ptr->num_secondary_subencls + 1;
    *num_subencls = total_num_subencls;

    // Process the subenclosure descriptor list.
    for(subencl = 0; subencl < total_num_subencls; subencl ++ )
    {
        // The total number of groups is equal to the total number of type descriptor headers.
        total_num_groups += subencl_desc_ptr->num_elem_type_desc_hdrs;

        //For now I think it is safe to assume they are arranged as per the sub encl ids.
        memcpy(&subencl_desc[subencl], subencl_desc_ptr, sizeof(ses_subencl_desc_struct));

        //FILL in the version descriptor information
        for(ver_desc = 0; ver_desc < subencl_desc[subencl].num_ver_descs; ver_desc++, (*num_ver_descs)++)
        {
            memcpy(&ver_desc_array[*num_ver_descs],
                   &subencl_desc_ptr->ver_desc[ver_desc],
                   sizeof(ses_ver_desc_struct));
        }

        // FILL the buffer descriptor information

        ptr = (fbe_u8_t *)&subencl_desc_ptr->ver_desc[ver_desc];
        subencl_buf_desc_info[subencl].num_buf_descs = *ptr;
        // Get pointer to the first buffer desc, if it exists
        buf_desc_ptr = (ses_buf_desc_struct *)(ptr + 1);
        for(buf_desc = 0; buf_desc < subencl_buf_desc_info[subencl].num_buf_descs; buf_desc++, (*num_buf_descs)++)
        {
            memcpy(&subencl_buf_desc_info[subencl].buf_desc[buf_desc],
                   &buf_desc_ptr[buf_desc],
                   sizeof(ses_buf_desc_struct));
        }
        
        config_page_get_subencl_slot(subencl_desc_ptr, &subencl_slot[subencl]);

        subencl_desc_ptr = fbe_eses_get_next_ses_subencl_desc_p(subencl_desc_ptr);
    } // End of Processing the subenclosure descriptor list.

    *num_type_desc_headers = total_num_groups;

    // Process the type descriptor header list.
    elem_type_desc_hdr_ptr = (ses_type_desc_hdr_struct *)subencl_desc_ptr;

    for(group_id = 0; group_id < total_num_groups; group_id ++)
    {
        type_desc_hdr[group_id].elem_type = elem_type_desc_hdr_ptr->elem_type;
        type_desc_hdr[group_id].num_possible_elems = elem_type_desc_hdr_ptr->num_possible_elems;
        type_desc_hdr[group_id].subencl_id = elem_type_desc_hdr_ptr->subencl_id;
        offset_of_first_elem[group_id] = offset_in_ctrl_stat_pg;
        elem_index_of_first_elem[group_id] = elem_index;
        DPRINTF("%s:%d subencl_type %d subencl_id %d elem_type 0x%x elem_index %d\n", __func__, __LINE__, subencl_desc[elem_type_desc_hdr_ptr->subencl_id].subencl_type, elem_type_desc_hdr_ptr->subencl_id, elem_type_desc_hdr_ptr->elem_type, elem_index);

        // Get the byte offset in the control/status page for the first element in the group.
        // This is to consider the overall status element for each element group.
        offset_in_ctrl_stat_pg += FBE_ESES_CTRL_STAT_ELEM_SIZE;

        for (element = 0; element < elem_type_desc_hdr_ptr->num_possible_elems; element++)
        {
            offset_in_ctrl_stat_pg += FBE_ESES_CTRL_STAT_ELEM_SIZE;
            elem_index ++;
        }

        // Get the pointer to the next type descriptor header.
        elem_type_desc_hdr_ptr = (ses_type_desc_hdr_struct *)((fbe_u8_t *)elem_type_desc_hdr_ptr + FBE_ESES_ELEM_TYPE_DESC_HEADER_SIZE);
    }
    *encl_stat_diag_page_size = offset_in_ctrl_stat_pg;

    status = FBE_STATUS_OK;
    return(status);
}

fbe_status_t config_page_init(fbe_u8_t side)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;

    //if (conf_page_inited)
    //    return FBE_STATUS_OK;
    
    // Parse and build tabasco configuration page related structures.
    status = config_page_parse_given_config_page(tabasco_config_page_with_ps[side],
                                                tabasco_config_page_info_with_ps[side].subencl_desc_array,
                                                &tabasco_config_page_info_with_ps[side].subencl_slot[0],
                                                tabasco_config_page_info_with_ps[side].type_desc_array,
                                                tabasco_config_page_info_with_ps[side].elem_index_of_first_elem_array,
                                                tabasco_config_page_info_with_ps[side].offset_of_first_elem_array,
                                                tabasco_config_page_info_with_ps[side].ver_desc_array,
                                                tabasco_config_page_info_with_ps[side].subencl_buf_desc_info,
                                                &tabasco_config_page_info_with_ps[side].num_subencls,
                                                &tabasco_config_page_info_with_ps[side].num_type_desc_headers,
                                                &tabasco_config_page_info_with_ps[side].num_ver_descs,
                                                &tabasco_config_page_info_with_ps[side].num_buf_descs,
                                                &tabasco_config_page_info_with_ps[side].encl_stat_diag_page_size);

    //conf_page_inited = TRUE;

    return status;
}

static fbe_status_t config_page_info_get_subencl_ver_desc_position(terminator_eses_config_page_info_t *config_page_info,
                                                            ses_subencl_type_enum subencl_type,
                                                            terminator_eses_subencl_side side,
                                                            uint8_t *start_index,
                                                            uint8_t *num_ver_descs)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t subencl;

    *start_index = 0;
    *num_ver_descs = 0;
    for(subencl = 0; subencl < config_page_info->num_subencls; subencl++)
    {
        if((config_page_info->subencl_desc_array[subencl].subencl_type == subencl_type) &&
           (config_page_info->subencl_slot[subencl] == side))
        {
            *num_ver_descs = config_page_info->subencl_desc_array[subencl].num_ver_descs;
            status = FBE_STATUS_OK;
            break;

        }

        (*start_index) += config_page_info->subencl_desc_array[subencl].num_ver_descs;
    }

    return(status);
}

static fbe_status_t config_page_convert_fw_rev_to_subencl_prod_rev_level(
                                           ses_subencl_prod_rev_level_struct * subencl_prod_rev_level_p, 
                                           fbe_char_t* comp_rev_level_p)
{
    int8_t majorNumber[3] = {0};
    int8_t minorNumber[3] = {0};
    int8_t debugVersion[2] = {0};
    uint32_t temp1 = 0, temp2 = 0, temp3 = 0, temp = 0;
    uint8_t digits[5] = {0};

    majorNumber[0] = *comp_rev_level_p;
    majorNumber[1] = *(comp_rev_level_p + 1);
    if(*(comp_rev_level_p + 2) == '.') 
    {
        minorNumber[0] = *(comp_rev_level_p + 3);
        minorNumber[1] = *(comp_rev_level_p + 4);
        debugVersion[0] = ' ';
    }
    else
    {
        minorNumber[0] = *(comp_rev_level_p + 2);
        minorNumber[1] = *(comp_rev_level_p + 3);
        debugVersion[0] = *(comp_rev_level_p + 4);
    }

    temp1 = atoi((char *)&majorNumber[0]) << 11;
    temp2 = atoi((char *)&minorNumber[0]) << 4;

    if(debugVersion[0] == ' ') 
    {
        temp3 = 0;
    }
    else
    {
        temp3 = debugVersion[0] - 'a' + 1;
    }

    temp = temp1|temp2|temp3;
    snprintf((char *)&digits[0], 5, "%04X", temp);
    memcpy(&subencl_prod_rev_level_p->digits[0], &digits[0], 4);

    return FBE_STATUS_OK;
}

#define ESES_REVISION_3_0   0x3

static fbe_status_t config_page_set_subencl_ver_descs_in_config_page(uint8_t *config_page,
                                                            ses_subencl_type_enum subencl_type,
                                                            terminator_eses_subencl_side side,
                                                            ses_ver_desc_struct *ver_desc_start,
                                                            uint8_t num_ver_descs,
                                                            uint16_t eses_version)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_subencl_desc_struct * subencl_desc_ptr = NULL;
    ses_pg_config_struct * config_pg_hdr = NULL;
    uint8_t subencl = 0;
    uint8_t total_num_subencls = 0;
    uint8_t *config_pg_ptr = (uint8_t *)config_page;
    ses_subencl_prod_rev_level_struct subencl_prod_rev_level;
    uint8_t subencl_slot = 0;


    memset(&subencl_prod_rev_level, 0, sizeof(subencl_prod_rev_level));

    config_pg_hdr = (ses_pg_config_struct *)config_page;
    subencl_desc_ptr = (ses_subencl_desc_struct *)((uint8_t *)config_pg_ptr
                                                    + FBE_ESES_PAGE_HEADER_SIZE);
    // Get the total number of subenclosures.
    // (Adding 1 for the primary subenclosure).
    total_num_subencls =  config_pg_hdr->num_secondary_subencls + 1;

    for(subencl = 0; subencl < total_num_subencls; subencl ++ )
    {
        config_page_get_subencl_slot(subencl_desc_ptr, &subencl_slot);

        if((subencl_type == subencl_desc_ptr->subencl_type) &&
           (side == subencl_slot) )
        {
            // We are now at the required Subenclosure Descriptor
            // Loop through the version descriptors
            if(num_ver_descs != subencl_desc_ptr->num_ver_descs)
            {
                return(FBE_STATUS_GENERIC_FAILURE);
            }
            memcpy(subencl_desc_ptr->ver_desc, ver_desc_start, (num_ver_descs)*sizeof(ses_ver_desc_struct));

            if(eses_version == ESES_REVISION_3_0)
            {
                //Convert the first version descriptor comp_rev_level to subencl_prod_rev_level.
                subencl_desc_ptr->subencl_prod_rev_level.digits[0] = ver_desc_start->cdes_rev.cdes_1_0_rev.comp_rev_level[0];
                subencl_desc_ptr->subencl_prod_rev_level.digits[1] = ver_desc_start->cdes_rev.cdes_1_0_rev.comp_rev_level[1];
                if(ver_desc_start->cdes_rev.cdes_1_0_rev.comp_rev_level[2] == '.')
                {
                    subencl_desc_ptr->subencl_prod_rev_level.digits[2] = ver_desc_start->cdes_rev.cdes_1_0_rev.comp_rev_level[3];
                    subencl_desc_ptr->subencl_prod_rev_level.digits[3] = ver_desc_start->cdes_rev.cdes_1_0_rev.comp_rev_level[4];
                }
                else
                {
                    subencl_desc_ptr->subencl_prod_rev_level.digits[2] = ver_desc_start->cdes_rev.cdes_1_0_rev.comp_rev_level[2];
                    subencl_desc_ptr->subencl_prod_rev_level.digits[3] = ver_desc_start->cdes_rev.cdes_1_0_rev.comp_rev_level[3];
                }
            }
            else
            {
                config_page_convert_fw_rev_to_subencl_prod_rev_level(&subencl_prod_rev_level, 
                                                                  &ver_desc_start->cdes_rev.cdes_1_0_rev.comp_rev_level[0]);
                memcpy(&subencl_desc_ptr->subencl_prod_rev_level, 
                       &subencl_prod_rev_level, 
                       4);
            }
            status = FBE_STATUS_OK;
            break;
        }
        subencl_desc_ptr = fbe_eses_get_next_ses_subencl_desc_p(subencl_desc_ptr);
    }

    return(status);
}

fbe_status_t config_page_set_all_ver_descs_in_config_page(fbe_u8_t *config_page,
                                                        terminator_eses_config_page_info_t *config_page_info,
                                                        ses_ver_desc_struct *ver_desc,
                                                        fbe_u16_t eses_version)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t subencl;
    uint8_t ver_desc_start_index, num_of_ver_descs;

    // The configuration page is loaded here.
    for(subencl = 0; subencl < config_page_info->num_subencls; subencl++)
    {
        status = config_page_info_get_subencl_ver_desc_position(config_page_info,
                                                                config_page_info->subencl_desc_array[subencl].subencl_type,
                                                                config_page_info->subencl_slot[subencl],
                                                                &ver_desc_start_index,
                                                                &num_of_ver_descs);
        if (status != FBE_STATUS_OK)
            return status;

        status = config_page_set_subencl_ver_descs_in_config_page(config_page,
                                                                   config_page_info->subencl_desc_array[subencl].subencl_type,
                                                                   config_page_info->subencl_slot[subencl],
                                                                   &ver_desc[ver_desc_start_index],
                                                                   num_of_ver_descs,
                                                                   eses_version);
        if (status != FBE_STATUS_OK)
            return status;
    }

    return(FBE_STATUS_OK);
}

fbe_status_t config_page_set_gen_code_by_config_page(uint8_t *config_page,
                                                     uint32_t gen_code)
{
    ses_pg_config_struct *config_pg_hdr;

    if(config_page == NULL)
    {
        return(FBE_STATUS_GENERIC_FAILURE);
    }

    config_pg_hdr = (ses_pg_config_struct *)config_page;
    gen_code = bswap32(gen_code);
    memcpy(&config_pg_hdr->gen_code, &gen_code, sizeof(uint32_t));

    return(FBE_STATUS_OK);
}

/**/

fbe_status_t sanity_check_cdb_receive_diagnostic_results(fbe_u8_t *cdb_page_ptr, fbe_u8_t *page_code)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    fbe_eses_receive_diagnostic_results_cdb_t *cdb_page;

    cdb_page = (fbe_eses_receive_diagnostic_results_cdb_t *)(cdb_page_ptr);
    if ((cdb_page->page_code_valid != 1) || 
        (cdb_page->control != 0 ))
    {
        return(status);
    }
    
    *page_code = cdb_page->page_code;

    status = FBE_STATUS_OK;
    return(status);
}

terminator_sas_encl_inq_data_t  encl_table [] = {
    {FBE_SAS_ENCLOSURE_TYPE_BULLET,     SWP16(0), "Simulate", "EMC     ", "ESES Enclosure  ", "0001", "PMCSIERA", 0x8054, 2, "0000000000000000",  0x0001, 0xfffe},
    {FBE_SAS_ENCLOSURE_TYPE_TABASCO,     SWP16(FBE_ESES_ENCLOSURE_BOARD_TYPE_TABASCO), "Sim-TBSC", "EMC     ", "ESES Enclosure  ", "0001", "PMCSIERA", SWP16(0x8054), 2, "CF2CY162200290",  SWP16(0x0002), SWP16(0x0017)},
    /*add stuff only above*/
    {FBE_SAS_ENCLOSURE_TYPE_LAST}
};


fbe_status_t sas_enclosure_get_inq_data (fbe_sas_enclosure_type_t encl_type, terminator_sas_encl_inq_data_t **inq_data)
{
    *inq_data = encl_table;

    while ((*inq_data)->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST) {
        if ((*inq_data)->encl_type == encl_type) {
            return FBE_STATUS_OK;
        }

        (*inq_data)++;
    }

    /*we did not find anything*/
    return FBE_STATUS_GENERIC_FAILURE;
}

#if 0
static fbe_status_t
terminator_virtual_phy_get_enclosure_uid(void *handle, fbe_u8_t **uid)
{
    static fbe_u8_t fakeuid[FBE_SCSI_INQUIRY_ENCLOSURE_SERIAL_NUMBER_SIZE] = {'0', '0', '0', '0'};

    fakeuid[3] += 1;
    *uid = fakeuid;
    return FBE_STATUS_OK;
}
#endif


/* The below PAGE SIZE functions are outdated. 
 * Should be modified in the near future.
 */
fbe_u32_t enclosure_status_diagnostic_page_size(fbe_sas_enclosure_type_t encl_type)
{
    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_BOXWOOD:
        case FBE_SAS_ENCLOSURE_TYPE_PINECONE:
        case FBE_SAS_ENCLOSURE_TYPE_STEELJAW:
        case FBE_SAS_ENCLOSURE_TYPE_RHEA:
            return((FBE_ESES_PAGE_HEADER_SIZE + (13*sizeof(ses_stat_elem_array_dev_slot_struct))));
            break;

        case FBE_SAS_ENCLOSURE_TYPE_DERRINGER:
        case FBE_SAS_ENCLOSURE_TYPE_TABASCO:
                return((FBE_ESES_PAGE_HEADER_SIZE + (26*sizeof(ses_stat_elem_array_dev_slot_struct))));
            break;

        case FBE_SAS_ENCLOSURE_TYPE_CITADEL:
        case FBE_SAS_ENCLOSURE_TYPE_FALLBACK:
        case FBE_SAS_ENCLOSURE_TYPE_KNOT:
        case FBE_SAS_ENCLOSURE_TYPE_RAMHORN:
        case FBE_SAS_ENCLOSURE_TYPE_MIRANDA:
        case FBE_SAS_ENCLOSURE_TYPE_CALYPSO:
            return((FBE_ESES_PAGE_HEADER_SIZE + (26*sizeof(ses_stat_elem_array_dev_slot_struct))));
            break;

        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE:
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            return((FBE_ESES_PAGE_HEADER_SIZE + (30*sizeof(ses_stat_elem_array_dev_slot_struct))));
            break;

        case FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP:
            return( FBE_ESES_PAGE_HEADER_SIZE );
            break;

        case FBE_SAS_ENCLOSURE_TYPE_BUNKER:
        case FBE_SAS_ENCLOSURE_TYPE_VIPER:
        case FBE_SAS_ENCLOSURE_TYPE_ANCHO:
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM:        
            return((FBE_ESES_PAGE_HEADER_SIZE + (16*sizeof(ses_stat_elem_array_dev_slot_struct))));
            break;
            // others here
        default:
            break;
    }
    return(0);
}


/*********************************************************************
* config_page_get_gen_code ()
**********************************************************************
*
*  Description:
*   Gets the Generation code being used by given virtual phy object.
*
*  Inputs:
*   virtual Phy handle & generation code to be returned.
*
*  Return Value: success or failure
*
*  Notes:
*
*  History:
*    Nov-21-2008    Rajesh V created
*
*********************************************************************/
fbe_status_t config_page_get_gen_code(terminator_sas_virtual_phy_info_t *virtual_phy_handle,
                                      fbe_u32_t *gen_code)
{
    fbe_status_t status = FBE_STATUS_OK;
    vp_config_diag_page_info_t *config_diag_page_info;

    config_diag_page_info = &virtual_phy_handle->eses_page_info.vp_config_diag_page_info;

    *gen_code = config_diag_page_info->gen_code;

    return status;
}

/*********************************************************************
* config_page_get_gen_code_by_config_page ()
**********************************************************************
*
*  Description:
*   Gets the gneration code in the given configuration page.
*
*  Inputs:
*   Configuration page, generation code to fill.
*
*  Return Value: success or failure
*
*  Notes:
*
*  History:
*    Nov-19-2008    Rajesh V created
*
*********************************************************************/
static fbe_status_t config_page_get_gen_code_by_config_page (uint8_t *config_page,
                                                      uint32_t *gen_code)
{
    ses_pg_config_struct *config_pg_hdr;

    if(config_page == NULL)
    {
        return(FBE_STATUS_GENERIC_FAILURE);
    }

    config_pg_hdr = (ses_pg_config_struct *)config_page;
    *gen_code = config_pg_hdr->gen_code;
    *gen_code = bswap32(*gen_code);

    return(FBE_STATUS_OK);
}

/******************************************
 * Function: calculateResumeChecksum
 *
 * Desc: calculates resume prom checksum
 *
 * Created: 01-14-08 - Joe Ash
 *******************************************/
static uint32_t calculateResumeChecksum(RESUME_PROM_STRUCTURE* resume_struct)
{
    uint32_t checkSum    = 0;
    uint32_t word_count  = 0;
    uint32_t bufSize     = 0;
    uint32_t *data       = (uint32_t*)resume_struct;
    uint32_t i;

    bufSize = sizeof(RESUME_PROM_STRUCTURE) - RESUME_PROM_CHECKSUM_SIZE;

    word_count =  bufSize / sizeof(uint32_t);

    /* calculate the new checksum */
    for (i = 0; i < word_count; i++, data++)
    {
        checkSum ^= *data;
    }

    checkSum ^= RESUME_PROM_CHECKSUM_SEED;

    return(checkSum);
} 

/*********************************************************************
* config_page_initialize_config_page_info ()
**********************************************************************
*
*  Description:
*   Fills the configuration page related info in the virtual
*   phy object during its creation.
*
*  Inputs:
*   Pointer to the virtual PHy configuration page information.
*
*  Return Value: success or failure
*
*  Notes:
*
*  History:
*    Nov-19-2008    Rajesh V created
*
*********************************************************************/
static fbe_status_t config_page_initialize_config_page_info(fbe_sas_enclosure_type_t encl_type, vp_config_diag_page_info_t *vp_config_diag_page_info)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t subencl, buf_desc, buf;
    //terminator_sp_id_t spid;
    RESUME_PROM_STRUCTURE * pResumeData = NULL;
    dev_eeprom_rev0_info_struct * pEepromData = NULL;
    uint32_t checksum = 0;
    uint32_t viking_ps_buf_len = 0;
    
    // Moved to sas_virtual_phy_info_new()
#if 0
    status =  fbe_terminator_api_get_sp_id(&spid);

    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_TABASCO:
            vp_config_diag_page_info->config_page = tabasco_config_page_with_ps[spid];
            vp_config_diag_page_info->config_page_info = &tabasco_config_page_info_with_ps;
            vp_config_diag_page_info->config_page_size = sizeof(tabasco_config_page_with_ps[spid]);
            break;
        default:
            return(FBE_STATUS_GENERIC_FAILURE);
    }
#endif

    status = config_page_get_gen_code_by_config_page(vp_config_diag_page_info->config_page, &vp_config_diag_page_info->gen_code);
    if (status != FBE_STATUS_OK)
        return -1;

    // Initialize Version descriptor information.
    vp_config_diag_page_info->ver_desc = NULL;
    if(vp_config_diag_page_info->config_page_info->num_ver_descs != 0)
    {
        vp_config_diag_page_info->ver_desc = (ses_ver_desc_struct *)malloc((vp_config_diag_page_info->config_page_info->num_ver_descs)* (sizeof(ses_ver_desc_struct)));
        if(vp_config_diag_page_info->ver_desc == NULL)
        {
            return(FBE_STATUS_GENERIC_FAILURE);
        }

        // Copy the version descriptor array built from the
        // configuration page into the version descriptor
        // array of the virtual phy/ enclosure object.
        memcpy(vp_config_diag_page_info->ver_desc,
            vp_config_diag_page_info->config_page_info->ver_desc_array,
            (vp_config_diag_page_info->config_page_info->num_ver_descs)*sizeof(ses_ver_desc_struct));
    }


    // Initialize the buffer information

    vp_config_diag_page_info->num_bufs = 0;
    vp_config_diag_page_info->buf_info = NULL;
    if(vp_config_diag_page_info->config_page_info->num_buf_descs != 0)
    {
        vp_config_diag_page_info->buf_info = (terminator_eses_buf_info_t *)malloc((vp_config_diag_page_info->config_page_info->num_buf_descs) * (sizeof(terminator_eses_buf_info_t)));

        if(vp_config_diag_page_info->buf_info == NULL)
        {
            return(FBE_STATUS_INSUFFICIENT_RESOURCES);
        }

        vp_config_diag_page_info->num_bufs = vp_config_diag_page_info->config_page_info->num_buf_descs;

        buf = 0;
        // Get the buffer descirptors in each of the subenclosure and fill the buffer information
        // in the virtual phy accordingly.
        for(subencl=0; subencl < vp_config_diag_page_info->config_page_info->num_subencls; subencl++)
        {
            for(buf_desc=0; buf_desc < vp_config_diag_page_info->config_page_info->subencl_buf_desc_info[subencl].num_buf_descs; buf_desc++, buf++)
            {
                vp_config_diag_page_info->buf_info[buf].buf_id = vp_config_diag_page_info->config_page_info->subencl_buf_desc_info[subencl].buf_desc[buf_desc].buf_id;
                vp_config_diag_page_info->buf_info[buf].writable = vp_config_diag_page_info->config_page_info->subencl_buf_desc_info[subencl].buf_desc[buf_desc].writable;
                vp_config_diag_page_info->buf_info[buf].buf_type = vp_config_diag_page_info->config_page_info->subencl_buf_desc_info[subencl].buf_desc[buf_desc].buf_type;
                vp_config_diag_page_info->buf_info[buf].buf_offset_boundary = 0x2;
                
                if( ((encl_type == FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP) || 
                     (encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP) ||
                     (encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP) ||
                     (encl_type == FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP) ||
                     (encl_type == FBE_SAS_ENCLOSURE_TYPE_TABASCO))  &&
                   (vp_config_diag_page_info->config_page_info->subencl_desc_array[subencl].subencl_type == SES_SUBENCL_TYPE_PS) &&
                   (vp_config_diag_page_info->buf_info[buf].buf_type == SES_BUF_TYPE_EEPROM)) 
                {
                    /* Viking PS resume buffer size is 336. */
                    viking_ps_buf_len = 336;
                    vp_config_diag_page_info->buf_info[buf].buf = (uint8_t *)malloc(viking_ps_buf_len);
                    vp_config_diag_page_info->buf_info[buf].buf_len = viking_ps_buf_len;
                    memset(vp_config_diag_page_info->buf_info[buf].buf, 0, viking_ps_buf_len);
                    pEepromData = (dev_eeprom_rev0_info_struct *)vp_config_diag_page_info->buf_info[buf].buf;
                    strncpy((char *)&pEepromData->emc_serialnum[0], "SimulateSN", 10);
                }
                else if(vp_config_diag_page_info->buf_info[buf].buf_type == SES_BUF_TYPE_EEPROM)
                {
                    vp_config_diag_page_info->buf_info[buf].buf = (uint8_t *)malloc(sizeof(RESUME_PROM_STRUCTURE));
                    vp_config_diag_page_info->buf_info[buf].buf_len = sizeof(RESUME_PROM_STRUCTURE);
                    memset(vp_config_diag_page_info->buf_info[buf].buf, 0, sizeof(RESUME_PROM_STRUCTURE));
                    pResumeData = (RESUME_PROM_STRUCTURE *)vp_config_diag_page_info->buf_info[buf].buf;
                    checksum = calculateResumeChecksum(pResumeData);
                    pResumeData->resume_prom_checksum = checksum;
                }
                else
                {
                    vp_config_diag_page_info->buf_info[buf].buf = NULL;
                    vp_config_diag_page_info->buf_info[buf].buf_len = 0;
                }
            }
        }
    }

    return(FBE_STATUS_OK);
}

static fbe_status_t __attribute__((unused)) mode_page_initialize_mode_page_info(terminator_eses_vp_mode_page_info_t *vp_mode_page_info)
{
    fbe_eses_pg_eep_mode_pg_t *default_eep_mode_pg, *current_eep_mode_pg, *saved_eep_mode_pg, *changeable_eep_mode_pg;
    fbe_eses_pg_eenp_mode_pg_t *default_eenp_mode_pg, *current_eenp_mode_pg, *saved_eenp_mode_pg, *changeable_eenp_mode_pg;
    
    memset(vp_mode_page_info, 0, sizeof(terminator_eses_vp_mode_page_info_t));

    default_eep_mode_pg = 
        &vp_mode_page_info->eep_mode_pg[TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_DEFAULT];
    current_eep_mode_pg = 
        &vp_mode_page_info->eep_mode_pg[TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_CURRENT];
    saved_eep_mode_pg = 
        &vp_mode_page_info->eep_mode_pg[TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_SAVED];
    changeable_eep_mode_pg = 
        &vp_mode_page_info->eep_mode_pg[TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_CHANGEABLE];

    default_eenp_mode_pg = 
        &vp_mode_page_info->eenp_mode_pg[TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_DEFAULT];
    current_eenp_mode_pg = 
        &vp_mode_page_info->eenp_mode_pg[TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_CURRENT];
    saved_eenp_mode_pg = 
        &vp_mode_page_info->eenp_mode_pg[TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_SAVED];
    changeable_eenp_mode_pg = 
        &vp_mode_page_info->eenp_mode_pg[TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_CHANGEABLE];

    // Set the saved values for EMC ESES Persistent mode page.
    saved_eep_mode_pg->mode_pg_common_hdr.ps = 1;
    saved_eep_mode_pg->mode_pg_common_hdr.spf = 0;
    saved_eep_mode_pg->mode_pg_common_hdr.pg_code = SES_PG_CODE_EMC_ESES_PERSISTENT_MODE_PG;
    saved_eep_mode_pg->mode_pg_common_hdr.pg_len = FBE_ESES_EMC_ESES_PERSISTENT_MODE_PAGE_LEN;
    // All other values in EMC ESES persistent mode are zero during initialization (like HA MODE-etc).
    // Copy the saved parameter values of EMC ESES persistent page to default & current parameter values.
    memcpy(default_eep_mode_pg,
           saved_eep_mode_pg,
           sizeof(fbe_eses_pg_eep_mode_pg_t));
    memcpy(current_eep_mode_pg,
           saved_eep_mode_pg,
           sizeof(fbe_eses_pg_eep_mode_pg_t));
    memcpy(changeable_eep_mode_pg,
           saved_eep_mode_pg,
           sizeof(fbe_eses_pg_eep_mode_pg_t));
    // Set the changeable parameters appropriately
    changeable_eep_mode_pg->disable_indicator_ctrl = 1;
    changeable_eep_mode_pg->ssu_disable = 1;
    changeable_eep_mode_pg->ha_mode = 1;
    changeable_eep_mode_pg->bad_exp_recovery_enabled = 1;

    // Set the saved values for EMC ESES NON Persistent mode page.
    saved_eenp_mode_pg->mode_pg_common_hdr.ps = 0;
    saved_eenp_mode_pg->mode_pg_common_hdr.spf = 0;
    saved_eenp_mode_pg->mode_pg_common_hdr.pg_code = SES_PG_CODE_EMC_ESES_NON_PERSISTENT_MODE_PG;
    saved_eenp_mode_pg->mode_pg_common_hdr.pg_len = FBE_ESES_EMC_ESES_NON_PERSISTENT_MODE_PAGE_LEN;
    // All other values in EMC ESES NON persistent mode are zero during initialization (like TEST MODE-etc).
    // Copy the saved values of EMC ESES persistent page to default & current pages.
    memcpy(default_eenp_mode_pg,
           saved_eenp_mode_pg,
           sizeof(fbe_eses_pg_eenp_mode_pg_t));
    memcpy(current_eenp_mode_pg,
           saved_eenp_mode_pg,
           sizeof(fbe_eses_pg_eenp_mode_pg_t));
    memcpy(changeable_eenp_mode_pg,
           saved_eenp_mode_pg,
           sizeof(fbe_eses_pg_eenp_mode_pg_t));
    // Set the changeable parameters appropriately.
    changeable_eenp_mode_pg->test_mode = 1;

    return(FBE_STATUS_OK);                   
}


/* The below functions starting with "eses" are interface for terminator into
 * ESES Page routines or vice-versa.
 */
static fbe_status_t eses_initialize_eses_page_info(fbe_sas_enclosure_type_t encl_type, terminator_vp_eses_page_info_t *eses_page_info)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;

    status = config_page_initialize_config_page_info(encl_type, &eses_page_info->vp_config_diag_page_info);
#if 0
    RETURN_ON_ERROR_STATUS;

    status = mode_page_initialize_mode_page_info(&eses_page_info->vp_mode_page_info);
    RETURN_ON_ERROR_STATUS;

    status = download_ctrl_page_initialize_page_info(
        &eses_page_info->vp_download_microcode_ctrl_diag_page_info);
    RETURN_ON_ERROR_STATUS;

    status = download_stat_page_initialize_page_info(
        &eses_page_info->vp_download_micrcode_stat_diag_page_info);
    RETURN_ON_ERROR_STATUS;
#endif
    return status;
}

static fbe_status_t terminator_virtual_phy_get_enclosure_type(terminator_sas_virtual_phy_info_t *info, fbe_sas_enclosure_type_t *encl_type)
{
    *encl_type = info->enclosure_type;
    return FBE_STATUS_OK;
}

/*********************************************************************
* enclosure_status_diagnostic_page_build_device_slot_status_elements()
*********************************************************************
*
*  Description: This builds the Array device slot status elements for 
*   the encl status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of expander elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the expander elements.
*   enclosure information, virtual phy device id and port number.
*
*  Return Value: success or failure
*
*  Notes: 
*  
*
*  History:
*    Aug08  created
*    
*********************************************************************/
fbe_status_t enclosure_status_diagnostic_page_build_device_slot_status_elements(
    fbe_u8_t *device_slot_status_elements_start_ptr, 
    fbe_u8_t **device_slot_status_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *info)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_array_dev_slot_struct *individual_array_dev_slot_stat_ptr = NULL;
    ses_stat_elem_array_dev_slot_struct *overall_array_dev_slot_stat_ptr = NULL;
    fbe_u8_t max_drive_slots = 0;
    fbe_u32_t i;
    fbe_sas_enclosure_type_t  encl_type = info->enclosure_type;

    /* get the enclosure type thru the virtual_phy_handle */
    overall_array_dev_slot_stat_ptr = (ses_stat_elem_array_dev_slot_struct *)device_slot_status_elements_start_ptr; 

    // set the fields in overall device slot status element
    // for now all of fields are ignored in overall status element
    memset (overall_array_dev_slot_stat_ptr, 0, sizeof(ses_stat_elem_array_dev_slot_struct));

    individual_array_dev_slot_stat_ptr = overall_array_dev_slot_stat_ptr;  
    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots, info->side);
    if(status != FBE_STATUS_OK)
    {
        return(status);
    }

    for(i=0; i<max_drive_slots; i++)
    {
        individual_array_dev_slot_stat_ptr++; 
        memset(individual_array_dev_slot_stat_ptr, 0, sizeof(ses_stat_elem_array_dev_slot_struct));
        /* Get the drive status page from terminator */
        fbe_copy_memory(individual_array_dev_slot_stat_ptr, &info->drive_slot_status[i], sizeof(ses_stat_elem_array_dev_slot_struct));
    }
    *device_slot_status_elements_end_ptr = (fbe_u8_t *)(individual_array_dev_slot_stat_ptr + 1);   

    return(status);
}

fbe_status_t encl_stat_diag_page_exp_phy_elem_get_exp_index(
    terminator_sas_virtual_phy_info_t *info,
    fbe_u8_t position,
    fbe_u8_t phy_id,
    fbe_u8_t *exp_index)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    fbe_u8_t start_element_index = 0;
    fbe_u8_t max_phys;
    fbe_sas_enclosure_type_t encl_type = info->enclosure_type;
    terminator_sp_id_t spid;

    status =  fbe_terminator_api_get_sp_id(info, &spid);

    status = sas_virtual_phy_max_phys(encl_type, &max_phys, info->side);
    RETURN_ON_ERROR_STATUS;

    //Some sanity checks on passed parameters
    if((position >= max_phys) ||
       (phy_id >= max_phys))
    {
        return(status);
    }

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(info,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_SAS_EXP, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &start_element_index);
    RETURN_ON_ERROR_STATUS;

    //Expander index is the same for all Phys.
    // This should be chagned to SAS exp once 
    // we get the config page contaning esc-elec elements.
    *exp_index = start_element_index;

    status = FBE_STATUS_OK;
    return(status);
}

/*********************************************************************
* enclosure_status_diagnostic_page_build_exp_phy_status_elements()
*********************************************************************
*
*  Description: This builds the expander Phy status elements for the encl
*   status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of expander elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the expander elements.
*   enclosure information, virtual phy device id and port number.
*
*  Return Value: success or failure
*
*  Notes: 
*  
*
*  History:
*    Aug08  created
*    
*********************************************************************/

static fbe_status_t enclosure_status_diagnostic_page_build_exp_phy_status_elements(
    fbe_u8_t *exp_phy_status_elements_start_ptr, 
    fbe_u8_t **exp_phy_status_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *info)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_exp_phy_struct *individual_exp_phy_stat_ptr = NULL;
    ses_stat_elem_exp_phy_struct *overall_exp_phy_stat_ptr = NULL;
    fbe_u8_t max_phy_slots = 0;
    fbe_u8_t i;
    fbe_sas_enclosure_type_t            encl_type = info->enclosure_type;

    overall_exp_phy_stat_ptr = (ses_stat_elem_exp_phy_struct *)exp_phy_status_elements_start_ptr; 

    // set the fields in expander phy status element
    // for now all of fields are ignored in overall status element
    memset (overall_exp_phy_stat_ptr, 0, sizeof(ses_stat_elem_exp_phy_struct));

    individual_exp_phy_stat_ptr = overall_exp_phy_stat_ptr;  
    status = sas_virtual_phy_max_phys(encl_type, &max_phy_slots, info->side);
    if(status != FBE_STATUS_OK)
    {
        return(status);
    }

    for(i=0; i<max_phy_slots; i++)
    {
        individual_exp_phy_stat_ptr++; 
        memset(individual_exp_phy_stat_ptr, 0, sizeof(ses_stat_elem_exp_phy_struct));
        /* Get the phy status page for terminator */
        fbe_copy_memory(individual_exp_phy_stat_ptr, &info->phy_status[i], sizeof(ses_stat_elem_exp_phy_struct));
        status = encl_stat_diag_page_exp_phy_elem_get_exp_index(info,
                                                                i,
                                                                individual_exp_phy_stat_ptr->phy_id, 
                                                                &individual_exp_phy_stat_ptr->exp_index);
        RETURN_ON_ERROR_STATUS;

    }

    // build virtual phy
    individual_exp_phy_stat_ptr++;
    individual_exp_phy_stat_ptr->phy_rdy = 0x1;
    individual_exp_phy_stat_ptr->link_rdy = 0x1;
    individual_exp_phy_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
    individual_exp_phy_stat_ptr->phy_id = max_phy_slots;
    individual_exp_phy_stat_ptr->exp_index = (individual_exp_phy_stat_ptr - 1)->exp_index;

    *exp_phy_status_elements_end_ptr = (fbe_u8_t *)(individual_exp_phy_stat_ptr + 1);   

    return(status);
}

/*********************************************************************
* enclosure_status_diagnostic_page_build_peer_exp_phy_status_elements()
*********************************************************************
*
*  Description: This builds the peer expander Phy status elements for the
*   encl status diagnostic page.
*
*  Inputs:
*   status_elements_start_ptr - pointer to the start of expander elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the expander elements.
*   enclosure information, virtual phy device id and port number.
*
*  Return Value: success or failure
*
*  Notes:
*
*
*  History:
*    Aug08  created
*
*********************************************************************/

static fbe_status_t enclosure_status_diagnostic_page_build_peer_exp_phy_status_elements(
    fbe_u8_t *exp_phy_status_elements_start_ptr,
    fbe_u8_t **exp_phy_status_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *info)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_exp_phy_struct *individual_exp_phy_stat_ptr = NULL;
    ses_stat_elem_exp_phy_struct *overall_exp_phy_stat_ptr = NULL;
    // fbe_u8_t max_phy_slots = 0;
    fbe_u8_t max_conn_id_count = 0;
    fbe_u8_t max_single_lane_port_conn_count = 0;
    fbe_u8_t max_port_conn_count = 0;
    fbe_u8_t i, j;
    fbe_sas_enclosure_type_t encl_type = info->enclosure_type;
    terminator_sp_id_t spid;

    // Get mapping based on encl_type
    status = sas_virtual_phy_max_conn_id_count(encl_type, &max_conn_id_count, info->side);
    RETURN_ON_ERROR_STATUS;

    status = sas_virtual_phy_max_single_lane_conns_per_port(encl_type, &max_single_lane_port_conn_count, info->side);
    RETURN_ON_ERROR_STATUS;

    status = sas_virtual_phy_max_conns_per_port(encl_type, &max_port_conn_count, info->side);
    RETURN_ON_ERROR_STATUS;

    status = fbe_terminator_api_get_sp_id(info, &spid);
    RETURN_ON_ERROR_STATUS;

    overall_exp_phy_stat_ptr = (ses_stat_elem_exp_phy_struct *)exp_phy_status_elements_start_ptr;

    // set the fields in expander phy status element
    // for now all of fields are ignored in overall status element
    memset (overall_exp_phy_stat_ptr, 0, sizeof(ses_stat_elem_exp_phy_struct));

    individual_exp_phy_stat_ptr = overall_exp_phy_stat_ptr;
    for (j = 0; j < max_conn_id_count; j++) {
        for (i = 0; i < max_single_lane_port_conn_count; i++) {
            individual_exp_phy_stat_ptr++;

            // set status
            individual_exp_phy_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

            // set expander index
            status = config_page_get_start_elem_index_in_stat_page(info,
                                                                   SES_SUBENCL_TYPE_LCC,
                                                                   !spid,
                                                                   SES_ELEM_TYPE_SAS_EXP,
                                                                   FALSE,
                                                                   0,
                                                                   FALSE,
                                                                   NULL,
                                                                   &individual_exp_phy_stat_ptr->exp_index);

            // set phy id
            uint8_t phy_id = 0;
            status = sas_virtual_phy_get_individual_conn_to_phy_mapping(i, j, &phy_id, encl_type, info->side);
            individual_exp_phy_stat_ptr->phy_id = phy_id;

            // FIXME
            // set phy_rdy and link_rdy
            switch (j) {
                case 0:
                    individual_exp_phy_stat_ptr->phy_rdy = 0x1;
                    individual_exp_phy_stat_ptr->link_rdy = individual_exp_phy_stat_ptr->phy_rdy;
                    break;
                case 1:
                    individual_exp_phy_stat_ptr->phy_rdy = 0x0;
                    individual_exp_phy_stat_ptr->link_rdy = individual_exp_phy_stat_ptr->phy_rdy;
                    break;
                default:
                    individual_exp_phy_stat_ptr->phy_rdy = 0x0;
                    individual_exp_phy_stat_ptr->link_rdy = individual_exp_phy_stat_ptr->phy_rdy;
                    break;
            }
        }
    }

    *exp_phy_status_elements_end_ptr = (fbe_u8_t *)(individual_exp_phy_stat_ptr + 1);

    return(status);
}

static fbe_status_t encl_stat_diag_page_sas_conn_elem_get_conn_phyical_link(
    terminator_sas_virtual_phy_info_t *info,
    fbe_u8_t position, 
    fbe_sas_enclosure_type_t encl_type,
    fbe_u8_t *conn_physical_link)
{
    fbe_status_t status = FBE_STATUS_OK;
    fbe_u8_t max_conns_per_port;
    fbe_u8_t max_conns_per_lcc; 

    status = sas_virtual_phy_max_conns_per_port(encl_type, &max_conns_per_port, info->side);
    RETURN_ON_ERROR_STATUS;

    status = sas_virtual_phy_max_conns_per_lcc(encl_type, &max_conns_per_lcc, info->side);
    RETURN_ON_ERROR_STATUS;

    if((status != FBE_STATUS_OK) || (position >= max_conns_per_lcc))
    {
        return(FBE_STATUS_GENERIC_FAILURE);
    }

    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_VIPER:
        case FBE_SAS_ENCLOSURE_TYPE_MAGNUM:
        case FBE_SAS_ENCLOSURE_TYPE_BUNKER:
        case FBE_SAS_ENCLOSURE_TYPE_CITADEL:
        case FBE_SAS_ENCLOSURE_TYPE_DERRINGER:
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM:
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE:
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_FALLBACK:
        case FBE_SAS_ENCLOSURE_TYPE_BOXWOOD:
        case FBE_SAS_ENCLOSURE_TYPE_KNOT:
        case FBE_SAS_ENCLOSURE_TYPE_PINECONE:
        case FBE_SAS_ENCLOSURE_TYPE_STEELJAW:
        case FBE_SAS_ENCLOSURE_TYPE_RAMHORN:
        case FBE_SAS_ENCLOSURE_TYPE_ANCHO:
        case FBE_SAS_ENCLOSURE_TYPE_RHEA:
        case FBE_SAS_ENCLOSURE_TYPE_MIRANDA:
        case FBE_SAS_ENCLOSURE_TYPE_CALYPSO:
        case FBE_SAS_ENCLOSURE_TYPE_TABASCO:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP:
            if((position%max_conns_per_port) == 0)
            {
                *conn_physical_link = 0xFF;
            }
            else 
            {
                *conn_physical_link = ((position%max_conns_per_port)-1);
            }
            break;

        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
            if((position == 0) || 
               (position == 5) ||
               (position == 10) ||
               (position == 16) ||
               (position == 22) ||
               (position == 28) ||
               (position == 34) ||
               (position == 39)) 
            {
                *conn_physical_link = 0xFF;
            }
            else if(position > 0 && position < 10) 
            {
                *conn_physical_link = (position%5) - 1;
            }
            else if(position > 10 && position < 34) 
            {
                *conn_physical_link = ((position - 10)%6) - 1;
            }
            else if(position > 34 && position < 43) 
            {
                *conn_physical_link = ((position - 34)%5) - 1;
            }
            else
            {
                *conn_physical_link = 0xFF;
            }
            break;

        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP:
            if((position == 0) || 
               (position == 5) ||
               (position == 10) ||
               (position == 15) ||
               (position == 20)) 
            {
                *conn_physical_link = 0xFF;
            }
            else if(position > 0 && position < 20) 
            {
                *conn_physical_link = (position%5) - 1;
            }
            else if(position > 20 && position < 28) 
            {
                *conn_physical_link = ((position - 20)%9) - 1;
            }
            else
            {
                *conn_physical_link = 0xFF;
            }
            break;

    case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
// JJB NAGA_80 cleanup
    case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            if((position == 0) || 
               (position == 5) ||
               (position == 10) ||
               (position == 15) ||
               (position == 20) ||
               (position == 29))
            {
                *conn_physical_link = 0xFF;
            }
            else if(position > 0 && position < 20) 
            {
                *conn_physical_link = (position%5) - 1;
            }
            else if(position > 20 && position < 38) 
            {
                *conn_physical_link = ((position - 20)%9) - 1;
            }
            else
            {
                *conn_physical_link = 0xFF;
            }
            break;

        default:
            status = FBE_STATUS_GENERIC_FAILURE;
            break;
    }
    return(status);

}




/*!****************************************************************************
 * @fn      fbe_status_t terminator_map_position_max_conns_to_range_conn_id()
 *
 * @brief
 *  This function maps a connector into one of several possible ranges and connector id. 
 *
 * @param position - connector position
 * @param max_conns - maximum connectors per range
 * @param return_range (OUTPUT)- pointer to the return connector range enum 
 * @param conn_id (OUTPUT) - pointer to the return connector id. 

 *
 * @return
 *  FBE_STATUS_OK if the connector is in a valid range.
 * @Note the return_range is set to illegal even if the return status
 * is FAILURE.
 * @Note This function should not be used to calculate range for Voyager_ee as for Voyager
 * ee primary port we are expecting connector is DOWNSTREAM but this function will assume
 * UPSTREAM.
 *
 * HISTORY
 *  05/11/10 :  Gerry Fredette -- Created.
 *  26-Nov-2-13: PHE - Added the support for Viking.
 *
 *****************************************************************************/
static fbe_status_t terminator_map_position_max_conns_to_range_conn_id(
    fbe_sas_enclosure_type_t encl_type,
    fbe_u8_t position, 
    fbe_u8_t max_conns, 
    terminator_conn_map_range_t *return_range, 
    fbe_u8_t *conn_id)
{
    if(encl_type == FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP)
    {
        if(position <= 4)
        {
            // Indicates the connector belongs to the primary port
            *return_range = CONN_IS_DOWNSTREAM;
            *conn_id = 0;
        }
        else if(position >= 5 && position <= 9)
        {
            // Indicates the connector belongs to the primary port
            *return_range = CONN_IS_UPSTREAM;
            *conn_id = 1;
        }
        else if(position >= 10 && position <= 33) 
        {
            // search child list to match connector id. for SXP0, SXP1, SXP2, or SXP3 (Viking)
            *return_range = CONN_IS_INTERNAL_RANGE1;
            *conn_id = 2 + (position - 10)/6;
        }
        else if(position >= 34 && position <= 43) 
        {
            // search child list to match connector id. for SXP0, SXP1, SXP2, or SXP3 (Viking)
            *return_range = CONN_IS_RANGE0;
            *conn_id = 6 + (position - 34)/5;
        }
        else
        {
            *return_range = CONN_IS_ILLEGAL;
            return FBE_STATUS_GENERIC_FAILURE;
        }
    }
    else if(encl_type == FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP)
    {
        if(position <= 4)
        {
            // Indicates the connector belongs to the primary port
            *return_range = CONN_IS_DOWNSTREAM;
            *conn_id = 0;
        }
        else if(position >= 5 && position <= 9)
        {
            // Indicates the connector belongs to the primary port
            *return_range = CONN_IS_UPSTREAM;
            *conn_id = 1;
        }
        else if(position >= 20 && position <= 28) 
        {
            // search child list to match connector id. for SXP0, SXP1, SXP2, or SXP3 (Viking)
            *return_range = CONN_IS_INTERNAL_RANGE1;
            *conn_id = 4;
        }
        else if(position >= 10 && position <= 19) 
        {
            // search child list to match connector id. for SXP0, SXP1, SXP2, or SXP3 (Viking)
            *return_range = CONN_IS_RANGE0;
            *conn_id = 2 + (position - 10)/5;
        }
        else
        {
            *return_range = CONN_IS_ILLEGAL;
            return FBE_STATUS_GENERIC_FAILURE;
        }
    }
    // JJB NAGA_80 cleanup
    else if((encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP)||(encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP))
    {
        if(position <= 4)
        {
            // Indicates the connector belongs to the primary port
            *return_range = CONN_IS_DOWNSTREAM;
            *conn_id = 0;
        }
        else if(position >= 5 && position <= 9)
        {
            // Indicates the connector belongs to the primary port
            *return_range = CONN_IS_UPSTREAM;
            *conn_id = 1;
        }
        else if(position >= 20 && position <= 37) 
        {
            // search child list to match connector id. for SXP0, SXP1, SXP2, or SXP3 (Viking)
            *return_range = CONN_IS_INTERNAL_RANGE1;
            *conn_id = 4 + (position - 20)/9; // this is magic to get 4 and 5 as connector id
        }
        else if(position >= 10 && position <= 19) 
        {
            // search child list to match connector id. for SXP0, SXP1, SXP2, or SXP3 (Viking)
            *return_range = CONN_IS_RANGE0;
            *conn_id = 2 + (position - 10)/5;
        }
        else
        {
            *return_range = CONN_IS_ILLEGAL;
            return FBE_STATUS_GENERIC_FAILURE;
        }
    }
    else 
    {
        if(position < max_conns)
        {
            // Indicates the connector belongs to the primary port
            *return_range = CONN_IS_UPSTREAM;
        }
        else if(position >= max_conns && (position < max_conns * 2))
        {
            // Indicates the connector belongs to the extension port
            *return_range = CONN_IS_DOWNSTREAM;
        }
        else if((position >= max_conns * 2) && (position < max_conns * 4))
        {
            // search child list to match connector id. 
            *return_range = CONN_IS_RANGE0;
        }
        else if((position >= max_conns * 4) && (position < max_conns * 6))
        {
            // search child list to match connector id. for EE0 or EE1 (Voyager)
            *return_range = CONN_IS_INTERNAL_RANGE1;
        }
        else
        {
            *return_range = CONN_IS_ILLEGAL;
            return FBE_STATUS_GENERIC_FAILURE;
        }
    
        if (conn_id)
        {
            // The conn_id is 0 for downstream, 1 for upstream and 2, 3, ... for children.
            // EE0 is 2, EE1 is 3.   EE3 is 4 and EE4 is 5.
            *conn_id = position / max_conns;  
        }
    }

    return FBE_STATUS_OK;
}


/*
 * This function returns the status of the upstream enclosure(primary)
 * for the given virtual phy id. The status (inserted or not
 * inserted) is indicated in by the "inserted" parameter passed.
 */
static fbe_status_t terminator_get_upstream_wideport_device_status(
    terminator_sas_virtual_phy_info_t *info,
    fbe_bool_t *inserted)
{
    fbe_status_t status = FBE_STATUS_OK;

    //As there exists always some thing on the upstream port of the enclosure,
    // for now return the upstream wideport status as OK. In future we may have
    // to consider the SAS cable state--etc.
    *inserted = FBE_TRUE;
    status = FBE_STATUS_OK;
    return(status);
}

/*
 * This function returns the status of the downstream enclosure(expansion)
 * for the given virtual phy id. The status (inserted or not
 * inserted) is indicated in by the "inserted" parameter passed.
 */
static fbe_status_t terminator_get_downstream_wideport_device_status(
    terminator_sas_virtual_phy_info_t *info,
    fbe_bool_t *inserted)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;

    *inserted = FBE_FALSE;
//TODO: simply set not inserted atm
#if 0
    base_component_t *parent_encl = NULL;
    base_component_t *downstream_encl = NULL;

    base_component_get_parent(virtual_phy_handle, &parent_encl);
    if(base_component_get_component_type(parent_encl) != TERMINATOR_COMPONENT_TYPE_ENCLOSURE)
    {
        return(status);
    }

    downstream_encl = base_component_get_child_by_type(parent_encl,
                                                       TERMINATOR_COMPONENT_TYPE_ENCLOSURE);

    // we need to find the next downstream enclosure, not EEs
    while((downstream_encl != NULL) && 
          ((base_component_get_component_type(downstream_encl) != TERMINATOR_COMPONENT_TYPE_ENCLOSURE) ||
           (sas_enclosure_get_logical_parent_type((terminator_enclosure_t *)downstream_encl) != TERMINATOR_COMPONENT_TYPE_PORT)))
    {
        downstream_encl = base_component_get_next_child(parent_encl, downstream_encl);
    }


    if(downstream_encl != NULL)
    {
        //downstream enclosure exists

        // The second check of checking if the enclosure state is LOGIN COMPLETE
        // is only temparary. We need to actually set and check state (NOT EQUALS)
        // TERMINATOR_COMPONENT_STATE_NOT_PRESENT & clear it appropriately. The
        // phy status (and not connector) should change if enclosure is logged out.
        // Removed the second check. - PHE
        *inserted = FBE_TRUE;
    }
#endif
    status = FBE_STATUS_OK;
    return(status);
}


/*!*************************************************************************
* @fn terminator_get_child_expander_wideport_device_status_by_connector_id(
*    fbe_terminator_device_ptr_t virtual_phy_handle,
*    fbe_u8_t conn_id,
*    fbe_bool_t *inserted)
***************************************************************************
*
* @brief
*       This function gets device status of child expander port(Edge Expander)
*       on the basis of connector ID.
*
* @param    virtual_phy_handle - handle to virtual phy.
* @param    conn_id - internal connector ID.
* @param    *inserted - pointer to value where we will update if device 
*                       is inserted or not.
*
* @return     fbe_status_t.
*
* NOTES
*
* HISTORY
*   28-APR-2010: Created -Dipak Patel
*
***************************************************************************/
static fbe_status_t terminator_get_child_expander_wideport_device_status_by_connector_id(
    terminator_sas_virtual_phy_info_t *info,
    fbe_u8_t conn_id,
    fbe_bool_t *inserted)
{

    *inserted = FBE_FALSE;
#if 0
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    base_component_t *parent_encl = NULL;
    base_component_t *downstream_encl = NULL;
    terminator_sas_enclosure_info_t *attributes = NULL;

    status = base_component_get_parent(virtual_phy_handle, &parent_encl);
    RETURN_ON_ERROR_STATUS;
    if(base_component_get_component_type(parent_encl) != TERMINATOR_COMPONENT_TYPE_ENCLOSURE)
    {
        return FBE_STATUS_GENERIC_FAILURE;
    }
    
    downstream_encl = base_component_get_first_child(parent_encl);
    
    while (downstream_encl != NULL)
    {
        if(base_component_get_component_type(downstream_encl) == TERMINATOR_COMPONENT_TYPE_ENCLOSURE)
        {
            attributes = base_component_get_attributes(downstream_encl);
            if(attributes->logical_parent_type == TERMINATOR_COMPONENT_TYPE_ENCLOSURE && attributes->connector_id == conn_id)
            {
                *inserted = FBE_TRUE;
                return (FBE_STATUS_OK);
            }
        }
        downstream_encl = base_component_get_next_child(parent_encl, downstream_encl);
    }
#endif
    /* As we want to report elem_stat_code in all the cases, we will always 
       return FBE_STATUS_OK eventhough we are not able to find match. */

    return FBE_STATUS_OK;;
}

// Get the SAS connector eses status ( complete element structure)
static fbe_status_t terminator_get_sas_conn_eses_status(
    terminator_sas_virtual_phy_info_t *info,
    terminator_eses_sas_conn_id sas_conn_id,
    ses_stat_elem_sas_conn_struct *sas_conn_stat)
{
    fbe_status_t status = FBE_STATUS_OK;
    sas_conn_stat->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
#if 0
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    fbe_terminator_device_ptr_t matching_port = NULL;

    terminator_get_port_ptr(virtual_phy_handle, &matching_port);
    if (matching_port == NULL)
    {
        return status;
    }

    /* make sure this is a virtual phy */
    if (base_component_get_component_type(virtual_phy_handle)!=TERMINATOR_COMPONENT_TYPE_VIRTUAL_PHY)
    {
        return status;
    }
    /* Caller should lock the port configuration so no one else can change it while we modify configuration */
    status = sas_virtual_phy_get_sas_conn_eses_status(virtual_phy_handle, sas_conn_id, sas_conn_stat);
#endif

    return status;
}

static fbe_status_t encl_stat_diag_page_sas_conn_elem_get_sas_conn_status(
    terminator_sas_virtual_phy_info_t *info,
    fbe_u8_t position,
    ses_stat_elem_sas_conn_struct *sas_conn_stat_ptr)
{
    fbe_status_t status = FBE_STATUS_OK;
    fbe_u8_t max_conns_per_port;
    fbe_u8_t max_conns_per_lcc; 
    fbe_bool_t inserted = FBE_FALSE;
    fbe_sas_enclosure_type_t            encl_type = info->enclosure_type;
    fbe_u8_t conn_id;
    terminator_conn_map_range_t         range = CONN_IS_UPSTREAM;

    status = sas_virtual_phy_max_conns_per_port(encl_type, &max_conns_per_port, info->side);
    RETURN_ON_ERROR_STATUS;
 
    status = sas_virtual_phy_max_conns_per_lcc(encl_type, &max_conns_per_lcc, info->side);
    RETURN_ON_ERROR_STATUS;

    if((status != FBE_STATUS_OK) || (position >= max_conns_per_lcc))
    {
        return(FBE_STATUS_GENERIC_FAILURE);
    }

    inserted = FBE_FALSE;

    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_DERRINGER:
        case FBE_SAS_ENCLOSURE_TYPE_VIPER:
        case FBE_SAS_ENCLOSURE_TYPE_BUNKER:
        case FBE_SAS_ENCLOSURE_TYPE_CITADEL:
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM:
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_FALLBACK:
        case FBE_SAS_ENCLOSURE_TYPE_BOXWOOD:
        case FBE_SAS_ENCLOSURE_TYPE_KNOT:
        case FBE_SAS_ENCLOSURE_TYPE_PINECONE:
        case FBE_SAS_ENCLOSURE_TYPE_STEELJAW:
        case FBE_SAS_ENCLOSURE_TYPE_RAMHORN:
        case FBE_SAS_ENCLOSURE_TYPE_ANCHO:
        case FBE_SAS_ENCLOSURE_TYPE_RHEA:
        case FBE_SAS_ENCLOSURE_TYPE_MIRANDA:
        case FBE_SAS_ENCLOSURE_TYPE_CALYPSO:
        case FBE_SAS_ENCLOSURE_TYPE_TABASCO:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            terminator_map_position_max_conns_to_range_conn_id(encl_type, position, max_conns_per_port, &range, &conn_id);
            switch(range)
            {
                case CONN_IS_DOWNSTREAM:
                    // Indicates the connector belongs to the extension port
                    status = terminator_get_downstream_wideport_device_status(info, &inserted);    
                    break;
                case CONN_IS_UPSTREAM:
                    // Indicates the connector belongs to the primary port
                    status = terminator_get_upstream_wideport_device_status(info, &inserted);
                    break;
                case CONN_IS_RANGE0:
                    if((encl_type == FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM) ||
                        (encl_type == FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP) ||
                        (encl_type == FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP) ||
                        (encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP) ||
                        (encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP))
                    {
                        // Indicates the connector external unused.
                        inserted = FALSE;
                        status = FBE_STATUS_OK;
                    }
                    break;
                case CONN_IS_INTERNAL_RANGE1:
                case CONN_IS_INTERNAL_RANGE2:
                    // search child list to match connector id. for EE0 or EE1 (Voyager or Viking) and EE2 or EE3(viking)
                    // The position range for EE0 and EE1 would be from (max_conns* 4) to ((max_conn*6) -1)
                    // The position range for EE2 and EE3 would be from (max_conns* 4) to ((max_conn*6) -1) 
                    status = terminator_get_child_expander_wideport_device_status_by_connector_id(info, conn_id, &inserted);
                    break;
                case CONN_IS_ILLEGAL:
                default:
                    status = FBE_STATUS_GENERIC_FAILURE;
                    break;
            }
            break;

        case FBE_SAS_ENCLOSURE_TYPE_MAGNUM:
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE:
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP:
            // Magnum, Voyager EE has a 4XSAS connector to the SAS controller. It will not have any
            // expansion port connectors.
            status = terminator_get_upstream_wideport_device_status(info, &inserted);
            break;

        default:
            status = FBE_STATUS_GENERIC_FAILURE;
            break;
    }
            

    if(status == FBE_STATUS_OK)
    {
        if(inserted)
        {
            status = terminator_get_sas_conn_eses_status(info, position, sas_conn_stat_ptr);
        }
        else
        {
            sas_conn_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_NOT_INSTALLED;
        }
    }

    if((range == CONN_IS_INTERNAL_RANGE1) || (range == CONN_IS_INTERNAL_RANGE2))
    {
        sas_conn_stat_ptr->conn_type = FBE_ESES_CONN_TYPE_INTERNAL_CONN;
    }
    else
    {   
        sas_conn_stat_ptr->conn_type = FBE_ESES_CONN_TYPE_MINI_SAS_HD_4X;
    }
    return(status);
}




/*********************************************************************
*  enclosure_status_diagnostic_page_build_peer_sas_conn_status_elements ()
*********************************************************************
*
*  Description: This builds the peer SAS connector elements for the encl 
*   status diagnostic page. (PEER LCC)
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of expander elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the expander elements.
*   enclosure information, virtual phy device id and port number.
*
*  Return Value: success or failure
*
*  Notes: 
*   We assume peeer lcc is not installed and fill its conn elem.
*
*  History:
*    Aug08  created
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_peer_sas_conn_status_elements(
    fbe_u8_t *sas_conn_status_elements_start_ptr, 
    fbe_u8_t **sas_conn_status_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *info)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_sas_conn_struct *individual_sas_conn_stat_ptr = NULL;
    ses_stat_elem_sas_conn_struct *overall_sas_conn_stat_ptr = NULL;
    fbe_u8_t max_total_conns = 0;
    fbe_u8_t max_conns_per_port = 0;
    fbe_u8_t i;
    fbe_sas_enclosure_type_t            encl_type = info->enclosure_type;

    overall_sas_conn_stat_ptr = (ses_stat_elem_sas_conn_struct *)sas_conn_status_elements_start_ptr; 

    // for now all of fields are ignored in overall status element
    memset (overall_sas_conn_stat_ptr, 0, sizeof(ses_stat_elem_sas_conn_struct));

    individual_sas_conn_stat_ptr = overall_sas_conn_stat_ptr;  

    status = sas_virtual_phy_max_conns_per_port(encl_type, &max_conns_per_port, info->side);
    RETURN_ON_ERROR_STATUS;

    if (max_conns_per_port == 0)
    {
        printf( "%s: max connections per port is zero.\n", __FUNCTION__);
        return FBE_STATUS_GENERIC_FAILURE;
    }

    status = sas_virtual_phy_max_conns_per_lcc(encl_type, &max_total_conns, info->side);
    RETURN_ON_ERROR_STATUS;

    // Fill in the peer LCC SAS connector element statuses. 
    // There is also a overall SAS connector element as the peer lcc is a
    // seperate subenclosure. For now assume that peer lcc is not inserted.
    //for(i=0; i<max_total_conns; i++)
    //{

    //    individual_sas_conn_stat_ptr++; 
    //    memset(individual_sas_conn_stat_ptr, 0, sizeof(ses_stat_elem_sas_conn_struct));
    //    individual_sas_conn_stat_ptr->conn_type = FBE_ESES_CONN_TYPE_MINI_SAS_HD_4X;

    //    encl_stat_diag_page_sas_conn_elem_get_conn_phyical_link(i, 
    //                                                            encl_type, 
    //                                                            &individual_sas_conn_stat_ptr->conn_physical_link); 
    //
    //    individual_sas_conn_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_NOT_INSTALLED;
    //}
    for(i=0; i<max_total_conns; i++)
    {
        individual_sas_conn_stat_ptr++; 
        memset(individual_sas_conn_stat_ptr, 0, sizeof(ses_stat_elem_sas_conn_struct));
        status = encl_stat_diag_page_sas_conn_elem_get_conn_phyical_link(info, i, 
            encl_type, &individual_sas_conn_stat_ptr->conn_physical_link);
        RETURN_ON_ERROR_STATUS;
        /* Get the conn status from terminator */
        status = encl_stat_diag_page_sas_conn_elem_get_sas_conn_status(info, i, individual_sas_conn_stat_ptr);
        if(status != FBE_STATUS_OK)
        {
            return(status);
        }
    }
    *sas_conn_status_elements_end_ptr = (fbe_u8_t *)( individual_sas_conn_stat_ptr + 1);    

    return(status);    
}

/*********************************************************************
*  enclosure_status_diagnostic_page_build_local_sas_conn_status_elements ()
*********************************************************************
*
*  Description: This builds the local SAS connector elements for the encl 
*   status diagnostic page. (LOCAL LCC)
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of expander elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the expander elements.
*   enclosure information, virtual phy device id and port number.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*    Aug08  created
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_local_sas_conn_status_elements(
    fbe_u8_t *sas_conn_status_elements_start_ptr, 
    fbe_u8_t **sas_conn_status_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *info)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_sas_conn_struct *individual_sas_conn_stat_ptr = NULL;
    ses_stat_elem_sas_conn_struct *overall_sas_conn_stat_ptr = NULL;
    fbe_u8_t max_conns = 0;
    fbe_u8_t i;
    fbe_sas_enclosure_type_t            encl_type = info->enclosure_type;

    overall_sas_conn_stat_ptr = (ses_stat_elem_sas_conn_struct *)sas_conn_status_elements_start_ptr; 

    // for now all of fields are ignored in overall status element
    memset (overall_sas_conn_stat_ptr, 0, sizeof(ses_stat_elem_sas_conn_struct));

    individual_sas_conn_stat_ptr = overall_sas_conn_stat_ptr;  
    status = sas_virtual_phy_max_conns_per_lcc(encl_type, &max_conns, info->side);
    RETURN_ON_ERROR_STATUS;

    for(i=0; i<max_conns; i++)
    {
        individual_sas_conn_stat_ptr++; 
        memset(individual_sas_conn_stat_ptr, 0, sizeof(ses_stat_elem_sas_conn_struct));
        status = encl_stat_diag_page_sas_conn_elem_get_conn_phyical_link(info, i, 
            encl_type, &individual_sas_conn_stat_ptr->conn_physical_link);
        RETURN_ON_ERROR_STATUS;
        /* Get the conn status from terminator */
        status = encl_stat_diag_page_sas_conn_elem_get_sas_conn_status(info, i, individual_sas_conn_stat_ptr);
        if(status != FBE_STATUS_OK)
        {
            return(status);
        }

    }
    *sas_conn_status_elements_end_ptr = (fbe_u8_t *)( individual_sas_conn_stat_ptr + 1);

    return(status);    
}


/*********************************************************************
*  enclosure_status_diagnostic_page_build_midplane_sas_conn_status_elements ()
*********************************************************************
*
*  Description: This builds the midplane SAS connector elements for the encl 
*   status diagnostic page. (MIDPLANE)
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of expander elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the expander elements.
*   enclosure information, virtual phy device id and port number.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*    July05  created
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_midplane_sas_conn_status_elements(
    fbe_u8_t *sas_conn_status_elements_start_ptr, 
    fbe_u8_t **sas_conn_status_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *info)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_sas_conn_struct *individual_sas_conn_stat_ptr = NULL;
    ses_stat_elem_sas_conn_struct *overall_sas_conn_stat_ptr = NULL;
    fbe_u8_t downstream_phys;
    fbe_sas_enclosure_type_t encl_type = info->enclosure_type;

    // for now all of fields are ignored in overall status element
    overall_sas_conn_stat_ptr = (ses_stat_elem_sas_conn_struct *)sas_conn_status_elements_start_ptr; 
    memset(overall_sas_conn_stat_ptr, 0, sizeof(ses_stat_elem_sas_conn_struct));
    overall_sas_conn_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_NOT_INSTALLED;

    // prepare individual midplane sas conn element
    individual_sas_conn_stat_ptr = overall_sas_conn_stat_ptr;  

    // for every downstream phys for drive, read from array device slot to see
    // if this drive is inserted
    sas_virtual_phy_max_drive_slots(encl_type, &downstream_phys, info->side);
    fbe_u8_t i;
    for (i = 0; i < downstream_phys; i++)
    {
        individual_sas_conn_stat_ptr++; 
        memset(individual_sas_conn_stat_ptr, 0, sizeof(ses_stat_elem_sas_conn_struct));
        individual_sas_conn_stat_ptr->cmn_stat.elem_stat_code = info->drive_slot_status[i].cmn_stat.elem_stat_code;
        individual_sas_conn_stat_ptr->conn_type = FBE_ESES_CONN_TYPE_SAS_DRIVE; 
        individual_sas_conn_stat_ptr->conn_physical_link = 0xFF;

    }

    // at last, set virtual phy 
    individual_sas_conn_stat_ptr++; 
    memset(individual_sas_conn_stat_ptr, 0, sizeof(ses_stat_elem_sas_conn_struct));
    individual_sas_conn_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK; 
    individual_sas_conn_stat_ptr->conn_type = FBE_ESES_CONN_TYPE_VIRTUAL_CONN; 
    individual_sas_conn_stat_ptr->conn_physical_link = 0xFF;

    *sas_conn_status_elements_end_ptr = (fbe_u8_t *)( individual_sas_conn_stat_ptr + 1);

    return(status);    
}

/*********************************************************************
*  enclosure_status_diagnostic_page_build_sas_exp_status_elements ()
*********************************************************************
*
*  Description: This builds the SAS expander elements for the encl 
*   status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of expander elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the expander elements.
*   enclosure information, virtual phy device id and port number.
*
*  Return Value: success or failure
*
*  Notes: 
*   There is only one sas exp element.
*
*  History:
*    Aug08  created
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_sas_exp_status_elements(
    fbe_u8_t *sas_exp_status_elements_start_ptr, 
    fbe_u8_t **sas_exp_status_elements_end_ptr)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_sas_exp_struct *individual_sas_exp_stat_ptr = NULL;
    ses_stat_elem_sas_exp_struct *overall_sas_exp_stat_ptr = NULL;
    
    overall_sas_exp_stat_ptr = (ses_stat_elem_sas_exp_struct *)sas_exp_status_elements_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_sas_exp_stat_ptr, 0, sizeof(ses_stat_elem_sas_exp_struct));
    individual_sas_exp_stat_ptr = (overall_sas_exp_stat_ptr + 1); 
    memset (individual_sas_exp_stat_ptr, 0, sizeof(ses_stat_elem_sas_exp_struct));
    individual_sas_exp_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

    *sas_exp_status_elements_end_ptr = (fbe_u8_t *)(individual_sas_exp_stat_ptr + 1);

    return(status);
}

// Get the power supply eses status ( complete element structure)
static fbe_status_t terminator_get_ps_eses_status(
    terminator_sas_virtual_phy_info_t *virtual_phy_handle,
    terminator_eses_ps_id ps_id,
    ses_stat_elem_ps_struct *ps_stat)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    fbe_u8_t max_ps_elems;

    status = sas_virtual_phy_max_ps_elems(virtual_phy_handle->enclosure_type, &max_ps_elems, virtual_phy_handle->side);
    RETURN_ON_ERROR_STATUS;
    /* Should be a supported/valid PS number */
    if (ps_id >= max_ps_elems)
    {
        status = FBE_STATUS_GENERIC_FAILURE;
        return status;
    }
    
    fbe_copy_memory(ps_stat, &virtual_phy_handle->ps_status[ps_id], sizeof(ses_stat_elem_ps_struct));

    return status;
}

// Get the cooling element status ( complete element structure)
static fbe_status_t terminator_get_cooling_eses_status(
    terminator_sas_virtual_phy_info_t *virtual_phy_handle,
    terminator_eses_cooling_id cooling_id,
    ses_stat_elem_cooling_struct *cooling_stat)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    fbe_u8_t max_cooling_elems;

    status = sas_virtual_phy_max_cooling_elems(virtual_phy_handle->enclosure_type, &max_cooling_elems, virtual_phy_handle->side);
    RETURN_ON_ERROR_STATUS;
    /* Should be a valid phy number */
    if (cooling_id >= max_cooling_elems)
    {
        status = FBE_STATUS_GENERIC_FAILURE;
        return status;
    }
    fbe_copy_memory(cooling_stat, &virtual_phy_handle->cooling_status[cooling_id], sizeof(ses_stat_elem_cooling_struct));

    return status;
}

static fbe_status_t terminator_get_display_eses_status(terminator_sas_virtual_phy_info_t *virtual_phy_handle,
                                               terminator_eses_display_character_id display_character_id,
                                               ses_stat_elem_display_struct *display_stat_elem)
{
    fbe_status_t status = FBE_STATUS_OK;
    fbe_u8_t max_display_characters;

    status = sas_virtual_phy_max_display_characters(virtual_phy_handle->enclosure_type, &max_display_characters, virtual_phy_handle->side);
    RETURN_ON_ERROR_STATUS;

    /* There is one display element for each display character */
    if (display_character_id >= max_display_characters)
    {
        status = FBE_STATUS_GENERIC_FAILURE;
        return status;
    }
    // There is one display status element for each display character.
    fbe_copy_memory(display_stat_elem, &virtual_phy_handle->display_status[display_character_id], sizeof(ses_stat_elem_display_struct));
    return(FBE_STATUS_OK);
}

// Get the overall temp_sensor element status ( complete element structure)
static fbe_status_t terminator_get_overall_temp_sensor_eses_status(
    terminator_sas_virtual_phy_info_t *virtual_phy_handle,
    terminator_eses_temp_sensor_id temp_sensor_id,
    ses_stat_elem_temp_sensor_struct *temp_sensor_stat)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    fbe_u8_t max_temp_sensor_elems;
    fbe_sas_enclosure_type_t encl_type;

    encl_type = virtual_phy_handle->enclosure_type;
    status = sas_virtual_phy_max_temp_sensor_elems(encl_type, &max_temp_sensor_elems, virtual_phy_handle->side);
    RETURN_ON_ERROR_STATUS;

    /* Should be a valid phy number */
    if (temp_sensor_id >= max_temp_sensor_elems)
    {
        status = FBE_STATUS_GENERIC_FAILURE;
        return status;
    }
    fbe_copy_memory(temp_sensor_stat, &virtual_phy_handle->overall_temp_sensor_status[temp_sensor_id],
        sizeof(ses_stat_elem_temp_sensor_struct));

    status = FBE_STATUS_OK;
    return(status);
}

// Get the temp_sensor element status ( complete element structure)
static fbe_status_t terminator_get_temp_sensor_eses_status(
    terminator_sas_virtual_phy_info_t *virtual_phy_handle,
    terminator_eses_temp_sensor_id temp_sensor_id,
    ses_stat_elem_temp_sensor_struct *temp_sensor_stat)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    fbe_u8_t max_temp_sensor_elems;
    fbe_sas_enclosure_type_t encl_type;

    encl_type = virtual_phy_handle->enclosure_type;

    status = sas_virtual_phy_max_temp_sensor_elems(encl_type, &max_temp_sensor_elems, virtual_phy_handle->side);
    RETURN_ON_ERROR_STATUS;

    /* Should be a valid phy number */
    if (temp_sensor_id >= max_temp_sensor_elems)
    {
        status = FBE_STATUS_GENERIC_FAILURE;
        return status;
    }
    fbe_copy_memory(temp_sensor_stat, &virtual_phy_handle->temp_sensor_status[temp_sensor_id],
        sizeof(ses_stat_elem_temp_sensor_struct));

    status = FBE_STATUS_OK;
    return(status);
}

/* Get the Chassis(Enclosure element in Chassis Subenclosure) eses status
 */
static fbe_status_t terminator_get_chassis_encl_eses_status(terminator_sas_virtual_phy_info_t *virtual_phy_handle,
                                                     ses_stat_elem_encl_struct *encl_stat)
{
    fbe_status_t status = FBE_STATUS_OK;

    fbe_copy_memory(encl_stat, &virtual_phy_handle->chassis_encl_status, sizeof(ses_stat_elem_encl_struct));

    return status;
}

/* Get the encl(Enclosure element in Local LCC Subenclosure) eses status
 */
static fbe_status_t terminator_get_encl_eses_status(terminator_sas_virtual_phy_info_t *virtual_phy_handle,
                                             ses_stat_elem_encl_struct *encl_stat)
{
    fbe_status_t status = FBE_STATUS_OK;

    fbe_copy_memory(encl_stat, &virtual_phy_handle->encl_status, sizeof(ses_stat_elem_encl_struct));
    return status;
}

static fbe_status_t enclosure_status_diagnostic_page_build_psa0_status_elements(
    fbe_u8_t *sas_ps_elements_start_ptr, 
    fbe_u8_t **sas_ps_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_ps_struct *individual_ps_stat_ptr = NULL;
    ses_stat_elem_ps_struct *overall_ps_stat_ptr = NULL;
    fbe_sas_enclosure_type_t  encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_eses_ps_id     ps_id;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(virtual_phy_handle, &encl_type);
    
    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            ps_id = PS_2;
            break;
        default:
            return FBE_STATUS_GENERIC_FAILURE;
    }

    overall_ps_stat_ptr = (ses_stat_elem_ps_struct *)sas_ps_elements_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, overall_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_ps_stat_ptr = (overall_ps_stat_ptr + 1); 
    memset (individual_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, individual_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_ps_stat_ptr = (overall_ps_stat_ptr + 2); 
    memset (individual_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, individual_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    *sas_ps_elements_end_ptr = (fbe_u8_t *)(individual_ps_stat_ptr + 1);

    return(status);    
}

static fbe_status_t 
enclosure_status_diagnostic_page_build_psa1_status_elements(
    fbe_u8_t *sas_ps_elements_start_ptr, 
    fbe_u8_t **sas_ps_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_ps_struct *individual_ps_stat_ptr = NULL;
    ses_stat_elem_ps_struct *overall_ps_stat_ptr = NULL;
    fbe_sas_enclosure_type_t  encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_eses_ps_id     ps_id;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(virtual_phy_handle, &encl_type);

    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            ps_id = PS_0;
            break;
        default:
            return FBE_STATUS_GENERIC_FAILURE;
    }

    overall_ps_stat_ptr = (ses_stat_elem_ps_struct *)sas_ps_elements_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, overall_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_ps_stat_ptr = (overall_ps_stat_ptr + 1); 
    memset (individual_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, individual_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_ps_stat_ptr = (overall_ps_stat_ptr + 2); 
    memset (individual_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, individual_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    *sas_ps_elements_end_ptr = (fbe_u8_t *)(individual_ps_stat_ptr + 1);

    return(status);    
}

static fbe_status_t 
enclosure_status_diagnostic_page_build_psb0_status_elements(
    fbe_u8_t *sas_ps_elements_start_ptr, 
    fbe_u8_t **sas_ps_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_ps_struct *individual_ps_stat_ptr = NULL;
    ses_stat_elem_ps_struct *overall_ps_stat_ptr = NULL;
    fbe_sas_enclosure_type_t  encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_eses_ps_id     ps_id;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(virtual_phy_handle, &encl_type);
    
    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            ps_id = PS_3;
            break;
        default:
            return FBE_STATUS_GENERIC_FAILURE;
    }

    overall_ps_stat_ptr = (ses_stat_elem_ps_struct *)sas_ps_elements_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, overall_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_ps_stat_ptr = (overall_ps_stat_ptr + 1); 
    memset (individual_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, individual_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_ps_stat_ptr = (overall_ps_stat_ptr + 2); 
    memset (individual_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, individual_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    *sas_ps_elements_end_ptr = (fbe_u8_t *)(individual_ps_stat_ptr + 1);

    return(status);    
}

static fbe_status_t 
enclosure_status_diagnostic_page_build_psb1_status_elements(
    fbe_u8_t *sas_ps_elements_start_ptr, 
    fbe_u8_t **sas_ps_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_ps_struct *individual_ps_stat_ptr = NULL;
    ses_stat_elem_ps_struct *overall_ps_stat_ptr = NULL;
    fbe_sas_enclosure_type_t  encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_eses_ps_id     ps_id;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(virtual_phy_handle, &encl_type);
    
    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            ps_id = PS_1;
            break;
        default:
            return FBE_STATUS_GENERIC_FAILURE;
    }

    overall_ps_stat_ptr = (ses_stat_elem_ps_struct *)sas_ps_elements_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, overall_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_ps_stat_ptr = (overall_ps_stat_ptr + 1); 
    memset (individual_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, individual_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_ps_stat_ptr = (overall_ps_stat_ptr + 2); 
    memset (individual_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, ps_id, individual_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    *sas_ps_elements_end_ptr = (fbe_u8_t *)(individual_ps_stat_ptr + 1);

    return(status);    
}

/*********************************************************************
* enclosure_status_diagnostic_page_build_psa_status_elements ()
*********************************************************************
*
*  Description: This builds the Power Supply(A) status 
*   elements in the encl status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of Power Supply elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the Power Supply elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*   Oct-??-2008 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t 
enclosure_status_diagnostic_page_build_psa_status_elements(
    fbe_u8_t *sas_ps_elements_start_ptr, 
    fbe_u8_t **sas_ps_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_ps_struct *individual_ps_stat_ptr = NULL;
    ses_stat_elem_ps_struct *overall_ps_stat_ptr = NULL;

    overall_ps_stat_ptr = (ses_stat_elem_ps_struct *)sas_ps_elements_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, PS_0, overall_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_ps_stat_ptr = (overall_ps_stat_ptr + 1); 
    memset (individual_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, PS_0, individual_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;
    *sas_ps_elements_end_ptr = (fbe_u8_t *)(individual_ps_stat_ptr + 1);

    return(status);    
}

/*********************************************************************
* enclosure_status_diagnostic_page_build_psb_status_elements ()
*********************************************************************
*
*  Description: This builds the Power Supply(B) status 
*   elements in the encl status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of Power Supply elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the Power Supply elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*   Oct-??-2008 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_psb_status_elements(
    fbe_u8_t *sas_ps_elements_start_ptr, 
    fbe_u8_t **sas_ps_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_ps_struct *individual_ps_stat_ptr = NULL;
    ses_stat_elem_ps_struct *overall_ps_stat_ptr = NULL;
    
    overall_ps_stat_ptr = (ses_stat_elem_ps_struct *)sas_ps_elements_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, PS_1, overall_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;


    individual_ps_stat_ptr = (overall_ps_stat_ptr + 1); 
    memset (individual_ps_stat_ptr, 0, sizeof(ses_stat_elem_ps_struct));
    status = terminator_get_ps_eses_status(virtual_phy_handle, PS_1, individual_ps_stat_ptr);
    RETURN_ON_ERROR_STATUS;
    *sas_ps_elements_end_ptr = (fbe_u8_t *)(individual_ps_stat_ptr + 1);

    return(status);    
}

/*********************************************************************
* enclosure_status_diagnostic_page_build_psa_cooling_status_elements ()
*********************************************************************
*
*  Description: This builds the Power Supply(A) Cooling status 
*   elements in the encl status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of Cooling elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the cooling elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*   Oct-??-2008 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_psa_cooling_status_elements(
    fbe_u8_t *sas_cooling_elements_start_ptr, 
    fbe_u8_t **sas_cooling_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_cooling_struct *individual_cooling_stat_ptr = NULL;
    ses_stat_elem_cooling_struct *overall_cooling_stat_ptr = NULL;
    
    overall_cooling_stat_ptr = (ses_stat_elem_cooling_struct *)sas_cooling_elements_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_cooling_stat_ptr, 0, sizeof(ses_stat_elem_cooling_struct));

    individual_cooling_stat_ptr = (overall_cooling_stat_ptr + 1); 
    memset (individual_cooling_stat_ptr, 0, sizeof(ses_stat_elem_cooling_struct));
    status = terminator_get_cooling_eses_status(virtual_phy_handle, COOLING_0, individual_cooling_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_cooling_stat_ptr = (overall_cooling_stat_ptr + 2); 
    memset (individual_cooling_stat_ptr, 0, sizeof(ses_stat_elem_cooling_struct));
    status = terminator_get_cooling_eses_status(virtual_phy_handle, COOLING_1, individual_cooling_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    *sas_cooling_elements_end_ptr = (fbe_u8_t *)(individual_cooling_stat_ptr + 1);
    return(status);    
}

/*********************************************************************
* enclosure_status_diagnostic_page_build_psb_cooling_status_elements ()
*********************************************************************
*
*  Description: This builds the Power Supply(B) Cooling status 
*   elements in the encl status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of cooling elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the cooling elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*   Oct-??-2008 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t 
enclosure_status_diagnostic_page_build_psb_cooling_status_elements(
    fbe_u8_t *sas_cooling_elements_start_ptr, 
    fbe_u8_t **sas_cooling_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_cooling_struct *individual_cooling_stat_ptr = NULL;
    ses_stat_elem_cooling_struct *overall_cooling_stat_ptr = NULL;
    
    overall_cooling_stat_ptr = (ses_stat_elem_cooling_struct *)sas_cooling_elements_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_cooling_stat_ptr, 0, sizeof(ses_stat_elem_cooling_struct));

    individual_cooling_stat_ptr = (overall_cooling_stat_ptr + 1); 
    memset (individual_cooling_stat_ptr, 0, sizeof(ses_stat_elem_cooling_struct));
    status = terminator_get_cooling_eses_status(virtual_phy_handle, COOLING_2, individual_cooling_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_cooling_stat_ptr = (overall_cooling_stat_ptr + 2); 
    memset (individual_cooling_stat_ptr, 0, sizeof(ses_stat_elem_cooling_struct));
    status = terminator_get_cooling_eses_status(virtual_phy_handle, COOLING_3, individual_cooling_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    *sas_cooling_elements_end_ptr = (fbe_u8_t *)(individual_cooling_stat_ptr + 1);
    return(status);    
}

/*********************************************************************
* enclosure_status_diagnostic_page_build_local_temp_sensor_status_elements ()
*********************************************************************
*
*  Description: This builds the Local Temperature sensor status 
*   elements in the encl status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of temp sensor elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the temp sensor elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*   Oct-??-2008 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t 
enclosure_status_diagnostic_page_build_local_temp_sensor_status_elements(
    fbe_u8_t *sas_temp_sensor_elements_start_ptr, 
    fbe_u8_t **sas_temp_sensor_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_temp_sensor_struct *individual_temp_sensor_stat_ptr = NULL;
    ses_stat_elem_temp_sensor_struct *overall_temp_sensor_stat_ptr = NULL;
    fbe_u8_t max_temp_sensor_elems_per_lcc = 0;
    fbe_sas_enclosure_type_t            encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    fbe_u8_t i;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(virtual_phy_handle, &encl_type);
    
    overall_temp_sensor_stat_ptr = (ses_stat_elem_temp_sensor_struct *)sas_temp_sensor_elements_start_ptr; 
    memset (overall_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
    status = terminator_get_overall_temp_sensor_eses_status(virtual_phy_handle, TEMP_SENSOR_0, overall_temp_sensor_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_temp_sensor_stat_ptr = overall_temp_sensor_stat_ptr;  
    status = sas_virtual_phy_max_temp_sensor_elems(encl_type, &max_temp_sensor_elems_per_lcc, virtual_phy_handle->side);
    RETURN_ON_ERROR_STATUS;

    for(i=0; i<max_temp_sensor_elems_per_lcc; i++)
    {
        individual_temp_sensor_stat_ptr++; 
        memset(individual_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_sas_conn_struct));
        status = terminator_get_temp_sensor_eses_status(virtual_phy_handle, TEMP_SENSOR_0, individual_temp_sensor_stat_ptr);
        RETURN_ON_ERROR_STATUS;
    }

    *sas_temp_sensor_elements_end_ptr = (fbe_u8_t *)(individual_temp_sensor_stat_ptr + 1);
    return(status);    
}

/*********************************************************************
* enclosure_status_diagnostic_page_build_peer_temp_sensor_status_elements ()
*********************************************************************
*
*  Description: This builds the Peer Temperature sensor status 
*   elements in the encl status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of temp sensor elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the temp sensor elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*   Oct-??-2008 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t 
enclosure_status_diagnostic_page_build_peer_temp_sensor_status_elements(
    fbe_u8_t *sas_temp_sensor_elements_start_ptr, 
    fbe_u8_t **sas_temp_sensor_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_temp_sensor_struct *individual_temp_sensor_stat_ptr = NULL;
    ses_stat_elem_temp_sensor_struct *overall_temp_sensor_stat_ptr = NULL;
    fbe_u8_t max_temp_sensor_elems_per_lcc = 0;
    fbe_sas_enclosure_type_t            encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    fbe_u8_t i;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(virtual_phy_handle, &encl_type);
    
    overall_temp_sensor_stat_ptr = (ses_stat_elem_temp_sensor_struct *)sas_temp_sensor_elements_start_ptr; 
    memset (overall_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
    // As peer LCC(containing subenclosure for temp sensor) is assumed NOT INSTALLED
    overall_temp_sensor_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_NOT_INSTALLED;

    individual_temp_sensor_stat_ptr = overall_temp_sensor_stat_ptr;  
    status = sas_virtual_phy_max_temp_sensor_elems(encl_type, &max_temp_sensor_elems_per_lcc, virtual_phy_handle->side);
    RETURN_ON_ERROR_STATUS;

    for(i=0; i<max_temp_sensor_elems_per_lcc; i++)
    {
        individual_temp_sensor_stat_ptr++; 
        memset (individual_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
        // As peer LCC(containing subenclosure for temp sensor) is assumed NOT INSTALLED 
        individual_temp_sensor_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_NOT_INSTALLED;
        individual_temp_sensor_stat_ptr->ot_failure = 0;
        individual_temp_sensor_stat_ptr->ot_warning = 0;
    }

    *sas_temp_sensor_elements_end_ptr = (fbe_u8_t *)(individual_temp_sensor_stat_ptr + 1);
    return(status);    
}

#if 0
/*********************************************************************
* enclosure_status_diagnostic_page_build_local_ps_temp_sensor_status_elements ()
*********************************************************************
*
*  Description: This builds the Local Power Supply Temperature sensor status 
*   elements in the encl status diagnostic page. (Power Supply Subenclosure)
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of temp sensor elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the temp sensor elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*   Dec-6-2010 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t 
enclosure_status_diagnostic_page_build_local_ps_temp_sensor_status_elements(
    fbe_u8_t *sas_temp_sensor_elements_start_ptr, 
    fbe_u8_t **sas_temp_sensor_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_temp_sensor_struct *individual_temp_sensor_stat_ptr = NULL;
    ses_stat_elem_temp_sensor_struct *overall_temp_sensor_stat_ptr = NULL;
    
    overall_temp_sensor_stat_ptr = (ses_stat_elem_temp_sensor_struct *)sas_temp_sensor_elements_start_ptr; 
    memset (overall_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
    overall_temp_sensor_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

    individual_temp_sensor_stat_ptr = (overall_temp_sensor_stat_ptr + 1); 
    memset (individual_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
    individual_temp_sensor_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

    individual_temp_sensor_stat_ptr ++;
    memset (individual_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
    individual_temp_sensor_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

    *sas_temp_sensor_elements_end_ptr = (fbe_u8_t *)(individual_temp_sensor_stat_ptr + 1);
    return(status);    
}

/*********************************************************************
* enclosure_status_diagnostic_page_build_peer_ps_temp_sensor_status_elements ()
*********************************************************************
*
*  Description: This builds the Peer Power Supply Temperature sensor status 
*   elements in the encl status diagnostic page. (Power Supply Subenclosure)
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of temp sensor elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the temp sensor elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*   Dec-6-2010 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t 
enclosure_status_diagnostic_page_build_peer_ps_temp_sensor_status_elements(
    fbe_u8_t *sas_temp_sensor_elements_start_ptr, 
    fbe_u8_t **sas_temp_sensor_elements_end_ptr)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_temp_sensor_struct *individual_temp_sensor_stat_ptr = NULL;
    ses_stat_elem_temp_sensor_struct *overall_temp_sensor_stat_ptr = NULL;
    
    overall_temp_sensor_stat_ptr = (ses_stat_elem_temp_sensor_struct *)sas_temp_sensor_elements_start_ptr; 
    memset (overall_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
    overall_temp_sensor_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

    individual_temp_sensor_stat_ptr = (overall_temp_sensor_stat_ptr + 1); 
    memset (individual_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
    individual_temp_sensor_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

    individual_temp_sensor_stat_ptr ++;
    memset (individual_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
    individual_temp_sensor_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

    *sas_temp_sensor_elements_end_ptr = (fbe_u8_t *)(individual_temp_sensor_stat_ptr + 1);
    return(status);    
}

#endif

/*********************************************************************
*  enclosure_status_diagnostic_page_build_chassis_encl_status_elements ()
*********************************************************************
*
*  Description: This builds the Chassis encl status elements in the encl 
*   status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of encl elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the encl elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*   There is only one encl status element.
*
*  History:
*   Oct-31-2008 -- Rajesh V Created.
*    
*********************************************************************/

static fbe_status_t enclosure_status_diagnostic_page_build_chassis_encl_status_elements(
    fbe_u8_t *encl_stat_elem_start_ptr, 
    fbe_u8_t **encl_stat_elem_end_ptr,
     terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_encl_struct *individual_encl_stat_ptr = NULL;
    ses_stat_elem_encl_struct *overall_encl_stat_ptr = NULL;
    
    overall_encl_stat_ptr = (ses_stat_elem_encl_struct *)encl_stat_elem_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_encl_stat_ptr, 0, sizeof(ses_stat_elem_encl_struct));

    individual_encl_stat_ptr = (overall_encl_stat_ptr + 1); 
    status = terminator_get_chassis_encl_eses_status(virtual_phy_handle, individual_encl_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    printf("%s:Raj:Stat page Chassis STAT elem, failure_rqsted set to %d \n",
                        __FUNCTION__, individual_encl_stat_ptr->failure_rqsted);

    *encl_stat_elem_end_ptr = (fbe_u8_t *)(individual_encl_stat_ptr + 1);
    return(status);    
}

/*********************************************************************
*  enclosure_status_diagnostic_page_build_local_encl_status_elements ()
*********************************************************************
*
*  Description: This builds the local LCC's encl status elements in 
*   Encl status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of encl elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the encl elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*   There is only one encl status element.
*
*  History:
*   Oct-31-2008 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_local_encl_status_elements(
    fbe_u8_t *encl_stat_elem_start_ptr, 
    fbe_u8_t **encl_stat_elem_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_encl_struct *individual_encl_stat_ptr = NULL;
    ses_stat_elem_encl_struct *overall_encl_stat_ptr = NULL;
    
    overall_encl_stat_ptr = (ses_stat_elem_encl_struct *)encl_stat_elem_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_encl_stat_ptr, 0, sizeof(ses_stat_elem_encl_struct));

    individual_encl_stat_ptr = (overall_encl_stat_ptr + 1); 
    status = terminator_get_encl_eses_status(virtual_phy_handle, individual_encl_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    *encl_stat_elem_end_ptr = (fbe_u8_t *)(individual_encl_stat_ptr + 1);
    return(status);    
}

/*********************************************************************
*  enclosure_status_diagnostic_page_build_peer_encl_status_elements ()
*********************************************************************
*
*  Description: This builds the Peer LCC's encl status elements in 
*   Encl status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of encl elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the encl elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*   There is only one encl status element.
*
*  History:
*   Oct-31-2008 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_peer_encl_status_elements(
    fbe_u8_t *encl_stat_elem_start_ptr, 
    fbe_u8_t **encl_stat_elem_end_ptr)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_encl_struct *individual_encl_stat_ptr = NULL;
    ses_stat_elem_encl_struct *overall_encl_stat_ptr = NULL;
    
    overall_encl_stat_ptr = (ses_stat_elem_encl_struct *)encl_stat_elem_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_encl_stat_ptr, 0, sizeof(ses_stat_elem_encl_struct));

    individual_encl_stat_ptr = (overall_encl_stat_ptr + 1); 
    memset (individual_encl_stat_ptr, 0, sizeof(ses_stat_elem_encl_struct));
    individual_encl_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

    *encl_stat_elem_end_ptr = (fbe_u8_t *)(individual_encl_stat_ptr + 1);
    return(status);    
}

/*********************************************************************
*  enclosure_status_diagnostic_page_build_ee_encl_status_elements ()
*********************************************************************
*
*   Description: This builds the EE LCC's encl status elements in 
*                Encl status diagnostic page. 
*
*   Inputs: 
*   status_elements_start_ptr - pointer to the start of encl elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*                             end address of the encl elements.
*   Virtual phy handle.
*
*   Return Value: success or failure
*
*   Notes: 
*   
*
*   History:
*   Jun-03-2011 -- Rashmi Sawale Created.
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_ee_encl_status_elements(fbe_u8_t *encl_stat_elem_start_ptr, 
                                                                            fbe_u8_t **encl_stat_elem_end_ptr)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_encl_struct *individual_encl_stat_ptr = NULL;
    ses_stat_elem_encl_struct *overall_encl_stat_ptr = NULL;
    
    overall_encl_stat_ptr = (ses_stat_elem_encl_struct *)encl_stat_elem_start_ptr; 
    memset (overall_encl_stat_ptr, 0, sizeof(ses_stat_elem_encl_struct));
    overall_encl_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
        
    individual_encl_stat_ptr = (overall_encl_stat_ptr + 1); 
    memset (individual_encl_stat_ptr, 0, sizeof(ses_stat_elem_encl_struct));
    individual_encl_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

    *encl_stat_elem_end_ptr = (fbe_u8_t *)(individual_encl_stat_ptr + 1);
    return(status);    
}

/*********************************************************************
*  enclosure_status_diagnostic_page_build_ext_cooling_status_elements ()
*********************************************************************
*
*   Description: This builds the ext Cooling status elements in 
*                Encl status diagnostic page. 
*
*   Inputs: 
*   status_elements_start_ptr - pointer to the start of encl elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*                             end address of the encl elements.
*   Virtual phy handle.
*
*   Return Value: success or failure
*
*   Notes: 
*   
*
*   History:
*   Jun-03-2011 -- Rashmi Sawale Created.
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_ext_cooling_status_elements(
                                                fbe_u8_t *encl_stat_elem_start_ptr, 
                                                fbe_u8_t **encl_stat_elem_end_ptr,
                                                terminator_sas_virtual_phy_info_t *virtual_phy_handle,
                                                fbe_u8_t  fanSlot)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_cooling_struct *individual_ext_cooling_stat_ptr = NULL;
    ses_stat_elem_cooling_struct *overall_ext_cooling_stat_ptr = NULL;
    
    overall_ext_cooling_stat_ptr = (ses_stat_elem_cooling_struct *)encl_stat_elem_start_ptr; 
    memset (overall_ext_cooling_stat_ptr, 0, sizeof(ses_stat_elem_cooling_struct));
    status = terminator_get_cooling_eses_status(virtual_phy_handle, fanSlot+COOLING_4, overall_ext_cooling_stat_ptr);
    RETURN_ON_ERROR_STATUS;
   
    individual_ext_cooling_stat_ptr = (overall_ext_cooling_stat_ptr + 1); 
    memset (individual_ext_cooling_stat_ptr, 0, sizeof(ses_stat_elem_encl_struct));
    status = terminator_get_cooling_eses_status(virtual_phy_handle, fanSlot+COOLING_4, individual_ext_cooling_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    *encl_stat_elem_end_ptr = (fbe_u8_t *)(individual_ext_cooling_stat_ptr + 1);
    return(status);
}

/*********************************************************************
*  enclosure_status_diagnostic_page_build_bem_cooling_status_elements ()
*********************************************************************
*
*   Description: This builds the bem Cooling status elements in 
*                Encl status diagnostic page. 
*
*   Inputs: 
*   status_elements_start_ptr - pointer to the start of encl elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*                             end address of the encl elements.
*   Virtual phy handle.
*
*   Return Value: success or failure
*
*   Notes: 
*   
*
*   History:
*   18-07-2012 -- Rui Chang Created.
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_bem_cooling_status_elements(
                                                fbe_u8_t *encl_stat_elem_start_ptr, 
                                                fbe_u8_t **encl_stat_elem_end_ptr,
                                                terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{

 

    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_cooling_struct *individual_bem_cooling_stat_ptr = NULL;
    ses_stat_elem_cooling_struct *overall_bem_cooling_stat_ptr = NULL;
    fbe_u8_t slot_id=0;
    fbe_u8_t max_bem_cooling_elems_count=0;
    fbe_sas_enclosure_type_t            encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(virtual_phy_handle, &encl_type);

    status = sas_virtual_phy_max_bem_cooling_elems(encl_type, &max_bem_cooling_elems_count, virtual_phy_handle->side);
    RETURN_ON_ERROR_STATUS;

    overall_bem_cooling_stat_ptr = (ses_stat_elem_cooling_struct *)encl_stat_elem_start_ptr; 
    memset (overall_bem_cooling_stat_ptr, 0, sizeof(ses_stat_elem_cooling_struct));
    overall_bem_cooling_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK; 

    individual_bem_cooling_stat_ptr = overall_bem_cooling_stat_ptr;  

    for(slot_id = 0; slot_id < max_bem_cooling_elems_count; slot_id++) 
    {
        individual_bem_cooling_stat_ptr ++; 
        memset (individual_bem_cooling_stat_ptr, 0, sizeof(ses_stat_elem_encl_struct));
        status = terminator_get_cooling_eses_status(virtual_phy_handle, slot_id, individual_bem_cooling_stat_ptr);
        RETURN_ON_ERROR_STATUS;
    }

    *encl_stat_elem_end_ptr = (fbe_u8_t *)(individual_bem_cooling_stat_ptr + 1);
    return(status);
}

/*********************************************************************
*  enclosure_status_diagnostic_page_build_bem_cooling_status_elements ()
*********************************************************************
*
*   Description: This builds the chassis cooling status elements in 
*                Encl status diagnostic page. 
*
*   Inputs: 
*   status_elements_start_ptr - pointer to the start of encl elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*                             end address of the encl elements.
*   Virtual phy handle.
*
*   Return Value: success or failure
*
*   Notes: 
*   
*
*   History:
*   25-08-2012 -- Rui Chang Created.
*...12/13/2013 -- zhoue1 Add Viking support
*    
*********************************************************************/
static fbe_status_t enclosure_status_diagnostic_page_build_chassis_cooling_status_elements(
                                                fbe_u8_t *encl_stat_elem_start_ptr, 
                                                fbe_u8_t **encl_stat_elem_end_ptr,
                                                terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{

 

    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_cooling_struct individual_bem_cooling_stat;
    ses_stat_elem_cooling_struct *individual_chassis_cooling_stat_ptr = NULL;
    ses_stat_elem_cooling_struct *overall_chassis_cooling_stat_ptr = NULL;
    fbe_u8_t slot_id=0;
    fbe_u8_t max_bem_cooling_elems_count=0;
    fbe_u8_t max_chassis_cooling_elems_count=0;
    fbe_sas_enclosure_type_t            encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    fbe_u8_t  faultBemFanCount=0;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(virtual_phy_handle, &encl_type);

    overall_chassis_cooling_stat_ptr = (ses_stat_elem_cooling_struct *)encl_stat_elem_start_ptr; 
    memset (overall_chassis_cooling_stat_ptr, 0, sizeof(ses_stat_elem_cooling_struct));
            
    /* Jetfire chassis cooling status depend on BEM fan status*/
// ENCL_CLEANUP - needed for Moons?
    if ((encl_type == FBE_SAS_ENCLOSURE_TYPE_FALLBACK) ||
        (encl_type == FBE_SAS_ENCLOSURE_TYPE_CALYPSO))
    {
        status = sas_virtual_phy_max_bem_cooling_elems(encl_type, &max_bem_cooling_elems_count, virtual_phy_handle->side);
        RETURN_ON_ERROR_STATUS;
    
        for(slot_id = 0; slot_id < max_bem_cooling_elems_count; slot_id++) 
        {
            status = terminator_get_cooling_eses_status(virtual_phy_handle, slot_id, &individual_bem_cooling_stat);
            RETURN_ON_ERROR_STATUS;
            if (individual_bem_cooling_stat.cmn_stat.elem_stat_code == SES_STAT_CODE_CRITICAL)
            {
                faultBemFanCount++;
            }
        }
    
        if (faultBemFanCount < 1)
        {
            overall_chassis_cooling_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK; 
        }
        else if (faultBemFanCount == 1)
        {
            overall_chassis_cooling_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_NONCRITICAL; 
        }
        else
        {
            overall_chassis_cooling_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_CRITICAL; 
        }

    }
    else if ((encl_type == FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP) ||
             (encl_type == FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP) ||
             (encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP) ||
             (encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP))
    {
        status = sas_virtual_phy_max_ext_cooling_elems(encl_type, &max_chassis_cooling_elems_count, virtual_phy_handle->side);
        RETURN_ON_ERROR_STATUS;
        
        individual_chassis_cooling_stat_ptr = overall_chassis_cooling_stat_ptr;
        status = terminator_get_cooling_eses_status(virtual_phy_handle, slot_id, individual_chassis_cooling_stat_ptr);
        RETURN_ON_ERROR_STATUS;

        for(slot_id = 0; slot_id < max_chassis_cooling_elems_count; slot_id++) 
        {
            individual_chassis_cooling_stat_ptr++;
            memset(individual_chassis_cooling_stat_ptr, 0, sizeof(ses_stat_elem_sas_conn_struct));
            status = terminator_get_cooling_eses_status(virtual_phy_handle, slot_id, individual_chassis_cooling_stat_ptr);
            RETURN_ON_ERROR_STATUS;
        }
    }
    else
    {
            overall_chassis_cooling_stat_ptr->cmn_stat.elem_stat_code = SES_STAT_CODE_OK; 
    }

    *encl_stat_elem_end_ptr = (fbe_u8_t *)(overall_chassis_cooling_stat_ptr + 1);
    return(status);

}

/*********************************************************************
* enclosure_status_diagnostic_page_build_chassis_temp_sensor_status_elements ()
*********************************************************************
*
*  Description: This builds the Chassis Temperature sensor status 
*   elements in the encl status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of temp sensor elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of the temp sensor elements.
*   Virtual phy handle.
*
*  Return Value: success or failure
*
*  Notes: 
*
*  History:
*   Dec-20-2009 -- Rajesh V Created.
*    
*********************************************************************/
static fbe_status_t 
enclosure_status_diagnostic_page_build_chassis_temp_sensor_status_elements(
    fbe_u8_t *sas_temp_sensor_elements_start_ptr, 
    fbe_u8_t **sas_temp_sensor_elements_end_ptr,
    terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_OK;
    ses_stat_elem_temp_sensor_struct *individual_temp_sensor_stat_ptr = NULL;
    ses_stat_elem_temp_sensor_struct *overall_temp_sensor_stat_ptr = NULL;
    
    overall_temp_sensor_stat_ptr = 
        (ses_stat_elem_temp_sensor_struct *)sas_temp_sensor_elements_start_ptr; 
    memset (overall_temp_sensor_stat_ptr, 
            0, 
            sizeof(ses_stat_elem_temp_sensor_struct));
    overall_temp_sensor_stat_ptr->cmn_stat.elem_stat_code = 
        SES_STAT_CODE_OK;


    individual_temp_sensor_stat_ptr = (overall_temp_sensor_stat_ptr + 1); 
    memset (individual_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
    individual_temp_sensor_stat_ptr->cmn_stat.elem_stat_code = 
        SES_STAT_CODE_OK;

    /* 
    // Temperature sensor for air inlet temperature.;
    individual_temp_sensor_stat_ptr = (overall_temp_sensor_stat_ptr + 1); 
    memset (individual_temp_sensor_stat_ptr, 0, sizeof(ses_stat_elem_temp_sensor_struct));
    status = terminator_get_temp_sensor_eses_status(virtual_phy_handle,                                                                                                          ;
                                                    TEMP_SENSOR_2,                                                                                                               ;
                                                    individual_temp_sensor_stat_ptr);
    RETURN_ON_ERROR_STATUS; 
    */ 

    *sas_temp_sensor_elements_end_ptr = 
        (fbe_u8_t *)(overall_temp_sensor_stat_ptr + 1);
    return(status);    
}



static fbe_status_t enclosure_status_diagnostic_page_build_display0_status_elements(fbe_u8_t *display_stat_elem_start_ptr, 
                                                                            fbe_u8_t **display_stat_elem_end_ptr,
                                                                            terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_stat_elem_display_struct *individual_display_stat_ptr = NULL;
    ses_stat_elem_display_struct *overall_display_stat_ptr = NULL;
    
    overall_display_stat_ptr = (ses_stat_elem_display_struct *)display_stat_elem_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_display_stat_ptr, 0, sizeof(ses_stat_elem_display_struct));

    // There is a display status element for each display character (One-One mapping)
    individual_display_stat_ptr = (overall_display_stat_ptr + 1); 
    status = terminator_get_display_eses_status(virtual_phy_handle, DISPLAY_CHARACTER_0, individual_display_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    individual_display_stat_ptr ++;
    status = terminator_get_display_eses_status(virtual_phy_handle, DISPLAY_CHARACTER_1, individual_display_stat_ptr);
    RETURN_ON_ERROR_STATUS;


    *display_stat_elem_end_ptr = (fbe_u8_t *)(individual_display_stat_ptr + 1);
    return(FBE_STATUS_OK);    
}

static fbe_status_t enclosure_status_diagnostic_page_build_display1_status_elements(fbe_u8_t *display_stat_elem_start_ptr, 
                                                                            fbe_u8_t **display_stat_elem_end_ptr,
                                                                            terminator_sas_virtual_phy_info_t *virtual_phy_handle)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_stat_elem_display_struct *individual_display_stat_ptr = NULL;
    ses_stat_elem_display_struct *overall_display_stat_ptr = NULL;
    
    overall_display_stat_ptr = (ses_stat_elem_display_struct *)display_stat_elem_start_ptr; 
    // for now all of fields are ignored in overall status element
    memset (overall_display_stat_ptr, 0, sizeof(ses_stat_elem_display_struct));

    // There is a display status element for each display character (One-One mapping)
    individual_display_stat_ptr = (overall_display_stat_ptr + 1); 
    status = terminator_get_display_eses_status(virtual_phy_handle, DISPLAY_CHARACTER_2, individual_display_stat_ptr);
    RETURN_ON_ERROR_STATUS;

    *display_stat_elem_end_ptr = (fbe_u8_t *)(individual_display_stat_ptr + 1);
    return(FBE_STATUS_OK);    
}


/*********************************************************************
*  get_start_elem_offset_by_config_page_info ()
**********************************************************************
*
*  Description:
*   This function returns the offset of the given status element type
*    in Encl stat page, WRT to the given configuration page.
*
*  Inputs:
*   config_page_info - pointer to configuration page to use.
*   subencl_type - subenclosure type
*   side - side (A or B indicated by 0 & 1 now)
*   elem_type - Status element type.
*   offset - pointer to offset to be returned
*
*  Return Value: success or failure
*
*  Notes:
*
*  History:
*    Sep08  Rajesh V created
*
*********************************************************************/
static fbe_status_t get_start_elem_offset_by_config_page_info(terminator_eses_config_page_info_t *config_page_info,
                                                        ses_subencl_type_enum subencl_type,
                                                        terminator_eses_subencl_side side,
                                                        ses_elem_type_enum elem_type,
                                                        fbe_bool_t consider_num_possible_elems,
                                                        fbe_u8_t num_possible_elems,
                                                        fbe_bool_t consider_sub_encl_id,
                                                        fbe_u8_t sub_encl_id,
                                                        fbe_bool_t consider_type_desc_text,
                                                        fbe_u8_t *type_desc_text,
                                                        fbe_u16_t *offset)
{
    fbe_u8_t i;

    // Ignore type descriptor text related parameters for now.
    UNREFERENCED_PARAMETER(consider_type_desc_text);
    UNREFERENCED_PARAMETER(type_desc_text);
/*
    if (elem_type == SES_ELEM_TYPE_TEMP_SENSOR)
    {
        DPRINTF("exp, subencl_type=%d, side=%d, elem_type=%d, consider_number=%d, consider_id=%d\n", subencl_type, side, elem_type, consider_num_possible_elems, consider_sub_encl_id );
        for(i=0; i < config_page_info->num_type_desc_headers ;i++)
        {
            DPRINTF("i=%d, subencl_type=%d, side=%d, elem_type=%d, offset=%d, number=%d, subencl_id=%d, first_index=%d\n", i,
                    config_page_info->subencl_desc_array[config_page_info->type_desc_array[i].subencl_id].subencl_type,
                    config_page_info->subencl_desc_array[config_page_info->type_desc_array[i].subencl_id].side,
                    config_page_info->type_desc_array[i].elem_type,
                    config_page_info->offset_of_first_elem_array[i],
                    config_page_info->type_desc_array[i].num_possible_elems,
                    config_page_info->type_desc_array[i].subencl_id,
                    config_page_info->elem_index_of_first_elem_array[i]
                    );
        }
    }

*/
    for(i=0; i < config_page_info->num_type_desc_headers ;i++)
    {
        if((config_page_info->type_desc_array[i].elem_type == elem_type) &&
           ((!consider_num_possible_elems) || (config_page_info->type_desc_array[i].num_possible_elems == num_possible_elems)) &&
           ((!consider_sub_encl_id) || (config_page_info->type_desc_array[i].subencl_id == sub_encl_id)) &&
           ((config_page_info->subencl_desc_array[config_page_info->type_desc_array[i].subencl_id].subencl_type)
                == subencl_type ) &&
           ((config_page_info->subencl_desc_array[config_page_info->type_desc_array[i].subencl_id].side)
                == side) )
        {
            *offset = config_page_info->offset_of_first_elem_array[i];
            return(FBE_STATUS_OK);
        }
    }

    // Assume FBE_STATUS_COMPONENT_NOT_FOUND implies the particular element
    // is not present in the configuration page. Ex: we dont have PS elems
    // in the configuration page for magnum as they are monitored by specl
    // on magnum DPE and not monitored by the ESES talking EMA.
    return(FBE_STATUS_COMPONENT_NOT_FOUND);
}

/*********************************************************************
* get_start_elem_index_by_config_page_info ()
**********************************************************************
*
*  Description:
*   This function returns the index of the first element(individual) of
*   the given status element type in Encl stat page, WRT to
*   the given configuration page.
*
*  Inputs:
*   config_page_info - pointer to configuration page to use.
*   subencl_type - subenclosure type
*   side - side (A or B indicated by 0 & 1 now)
*   elem_type - Status element type.
*   offset - pointer to offset to be returned
*
*  Return Value: success or failure
*
*  Notes:
*
*  History:
*    Sep08  Rajesh V created
*
*********************************************************************/
static fbe_status_t get_start_elem_index_by_config_page_info(terminator_eses_config_page_info_t *config_page_info,
                                                    ses_subencl_type_enum subencl_type,
                                                    terminator_eses_subencl_side side,
                                                    ses_elem_type_enum elem_type,
                                                    fbe_bool_t consider_num_possible_elems,
                                                    fbe_u8_t num_possible_elems,
                                                    fbe_bool_t consider_type_desc_text,
                                                    fbe_u8_t *type_desc_text,
                                                    fbe_u8_t *index)
{
    fbe_u8_t i;

    // Ignore type descriptor text related parameters for now.
    UNREFERENCED_PARAMETER(consider_type_desc_text);
    UNREFERENCED_PARAMETER(type_desc_text);

    for(i=0; i < config_page_info->num_type_desc_headers ;i++)
    {
/*
        DPRINTF("%s:%d expected subencl_type %d expected side %d expected elem_type 0x%x, searching subencl type 0x%x subencl id %d side %d elem type 0x%x\n",
                __func__, __LINE__,
                subencl_type, side, elem_type,
                config_page_info->subencl_desc_array[config_page_info->type_desc_array[i].subencl_id].subencl_type,
                config_page_info->type_desc_array[i].subencl_id,
                config_page_info->subencl_desc_array[config_page_info->type_desc_array[i].subencl_id].side,
                config_page_info->type_desc_array[i].elem_type);
*/
        if((config_page_info->type_desc_array[i].elem_type == elem_type) &&
           ((!consider_num_possible_elems) || (config_page_info->type_desc_array[i].num_possible_elems == num_possible_elems)) &&
           ((config_page_info->subencl_desc_array[config_page_info->type_desc_array[i].subencl_id].subencl_type)
                == subencl_type ) &&
            ((config_page_info->subencl_desc_array[config_page_info->type_desc_array[i].subencl_id].side)
                == side) )
        {
            *index = config_page_info->elem_index_of_first_elem_array[i];
            return(FBE_STATUS_OK);
        }
    }

    DPRINTF("%s:%d not found index for side %d, elem type 0x%x\n", __func__, __LINE__, side, elem_type);
    // Assume FBE_STATUS_COMPONENT_NOT_FOUND implies the particular element
    // is not present in the configuration page. Ex: we dont have PS elems
    // in the configuration page for magnum as they are monitored by specl
    // on magnum DPE and not monitored by the ESES talking EMA.
    return(FBE_STATUS_COMPONENT_NOT_FOUND);
}

/*********************************************************************
*  config_page_get_start_elem_index_in_stat_page()
**********************************************************************
*
*  Description:
*   This function returns the current index of first element(individual)
*   of the given status element type in Encl stat page.
*
*  Inputs:
*   subencl_type - subenclosure type
*   side - side (A or B indicated by 0 & 1 now)
*   elem_type - Status element type.
*   offset - pointer to offset to be returned
*
*  Return Value: success or failure
*
*  Notes:
*
*  History:
*    Sep08  Rajesh V created
*
*********************************************************************/

fbe_status_t config_page_get_start_elem_index_in_stat_page(terminator_sas_virtual_phy_info_t *info,
                                                            ses_subencl_type_enum subencl_type,
                                                            terminator_eses_subencl_side side,
                                                            ses_elem_type_enum elem_type,
                                                            fbe_bool_t consider_num_possible_elems,
                                                            fbe_u8_t num_possible_elems,
                                                            fbe_bool_t consider_type_desc_text,
                                                            fbe_u8_t *type_desc_text,
                                                            fbe_u8_t *index)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_eses_config_page_info_t *current_config_page_info;
    vp_config_diag_page_info_t *vp_config_diag_page_info;

    // Get the virtual phy configuration information for the given Virtual Phy handle
    vp_config_diag_page_info = &info->eses_page_info.vp_config_diag_page_info;

    // Get the base configuration page information from the VP config info.
    current_config_page_info = vp_config_diag_page_info->config_page_info;

    status = get_start_elem_index_by_config_page_info(current_config_page_info,
                                                      subencl_type,
                                                      side,
                                                      elem_type,
                                                      consider_num_possible_elems,
                                                      num_possible_elems,
                                                      consider_type_desc_text,
                                                      type_desc_text,
                                                      index);
    RETURN_ON_ERROR_STATUS;

    return(FBE_STATUS_OK);
}

/*********************************************************************
*  config_page_get_start_elem_offset_in_stat_page ()
**********************************************************************
*
*  Description:
*   This function returns the current offset of the given status
*   element type in Encl stat page.
*
*  Inputs:
*   virtual phy object handle.
*   subencl_type - subenclosure type
*   side - side (A or B indicated by 0 & 1 now)
*   elem_type - Status element type.
*   offset - pointer to offset to be returned
*
*  Return Value: success or failure
*
*  Notes:
*
*  History:
*    Sep08  Rajesh V created
*
*********************************************************************/
static fbe_status_t config_page_get_start_elem_offset_in_stat_page(terminator_sas_virtual_phy_info_t *info,
                                                            ses_subencl_type_enum subencl_type,
                                                            terminator_eses_subencl_side side,
                                                            ses_elem_type_enum elem_type,
                                                            fbe_bool_t consider_num_possible_elems,
                                                            fbe_u8_t num_possible_elems,
                                                            fbe_bool_t consider_sub_encl_id,
                                                            fbe_u8_t sub_encl_id,
                                                            fbe_bool_t consider_type_desc_text,
                                                            fbe_u8_t *type_desc_text,
                                                            fbe_u16_t *offset)
{

    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_eses_config_page_info_t *current_config_page_info;
    vp_config_diag_page_info_t *vp_config_diag_page_info;

    // Get the virtual phy configuration information for the given Virtual Phy handle
    vp_config_diag_page_info = &info->eses_page_info.vp_config_diag_page_info;

    // Get the base configuration page information from the VP config info.
    current_config_page_info = vp_config_diag_page_info->config_page_info;

    status = get_start_elem_offset_by_config_page_info(current_config_page_info,
                                                       subencl_type,
                                                       side,
                                                       elem_type,
                                                       consider_num_possible_elems,
                                                       num_possible_elems,
                                                       consider_sub_encl_id,
                                                       sub_encl_id,
                                                       consider_type_desc_text,
                                                       type_desc_text,
                                                       offset);
    RETURN_ON_ERROR_STATUS;

    return(FBE_STATUS_OK);
}


/*********************************************************************
*  enclosure_status_diagnostic_page_build_status_elements ()
*********************************************************************
*
*  Description: This builds the Status elements for the encl 
*   status diagnostic page. 
*
*  Inputs: 
*   status_elements_start_ptr - pointer to the start of status elements
*   status_elements_end_ptr - pointer to be returned, that indicates the
*       end address of all status elements.
*   enclosure information, virtual phy device id and port number.
*
*  Return Value: success or failure
*
*  Notes: 
*   We also return peer LCC connector elements
*
*  History:
*    Aug08 created
*    Sep08 Start using offsets--etc from Configuration Diag Page module
*    
*********************************************************************/
fbe_status_t enclosure_status_diagnostic_page_build_status_elements(
    fbe_u8_t *encl_stat_diag_page_start_ptr, 
    fbe_u8_t **stat_elem_end_ptr, 
    terminator_sas_virtual_phy_info_t *info)
{

    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    fbe_u8_t *stat_elem_start_ptr = encl_stat_diag_page_start_ptr;
    //This contains the start offset of the particular element set as 
    //dictated by configuration page.
    fbe_u16_t stat_elem_byte_offset = 0;
    terminator_sp_id_t spid;
    fbe_u32_t slot_id;
    fbe_u8_t max_lcc_count = 0;
    fbe_u8_t max_ee_lcc_count = 0;
    fbe_u8_t max_ext_cooling_elems_count = 0;
    fbe_sas_enclosure_type_t  encl_type = info->enclosure_type;
    status =  fbe_terminator_api_get_sp_id(info, &spid);

    // Build Array device slot status elements.
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid,
                                                            SES_ELEM_TYPE_ARRAY_DEV_SLOT,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);
    
    
    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element. This is legal as for ex: we dont have
        // PS elems in the configuration page for magnum as they are monitored by specl
        // on magnum DPE and not monitored by the "ESES talking EMA".
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {

        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_device_slot_status_elements(stat_elem_start_ptr, 
                                                                                    stat_elem_end_ptr, 
                                                                                    info);
        RETURN_ON_ERROR_STATUS;
    }

    // Build Expander Phy status elements.
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid, 
                                                            SES_ELEM_TYPE_EXP_PHY, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);


    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {

        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_exp_phy_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr, 
                                                                                info);
        RETURN_ON_ERROR_STATUS;
    }

    // Build Peer Expander non-drive Phy status elements.
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            !spid,
                                                            SES_ELEM_TYPE_EXP_PHY,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);


    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {

        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_peer_exp_phy_status_elements(stat_elem_start_ptr,
                                                                                stat_elem_end_ptr,
                                                                                info);
        RETURN_ON_ERROR_STATUS;
    }
    // Build Expander Phy status elements.
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid,
                                                            SES_ELEM_TYPE_EXP_PHY,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);


    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {

        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_exp_phy_status_elements(stat_elem_start_ptr,
                                                                                stat_elem_end_ptr,
                                                                                info);
        RETURN_ON_ERROR_STATUS;
    }

    // Build Local SAS Connector status elements.
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid, 
                                                            SES_ELEM_TYPE_SAS_CONN, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

    
    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_local_sas_conn_status_elements(stat_elem_start_ptr, 
                                                                                    stat_elem_end_ptr, 
                                                                                    info);
        RETURN_ON_ERROR_STATUS;
    }


    // Build Peer SAS Connector status elements.
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            !spid, 
                                                            SES_ELEM_TYPE_SAS_CONN, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);


    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_peer_sas_conn_status_elements(stat_elem_start_ptr, 
                                                                                  stat_elem_end_ptr, 
                                                                                  info);
        RETURN_ON_ERROR_STATUS;
    }


    // Build Midplane SAS Connector status elements.
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_CHASSIS,
                                                            FBE_ESES_SUBENCL_SIDE_ID_MIDPLANE, 
                                                            SES_ELEM_TYPE_SAS_CONN,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

    
    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_midplane_sas_conn_status_elements(stat_elem_start_ptr, 
                                                                                    stat_elem_end_ptr, 
                                                                                    info);
        RETURN_ON_ERROR_STATUS;
    }

    // Build local SAS Exp status elements.
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid, 
                                                            SES_ELEM_TYPE_SAS_EXP, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

   
    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_sas_exp_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr);
        RETURN_ON_ERROR_STATUS;
    }

    // Build peer SAS Exp status elements.
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            !spid, 
                                                            SES_ELEM_TYPE_SAS_EXP, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

   
    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_sas_exp_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr);
        RETURN_ON_ERROR_STATUS;
    }

    if ( (encl_type == FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP) ||
         (encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP) ||
         (encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP))
    {
        // Build PS A1 status element
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_PS,
                                                                spid, 
                                                                SES_ELEM_TYPE_PS, 
                                                                FALSE,
                                                                0,
                                                                TRUE,
                                                                3,
                                                                FALSE,
                                                                NULL,
                                                                &stat_elem_byte_offset);
        
        if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
           (status != FBE_STATUS_OK))
        {
            // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
            // does not have the particular element.
            return(status);
        }
        else if(status == FBE_STATUS_OK)
        {
            stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
            status = enclosure_status_diagnostic_page_build_psa1_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr, 
                                                                                info);
            RETURN_ON_ERROR_STATUS;  
        }
        
        // Build PS B1 status element
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_PS,
                                                                !spid, 
                                                                SES_ELEM_TYPE_PS, 
                                                                FALSE,
                                                                0,
                                                                TRUE,
                                                                4,
                                                                FALSE,
                                                                NULL,
                                                                &stat_elem_byte_offset);
        
        if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
           (status != FBE_STATUS_OK))
        {
            // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
            // does not have the particular element.
            return(status);
        }
        else if(status == FBE_STATUS_OK)
        {
            stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
            status = enclosure_status_diagnostic_page_build_psb1_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr, 
                                                                                info);
            RETURN_ON_ERROR_STATUS;  
        }
        
        // Build PS A0 status element
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_PS,
                                                                spid, 
                                                                SES_ELEM_TYPE_PS, 
                                                                FALSE,
                                                                0,
                                                                TRUE,
                                                                5,
                                                                FALSE,
                                                                NULL,
                                                                &stat_elem_byte_offset);
        
        if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
           (status != FBE_STATUS_OK))
        {
            // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
            // does not have the particular element.
            return(status);
        }
        else if(status == FBE_STATUS_OK)
        {
            stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
            status = enclosure_status_diagnostic_page_build_psa0_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr, 
                                                                                info);
            RETURN_ON_ERROR_STATUS;  
        }
        
        // Build PS B0 status element.
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_PS,
                                                                !spid, 
                                                                SES_ELEM_TYPE_PS, 
                                                                FALSE,
                                                                0,
                                                                TRUE,
                                                                6,
                                                                FALSE,
                                                                NULL,
                                                                &stat_elem_byte_offset);
        
        if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
           (status != FBE_STATUS_OK))
        {
            // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
            // does not have the particular element.
            return(status);
        }
        else if(status == FBE_STATUS_OK)
        {
            stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
            status = enclosure_status_diagnostic_page_build_psb0_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr, 
                                                                                info);
            RETURN_ON_ERROR_STATUS; 
        }
    }
    else
    {
        // Build PS A status element
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_PS,
                                                                spid, 
                                                                SES_ELEM_TYPE_PS, 
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &stat_elem_byte_offset);
        
        if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
           (status != FBE_STATUS_OK))
        {
            // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
            // does not have the particular element.
            return(status);
        }
        else if(status == FBE_STATUS_OK)
        {
            stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
            status = enclosure_status_diagnostic_page_build_psa_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr, 
                                                                                info);
            RETURN_ON_ERROR_STATUS;  
        }
        
        
        // Build PS B status element.
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_PS,
                                                                !spid, 
                                                                SES_ELEM_TYPE_PS, 
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &stat_elem_byte_offset);
        
        if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
           (status != FBE_STATUS_OK))
        {
            // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
            // does not have the particular element.
            return(status);
        }
        else if(status == FBE_STATUS_OK)
        {
            stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
            status = enclosure_status_diagnostic_page_build_psb_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr, 
                                                                                info);
            RETURN_ON_ERROR_STATUS; 
        }
    }


    //Build PS A cooling elements
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_PS,
                                                            spid, 
                                                            SES_ELEM_TYPE_COOLING, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);
   
    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_psa_cooling_status_elements(stat_elem_start_ptr, 
                                                                                    stat_elem_end_ptr, 
                                                                                    info);
        RETURN_ON_ERROR_STATUS;  
    }



    //Build PS B cooling elements
   
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_PS,
                                                            !spid, 
                                                            SES_ELEM_TYPE_COOLING, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);


    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_psb_cooling_status_elements(stat_elem_start_ptr, 
                                                                                    stat_elem_end_ptr, 
                                                                                    info);
        RETURN_ON_ERROR_STATUS;  
    }

    //Build Local Temperature Sensor elements
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            (encl_type == FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM)? 2:0, 
                                                            SES_ELEM_TYPE_TEMP_SENSOR, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        DPRINTF("1 stat_elem_byte_offset=%d\n", stat_elem_byte_offset);
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_local_temp_sensor_status_elements(stat_elem_start_ptr, 
                                                                                        stat_elem_end_ptr, 
                                                                                        info);
        RETURN_ON_ERROR_STATUS;  
    }

    //Build Peer Temperature Sensor elements
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            (encl_type == FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM)? 3:1,
                                                            SES_ELEM_TYPE_TEMP_SENSOR, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

     if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        DPRINTF("2 stat_elem_byte_offset=%d\n", stat_elem_byte_offset);
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_peer_temp_sensor_status_elements(stat_elem_start_ptr, 
                                                                                         stat_elem_end_ptr,
                                                                                         info);
        RETURN_ON_ERROR_STATUS; 
    }


    //Build Chassis Temperature Sensor elements
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_CHASSIS,
                                                            SIDE_UNDEFINED, 
                                                            SES_ELEM_TYPE_TEMP_SENSOR, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);
     if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        DPRINTF("3 stat_elem_byte_offset=%d\n", stat_elem_byte_offset);
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_chassis_temp_sensor_status_elements(stat_elem_start_ptr, 
                                                                                            stat_elem_end_ptr,
                                                                                            info);
        RETURN_ON_ERROR_STATUS; 
    }

    //Build chassis Enclosure element (encl element in CHASSIS Subenclosure)
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_CHASSIS,
                                                            SIDE_UNDEFINED, 
                                                            SES_ELEM_TYPE_ENCL, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);
    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_chassis_encl_status_elements(stat_elem_start_ptr, 
                                                                                    stat_elem_end_ptr, 
                                                                                    info);
        RETURN_ON_ERROR_STATUS;
    }



    //Build local lcc Enclosure element  (encl element in LOCAL LCC Subenclosure)
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid, 
                                                            SES_ELEM_TYPE_ENCL, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_local_encl_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr, 
                                                                                info);
        RETURN_ON_ERROR_STATUS;
    }


    //Build Peer lcc Enclosure element (encl element in PEER LCC Subenclosure)
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            !spid, 
                                                            SES_ELEM_TYPE_ENCL, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_peer_encl_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr);
        RETURN_ON_ERROR_STATUS;
    }

    status = sas_virtual_phy_max_lccs(encl_type, &max_lcc_count, info->side);
    status = sas_virtual_phy_max_ee_lccs(encl_type, &max_ee_lcc_count, info->side);

    //Build ee lcc Enclosure element
    for(slot_id = (max_lcc_count - max_ee_lcc_count); slot_id < max_lcc_count; slot_id++)
    {
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                         SES_SUBENCL_TYPE_LCC,
                                                         slot_id, 
                                                         SES_ELEM_TYPE_ENCL, 
                                                         FALSE,
                                                         0,
                                                         FALSE,
                                                         0,
                                                         FALSE,
                                                         NULL,
                                                         &stat_elem_byte_offset);

        if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
           (status != FBE_STATUS_OK))
        {
            // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
            // does not have the particular element.
            return(status);
        }
        else if(status == FBE_STATUS_OK)
        {
            stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
            status = enclosure_status_diagnostic_page_build_ee_encl_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr);
            RETURN_ON_ERROR_STATUS;
        }
    }

    //Build ext fan Enclosure element 
    status = sas_virtual_phy_max_ext_cooling_elems(encl_type, &max_ext_cooling_elems_count, info->side);
 
    for(slot_id = 0; slot_id < max_ext_cooling_elems_count; slot_id++) 
    {
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                         SES_SUBENCL_TYPE_COOLING,
                                                         slot_id, 
                                                         SES_ELEM_TYPE_COOLING, 
                                                         FALSE,
                                                         0,
                                                         FALSE,
                                                         0,
                                                         FALSE,
                                                         NULL,
                                                         &stat_elem_byte_offset);

        if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
           (status != FBE_STATUS_OK))
        {
            // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
            // does not have the particular element.
            return(status);
        }
        else if(status == FBE_STATUS_OK)
        {
            stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
            status = enclosure_status_diagnostic_page_build_ext_cooling_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr, 
                                                                                info,
                                                                                slot_id);
            RETURN_ON_ERROR_STATUS;
        } 
    }
    
    //Build BEM fan Enclosure element 
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                         SES_SUBENCL_TYPE_LCC,
                                                         spid,
                                                         SES_ELEM_TYPE_COOLING,
                                                         FALSE,
                                                         0,
                                                         FALSE,
                                                         0,
                                                         FALSE,
                                                         NULL,
                                                         &stat_elem_byte_offset);
        if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
           (status != FBE_STATUS_OK))
        {
            // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
            // does not have the particular element.
            return(status);
        }
        else if(status == FBE_STATUS_OK)
        {
            stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
            status = enclosure_status_diagnostic_page_build_bem_cooling_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr,
                                                                                info);
            RETURN_ON_ERROR_STATUS;
        }
 
    //Build Chassis Cooling status
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                         SES_SUBENCL_TYPE_CHASSIS,
                                                         SIDE_UNDEFINED, 
                                                         SES_ELEM_TYPE_COOLING, 
                                                         FALSE,
                                                         0,
                                                         FALSE,
                                                         0,
                                                         FALSE,
                                                         NULL,
                                                         &stat_elem_byte_offset);

        if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
           (status != FBE_STATUS_OK))
        {
            // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
            // does not have the particular element.
            return(status);
        }
        else if(status == FBE_STATUS_OK)
        {
            stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
            status = enclosure_status_diagnostic_page_build_chassis_cooling_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr,
                                                                                info);
            RETURN_ON_ERROR_STATUS;
        }
    // NOTE: In future, all the below display functions should be implemented
    // in a single function. This may be useful as we get enclosures with
    // more than 2 displays and different characters in each display. The single
    // function should determine these display parameters and builds display elements
    // based on the enclosures display properties.
    
    // Build the display elements for the first physical DISPLAY on enclosure.

    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid, 
                                                            SES_ELEM_TYPE_DISPLAY, 
                                                            TRUE,
                                                            2,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_display0_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr,
                                                                                info);
        RETURN_ON_ERROR_STATUS;
    }


    // Build the display elements for the second physical DISPLAY on enclosure.
    status = config_page_get_start_elem_offset_in_stat_page(info,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid, 
                                                            SES_ELEM_TYPE_DISPLAY, 
                                                            TRUE,
                                                            1,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

   
    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_display1_status_elements(stat_elem_start_ptr, 
                                                                                stat_elem_end_ptr,
                                                                                info);
        RETURN_ON_ERROR_STATUS;
    }

/*******************************************************
    //Build Local PS Temperature Sensor elements
    status = config_page_get_start_elem_offset_in_stat_page(virtual_phy_handle,
                                                            SES_SUBENCL_TYPE_PS,
                                                            spid, 
                                                            SES_ELEM_TYPE_TEMP_SENSOR, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

    if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_local_ps_temp_sensor_status_elements(stat_elem_start_ptr, 
                                                                                        stat_elem_end_ptr, 
                                                                                        virtual_phy_handle);
        RETURN_ON_ERROR_STATUS;  
    }

    //Build Peer PS Temperature Sensor elements
    status = config_page_get_start_elem_offset_in_stat_page(virtual_phy_handle,
                                                            SES_SUBENCL_TYPE_PS,
                                                            !spid, 
                                                            SES_ELEM_TYPE_TEMP_SENSOR, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &stat_elem_byte_offset);

     if((status != FBE_STATUS_COMPONENT_NOT_FOUND) &&
       (status != FBE_STATUS_OK))
    {
        // FBE_STATUS_COMPONENT_NOT_FOUND is allowed as it means the configuration page
        // does not have the particular element.
        return(status);
    }
    else if(status == FBE_STATUS_OK)
    {
        stat_elem_start_ptr = encl_stat_diag_page_start_ptr + stat_elem_byte_offset;
        status = enclosure_status_diagnostic_page_build_peer_ps_temp_sensor_status_elements(stat_elem_start_ptr, 
                                                                                        stat_elem_end_ptr);
        RETURN_ON_ERROR_STATUS; 
    }
 
*********************************************************/

    return(FBE_STATUS_OK);

}

#define GET_OFFSET(x)  (uint8_t)((x - FBE_ESES_PAGE_HEADER_SIZE) / (FBE_ESES_CTRL_STAT_ELEM_SIZE))
fbe_status_t emc_statistics_stat_page_build_power_supply_stats(terminator_sas_virtual_phy_info_t *s, uint8_t *device_slot_stats_start_ptr,
                                                 uint8_t **device_slot_stats_end_ptr)
{
    fbe_status_t                   status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sp_id_t             spid;
    fbe_u16_t                      offset = 0;
    fbe_eses_power_supply_stats_t* pwr_elem_stats = (fbe_eses_power_supply_stats_t*)device_slot_stats_start_ptr;

    status =  fbe_terminator_api_get_sp_id(s, &spid);
    if ( s->enclosure_type == FBE_SAS_ENCLOSURE_TYPE_TABASCO)
    {
        // Build PS A status element
        status = config_page_get_start_elem_offset_in_stat_page(s,
                                                                SES_SUBENCL_TYPE_PS,
                                                                spid,
                                                                SES_ELEM_TYPE_PS,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &offset);
        if (status == FBE_STATUS_OK)
        {
            offset = (uint8_t)((offset - FBE_ESES_PAGE_HEADER_SIZE) / (FBE_ESES_CTRL_STAT_ELEM_SIZE));
            offset += 1;    // skip overall element.
            memset(pwr_elem_stats, 0, sizeof(fbe_eses_power_supply_stats_t));
            pwr_elem_stats->common_stats.elem_offset = offset;
            pwr_elem_stats->common_stats.stats_len = sizeof(fbe_eses_power_supply_stats_t) - sizeof(ses_common_statistics_field_t);
            pwr_elem_stats->ac_fail_count = 0;
            pwr_elem_stats ++;
        }
        else if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
            return status;
        }

        // Build PS B status element
        status = config_page_get_start_elem_offset_in_stat_page(s,
                                                                SES_SUBENCL_TYPE_PS,
                                                                1 - spid,
                                                                SES_ELEM_TYPE_PS,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &offset);
        if (status == FBE_STATUS_OK)
        {
            offset = (uint8_t)((offset - FBE_ESES_PAGE_HEADER_SIZE) / (FBE_ESES_CTRL_STAT_ELEM_SIZE));
            offset += 1;    // skip overall element.
            memset(pwr_elem_stats, 0, sizeof(fbe_eses_power_supply_stats_t));
            pwr_elem_stats->common_stats.elem_offset = offset;
            pwr_elem_stats->common_stats.stats_len = sizeof(fbe_eses_power_supply_stats_t) - sizeof(ses_common_statistics_field_t);
            pwr_elem_stats->ac_fail_count = 0;
            pwr_elem_stats ++;
        }
        else if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
            return status;
        }
    }
    *device_slot_stats_end_ptr = (uint8_t*) pwr_elem_stats;
    return FBE_STATUS_OK;
}

fbe_status_t emc_statistics_stat_page_build_cooling_stats(terminator_sas_virtual_phy_info_t *info, uint8_t *device_slot_stats_start_ptr,
                                                 uint8_t **device_slot_stats_end_ptr)
{
    fbe_status_t                 status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sp_id_t           spid;
    fbe_u16_t                    elem_offset;
    fbe_eses_cooling_stats_t    *elem_ptr;

    fbe_u8_t                     elem_len;
    if (info->enclosure_type == FBE_SAS_ENCLOSURE_TYPE_TABASCO)
    {
        elem_len = sizeof(fbe_eses_cooling_stats_t) - sizeof(ses_common_statistics_field_t);
        elem_ptr = (fbe_eses_cooling_stats_t*) device_slot_stats_start_ptr;
        status =  fbe_terminator_api_get_sp_id(info, &spid);
        //Build PS A cooling elements
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_PS,
                                                                spid,
                                                                SES_ELEM_TYPE_COOLING,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &elem_offset);

        if(status == FBE_STATUS_OK)
        {
            fbe_u8_t max_cooling_elems = 0;
            elem_offset = GET_OFFSET(elem_offset);

            elem_offset += 1; // skip overall.
            status = sas_virtual_phy_max_cooling_elems(info->enclosure_type, &max_cooling_elems, info->side);
            for (; max_cooling_elems > 0; max_cooling_elems--)
            {
                // individual elem
                memset(elem_ptr, 0, sizeof(fbe_eses_cooling_stats_t));
                elem_ptr->common_stats.elem_offset = elem_offset;
                elem_ptr->common_stats.stats_len = elem_len;
                elem_ptr->fail_count = 0;

                elem_ptr ++;
                elem_offset += 1;
            }
        }
        else if(status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
            return(status);
        }



        //Build PS B cooling elements

        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_PS,
                                                                1 - spid,
                                                                SES_ELEM_TYPE_COOLING,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &elem_offset);


        if(status == FBE_STATUS_OK)
        {
            fbe_u8_t max_cooling_elems = 0;
            elem_offset = GET_OFFSET(elem_offset);

            elem_offset += 1; // skip overall.
            status = sas_virtual_phy_max_cooling_elems(info->enclosure_type, &max_cooling_elems, !(info->side));
            for (; max_cooling_elems > 0; max_cooling_elems--)
            {
                // individual elem
                memset(elem_ptr, 0, sizeof(fbe_eses_cooling_stats_t));
                elem_ptr->common_stats.elem_offset = elem_offset;
                elem_ptr->common_stats.stats_len = elem_len;
                elem_ptr->fail_count = 0;

                elem_ptr ++;
                elem_offset += 1;
            }
        }
        else if(status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
            return(status);
        }
    }

    *device_slot_stats_end_ptr = (uint8_t*) elem_ptr;
    return FBE_STATUS_OK;
}


fbe_status_t emc_statistics_stat_page_build_temp_sensor_stats(terminator_sas_virtual_phy_info_t *info, uint8_t *device_slot_stats_start_ptr,
                                                 uint8_t **device_slot_stats_end_ptr)
{
    fbe_status_t                 status = FBE_STATUS_OK;
    terminator_sp_id_t           spid;
    fbe_u16_t                    stat_elem_byte_offset;
    fbe_eses_temperature_stats_t *elem_ptr;
    fbe_sas_enclosure_type_t     encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    fbe_u8_t                     elem_len;

    encl_type = info->enclosure_type;
    elem_len = sizeof(fbe_eses_temperature_stats_t) - sizeof(ses_common_statistics_field_t);
    elem_ptr = (fbe_eses_temperature_stats_t*) device_slot_stats_start_ptr;
    status =  fbe_terminator_api_get_sp_id(info, &spid);

    if (encl_type == FBE_SAS_ENCLOSURE_TYPE_TABASCO)
    {
        //Build Local Temperature Sensor elements
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_LCC,
                                                                spid,
                                                                SES_ELEM_TYPE_TEMP_SENSOR,
                                                                TRUE,
                                                                2,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &stat_elem_byte_offset);

        if ( status ==  FBE_STATUS_OK )
        {
            int i;
            stat_elem_byte_offset = GET_OFFSET(stat_elem_byte_offset) + 1; // skip overall element

            for (i = 0; i < 2; i++)
            {
                memset(elem_ptr, 0, sizeof(fbe_eses_temperature_stats_t));

                elem_ptr->common_stats.elem_offset = stat_elem_byte_offset;
                elem_ptr->common_stats.stats_len = elem_len;
                elem_ptr->ot_fail_count = 0;
                elem_ptr->ot_warning_count = 0;
                elem_ptr ++;
                stat_elem_byte_offset++;
            }

        }else if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
            return status;
        }

        //Build Chassis Temperature Sensor elements
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_CHASSIS,
                                                                SIDE_UNDEFINED,
                                                                SES_ELEM_TYPE_TEMP_SENSOR,
                                                                TRUE,
                                                                7,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &stat_elem_byte_offset);
        if ( status ==  FBE_STATUS_OK )
        {
           int i;
           stat_elem_byte_offset = GET_OFFSET(stat_elem_byte_offset) + 1; // skip overall element
           for (i = 0; i < 7; i++)
           {
               memset(elem_ptr, 0, sizeof(fbe_eses_temperature_stats_t));

               elem_ptr->common_stats.elem_offset = stat_elem_byte_offset;
               elem_ptr->common_stats.stats_len = elem_len;
               elem_ptr->ot_fail_count = 0;
               elem_ptr->ot_warning_count = 0;
               elem_ptr ++;
               stat_elem_byte_offset++;
           }

        }else if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
           return status;
        }

        //Build PS local side Temperature Sensor elements
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_PS,
                                                                spid,
                                                                SES_ELEM_TYPE_TEMP_SENSOR,
                                                                TRUE,
                                                                2,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &stat_elem_byte_offset);

        if ( status ==  FBE_STATUS_OK )
        {
            int i;
            stat_elem_byte_offset = GET_OFFSET(stat_elem_byte_offset) + 1; // skip overall element

            for (i = 0; i < 2; i++)
            {
                memset(elem_ptr, 0, sizeof(fbe_eses_temperature_stats_t));

                elem_ptr->common_stats.elem_offset = stat_elem_byte_offset;
                elem_ptr->common_stats.stats_len = elem_len;
                elem_ptr->ot_fail_count = 0;
                elem_ptr->ot_warning_count = 0;
                elem_ptr ++;
                stat_elem_byte_offset++;
            }

        }else if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
            return status;
        }

        //Build PS peer side Temperature Sensor elements
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                SES_SUBENCL_TYPE_PS,
                                                                1 - spid,
                                                                SES_ELEM_TYPE_TEMP_SENSOR,
                                                                TRUE,
                                                                2,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &stat_elem_byte_offset);

        if ( status ==  FBE_STATUS_OK )
        {
            int i;
            stat_elem_byte_offset = GET_OFFSET(stat_elem_byte_offset) + 1; // skip overall element

            for (i = 0; i < 2; i++)
            {
                memset(elem_ptr, 0, sizeof(fbe_eses_temperature_stats_t));

                elem_ptr->common_stats.elem_offset = stat_elem_byte_offset;
                elem_ptr->common_stats.stats_len = elem_len;
                elem_ptr->ot_fail_count = 0;
                elem_ptr->ot_warning_count = 0;
                elem_ptr ++;
                stat_elem_byte_offset++;
            }

        }else if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
            return status;
        }

    }

    *device_slot_stats_end_ptr = (uint8_t*)elem_ptr;
    return FBE_STATUS_OK;
}

fbe_status_t emc_statistics_stat_page_build_sas_exp_stats(terminator_sas_virtual_phy_info_t *info, uint8_t *device_slot_stats_start_ptr,
                                                 uint8_t **device_slot_stats_end_ptr)
{
    fbe_status_t                 status = FBE_STATUS_OK;
    terminator_sp_id_t           spid;
    fbe_u16_t                    stat_elem_byte_offset;
    fbe_eses_exp_stats_t         *elem_ptr;
    fbe_sas_enclosure_type_t     encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    fbe_u8_t                     elem_len;

    encl_type = info->enclosure_type;
    elem_len = sizeof(fbe_eses_exp_stats_t) - sizeof(ses_common_statistics_field_t);
    elem_ptr = (fbe_eses_exp_stats_t*) device_slot_stats_start_ptr;
    status =  fbe_terminator_api_get_sp_id(info, &spid);

    if (encl_type == FBE_SAS_ENCLOSURE_TYPE_TABASCO)
    {
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                        SES_SUBENCL_TYPE_LCC,
                                                                        spid,
                                                                        SES_ELEM_TYPE_SAS_EXP,
                                                                        TRUE,
                                                                        1,
                                                                        FALSE,
                                                                        0,
                                                                        FALSE,
                                                                        NULL,
                                                                        &stat_elem_byte_offset);
        if ( status ==  FBE_STATUS_OK )
        {
            stat_elem_byte_offset = GET_OFFSET(stat_elem_byte_offset) + 1; // skip overall element

            memset(elem_ptr, 0, sizeof(fbe_eses_exp_stats_t));

            elem_ptr->common_stats.elem_offset = stat_elem_byte_offset;
            elem_ptr->common_stats.stats_len = elem_len;
            elem_ptr->exp_change_count = bswap16(7014);
            elem_ptr ++;
            stat_elem_byte_offset++;

        }else if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
            return status;
        }
        status = config_page_get_start_elem_offset_in_stat_page(info,
                                                                        SES_SUBENCL_TYPE_LCC,
                                                                        1 - spid,
                                                                        SES_ELEM_TYPE_SAS_EXP,
                                                                        TRUE,
                                                                        1,
                                                                        FALSE,
                                                                        0,
                                                                        FALSE,
                                                                        NULL,
                                                                        &stat_elem_byte_offset);
        if ( status ==  FBE_STATUS_OK )
        {
            stat_elem_byte_offset = GET_OFFSET(stat_elem_byte_offset) + 1; // skip overall element

            memset(elem_ptr, 0, sizeof(fbe_eses_temperature_stats_t));

            elem_ptr->common_stats.elem_offset = stat_elem_byte_offset;
            elem_ptr->common_stats.stats_len = elem_len;
            elem_ptr->exp_change_count = bswap16(7014);
            elem_ptr ++;
            stat_elem_byte_offset++;

        }else if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
            return status;
        }
    }
    *device_slot_stats_end_ptr = (uint8_t*)elem_ptr;
     return FBE_STATUS_OK;
}

fbe_status_t emc_statistics_stat_page_build_exp_phy_stats(terminator_sas_virtual_phy_info_t *s, uint8_t *device_slot_stats_start_ptr,
                                                 uint8_t **device_slot_stats_end_ptr)
{
    fbe_status_t                 status = FBE_STATUS_GENERIC_FAILURE;
    uint32_t                    i, j;
    terminator_sp_id_t          spid;
    fbe_sas_enclosure_type_t     encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    fbe_eses_exp_phy_stats_t     *overall_exp_phy_stats = NULL;
    fbe_eses_exp_phy_stats_t     *individual_exp_phy_stats = NULL;
    uint16_t                     overall_exp_phy_elem_addr_offset = 0;
    uint8_t                     individual_exp_phy_elem_offset = 0;
    uint8_t                     overall_exp_phy_elem_offset = 0;
    uint8_t                     exp_phy_elem_stats_len = 0;
    uint8_t                     *source_ptr = NULL;
    uint8_t                     max_phys = 0;
    uint8_t                     mapped_phy_id = 127;
    uint8_t                     max_drive_slots = 0;

    uint32_t   invalid_dword_count = 0;
    uint32_t   disparity_error_count = 0;
    uint32_t   loss_dword_sync_count = 0;
    uint32_t   phy_reset_fail_count = 0;
    uint32_t   code_violation_count = 0;
    uint8_t    phy_change_count = 0;
    uint16_t   crc_pmon_accum_count = 0;
    uint16_t   in_connect_crc_count = 0;

    fbe_u8_t max_conn_id_count = 0;
    fbe_u8_t max_single_lane_port_conn_count = 0;
    fbe_u8_t max_port_conn_count = 0;

    encl_type = s->enclosure_type;

    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots, s->side);
    if (status != FBE_STATUS_OK)
            return status;

    status =  fbe_terminator_api_get_sp_id(s, &spid);
     //start pop phy statistics info
    source_ptr = device_slot_stats_start_ptr;

    status = config_page_get_start_elem_offset_in_stat_page(s,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid,
                                                            SES_ELEM_TYPE_EXP_PHY,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &overall_exp_phy_elem_addr_offset);

    if (status != FBE_STATUS_OK)
        return status;
    overall_exp_phy_elem_offset =
        (uint8_t)((overall_exp_phy_elem_addr_offset - FBE_ESES_PAGE_HEADER_SIZE) /
                   (FBE_ESES_CTRL_STAT_ELEM_SIZE));

    exp_phy_elem_stats_len = sizeof(fbe_eses_exp_phy_stats_t) - sizeof(ses_common_statistics_field_t);

    overall_exp_phy_stats = (fbe_eses_exp_phy_stats_t *) source_ptr;
    memset(overall_exp_phy_stats, 0, sizeof(fbe_eses_exp_phy_stats_t));
    overall_exp_phy_stats->common_stats.elem_offset = overall_exp_phy_elem_offset;
    overall_exp_phy_stats->common_stats.stats_len = exp_phy_elem_stats_len;

    individual_exp_phy_elem_offset = overall_exp_phy_elem_offset;
    individual_exp_phy_stats = overall_exp_phy_stats;

    status = sas_virtual_phy_max_phys(encl_type, &max_phys, s->side);

    if (status != FBE_STATUS_OK)
    {
        printf("%s: get max phys failed.\n",
                         __FUNCTION__);

        return status;
    }

    for(i=0; i<max_phys; i++)
    {
        individual_exp_phy_stats ++;
        individual_exp_phy_elem_offset ++;

        memset(individual_exp_phy_stats, 0, sizeof(fbe_eses_exp_phy_stats_t));
        individual_exp_phy_stats->common_stats.elem_offset = individual_exp_phy_elem_offset;
        individual_exp_phy_stats->common_stats.stats_len = exp_phy_elem_stats_len;
        mapped_phy_id = 255;

        //start inject fake count info
        //for slot phy and connector phy we inject fake data differently just for fun
        for( j = 0; j < max_drive_slots; j++ )
        {
            // Get the corresponding PHY status
            status = sas_virtual_phy_get_drive_slot_to_phy_mapping(j, &mapped_phy_id, encl_type, s->side);
            if (status != FBE_STATUS_OK)
            {
                DPRINTF("%s sas_virtual_phy_get_drive_slot_to_phy_mapping failed!\n", __FUNCTION__);
                return status;
            }
            if(mapped_phy_id == i) // it is drive slot phy
            {
                individual_exp_phy_stats->invalid_dword = bswap32(j++); // we swap the byte order to simulate big endian which is used by expander
                individual_exp_phy_stats->disparity_error = bswap32(j++);
                individual_exp_phy_stats->loss_dword_sync = bswap32(j++);
                individual_exp_phy_stats->phy_reset_fail = bswap32(j++);
                individual_exp_phy_stats->code_violation = bswap32(j++);
                individual_exp_phy_stats->phy_change = j++;
                individual_exp_phy_stats->crc_pmon_accum = bswap16(j++);
                individual_exp_phy_stats->in_connect_crc = bswap16(j++);
                break;
            }
        }

    // it is connector phy
        if(mapped_phy_id != i)
        {
            individual_exp_phy_stats->invalid_dword = bswap32(invalid_dword_count++);
            individual_exp_phy_stats->disparity_error = bswap32(disparity_error_count++);
            individual_exp_phy_stats->loss_dword_sync = bswap32(loss_dword_sync_count++);
            individual_exp_phy_stats->phy_reset_fail = bswap32(phy_reset_fail_count++);
            individual_exp_phy_stats->code_violation = bswap32(code_violation_count++);
            individual_exp_phy_stats->phy_change = phy_change_count++;
            individual_exp_phy_stats->crc_pmon_accum = bswap16(crc_pmon_accum_count++);
            individual_exp_phy_stats->in_connect_crc = bswap16(in_connect_crc_count++);
        }
        //end inject fake count info

    }

    // fill phy status of connectors.

    status = config_page_get_start_elem_offset_in_stat_page(s,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            !spid,
                                                            SES_ELEM_TYPE_EXP_PHY,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &overall_exp_phy_elem_addr_offset);


    overall_exp_phy_elem_offset = GET_OFFSET(overall_exp_phy_elem_addr_offset);


    // fill overall element.
    individual_exp_phy_stats ++;
    memset(individual_exp_phy_stats, 0, sizeof(fbe_eses_exp_phy_stats_t));
    individual_exp_phy_stats->common_stats.elem_offset = overall_exp_phy_elem_offset;
    individual_exp_phy_stats->common_stats.stats_len = exp_phy_elem_stats_len;

    individual_exp_phy_stats->invalid_dword = bswap32(invalid_dword_count++);
    individual_exp_phy_stats->disparity_error = bswap32(disparity_error_count++);
    individual_exp_phy_stats->loss_dword_sync = bswap32(loss_dword_sync_count++);
    individual_exp_phy_stats->phy_reset_fail = bswap32(phy_reset_fail_count++);
    individual_exp_phy_stats->code_violation = bswap32(code_violation_count++);
    individual_exp_phy_stats->phy_change = phy_change_count++;
    individual_exp_phy_stats->crc_pmon_accum = bswap16(crc_pmon_accum_count++);
    individual_exp_phy_stats->in_connect_crc = bswap16(in_connect_crc_count++);
    overall_exp_phy_elem_offset ++;

    // Get mapping based on encl_type
    status = sas_virtual_phy_max_conn_id_count(encl_type, &max_conn_id_count, s->side);
    RETURN_ON_ERROR_STATUS;

    status = sas_virtual_phy_max_single_lane_conns_per_port(encl_type, &max_single_lane_port_conn_count, s->side);
    RETURN_ON_ERROR_STATUS;

    status = sas_virtual_phy_max_conns_per_port(encl_type, &max_port_conn_count, s->side);
    RETURN_ON_ERROR_STATUS;

    status = fbe_terminator_api_get_sp_id(s, &spid);
    RETURN_ON_ERROR_STATUS;

    // fill individual elements.
    individual_exp_phy_elem_offset = overall_exp_phy_elem_offset;
    for (j = 0; j < max_conn_id_count; j++)
    {
        for (i = 0; i < max_single_lane_port_conn_count; i++)
        {

        individual_exp_phy_stats ++;
        memset(individual_exp_phy_stats, 0, sizeof(fbe_eses_exp_phy_stats_t));
        individual_exp_phy_stats->common_stats.elem_offset = individual_exp_phy_elem_offset;
        individual_exp_phy_stats->common_stats.stats_len = exp_phy_elem_stats_len;

        individual_exp_phy_stats->invalid_dword = bswap32(invalid_dword_count++);
        individual_exp_phy_stats->disparity_error = bswap32(disparity_error_count++);
        individual_exp_phy_stats->loss_dword_sync = bswap32(loss_dword_sync_count++);
        individual_exp_phy_stats->phy_reset_fail = bswap32(phy_reset_fail_count++);
        individual_exp_phy_stats->code_violation = bswap32(code_violation_count++);
        individual_exp_phy_stats->phy_change = phy_change_count++;
        individual_exp_phy_stats->crc_pmon_accum = bswap16(crc_pmon_accum_count++);
        individual_exp_phy_stats->in_connect_crc = bswap16(in_connect_crc_count++);

        individual_exp_phy_elem_offset ++;
        }
    }

    *device_slot_stats_end_ptr = (uint8_t *)(individual_exp_phy_stats + 1);

    return(FBE_STATUS_OK);
}


fbe_status_t
emc_statistics_stat_page_build_device_slot_stats(terminator_sas_virtual_phy_info_t *s, uint8_t *device_slot_stats_start_ptr,
                                                 uint8_t **device_slot_stats_end_ptr)
{
    fbe_status_t                 status = FBE_STATUS_GENERIC_FAILURE;
    fbe_eses_device_slot_stats_t *overall_device_slot_stats = NULL;
    fbe_eses_device_slot_stats_t *individual_device_slot_stats = NULL;
    uint16_t                    overall_device_slot_elem_addr_offset = 0;
    uint8_t                     individual_device_slot_elem_offset = 0;
    uint8_t                     overall_device_slot_elem_offset = 0;
    uint8_t                     max_drive_slots = 0;
    uint32_t                    i;
    uint8_t                     device_slot_elem_stats_len = 0;
    fbe_sas_enclosure_type_t     encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_sp_id_t spid;




    /* get the enclosure type thru the virtual_phy_handle */
    encl_type = s->enclosure_type;

    status =  fbe_terminator_api_get_sp_id(s, &spid);

    status = config_page_get_start_elem_offset_in_stat_page(s,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid,
                                                            SES_ELEM_TYPE_ARRAY_DEV_SLOT,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &overall_device_slot_elem_addr_offset);

    if (status != FBE_STATUS_OK)
        return status;
    overall_device_slot_elem_offset =
        (uint8_t)((overall_device_slot_elem_addr_offset - FBE_ESES_PAGE_HEADER_SIZE) /
                   (FBE_ESES_CTRL_STAT_ELEM_SIZE));

    device_slot_elem_stats_len =
        sizeof(fbe_eses_device_slot_stats_t) - sizeof(ses_common_statistics_field_t);

    overall_device_slot_stats = (fbe_eses_device_slot_stats_t *)device_slot_stats_start_ptr;
    memset (overall_device_slot_stats, 0, sizeof(fbe_eses_device_slot_stats_t));
    overall_device_slot_stats->common_stats.elem_offset = overall_device_slot_elem_offset;
    overall_device_slot_stats->common_stats.stats_len = device_slot_elem_stats_len;
    //Ignore statistics in overall array device slot element

    individual_device_slot_stats = overall_device_slot_stats;
    individual_device_slot_elem_offset = overall_device_slot_elem_offset;

    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots, s->side);
    if (status != FBE_STATUS_OK)
        return status;

    for(i=0; i<max_drive_slots; i++)
    {
        individual_device_slot_stats ++;
        individual_device_slot_elem_offset ++;

        memset(individual_device_slot_stats, 0, sizeof(fbe_eses_device_slot_stats_t));
        individual_device_slot_stats->common_stats.elem_offset = individual_device_slot_elem_offset;
        individual_device_slot_stats->common_stats.stats_len = device_slot_elem_stats_len;
        status = sas_virtual_phy_get_drive_slot_insert_count(s, i, &individual_device_slot_stats->insert_count);
        if (status != FBE_STATUS_OK)
            return status;

        status = sas_virtual_phy_get_drive_power_down_count(s, i, &individual_device_slot_stats->power_down_count);
        if (status != FBE_STATUS_OK)
            return status;

    }
    *device_slot_stats_end_ptr = (uint8_t *)(individual_device_slot_stats + 1);

    return(FBE_STATUS_OK);
}

static fbe_status_t addl_elem_stat_page_dev_slots_get_elem_index(terminator_sas_virtual_phy_info_t *s, 
                                                          uint8_t drive_slot_num,  
                                                          uint8_t *elem_index)
{
    fbe_status_t status = FBE_STATUS_OK;
    uint8_t start_element_index = 0;
    terminator_sp_id_t spid;

    status =  fbe_terminator_api_get_sp_id(s, &spid);

    DPRINTF("%s:%d get elem for side %d driver slot num %d\n", __func__, __LINE__, spid, drive_slot_num);
    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_ARRAY_DEV_SLOT, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &start_element_index);


    *elem_index = start_element_index + drive_slot_num;
    return(status);
}

//TODO: get driver sas address
static fbe_status_t addl_elem_stat_page_dev_slot_phy_desc_get_sas_addr(
    terminator_sas_virtual_phy_info_t *virtual_phy_handle,
    fbe_u8_t dev_slot_num, 
    fbe_u64_t *sas_address)
{
    fbe_status_t status = FBE_STATUS_OK;
    fbe_u8_t phy_id;
    fbe_u8_t max_phys;
    *sas_address = 0;
    uint16_t scsi_id = 0;
    SCSIDevice *d = NULL;
    SCSISESState *s = (SCSISESState *)virtual_phy_handle->ses_dev;
    SCSIBus *bus = scsi_bus_from_device((SCSIDevice*)s);

    status = sas_virtual_phy_get_drive_slot_to_phy_mapping(dev_slot_num, &phy_id, virtual_phy_handle->enclosure_type, virtual_phy_handle->side);
    RETURN_ON_ERROR_STATUS;

    status = sas_virtual_phy_max_phys(virtual_phy_handle->enclosure_type, &max_phys, virtual_phy_handle->side);
    RETURN_ON_ERROR_STATUS;
    
    scsi_id = phy_id + virtual_phy_handle->side * (max_phys + 1);

    if ((d = scsi_device_find(bus, 0, scsi_id, 0)) != NULL) {
        if (d->type == TYPE_DISK)
            *sas_address = d->port_wwn;
        else
            *sas_address = d->wwn;
    }

    return(status);
}

static fbe_status_t addl_elem_stat_page_phy_desc_get_phy_id(
    fbe_u8_t dev_slot_num, fbe_u8_t *phy_id)
{
    // use array or spec to get mapping as per the spec in future.
    *phy_id = dev_slot_num;

    return(FBE_STATUS_OK);

}

//TODO
static fbe_status_t terminator_update_enclosure_drive_slot_number(terminator_sas_virtual_phy_info_t *s, uint32_t *slot_number)
{
    return FBE_STATUS_OK;
}

static fbe_status_t addl_elem_stat_page_build_device_slot_elements(terminator_sas_virtual_phy_info_t *s,
    uint8_t *status_elements_start_ptr, 
    uint8_t **status_elements_end_ptr)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_addl_elem_stat_desc_hdr_struct *addl_elem_stat_desc_ptr = NULL;
    ses_array_dev_slot_prot_spec_info_struct *dev_slot_prot_spec_info_ptr = NULL;
    ses_array_dev_phy_desc_struct *dev_phy_desc_ptr = NULL;
    uint8_t max_drive_slots = 0;
    uint8_t i;
    fbe_sas_enclosure_type_t    encl_type = s->enclosure_type;
    uint32_t slot_number = 0;

    addl_elem_stat_desc_ptr = (ses_addl_elem_stat_desc_hdr_struct *)status_elements_start_ptr; 
    memset(addl_elem_stat_desc_ptr, 0, sizeof(ses_addl_elem_stat_desc_hdr_struct));

    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots, s->side);
    if(status != FBE_STATUS_OK)
    {
        return(status);
    }
    for(i=0; i<max_drive_slots; i++)
    {
        memset(addl_elem_stat_desc_ptr, 0, sizeof(ses_addl_elem_stat_desc_hdr_struct));
        // only SAS for now (6 is for SAS)
        addl_elem_stat_desc_ptr->protocol_id = 0x6; 
        addl_elem_stat_desc_ptr->eip = 0x1;
        addl_elem_stat_desc_ptr->desc_len = 0x22;

        //Using configuration diagnostic page offsets
        if((addl_elem_stat_page_dev_slots_get_elem_index(s, i, &addl_elem_stat_desc_ptr->elem_index)) !=
            FBE_STATUS_OK)
        {
            return(FBE_STATUS_GENERIC_FAILURE);
        }
        dev_slot_prot_spec_info_ptr = (ses_array_dev_slot_prot_spec_info_struct *)(((uint8_t *)addl_elem_stat_desc_ptr) + FBE_ESES_ADDL_ELEM_STATUS_DESC_HDR_SIZE);
        memset(dev_slot_prot_spec_info_ptr, 0, sizeof(ses_array_dev_slot_prot_spec_info_struct));
        dev_slot_prot_spec_info_ptr->num_phy_descs = 0x1;
        dev_slot_prot_spec_info_ptr->desc_type = 0;
        dev_slot_prot_spec_info_ptr->not_all_phys = 0x1;

        slot_number = i;
        // update the slot number
        status = terminator_update_enclosure_drive_slot_number(s, &slot_number);
        if(status != FBE_STATUS_OK)
        {
            return status;
        }
        dev_slot_prot_spec_info_ptr->dev_slot_num = (uint8_t)slot_number;

        dev_phy_desc_ptr = (ses_array_dev_phy_desc_struct *)(((uint8_t *)dev_slot_prot_spec_info_ptr) + FBE_ESES_ARRAY_DEV_SLOT_PROT_SPEC_INFO_HEADER_SIZE );
        memset(dev_phy_desc_ptr, 0, sizeof(ses_array_dev_phy_desc_struct));

        // This is not the actual phy to device mapping. So can be ignored in simulation.
        if(addl_elem_stat_page_phy_desc_get_phy_id(i,&dev_phy_desc_ptr->phy_id) !=
            FBE_STATUS_OK)
        {
            return(status);
        }

        dev_phy_desc_ptr->attached_sas_address = bswap64(((SCSISESState *)s->ses_dev)->qdev.wwn + 1);
        DPRINTF("%s:%d attached sas address 0x%lx\n", __func__, __LINE__, cpu_to_be64(dev_phy_desc_ptr->attached_sas_address));

        // this is sas address of the drive physical slot i;
        if(addl_elem_stat_page_dev_slot_phy_desc_get_sas_addr(s, i, &dev_phy_desc_ptr->sas_address) !=
            FBE_STATUS_OK)
        {
            return(status);
        }
        // convert to bigendian
        dev_phy_desc_ptr->sas_address = bswap64(dev_phy_desc_ptr->sas_address);

        DPRINTF("%s: encl_type:%d max_drives:%d slot_number:%d phy_id:%d sas_address: 0x%lx\n", 
                             __FUNCTION__, encl_type, max_drive_slots, slot_number, 
                             dev_phy_desc_ptr->phy_id, dev_phy_desc_ptr->sas_address);



        // remember to ask ping if this structure needs the field "attached sas address"
        // this represents the enclosure SMP physical sas address...

        addl_elem_stat_desc_ptr = (ses_addl_elem_stat_desc_hdr_struct *)
            (((uint8_t *)addl_elem_stat_desc_ptr) + 
            FBE_ESES_ADDL_ELEM_STATUS_DESC_HDR_SIZE + 
            FBE_ESES_ARRAY_DEV_SLOT_PROT_SPEC_INFO_HEADER_SIZE + 
            sizeof(ses_array_dev_phy_desc_struct)) ;
    }

    *status_elements_end_ptr = (uint8_t *)addl_elem_stat_desc_ptr;
    status = FBE_STATUS_OK;
    return(status);
}

static bool sas_virtual_phy_phy_corresponds_to_drive_slot(terminator_sas_virtual_phy_info_t *info, uint8_t phy_id, uint8_t *drive_slot, fbe_sas_enclosure_type_t encl_type)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t returned_phy_id, i;
    uint8_t max_drive_slots;

    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots, info->side);
    if (status != FBE_STATUS_OK)
        return status;

    for(i=0; i<max_drive_slots; i++)
    {
        status = sas_virtual_phy_get_drive_slot_to_phy_mapping(i, &returned_phy_id, encl_type, info->side);
        if(status != FBE_STATUS_OK)
        {
            break;
        }
        if(returned_phy_id == phy_id)
        {
            *drive_slot = i;
            return(TRUE);
        }
    }
    return(FALSE);
}

static fbe_status_t addl_elem_stat_page_sas_exp_get_sas_address(terminator_sas_virtual_phy_info_t *s, uint64_t *sas_address)
{
    *sas_address = ((SCSISESState *)s->ses_dev)->qdev.wwn + 1;
    DPRINTF("%s:%d get local sas address 0x%lx side %d\n", __func__, __LINE__, *sas_address, ((SCSISESState *)s->ses_dev)->side);
    return FBE_STATUS_OK;
}

static fbe_status_t addl_elem_stat_page_sas_peer_exp_get_sas_address(terminator_sas_virtual_phy_info_t *s, uint64_t *sas_address)
{
    SCSISESState *local_ses_dev = (SCSISESState *)s->ses_dev;
    SCSIBus *bus = scsi_bus_from_device(&local_ses_dev->qdev);
    BusChild *kid;

    *sas_address = 0;

    QTAILQ_FOREACH_REVERSE(kid, &bus->qbus.children, ChildrenHead, sibling){
        DeviceState *qdev = kid->child;
        SCSIDevice *dev = SCSI_DEVICE(qdev);
        if (dev->type == TYPE_ENCLOSURE && (!local_ses_dev->side) == ((SCSISESState *)dev)->side) {
            *sas_address = dev->wwn + 1;
            DPRINTF("%s:%d get peer sas address 0x%lx side %d\n", __func__, __LINE__, *sas_address, !local_ses_dev->side);
            return FBE_STATUS_OK;
        }
    }

    return FBE_STATUS_FAILED;
}

static fbe_status_t addl_elem_stat_page_sas_exp_get_elem_index(terminator_sas_virtual_phy_info_t *s, uint8_t *elem_index)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;    
    terminator_sp_id_t spid;

    *elem_index = 0;

    status =  fbe_terminator_api_get_sp_id(s, &spid);

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_SAS_EXP, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           elem_index);

    if (status != FBE_STATUS_OK)
        return status;
    
    status = FBE_STATUS_OK;
    return(status);
}

static fbe_status_t addl_elem_stat_page_sas_peer_exp_get_elem_index(terminator_sas_virtual_phy_info_t *s, uint8_t *elem_index)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;    
    terminator_sp_id_t spid;

    *elem_index = 0;

    status =  fbe_terminator_api_get_sp_id(s, &spid);

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           !spid, 
                                                           SES_ELEM_TYPE_SAS_EXP, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           elem_index);

    if (status != FBE_STATUS_OK)
        return status;
    
    status = FBE_STATUS_OK;
    return(status);
}


static fbe_status_t  addl_elem_stat_page_sas_exp_phy_desc_get_other_elem_index_for_drive_phy(
    terminator_sas_virtual_phy_info_t *s, uint8_t drive_slot, uint8_t *elem_index)
{
    fbe_status_t status = FBE_STATUS_OK;
    uint8_t start_element_index = 0;   
    uint8_t max_drive_slots;
    fbe_sas_enclosure_type_t encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_sp_id_t spid;

    status =  fbe_terminator_api_get_sp_id(s, &spid);

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_ARRAY_DEV_SLOT, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &start_element_index);
    if (status != FBE_STATUS_OK)
        return status;


    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(s, &encl_type);
    if (status != FBE_STATUS_OK)
        return status;

    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots, s->side);
    if (status != FBE_STATUS_OK)
        return status;

    if(drive_slot < max_drive_slots)
    {
        *elem_index = start_element_index + drive_slot;
        return(FBE_STATUS_OK);
    }

    return(FBE_STATUS_GENERIC_FAILURE);
}

static bool sas_virtual_phy_phy_corresponds_to_connector(
    uint8_t phy_id,
    uint8_t *connector,
    uint8_t *connector_id,
    fbe_sas_enclosure_type_t encl_type,
    fbe_u8_t side)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t i, j;
    uint8_t returned_phy_id;
    uint8_t max_single_lane_port_conns;
    uint8_t max_conn_id_count = 0;

    status = sas_virtual_phy_max_single_lane_conns_per_port(encl_type, &max_single_lane_port_conns, side);
    if (status != FBE_STATUS_OK)
        return status;

    status = sas_virtual_phy_max_conn_id_count(encl_type, &max_conn_id_count, side);
    if (status != FBE_STATUS_OK)
        return status;

    for(i=0; i<max_single_lane_port_conns; i++)
    {
        for (j=0; j < max_conn_id_count; j++)
        {
            status = sas_virtual_phy_get_individual_conn_to_phy_mapping(i, j, &returned_phy_id, encl_type, side);
            if(status != FBE_STATUS_OK)
            {
                break;
            }
            if(returned_phy_id == phy_id)
            {
                *connector = i;
                *connector_id = j; 
                return(TRUE);
            }
        }
    }
    return(FALSE);
}

static fbe_status_t addl_elem_stat_page_sas_exp_phy_desc_get_conn_elem_index_for_conn_phy(
    terminator_sas_virtual_phy_info_t *s,
    uint8_t connector, 
    uint8_t connector_id, 
    uint8_t *elem_index)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t start_element_index = 0;   
    uint8_t max_single_lane_port_conns;
    fbe_sas_enclosure_type_t encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_sp_id_t spid;

    status =  fbe_terminator_api_get_sp_id(s, &spid);

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_SAS_CONN, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &start_element_index);

    if (status != FBE_STATUS_OK)
        return status;
   
    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(s, &encl_type);
    if (status != FBE_STATUS_OK)
        return status;

    status = sas_virtual_phy_max_single_lane_conns_per_port(encl_type, &max_single_lane_port_conns, s->side);
    if (status != FBE_STATUS_OK)
        return status;
    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_VIPER:
        case FBE_SAS_ENCLOSURE_TYPE_MAGNUM:
        case FBE_SAS_ENCLOSURE_TYPE_BUNKER:
        case FBE_SAS_ENCLOSURE_TYPE_CITADEL:
        case FBE_SAS_ENCLOSURE_TYPE_DERRINGER:
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM:
        case FBE_SAS_ENCLOSURE_TYPE_FALLBACK:
        case FBE_SAS_ENCLOSURE_TYPE_BOXWOOD:
        case FBE_SAS_ENCLOSURE_TYPE_KNOT:
        case FBE_SAS_ENCLOSURE_TYPE_PINECONE:
        case FBE_SAS_ENCLOSURE_TYPE_STEELJAW:
        case FBE_SAS_ENCLOSURE_TYPE_RAMHORN:
        case FBE_SAS_ENCLOSURE_TYPE_ANCHO:
        case FBE_SAS_ENCLOSURE_TYPE_RHEA:
        case FBE_SAS_ENCLOSURE_TYPE_MIRANDA:
        case FBE_SAS_ENCLOSURE_TYPE_CALYPSO:
	    case FBE_SAS_ENCLOSURE_TYPE_TABASCO:
            if(connector < max_single_lane_port_conns)
            {
                
                *elem_index = start_element_index + connector + 1  + (max_single_lane_port_conns +1) * connector_id;

                //1 for the connector element that
                //represents the connector as a whole.(for primary port).

                    //expansion port connectors. +1 is again for the element 
                    //that represents connector element as a whole , this time for
                    // expansion port.

                status = FBE_STATUS_OK;
            }
            else
            {
                status = FBE_STATUS_GENERIC_FAILURE;
            }
            break;

        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP:
        {
            if(connector_id <= 4) 
            {
                // The 
                //other element index for connector phys should always
                //point to the whole connector element(whose physical link
                //is FFh)
                *elem_index = start_element_index  + connector + 1  + 5 * connector_id;
                status = FBE_STATUS_OK;
            }
            else if(connector_id >= 5 && connector_id < max_single_lane_port_conns)
            {
                // The 
                //other element index for connector phys should always
                //point to the whole connector element(whose physical link
                //is FFh)
                *elem_index = start_element_index  + connector + 1  + (5 * (connector_id - 1)) + 9;
                status = FBE_STATUS_OK;
            }
            else
            {
                status = FBE_STATUS_GENERIC_FAILURE;
            }

            break;
        }
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
            if(connector_id == 0 || connector_id == 1) 
            {
                //1 for the connector element that
                //represents the connector as a whole.
                *elem_index = start_element_index  + connector + 1 + 5 * connector_id;
                status = FBE_STATUS_OK;
            }
            else if(connector_id > 1 && connector_id < 6)
            {
                //1 for the connector element that
                //represents the connector as a whole.
                *elem_index = start_element_index  + connector + 1 + 5 * 2 + 6 * (connector_id - 2);
                status = FBE_STATUS_OK;
            }
            else if(connector_id == 6 || connector_id == 7)
            {
                //1 for the connector element that
                //represents the connector as a whole.
                *elem_index = start_element_index  + connector + 1 + 5 * 2 + 6 * 4 + 5 * (connector_id - 6);
                status = FBE_STATUS_OK;
            }
            else
            {
                status = FBE_STATUS_GENERIC_FAILURE;
            }
            break;

        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            if(connector_id <= 3) 
            {
                //1 for the connector element that
                //represents the connector as a whole.
                *elem_index = start_element_index  + connector + 1 + 5 * connector_id;
                status = FBE_STATUS_OK;
            }
            else if(connector_id == 4 || connector_id == 5)
            {
                //1 for the connector element that
                //represents the connector as a whole.
                *elem_index = start_element_index  + connector + 1 + 5 * 4 + 9 * (connector_id - 4);
                status = FBE_STATUS_OK;
            }
            else
            {
                status = FBE_STATUS_GENERIC_FAILURE;
            }

            break;
         /* As Voyager EE or Viking DRVSXP has only primary port, element index should be calculated as below */   
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE:
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP:
            if(connector < max_single_lane_port_conns)
            {
                
                *elem_index = start_element_index + connector + 1;
                status = FBE_STATUS_OK;
            }
            else
            {
                status = FBE_STATUS_GENERIC_FAILURE;
            }
            break;


        default:
            status = FBE_STATUS_GENERIC_FAILURE;
            break;
    }

    return(status);
}

static fbe_status_t addl_elem_stat_page_sas_exp_phy_desc_get_other_elem_index_for_conn_phy(
    terminator_sas_virtual_phy_info_t *s,
    uint8_t connector, 
    uint8_t connector_id, 
    uint8_t *elem_index)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t start_element_index = 0;
    uint8_t max_single_lane_port_conns;
    fbe_sas_enclosure_type_t encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_sp_id_t spid;

    status =  fbe_terminator_api_get_sp_id(s, &spid);

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_SAS_CONN, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &start_element_index);
   
    if (status != FBE_STATUS_OK)
        return status;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(s, &encl_type);
    if (status != FBE_STATUS_OK)
        return status;

    status = sas_virtual_phy_max_single_lane_conns_per_port(encl_type, &max_single_lane_port_conns, s->side);
    if (status != FBE_STATUS_OK)
        return status;

    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_VIPER:
        case FBE_SAS_ENCLOSURE_TYPE_MAGNUM:
        case FBE_SAS_ENCLOSURE_TYPE_BUNKER:
        case FBE_SAS_ENCLOSURE_TYPE_CITADEL:
        case FBE_SAS_ENCLOSURE_TYPE_DERRINGER:    
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM:
        case FBE_SAS_ENCLOSURE_TYPE_FALLBACK:
        case FBE_SAS_ENCLOSURE_TYPE_BOXWOOD:
        case FBE_SAS_ENCLOSURE_TYPE_KNOT:
        case FBE_SAS_ENCLOSURE_TYPE_PINECONE:
        case FBE_SAS_ENCLOSURE_TYPE_STEELJAW:
        case FBE_SAS_ENCLOSURE_TYPE_RAMHORN:
        case FBE_SAS_ENCLOSURE_TYPE_ANCHO:
        case FBE_SAS_ENCLOSURE_TYPE_RHEA:
        case FBE_SAS_ENCLOSURE_TYPE_MIRANDA:
        case FBE_SAS_ENCLOSURE_TYPE_CALYPSO:
	case FBE_SAS_ENCLOSURE_TYPE_TABASCO:
            if(connector < max_single_lane_port_conns)
            {
                // The 
                //other element index for connector phys should always
                //point to the whole connector element(whose physical link
                //is FFh)
                *elem_index = start_element_index  + (max_single_lane_port_conns +1) * connector_id;

                status = FBE_STATUS_OK;
            }
            else
            {
                status = FBE_STATUS_GENERIC_FAILURE;
            }
            break;

        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP:
            if(connector_id <= 4) 
            {
                // The 
                //other element index for connector phys should always
                //point to the whole connector element(whose physical link
                //is FFh)
                *elem_index = start_element_index  + 5 * connector_id;
                status = FBE_STATUS_OK;
            }
            else if(connector_id >= 5 && connector_id < max_single_lane_port_conns)
            {
                // The 
                //other element index for connector phys should always
                //point to the whole connector element(whose physical link
                //is FFh)
                *elem_index = start_element_index  + (5 * (connector_id - 1)) + 9;
                status = FBE_STATUS_OK;
            }
            else
            {
                status = FBE_STATUS_GENERIC_FAILURE;
            }
                   
            break;

        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
            if(connector_id == 0 || connector_id == 1) 
            {
                // The 
                //other element index for connector phys should always
                //point to the whole connector element(whose physical link
                //is FFh)
                *elem_index = start_element_index  + 5 * connector_id;
                status = FBE_STATUS_OK;
            }
            else if(connector_id > 1 && connector_id < 6)
            {
                // The 
                //other element index for connector phys should always
                //point to the whole connector element(whose physical link
                //is FFh)
                *elem_index = start_element_index  + 5 * 2 + 6 * (connector_id - 2);
                status = FBE_STATUS_OK;
            }
            else if(connector_id == 6 || connector_id == 7)
            {
                // The 
                //other element index for connector phys should always
                //point to the whole connector element(whose physical link
                //is FFh)
                *elem_index = start_element_index  + 5 * 2 + 6 * 4 + 5 * (connector_id - 6);
                status = FBE_STATUS_OK;
            }
            else
            {
                status = FBE_STATUS_GENERIC_FAILURE;
            }

            break;

        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            if(connector_id <= 3) 
            {
                // The 
                //other element index for connector phys should always
                //point to the whole connector element(whose physical link
                //is FFh)
                *elem_index = start_element_index  + 5 * connector_id;
                status = FBE_STATUS_OK;
            }
            else if(connector_id == 4 || connector_id == 5)
            {
                // The 
                //other element index for connector phys should always
                //point to the whole connector element(whose physical link
                //is FFh)
                *elem_index = start_element_index  + 5 * 4 + 9 * (connector_id - 4);
                status = FBE_STATUS_OK;
            }
            else
            {
                status = FBE_STATUS_GENERIC_FAILURE;
            }
                   
            break;

        /* As Voyager EE has only primary port, element index should be calculated as below */
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE:
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP:
            if(connector < max_single_lane_port_conns)
            {                
                *elem_index = start_element_index;

                status = FBE_STATUS_OK;
            }
            else
            {
                status = FBE_STATUS_GENERIC_FAILURE;
            }
            break;

        default:
            status = FBE_STATUS_GENERIC_FAILURE;
            break;
    }

    return(status);
}

static fbe_status_t  addl_elem_stat_page_sas_exp_phy_desc_get_conn_elem_index_for_drive_phy(
    terminator_sas_virtual_phy_info_t *s, uint8_t drive_slot, uint8_t *elem_index)
{
    fbe_status_t status = FBE_STATUS_OK;
    uint8_t start_element_index = 0;   
    uint8_t max_drive_slots;
    fbe_sas_enclosure_type_t encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_CHASSIS,
                                                           FBE_ESES_SUBENCL_SIDE_ID_MIDPLANE,
                                                           SES_ELEM_TYPE_SAS_CONN, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &start_element_index);
    if (status != FBE_STATUS_OK)
        return status;


    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(s, &encl_type);
    if (status != FBE_STATUS_OK)
        return status;

    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots, s->side);
    if (status != FBE_STATUS_OK)
        return status;

    if(drive_slot < max_drive_slots)
    {
        *elem_index = start_element_index + drive_slot;
        return(FBE_STATUS_OK);
    }

    return(FBE_STATUS_GENERIC_FAILURE);
}

static fbe_status_t addl_elem_stat_page_build_sas_exp_elements(terminator_sas_virtual_phy_info_t *s,
    uint8_t *status_elements_start_ptr, 
    uint8_t **status_elements_end_ptr)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_addl_elem_stat_desc_hdr_struct *addl_elem_stat_desc_ptr = NULL;
    ses_sas_exp_prot_spec_info_struct *sas_exp_prot_spec_info_ptr = NULL;
    ses_sas_exp_phy_desc_struct *sas_exp_phy_desc_ptr = NULL;
    uint8_t drive_slot = 0, connector = 0, i;
    uint8_t connector_id = 0;
    fbe_sas_enclosure_type_t           encl_type = s->enclosure_type;

    addl_elem_stat_desc_ptr = (ses_addl_elem_stat_desc_hdr_struct *)status_elements_start_ptr; 
    memset(addl_elem_stat_desc_ptr, 0, sizeof(ses_addl_elem_stat_desc_hdr_struct));



    // only SAS for now (6 is for SAS)
    addl_elem_stat_desc_ptr->protocol_id = 0x6; 
    addl_elem_stat_desc_ptr->eip = 0x1;

    //addl_elem_stat_desc_ptr->desc_len = sizeof(ses_esc_elec_prot_spec_info_struct) + 23*sizeof(ses_esc_elec_exp_phy_desc_struct) + 2;

    status = addl_elem_stat_page_sas_exp_get_elem_index(s, &addl_elem_stat_desc_ptr->elem_index);
    if (status != FBE_STATUS_OK)
        return status;

    sas_exp_prot_spec_info_ptr = (ses_sas_exp_prot_spec_info_struct *)
        ((uint8_t *)addl_elem_stat_desc_ptr + FBE_ESES_ADDL_ELEM_STATUS_DESC_HDR_SIZE );
    status = sas_virtual_phy_max_phys(encl_type, &sas_exp_prot_spec_info_ptr->num_exp_phy_descs, s->side);
    if (status != FBE_STATUS_OK)
        return status;

    sas_exp_prot_spec_info_ptr->desc_type = 0x1;

    DPRINTF("%s: num exp phy descs: %d, elem index 0x%d\n", __func__, sas_exp_prot_spec_info_ptr->num_exp_phy_descs, addl_elem_stat_desc_ptr->elem_index);
    status = addl_elem_stat_page_sas_exp_get_sas_address(s, &sas_exp_prot_spec_info_ptr->sas_address);
    if (status != FBE_STATUS_OK)
        return status;

    // convert to big endian
    sas_exp_prot_spec_info_ptr->sas_address = 
        bswap64(sas_exp_prot_spec_info_ptr->sas_address);

    sas_exp_phy_desc_ptr = (ses_sas_exp_phy_desc_struct *)
        ((uint8_t *)sas_exp_prot_spec_info_ptr + FBE_ESES_SAS_EXP_PROT_SPEC_INFO_HEADER_SIZE);
    // fill in the phy descs. As per recent changes to ESES, T10 spec it was
    // decided that the phy's are arranged in order as per their SAS phy 
    // identifiers(logical), which is too much an assumption :(
    
    for(i=0; i < sas_exp_prot_spec_info_ptr->num_exp_phy_descs; i++)
    {
        memset(sas_exp_phy_desc_ptr, 0, sizeof(ses_sas_exp_phy_desc_struct));
        if(sas_virtual_phy_phy_corresponds_to_drive_slot(s, i, &drive_slot, encl_type))
        {         
            sas_exp_phy_desc_ptr->conn_elem_index = 0xFF;
            status = addl_elem_stat_page_sas_exp_phy_desc_get_conn_elem_index_for_drive_phy(s, drive_slot, &sas_exp_phy_desc_ptr->conn_elem_index);
            RETURN_ON_ERROR_STATUS;

            status = addl_elem_stat_page_sas_exp_phy_desc_get_other_elem_index_for_drive_phy(
                s, drive_slot, &sas_exp_phy_desc_ptr->other_elem_index);
            DPRINTF("%s: encl_type:%d drive_slot:%d conn_elem_index:%d other_elem_index:%d status:0x%x\n", 
                             __FUNCTION__, encl_type, drive_slot,
                             sas_exp_phy_desc_ptr->conn_elem_index,
                             sas_exp_phy_desc_ptr->other_elem_index,
                             status);
            if (status != FBE_STATUS_OK)
                return status;
        }
        else if(sas_virtual_phy_phy_corresponds_to_connector(i, &connector, &connector_id, encl_type, s->side))
        {
            // The connector is a number between 0 and max_single_lane_port_conns, so the resulting
            // element index should be properly calculated.
            status = addl_elem_stat_page_sas_exp_phy_desc_get_other_elem_index_for_conn_phy(
                 s, connector, connector_id, &sas_exp_phy_desc_ptr->other_elem_index);
            if (status != FBE_STATUS_OK)
                return status;

            status = addl_elem_stat_page_sas_exp_phy_desc_get_conn_elem_index_for_conn_phy(s, connector, connector_id, &sas_exp_phy_desc_ptr->conn_elem_index);
            DPRINTF("%s: encl_type:%d connector:%d connector_id:%d elem_index:%d other_elem_index:%d\n", 
                             __FUNCTION__, encl_type, connector, connector_id, 
                             sas_exp_phy_desc_ptr->conn_elem_index,
                             sas_exp_phy_desc_ptr->other_elem_index);
            if (status != FBE_STATUS_OK)
                return status;
        } 
        else
        {

            DPRINTF("%s: encl_type:%d phy:%d is not a drive or connector, max:%d\n", 
                             __FUNCTION__, encl_type, i, sas_exp_prot_spec_info_ptr->num_exp_phy_descs);
            //Ex: Phy 8 in Viper doesnot correspond to drive slot or connector.
            sas_exp_phy_desc_ptr->other_elem_index = 0xFF; 
            sas_exp_phy_desc_ptr->conn_elem_index = 0xFF;

        }

        sas_exp_phy_desc_ptr++;      
    }

    //sas_exp_phy_desc_ptr->conn_elem_index = 0xFF;
    //sas_exp_phy_desc_ptr->other_elem_index = 0xFF;
    //sas_exp_phy_desc_ptr++;

    *status_elements_end_ptr = (uint8_t *)sas_exp_phy_desc_ptr;
    addl_elem_stat_desc_ptr->desc_len =(uint8_t) (*status_elements_end_ptr - status_elements_start_ptr - 2);
    status = FBE_STATUS_OK;
    return(status);        
}

static fbe_status_t addl_elem_stat_page_build_sas_peer_exp_elements(terminator_sas_virtual_phy_info_t *s,
    uint8_t *status_elements_start_ptr, 
    uint8_t **status_elements_end_ptr)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_addl_elem_stat_desc_hdr_struct *addl_elem_stat_desc_ptr = NULL;
    ses_sas_exp_prot_spec_info_struct *sas_exp_prot_spec_info_ptr = NULL;
    ses_sas_exp_phy_desc_struct *sas_exp_phy_desc_ptr = NULL;
    uint8_t drive_slot = 0, connector = 0, i;
    uint8_t connector_id = 0;
    fbe_sas_enclosure_type_t           encl_type = s->enclosure_type;

    addl_elem_stat_desc_ptr = (ses_addl_elem_stat_desc_hdr_struct *)status_elements_start_ptr; 
    memset(addl_elem_stat_desc_ptr, 0, sizeof(ses_addl_elem_stat_desc_hdr_struct));



    // only SAS for now (6 is for SAS)
    addl_elem_stat_desc_ptr->protocol_id = 0x6; 
    addl_elem_stat_desc_ptr->eip = 0x1;

    //addl_elem_stat_desc_ptr->desc_len = sizeof(ses_esc_elec_prot_spec_info_struct) + 23*sizeof(ses_esc_elec_exp_phy_desc_struct) + 2;

    status = addl_elem_stat_page_sas_peer_exp_get_elem_index(s, &addl_elem_stat_desc_ptr->elem_index);
    if (status != FBE_STATUS_OK)
        return status;

    sas_exp_prot_spec_info_ptr = (ses_sas_exp_prot_spec_info_struct *)
        ((uint8_t *)addl_elem_stat_desc_ptr + FBE_ESES_ADDL_ELEM_STATUS_DESC_HDR_SIZE );

    status = sas_virtual_phy_max_phys(encl_type, &sas_exp_prot_spec_info_ptr->num_exp_phy_descs, !s->side);
    if (status != FBE_STATUS_OK)
        return status;

    sas_exp_prot_spec_info_ptr->desc_type = 0x1;

    DPRINTF("%s: num exp phy descs: %d, elem index 0x%d\n", __func__, sas_exp_prot_spec_info_ptr->num_exp_phy_descs, addl_elem_stat_desc_ptr->elem_index);
    status = addl_elem_stat_page_sas_peer_exp_get_sas_address(s, &sas_exp_prot_spec_info_ptr->sas_address);
    if (status != FBE_STATUS_OK)
        return status;

    // convert to big endian
    sas_exp_prot_spec_info_ptr->sas_address = 
        bswap64(sas_exp_prot_spec_info_ptr->sas_address);

    sas_exp_phy_desc_ptr = (ses_sas_exp_phy_desc_struct *)
        ((uint8_t *)sas_exp_prot_spec_info_ptr + FBE_ESES_SAS_EXP_PROT_SPEC_INFO_HEADER_SIZE);
    // fill in the phy descs. As per recent changes to ESES, T10 spec it was
    // decided that the phy's are arranged in order as per their SAS phy 
    // identifiers(logical), which is too much an assumption :(
    for(i=0; i < sas_exp_prot_spec_info_ptr->num_exp_phy_descs; i++)
    {
        memset(sas_exp_phy_desc_ptr, 0, sizeof(ses_sas_exp_phy_desc_struct));
        if(sas_virtual_phy_phy_corresponds_to_drive_slot(s, i, &drive_slot, encl_type))
        {         
            sas_exp_phy_desc_ptr->conn_elem_index = 0xFF;
            sas_exp_phy_desc_ptr->other_elem_index = 0xFF;
            //status = addl_elem_stat_page_sas_exp_phy_desc_get_other_elem_index_for_drive_phy(
            //    s, drive_slot, &sas_exp_phy_desc_ptr->other_elem_index);
            //DPRINTF("%s: encl_type:%d drive_slot:%d other_elem_index:%d status:0x%x\n", 
            //                 __FUNCTION__, encl_type, drive_slot,
            //                 sas_exp_phy_desc_ptr->other_elem_index,
            //                 status);
            //if (status != FBE_STATUS_OK)
            //    return status;
        }
        else if(sas_virtual_phy_phy_corresponds_to_connector(i, &connector, &connector_id, encl_type, s->side))
        {
            // The connector is a number between 0 and max_single_lane_port_conns, so the resulting
            // element index should be properly calculated.
            status = addl_elem_stat_page_sas_exp_phy_desc_get_other_elem_index_for_conn_phy(
                 s, connector, connector_id, &sas_exp_phy_desc_ptr->other_elem_index);
            if (status != FBE_STATUS_OK)
                return status;

            status = addl_elem_stat_page_sas_exp_phy_desc_get_conn_elem_index_for_conn_phy(s, connector, connector_id, &sas_exp_phy_desc_ptr->conn_elem_index);
            DPRINTF("%s: encl_type:%d connector:%d connector_id:%d elem_index:%d other_elem_index:%d\n", 
                             __FUNCTION__, encl_type, connector, connector_id, 
                             sas_exp_phy_desc_ptr->conn_elem_index,
                             sas_exp_phy_desc_ptr->other_elem_index);
            if (status != FBE_STATUS_OK)
                return status;
        } 
        else
        {

            DPRINTF("%s: encl_type:%d phy:%d is not a drive or connector, max:%d\n", 
                             __FUNCTION__, encl_type, i, sas_exp_prot_spec_info_ptr->num_exp_phy_descs);
            //Ex: Phy 8 in Viper doesnot correspond to drive slot or connector.
            sas_exp_phy_desc_ptr->other_elem_index = 0xFF; 
            sas_exp_phy_desc_ptr->conn_elem_index = 0xFF;

        }

        sas_exp_phy_desc_ptr++;      
    }
    *status_elements_end_ptr = (uint8_t *)sas_exp_phy_desc_ptr;
    addl_elem_stat_desc_ptr->desc_len =(uint8_t) (*status_elements_end_ptr - status_elements_start_ptr - 2);
    status = FBE_STATUS_OK;
    return(status);        
}

static __attribute__((unused)) fbe_status_t addl_elem_stat_page_esc_elec_phy_desc_get_other_elem_index(
    terminator_sas_virtual_phy_info_t *info,
    fbe_u8_t phy_id, fbe_u8_t *elem_index)
{
    fbe_status_t status;

    terminator_sp_id_t spid;

    status =  fbe_terminator_api_get_sp_id(info, &spid);

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(info,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_ESC_ELEC, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           elem_index);

    if (status != FBE_STATUS_OK) {
        return status;
    }

    return status;
}

static fbe_status_t addl_elem_stat_page_esc_electronics_get_elem_index(terminator_sas_virtual_phy_info_t *info, fbe_u8_t *elem_index, terminator_eses_subencl_side spid)
{
    //we dont yet have esc elec elements in status page.
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;    
    //terminator_eses_subencl_side spid;

    *elem_index = 0;

   // status =  fbe_terminator_api_get_dae_side(info, &spid);

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(info,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_ESC_ELEC, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           elem_index);

    if (status != FBE_STATUS_OK)
        return status;

    return(FBE_STATUS_OK);
}


static fbe_status_t addl_elem_stat_page_build_esc_electronics_elements(
    terminator_sas_virtual_phy_info_t *info,
    fbe_u8_t *status_elements_start_ptr, 
    fbe_u8_t **status_elements_end_ptr)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_addl_elem_stat_desc_hdr_struct *addl_elem_stat_desc_ptr = NULL;
    ses_esc_elec_prot_spec_info_struct *esc_elec_prot_spec_info_ptr = NULL;
    ses_esc_elec_exp_phy_desc_struct *esc_elec_exp_phy_desc_ptr = NULL;
    //fbe_u8_t i, drive_slot;
    fbe_sas_enclosure_type_t encl_type = info->enclosure_type;
    fbe_u8_t max_phys;

    addl_elem_stat_desc_ptr = (ses_addl_elem_stat_desc_hdr_struct *)status_elements_start_ptr; 
    memset(addl_elem_stat_desc_ptr, 0, sizeof(ses_addl_elem_stat_desc_hdr_struct));
    // only SAS for now (6 is for SAS)
    addl_elem_stat_desc_ptr->protocol_id = 0x6; 
    addl_elem_stat_desc_ptr->eip = 0x1;

    //addl_elem_stat_desc_ptr->desc_len = (fbe_u8_t) (sizeof(ses_esc_elec_prot_spec_info_struct) + 23*sizeof(ses_esc_elec_exp_phy_desc_struct) + 2);

    //Ther element index should be changed once we get the configuration diagnostic page offsets
    // For esc electronics elements this is invalid as we dont return esc elements in the status 
    // page, yet. Even if we return there is only one ESC electronics element.
    status = addl_elem_stat_page_esc_electronics_get_elem_index(info, &addl_elem_stat_desc_ptr->elem_index, info->side);
    RETURN_ON_ERROR_STATUS;


    esc_elec_prot_spec_info_ptr = (ses_esc_elec_prot_spec_info_struct *)
        ((fbe_u8_t *)addl_elem_stat_desc_ptr + FBE_ESES_ADDL_ELEM_STATUS_DESC_HDR_SIZE );
    status = sas_virtual_phy_max_phys(encl_type, &max_phys, info->side);
    RETURN_ON_ERROR_STATUS;
    esc_elec_prot_spec_info_ptr->num_exp_phy_descs = 1;
    esc_elec_prot_spec_info_ptr->desc_type = 0x1;

    esc_elec_exp_phy_desc_ptr = (ses_esc_elec_exp_phy_desc_struct *)
        ((fbe_u8_t *)esc_elec_prot_spec_info_ptr + FBE_ESES_ESC_ELEC_PROT_SPEC_INFO_HEADER_SIZE);

    DPRINTF("%s:%d enclosure service controller electronic num exp phy descs %d element index %d\n", __func__, __LINE__, esc_elec_prot_spec_info_ptr->num_exp_phy_descs, addl_elem_stat_desc_ptr->elem_index);
    // fill in the phy descs
    //for(i=0; i < esc_elec_prot_spec_info_ptr->num_exp_phy_descs; i++)
    {
        memset(esc_elec_exp_phy_desc_ptr, 0, sizeof(ses_esc_elec_exp_phy_desc_struct));
        esc_elec_exp_phy_desc_ptr->phy_id = max_phys;
        //if(!sas_virtual_phy_phy_corresponds_to_drive_slot(info, i, &drive_slot, encl_type))
        //{
        //    esc_elec_exp_phy_desc_ptr->conn_elem_index = 0xFF; // fill this once connector elements filled in stat page
        //    esc_elec_exp_phy_desc_ptr->other_elem_index = 0xFF;
        //}
        //else
        {

        // for the phys not mapped to drive slots this will always be invalid.
            esc_elec_exp_phy_desc_ptr->conn_elem_index = 0xFF;
            esc_elec_exp_phy_desc_ptr->other_elem_index = 0xFF;
            status = addl_elem_stat_page_sas_exp_get_elem_index(info,
                    &esc_elec_exp_phy_desc_ptr->other_elem_index);
            RETURN_ON_ERROR_STATUS;
        }       
        
        // get sas address
        uint16_t scsi_id = (1 + info->side) * max_phys + info->side;
        SCSIDevice *d = NULL;
        SCSISESState *s = (SCSISESState *)info->ses_dev;
        SCSIBus *bus = scsi_bus_from_device((SCSIDevice*)s);

        if ((d = scsi_device_find(bus, 0, scsi_id, 0)) != NULL) {
            esc_elec_exp_phy_desc_ptr->sas_address = bswap64(d->wwn);
        }
        esc_elec_exp_phy_desc_ptr++;      
    }
    *status_elements_end_ptr = (fbe_u8_t *)esc_elec_exp_phy_desc_ptr;
    addl_elem_stat_desc_ptr->desc_len = (fbe_u8_t) (*status_elements_end_ptr - status_elements_start_ptr - 2);

    status = FBE_STATUS_OK;
    return(status);    
}

static fbe_status_t addl_elem_stat_page_build_peer_esc_electronics_elements(
    terminator_sas_virtual_phy_info_t *info,
    fbe_u8_t *status_elements_start_ptr, 
    fbe_u8_t **status_elements_end_ptr)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_addl_elem_stat_desc_hdr_struct *addl_elem_stat_desc_ptr = NULL;
    ses_esc_elec_prot_spec_info_struct *esc_elec_prot_spec_info_ptr = NULL;
    ses_esc_elec_exp_phy_desc_struct *esc_elec_exp_phy_desc_ptr = NULL;
    //fbe_u8_t i, drive_slot;
    fbe_sas_enclosure_type_t encl_type = info->enclosure_type;
    fbe_u8_t max_phys;

    addl_elem_stat_desc_ptr = (ses_addl_elem_stat_desc_hdr_struct *)status_elements_start_ptr; 
    memset(addl_elem_stat_desc_ptr, 0, sizeof(ses_addl_elem_stat_desc_hdr_struct));
    // only SAS for now (6 is for SAS)
    addl_elem_stat_desc_ptr->protocol_id = 0x6; 
    addl_elem_stat_desc_ptr->eip = 0x1;

    //addl_elem_stat_desc_ptr->desc_len = (fbe_u8_t) (sizeof(ses_esc_elec_prot_spec_info_struct) + 23*sizeof(ses_esc_elec_exp_phy_desc_struct) + 2);

    //Ther element index should be changed once we get the configuration diagnostic page offsets
    // For esc electronics elements this is invalid as we dont return esc elements in the status 
    // page, yet. Even if we return there is only one ESC electronics element.
    status = addl_elem_stat_page_esc_electronics_get_elem_index(info, &addl_elem_stat_desc_ptr->elem_index, !info->side);
    RETURN_ON_ERROR_STATUS;


    esc_elec_prot_spec_info_ptr = (ses_esc_elec_prot_spec_info_struct *)
        ((fbe_u8_t *)addl_elem_stat_desc_ptr + FBE_ESES_ADDL_ELEM_STATUS_DESC_HDR_SIZE );
    status = sas_virtual_phy_max_phys(encl_type, &max_phys, info->side);
    RETURN_ON_ERROR_STATUS;
    esc_elec_prot_spec_info_ptr->num_exp_phy_descs = 1;
    esc_elec_prot_spec_info_ptr->desc_type = 0x1;

    esc_elec_exp_phy_desc_ptr = (ses_esc_elec_exp_phy_desc_struct *)
        ((fbe_u8_t *)esc_elec_prot_spec_info_ptr + FBE_ESES_ESC_ELEC_PROT_SPEC_INFO_HEADER_SIZE);

    DPRINTF("%s:%d enclosure service controller electronic num exp phy descs %d element index %d\n", __func__, __LINE__, esc_elec_prot_spec_info_ptr->num_exp_phy_descs, addl_elem_stat_desc_ptr->elem_index);
    // fill in the phy descs
    //for(i=0; i < esc_elec_prot_spec_info_ptr->num_exp_phy_descs; i++)
    {
        memset(esc_elec_exp_phy_desc_ptr, 0, sizeof(ses_esc_elec_exp_phy_desc_struct));
        esc_elec_exp_phy_desc_ptr->phy_id = max_phys;
        //if(!sas_virtual_phy_phy_corresponds_to_drive_slot(info, i, &drive_slot, encl_type))
        //{
        //    esc_elec_exp_phy_desc_ptr->conn_elem_index = 0xFF; // fill this once connector elements filled in stat page
        //    esc_elec_exp_phy_desc_ptr->other_elem_index = 0xFF;
        //}
        //else
        {

        // for the phys not mapped to drive slots this will always be invalid.
            esc_elec_exp_phy_desc_ptr->conn_elem_index = 0xFF;
            esc_elec_exp_phy_desc_ptr->other_elem_index = 0xFF;
            status = config_page_get_start_elem_index_in_stat_page(info,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           !info->side, 
                                                           SES_ELEM_TYPE_SAS_EXP, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &esc_elec_exp_phy_desc_ptr->other_elem_index);


            RETURN_ON_ERROR_STATUS;
        }       
        
        // get sas address
        uint16_t scsi_id = (1 + !info->side) * max_phys + !info->side;
        SCSIDevice *d = NULL;
        SCSISESState *s = (SCSISESState *)info->ses_dev;
        SCSIBus *bus = scsi_bus_from_device((SCSIDevice*)s);

        if ((d = scsi_device_find(bus, 0, scsi_id, 0)) != NULL) {
            esc_elec_exp_phy_desc_ptr->sas_address = bswap64(d->wwn);
        }

        esc_elec_exp_phy_desc_ptr++;      
    }
    *status_elements_end_ptr = (fbe_u8_t *)esc_elec_exp_phy_desc_ptr;
    addl_elem_stat_desc_ptr->desc_len = (fbe_u8_t) (*status_elements_end_ptr - status_elements_start_ptr - 2);

    status = FBE_STATUS_OK;
    return(status);    
}


fbe_status_t addl_elem_stat_page_build_stat_descriptors(terminator_sas_virtual_phy_info_t *info,
    uint8_t *status_elements_start_ptr, 
    uint8_t **status_elements_end_ptr)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t *next_status_element_set_ptr = status_elements_start_ptr;

    status = addl_elem_stat_page_build_device_slot_elements(info,
        next_status_element_set_ptr, status_elements_end_ptr);
    if (status != FBE_STATUS_OK)
        return status;

    next_status_element_set_ptr = (*status_elements_end_ptr);

    status = addl_elem_stat_page_build_sas_exp_elements(info, 
        next_status_element_set_ptr, status_elements_end_ptr);
    if (status != FBE_STATUS_OK)
        return status;

/************************************************************************
 * Ignore ESC electronics elements as now all their functionality
 * was decided to transfer to SAS Exp elements.
 ***********************************************************************/
    next_status_element_set_ptr = (*status_elements_end_ptr);
    status = addl_elem_stat_page_build_esc_electronics_elements(info,
        next_status_element_set_ptr, status_elements_end_ptr);
    RETURN_ON_ERROR_STATUS;
 /***********************************************************************/
    status = FBE_STATUS_OK;

    next_status_element_set_ptr = (*status_elements_end_ptr);
    status = addl_elem_stat_page_build_sas_peer_exp_elements(info, 
        next_status_element_set_ptr, status_elements_end_ptr);
    if (status != FBE_STATUS_OK)
        return status;

    next_status_element_set_ptr = (*status_elements_end_ptr);
    status = addl_elem_stat_page_build_peer_esc_electronics_elements(info,
        next_status_element_set_ptr, status_elements_end_ptr);
    RETURN_ON_ERROR_STATUS;

    return(status);    
}

static fbe_status_t emc_stat_page_sas_conn_info_elem_get_local_elem_index(
    terminator_sas_virtual_phy_info_t *s, 
    uint8_t position,
    uint8_t *conn_elem_index)
{

    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t max_conns;
    uint8_t start_element_index = 0;
    fbe_sas_enclosure_type_t encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_sp_id_t spid;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(s, &encl_type);
    if (status != FBE_STATUS_OK)
        return status;

    status = sas_virtual_phy_max_conns_per_lcc(encl_type, &max_conns, s->side);
    if((status != FBE_STATUS_OK) || (position >= max_conns))
    {
        return(status);
    }

    status =  fbe_terminator_api_get_sp_id(s, &spid);

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_SAS_CONN, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &start_element_index);
    if (status != FBE_STATUS_OK) 
        return status;

    *conn_elem_index = (start_element_index + position);

    status = FBE_STATUS_OK;
    return(status);
}

static fbe_status_t emc_stat_page_sas_conn_info_elem_get_attach_sas_addr(terminator_sas_virtual_phy_info_t *s, uint8_t position,
                                                                         uint8_t conn_elem_index,
                                                                         uint8_t *conn_id,
                                                                         uint64_t *attached_sas_address,
                                                                         uint8_t *sub_encl_id)
{
    fbe_status_t status = FBE_STATUS_OK;
    uint8_t max_conns_per_port;
    uint8_t max_conns_per_lcc;
    fbe_sas_enclosure_type_t encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_conn_map_range_t range = CONN_IS_ILLEGAL;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(s, &encl_type);
    if (status != FBE_STATUS_OK)
        return status;

    status = sas_virtual_phy_max_conns_per_port(encl_type, &max_conns_per_port, s->side);
    if (status != FBE_STATUS_OK)
        return status;

    status = sas_virtual_phy_max_conns_per_lcc(encl_type, &max_conns_per_lcc, s->side);
    if (status != FBE_STATUS_OK)
        return status;

    if((status != FBE_STATUS_OK) || (position >= max_conns_per_lcc))
    {
        return(FBE_STATUS_GENERIC_FAILURE);
    }

    switch(encl_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_VIPER:
        case FBE_SAS_ENCLOSURE_TYPE_BUNKER:
        case FBE_SAS_ENCLOSURE_TYPE_CITADEL:	
        case FBE_SAS_ENCLOSURE_TYPE_DERRINGER:    

        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM:
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_FALLBACK:
        case FBE_SAS_ENCLOSURE_TYPE_BOXWOOD:
        case FBE_SAS_ENCLOSURE_TYPE_KNOT:
        case FBE_SAS_ENCLOSURE_TYPE_PINECONE:
        case FBE_SAS_ENCLOSURE_TYPE_STEELJAW:
        case FBE_SAS_ENCLOSURE_TYPE_RAMHORN:
        case FBE_SAS_ENCLOSURE_TYPE_ANCHO:
        case FBE_SAS_ENCLOSURE_TYPE_RHEA:
        case FBE_SAS_ENCLOSURE_TYPE_MIRANDA:
        case FBE_SAS_ENCLOSURE_TYPE_CALYPSO:
        case FBE_SAS_ENCLOSURE_TYPE_TABASCO:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            terminator_map_position_max_conns_to_range_conn_id(encl_type, position, max_conns_per_port, &range, conn_id);
            switch(range)
            {
                case CONN_IS_DOWNSTREAM:
                    // Indicates the connector belongs to the extension port
                    // FIXME
                    //status = terminator_get_downstream_wideport_sas_address(virtual_phy_handle, attached_sas_address);
                    *attached_sas_address = ((SCSISESState *)(s->ses_dev))->expansion_port_attached_sas_address;
                    *sub_encl_id = FBE_ESES_SUBENCL_SIDE_ID_INVALID;
                    break;
                case CONN_IS_UPSTREAM:
                    // Indicates the connector belongs to the primary port
                    // fixme
                    //status = terminator_get_upstream_wideport_sas_address(virtual_phy_handle, attached_sas_address);
                    //*attached_sas_address = 0x351866d000000000;
                    *attached_sas_address = ((SCSISESState *)(s->ses_dev))->primary_port_attached_sas_address;
                    *sub_encl_id = FBE_ESES_SUBENCL_SIDE_ID_INVALID;
                    break;
                case CONN_IS_RANGE0:
                    if((encl_type == FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM) ||
                       (encl_type == FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP) ||
                       (encl_type == FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP)||
                       (encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP) ||
                       (encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP))
                    {
                        /* Connectors in CONN_IS_RANGE0 are not used by Voyager ICM.*/
                        *attached_sas_address = FBE_SAS_ADDRESS_INVALID;
                        *sub_encl_id = FBE_ESES_SUBENCL_SIDE_ID_INVALID;
                        status = FBE_STATUS_OK;
                    }
                    break;

                case CONN_IS_INTERNAL_RANGE1:
                    // search child list to match connector id. for EE0 or EE1 (Voyager, Viking or Cayenne)
                    // fixme
                    //status = terminator_get_child_expander_wideport_sas_address_by_connector_id(virtual_phy_handle, *conn_id, attached_sas_address);
                    if(encl_type == FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM) 
                    {
                       *sub_encl_id = 2; // The id here is for the Voyager EE LCC
                    }
                    else if(encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP ||
                            encl_type == FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP    ||
                            encl_type == FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP)
                    {
                        *sub_encl_id = 0;
                    }
                    else
                    {
                       *sub_encl_id = FBE_ESES_SUBENCL_SIDE_ID_INVALID;  // ( the same LCC board)
                    }
                    break;
 
                case CONN_IS_ILLEGAL:
                default:
                    status = FBE_STATUS_GENERIC_FAILURE;
                    break;
            }
            break;
        
        case FBE_SAS_ENCLOSURE_TYPE_MAGNUM:
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE:
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP:
            // Magnum, Voyager EE has a 4XSAS connector to the SAS controller. It will not have any
            // expansion port connectors. This will return SAS address of port 0.    
            // FIXME
           // status = terminator_get_upstream_wideport_sas_address(virtual_phy_handle, attached_sas_address);
            break;

        default:
            status = FBE_STATUS_GENERIC_FAILURE;
            break;
    }

    return(status);
}

static fbe_status_t emc_stat_page_sas_conn_info_elem_get_peer_elem_index(
    terminator_sas_virtual_phy_info_t *s, 
    uint8_t position,
    uint8_t *conn_elem_index)
{

    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t max_conns;
    uint8_t start_element_index = 0;
    fbe_sas_enclosure_type_t encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    terminator_sp_id_t spid;

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(s, &encl_type);
    if (status != FBE_STATUS_OK)
        return status;

    status = sas_virtual_phy_max_conns_per_lcc(encl_type, &max_conns, s->side);
    if((status != FBE_STATUS_OK) ||
       (position >= max_conns))
    {
        return(status);
    }
     status =  fbe_terminator_api_get_sp_id(s, &spid);

    //Get from configuration page parsing routines.
    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           !spid, 
                                                           SES_ELEM_TYPE_SAS_CONN, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &start_element_index);



    *conn_elem_index = (start_element_index + position);

    status = FBE_STATUS_OK;
    return(status);
}


/*********************************************************************
* config_page_info_element_exists ()
**********************************************************************
*
*  Description:
*   This function determines if the given element exists or not in the
*   given base configuration information
*
*  Inputs:
*   subencl_type - subenclosure type
*   side - side (A or B indicated by 0 & 1 now)
*   elem_type - Status element type.
*   offset - pointer to offset to be returned
*   Other ESES attributes to distinguish similar kind of elements
*
*  Return Value: TRUE or FALSE
*
*  Notes:
*
*  History:
*    April-20-2009  Rajesh V created
*
*********************************************************************/
static fbe_bool_t config_page_info_element_exists(terminator_eses_config_page_info_t *config_page_info,
                                            ses_subencl_type_enum subencl_type,
                                            terminator_eses_subencl_side side,
                                            ses_elem_type_enum elem_type,
                                            fbe_bool_t consider_num_possible_elems,
                                            fbe_u8_t num_possible_elems,
                                            fbe_bool_t consider_type_desc_text,
                                            fbe_u8_t *type_desc_text)
{
    fbe_u8_t i;

    // Ignore type descriptor text related parameters for now.
    UNREFERENCED_PARAMETER(consider_type_desc_text);
    UNREFERENCED_PARAMETER(type_desc_text);

    for(i=0; i < config_page_info->num_type_desc_headers ;i++)
    {
        if((config_page_info->type_desc_array[i].elem_type == elem_type) &&
           ((!consider_num_possible_elems) || (config_page_info->type_desc_array[i].num_possible_elems == num_possible_elems)) &&
           ((config_page_info->subencl_desc_array[config_page_info->type_desc_array[i].subencl_id].subencl_type)
                == subencl_type ) &&
           ((config_page_info->subencl_desc_array[config_page_info->type_desc_array[i].subencl_id].side)
                == side) )
        {
            return(FBE_TRUE);
        }
    }

    return(FBE_FALSE);
}

static bool config_page_element_exists(terminator_sas_virtual_phy_info_t *s,
                                        ses_subencl_type_enum subencl_type,
                                        terminator_eses_subencl_side side,
                                        ses_elem_type_enum elem_type,
                                        bool consider_num_possible_elems,
                                        uint8_t num_possible_elems,
                                        bool consider_type_desc_text,
                                        uint8_t *type_desc_text)
{
    //fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_eses_config_page_info_t *current_config_page_info;
    vp_config_diag_page_info_t *vp_config_diag_page_info;

    // Get the virtual phy configuration information for the given Virtual Phy handle
    vp_config_diag_page_info = &s->eses_page_info.vp_config_diag_page_info;

    // Get the base configuration page information from the VP config info.
    current_config_page_info = vp_config_diag_page_info->config_page_info;

    return(config_page_info_element_exists(current_config_page_info,
                                        subencl_type,
                                        side,
                                        elem_type,
                                        consider_num_possible_elems,
                                        num_possible_elems,
                                        consider_type_desc_text,
                                        type_desc_text));
}

fbe_status_t emc_encl_stat_diag_page_build_sas_conn_inf_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *sas_conn_elem_start_ptr,
                                                             uint8_t **sas_conn_elem_end_ptr,
                                                             uint8_t *num_sas_conn_info_elem)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_sas_conn_info_elem_struct *sas_conn_inf_elem_ptr = NULL;    
    uint8_t i;
    fbe_sas_enclosure_type_t    encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    uint8_t num_sas_conn_info_elem_per_lcc = 0;
    terminator_sp_id_t spid;
    uint8_t max_conns_per_port;
    terminator_conn_map_range_t return_range = CONN_IS_ILLEGAL; 

    /* get the enclosure type thru the virtual_phy_handle */
    status = terminator_virtual_phy_get_enclosure_type(s, &encl_type);

    status = sas_virtual_phy_max_conns_per_lcc(encl_type, &num_sas_conn_info_elem_per_lcc, s->side);
    if (status != FBE_STATUS_OK)
        return status;

    status = sas_virtual_phy_max_conns_per_port(encl_type, &max_conns_per_port, s->side);
    if (status != FBE_STATUS_OK)
        return status;

    sas_conn_inf_elem_ptr = (ses_sas_conn_info_elem_struct *)(sas_conn_elem_start_ptr);
    for(i=0;i < num_sas_conn_info_elem_per_lcc; i++)
    {
        status = emc_stat_page_sas_conn_info_elem_get_local_elem_index(s, i, &sas_conn_inf_elem_ptr->conn_elem_index);
        if (status != FBE_STATUS_OK)
            return status;
        status = emc_stat_page_sas_conn_info_elem_get_attach_sas_addr(s, i, 
                                                                        sas_conn_inf_elem_ptr->conn_elem_index, 
                                                                        &sas_conn_inf_elem_ptr->conn_id,
                                                                        &sas_conn_inf_elem_ptr->attached_sas_address,
                                                                        &sas_conn_inf_elem_ptr->attached_sub_encl_id);
        if (status != FBE_STATUS_OK)
            return status;
        // convert to big endian 
        sas_conn_inf_elem_ptr->attached_sas_address = bswap64(sas_conn_inf_elem_ptr->attached_sas_address);
        sas_conn_inf_elem_ptr++;
    }
    *num_sas_conn_info_elem += num_sas_conn_info_elem_per_lcc;
    status =  fbe_terminator_api_get_sp_id(s, &spid);

    if(config_page_element_exists(s,
                                SES_SUBENCL_TYPE_LCC,
                                !spid, 
                                SES_ELEM_TYPE_SAS_CONN, 
                                FALSE,
                                0,
                                FALSE,
                                NULL))
    {
                                  
                                  
    //Assume the peer LCC is not installed and fill its connectors
    // For now all peer lCC connector related info is hardcoded.

        for(i=0;i < num_sas_conn_info_elem_per_lcc; i++)
        {
            status = emc_stat_page_sas_conn_info_elem_get_peer_elem_index(s, i, &sas_conn_inf_elem_ptr->conn_elem_index);
            if (status != FBE_STATUS_OK)
                return status;

            sas_conn_inf_elem_ptr->attached_sas_address = FBE_SAS_ADDRESS_INVALID;
            
            terminator_map_position_max_conns_to_range_conn_id(encl_type, 
                                                       i, 
                                                       max_conns_per_port, 
                                                       &return_range,   // Not used here.
                                                       &sas_conn_inf_elem_ptr->conn_id);

            sas_conn_inf_elem_ptr++;
        }
        *num_sas_conn_info_elem += num_sas_conn_info_elem_per_lcc;

    }

    *sas_conn_elem_end_ptr = (uint8_t *)sas_conn_inf_elem_ptr; 
    

    
    status = FBE_STATUS_OK;
    return(status);          
}


/*********************************************************************
* config_page_get_subencl_id ()
**********************************************************************
*
*  Description:
*   Gets Subenclosure ID
*
*  Inputs:
*   Virtual Phy configuration information, Subenclosure Type & side,
*   Subenclosure ID to be returned.
*
*  Return Value: success or failure
*
*  Notes:
*
*  History:
*    Nov-20-2008    Rajesh V created
*
*********************************************************************/
static fbe_status_t config_page_get_subencl_id(vp_config_diag_page_info_t vp_config_diag_page_info,
                                        ses_subencl_type_enum subencl_type,
                                        terminator_eses_subencl_side side,
                                        fbe_u8_t *subencl_id)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    fbe_u8_t subencl;

    for(subencl = 0; subencl < vp_config_diag_page_info.config_page_info->num_subencls; subencl++)
    {
        if((vp_config_diag_page_info.config_page_info->subencl_desc_array[subencl].subencl_type == subencl_type) &&
           (vp_config_diag_page_info.config_page_info->subencl_slot[subencl] == side))
        {
            status = FBE_STATUS_OK;
            *subencl_id = subencl;
            break;

        }
    }

    return(status);
}

static fbe_status_t config_page_info_get_buf_id_by_subencl_info(vp_config_diag_page_info_t vp_config_diag_page_info,
                                                        ses_subencl_type_enum subencl_type,
                                                        terminator_eses_subencl_side side,
                                                        ses_buf_type_enum buf_type,
                                                        bool consider_writable,
                                                        uint8_t writable,
                                                        bool consider_buf_index,
                                                        uint8_t buf_index,
                                                        bool consider_buf_spec_info,
                                                        uint8_t buf_spec_info,
                                                        uint8_t *buf_id)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    uint8_t i, subencl_id;
    terminator_eses_config_page_info_t *config_page_info;
    terminator_eses_subencl_buf_desc_info_t *subencl_buf_desc_info;


    status = config_page_get_subencl_id(vp_config_diag_page_info, subencl_type, side, &subencl_id);
    if (status != FBE_STATUS_OK)
        return status;

    // reset status before searching for buffer.
    status = FBE_STATUS_GENERIC_FAILURE;

    config_page_info = vp_config_diag_page_info.config_page_info;
    // ***The below is based on assumption that subenclosure descs
    // in current config page are arranged consecutively by
    // Subenclosure ID. Only a very minor change is required if
    // this assumption no longer holds in future ***.
    subencl_buf_desc_info = &config_page_info->subencl_buf_desc_info[subencl_id];
    // Find the required buffer descriptor in all the buffer descriptors that
    // belong to a particular Subenclosure.
    for(i=0; i < subencl_buf_desc_info->num_buf_descs; i++)
    {
        if( (subencl_buf_desc_info->buf_desc[i].buf_type == buf_type) &&
            ((!consider_writable) || (subencl_buf_desc_info->buf_desc[i].writable == writable)) &&
            ((!consider_buf_index) || (subencl_buf_desc_info->buf_desc[i].buf_index == buf_index)) &&
            ((!consider_buf_spec_info) || (subencl_buf_desc_info->buf_desc[i].buf_spec_info == buf_spec_info)) )
        {
            *buf_id = subencl_buf_desc_info->buf_desc[i].buf_id;
            status = FBE_STATUS_OK;
            break;
        }
    }
    return(status);
}

static fbe_status_t eses_get_buf_id_by_subencl_info(terminator_vp_eses_page_info_t *eses_page_info,
                                            ses_subencl_type_enum subencl_type,
                                            uint8_t side,
                                            ses_buf_type_enum buf_type,
                                            bool consider_writable,
                                            uint8_t writable,
                                            bool consider_buf_index,
                                            uint8_t buf_index,
                                            bool consider_buf_spec_info,
                                            uint8_t buf_spec_info,
                                            uint8_t *buf_id)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;

    status = config_page_info_get_buf_id_by_subencl_info(eses_page_info->vp_config_diag_page_info,
                                                         subencl_type,
                                                         side,
                                                         buf_type,
                                                         consider_writable,
                                                         writable,
                                                         consider_buf_index,
                                                         buf_index,
                                                         consider_buf_spec_info,
                                                         buf_spec_info,
                                                         buf_id);
    return(status);
}

fbe_status_t emc_encl_stat_diag_page_build_trace_buffer_inf_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *trace_buffer_elem_start_ptr, uint8_t **trace_buffer_elem_end_ptr, uint8_t *num_trace_buffer_info_elem)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_trace_buf_info_elem_struct *trace_buffer_inf_elem_ptr = NULL; 
    terminator_vp_eses_page_info_t *vp_eses_page_info;
    terminator_sp_id_t spid;

    vp_eses_page_info = &s->eses_page_info;

    *num_trace_buffer_info_elem = TERMINATOR_EMC_PAGE_NUM_TRACE_BUF_INFO_ELEMS;

    trace_buffer_inf_elem_ptr = (ses_trace_buf_info_elem_struct *)(trace_buffer_elem_start_ptr);
    memset(trace_buffer_inf_elem_ptr, 0, sizeof(ses_trace_buf_info_elem_struct));

    status =  fbe_terminator_api_get_sp_id(s, &spid);

    status = eses_get_buf_id_by_subencl_info(vp_eses_page_info, 
                                             SES_SUBENCL_TYPE_LCC, 
                                             spid, 
                                             SES_BUF_TYPE_ACTIVE_TRACE, 
                                             FALSE,
                                             0,
                                             FALSE,
                                             0,
                                             FALSE,
                                             0, 
                                             &trace_buffer_inf_elem_ptr->buf_id);

    if (status != FBE_STATUS_OK)
        return status;
    
    trace_buffer_inf_elem_ptr->buf_action =  FBE_ESES_TRACE_BUF_ACTION_STATUS_ACTIVE;

    status = config_page_get_start_elem_index_in_stat_page(s,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid, 
                                                            SES_ELEM_TYPE_SAS_EXP, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &trace_buffer_inf_elem_ptr->elem_index);
    if (status != FBE_STATUS_OK)
        return status;
   
    trace_buffer_inf_elem_ptr++;
    memset(trace_buffer_inf_elem_ptr, 0, sizeof(ses_trace_buf_info_elem_struct));
    
    status = eses_get_buf_id_by_subencl_info(vp_eses_page_info, 
                                             SES_SUBENCL_TYPE_LCC, 
                                             spid, 
                                             SES_BUF_TYPE_SAVED_TRACE, 
                                             FALSE,
                                             0,
                                             FALSE,
                                             0,
                                             FALSE,
                                             0, 
                                             &trace_buffer_inf_elem_ptr->buf_id);
    if (status != FBE_STATUS_OK)
        return status;
    
    trace_buffer_inf_elem_ptr->buf_action =  FBE_ESES_TRACE_BUF_ACTION_STATUS_CLIENT_INIT_SAVE_BUF;


    status = config_page_get_start_elem_index_in_stat_page(s,
                                                            SES_SUBENCL_TYPE_LCC,
                                                            spid, 
                                                            SES_ELEM_TYPE_SAS_EXP, 
                                                            FALSE,
                                                            0,
                                                            FALSE,
                                                            NULL,
                                                            &trace_buffer_inf_elem_ptr->elem_index);
   if (status != FBE_STATUS_OK) 
       return status;


    *trace_buffer_elem_end_ptr = (uint8_t *)(trace_buffer_inf_elem_ptr+1);

    return( FBE_STATUS_OK);          
}

fbe_status_t emc_encl_stat_diag_page_build_ps_info_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *ps_info_elem_start_ptr,
                                                         uint8_t                  **ps_info_elem_end_ptr,
                                                         uint8_t                   *num_ps_info_elem)
{
    fbe_status_t                    status         = FBE_STATUS_GENERIC_FAILURE;
    ses_ps_info_elem_struct        *ps_info_elem_p = NULL;
    ses_ps_info_elem_struct         terminatorEmcPsInfoStatus;
    terminator_sp_id_t              spid;
    fbe_u8_t index = 0;

    *num_ps_info_elem = 0;
    if (s->enclosure_type == FBE_SAS_ENCLOSURE_TYPE_TABASCO)
    {

        ps_info_elem_p        = (ses_ps_info_elem_struct *)ps_info_elem_start_ptr;

        status = sas_virtual_phy_get_emcPsInfoStatus(s, &terminatorEmcPsInfoStatus);
        RETURN_ON_ERROR_STATUS;

        status =  fbe_terminator_api_get_sp_id(s, &spid);
        RETURN_ON_ERROR_STATUS;

        status = config_page_get_start_elem_index_in_stat_page(s,
                                                                SES_SUBENCL_TYPE_PS,
                                                                spid,
                                                                SES_ELEM_TYPE_PS,
                                                                FALSE,
                                                                0,
                                                                FALSE,
                                                                NULL,
                                                                &index);
        RETURN_ON_ERROR_STATUS;


        status =  fbe_terminator_api_get_sp_id(s, &spid);
        if (status != FBE_STATUS_OK)
            return status;

        if (status == FBE_STATUS_OK)
        {
            *num_ps_info_elem += 1;
            memset(ps_info_elem_p, 0, sizeof(ses_ps_info_elem_struct));
            ps_info_elem_p->ps_elem_index = index;
            ps_info_elem_p->margining_test_mode    = terminatorEmcPsInfoStatus.margining_test_mode;
            ps_info_elem_p->margining_test_results = terminatorEmcPsInfoStatus.margining_test_results;
            ps_info_elem_p ++;

        }else if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
        {
            return status;
        }

        status = config_page_get_start_elem_index_in_stat_page(s,
                                                               SES_SUBENCL_TYPE_PS,
                                                               1 - spid,
                                                               SES_ELEM_TYPE_PS,
                                                               FALSE,
                                                               0,
                                                               FALSE,
                                                               NULL,
                                                               &index);
       RETURN_ON_ERROR_STATUS;



       if (status == FBE_STATUS_OK)
       {
           *num_ps_info_elem += 1;
           memset(ps_info_elem_p, 0, sizeof(ses_ps_info_elem_struct));
           ps_info_elem_p->ps_elem_index = index;
           ps_info_elem_p->margining_test_mode    = terminatorEmcPsInfoStatus.margining_test_mode;
           ps_info_elem_p->margining_test_results = terminatorEmcPsInfoStatus.margining_test_results;
           ps_info_elem_p ++;

       }else if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
       {
       // some enclosures (DPE) will not report PS data, so return no fault
           return status;
       }
    }

    *ps_info_elem_end_ptr = (uint8_t*)ps_info_elem_p;
    return FBE_STATUS_OK;
}
fbe_status_t emc_encl_stat_diag_page_build_general_info_temperature_sensor_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *general_info_elem_start_ptr, uint8_t **general_info_elem_end_ptr, uint8_t *num_general_info_elem)
{
    int i;
    fbe_u8_t index = 0;
    int element_count = 0;
    terminator_sp_id_t spid;
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_general_info_elem_temperature_sensor_struct* temp_info_p = NULL;
    temp_info_p = (ses_general_info_elem_temperature_sensor_struct*) (general_info_elem_start_ptr);

    status =  fbe_terminator_api_get_sp_id(s, &spid);
    if (s->enclosure_type == FBE_SAS_ENCLOSURE_TYPE_TABASCO)
    {
        int j;
        struct args_for_index
        {
            ses_subencl_type_enum subencl_type;
            terminator_eses_subencl_side side;
            fbe_u8_t num_possible_elems;
        } args[] =
        {
           {SES_SUBENCL_TYPE_LCC,  0, 2},
           {SES_SUBENCL_TYPE_CHASSIS, SIDE_UNDEFINED, 7},
           {SES_SUBENCL_TYPE_PS, 0, 2},
           {SES_SUBENCL_TYPE_PS, 0, 2}
        };


        args[0].side = spid;
        args[2].side = spid;
        args[3].side = 1-spid;

        for ( j = 0; j < sizeof(args) /  sizeof(args[0]); ++j)
        {
            status = config_page_get_start_elem_index_in_stat_page(s,
                                                                    args[j].subencl_type,
                                                                    args[j].side,
                                                                    SES_ELEM_TYPE_TEMP_SENSOR,
                                                                    TRUE,
                                                                    args[j].num_possible_elems,
                                                                    FALSE,
                                                                    NULL,
                                                                    &index);
            if (status == FBE_STATUS_OK)
            {
                element_count += args[j].num_possible_elems;
                for ( i = 0; i < args[j].num_possible_elems; i++)
                {
                    temp_info_p->elem_index = i + index;
                    temp_info_p->fru = 0;
                    temp_info_p->temperature = bswap16(4431);
                    temp_info_p->valid = 1;
                    temp_info_p += 1;
                }
                continue;
            }
            if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
            {
                return status;
            }
        }
    }
    *num_general_info_elem = element_count;
    *general_info_elem_end_ptr = (uint8_t*)temp_info_p;
    return status;
}

fbe_status_t emc_encl_stat_diag_page_build_general_info_enclosure_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *general_info_elem_start_ptr, uint8_t **general_info_elem_end_ptr, uint8_t *num_general_info_elem)
{
    fbe_u8_t elem_index = 0;
    int element_count = 0;
    terminator_sp_id_t spid;
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_general_info_elem_enclosure_struct* temp_info_p = (ses_general_info_elem_enclosure_struct*) (general_info_elem_start_ptr);

        if (status != FBE_STATUS_OK)
            return status;

    status =  fbe_terminator_api_get_sp_id(s, &spid);
    if (s->enclosure_type == FBE_SAS_ENCLOSURE_TYPE_TABASCO)
    {
       int j;
       struct args_for_index
       {
           ses_subencl_type_enum subencl_type;
           terminator_eses_subencl_side side;
           fbe_u8_t num_possible_elems;
       } args[] =
       {
          {SES_SUBENCL_TYPE_LCC,     SIDE_UNDEFINED, 1},
          {SES_SUBENCL_TYPE_LCC,     SIDE_UNDEFINED, 1},
          {SES_SUBENCL_TYPE_CHASSIS, SIDE_UNDEFINED, 1}
       };

       args[0].side = spid;
       args[1].side = 1-spid;


       for ( j = 0; j < sizeof(args) /  sizeof(args[0]); ++j)
       {
           status = config_page_get_start_elem_index_in_stat_page(s,
                                                                   args[j].subencl_type,
                                                                   args[j].side,
                                                                   SES_ELEM_TYPE_ENCL,
                                                                   TRUE,
                                                                   args[j].num_possible_elems,
                                                                   FALSE,
                                                                   NULL,
                                                                   &elem_index);
           if (status == FBE_STATUS_OK)
           {
               element_count += 1;
               temp_info_p->elem_index = elem_index;
               temp_info_p->fru = 0;
               temp_info_p->resume_from_fault = 0;
               temp_info_p->usb_led = 1;
               temp_info_p += 1;
               continue;
           }
           if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
           {
               return status;
           }
       }
    }
    *num_general_info_elem = element_count;
    *general_info_elem_end_ptr = (uint8_t*)temp_info_p;
    return status;
}
fbe_status_t emc_encl_stat_diag_page_build_general_info_esc_elec_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *general_info_elem_start_ptr, uint8_t **general_info_elem_end_ptr, uint8_t *num_general_info_elem)
{
    fbe_u8_t elem_index = 0;
    int element_count = 0;
    terminator_sp_id_t spid;
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_general_info_elem_esc_struct* temp_info_p = (ses_general_info_elem_esc_struct*) (general_info_elem_start_ptr);


    status =  fbe_terminator_api_get_sp_id(s, &spid);
    if (s->enclosure_type == FBE_SAS_ENCLOSURE_TYPE_TABASCO)
    {
       int j;
       struct args_for_index
       {
           ses_subencl_type_enum subencl_type;
           terminator_eses_subencl_side side;
           fbe_u8_t num_possible_elems;
       } args[] =
       {
          {SES_SUBENCL_TYPE_LCC,     SIDE_UNDEFINED, 1},
          {SES_SUBENCL_TYPE_LCC,     SIDE_UNDEFINED, 1},
       };

       args[0].side = spid;
       args[1].side = 1-spid;


       for ( j = 0; j < sizeof(args) /  sizeof(args[0]); ++j)
       {
           status = config_page_get_start_elem_index_in_stat_page(s,
                                                                   args[j].subencl_type,
                                                                   args[j].side,
                                                                   SES_ELEM_TYPE_ESC_ELEC,
                                                                   TRUE,
                                                                   args[j].num_possible_elems,
                                                                   FALSE,
                                                                   NULL,
                                                                   &elem_index);
           if (status == FBE_STATUS_OK)
           {
               element_count += 1;
               temp_info_p->elem_index = elem_index;
               temp_info_p->fru = 1;
               temp_info_p->drive_ecb_fault = 0;
               temp_info_p->sxp_eeprom_valid_rslt = 0;
               temp_info_p->primary_cdef_fault = 0;
               temp_info_p->secondary_cdef_fault = 0;
               temp_info_p->twi_fault = 0;
               temp_info_p += 1;
               continue;
           }
           if (status != FBE_STATUS_COMPONENT_NOT_FOUND)
           {
               return status;
           }
       }
    }
    *num_general_info_elem = element_count;
    *general_info_elem_end_ptr = (uint8_t*)temp_info_p;
    return status;
}

fbe_status_t emc_encl_stat_diag_page_build_general_info_expander_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *general_info_elem_start_ptr, uint8_t **general_info_elem_end_ptr, uint8_t *num_general_info_elem)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_general_info_elem_expander_struct * general_info_elem_p = NULL;
    //terminator_vp_eses_page_info_t *vp_eses_page_info;
    uint8_t reset_reason  = 0;
    terminator_sp_id_t spid;

    //vp_eses_page_info = &s->eses_page_info;

    *num_general_info_elem = 1;

    general_info_elem_p = (ses_general_info_elem_expander_struct *)(general_info_elem_start_ptr);
    memset(general_info_elem_p, 0, sizeof(ses_general_info_elem_expander_struct));

    general_info_elem_p->reset_reason =  reset_reason;
    status =  fbe_terminator_api_get_sp_id(s, &spid);

    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_SAS_EXP, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &general_info_elem_p->elem_index);
    if (status != FBE_STATUS_OK)
        return status;


    *general_info_elem_end_ptr = (uint8_t *)(general_info_elem_p+1); // next element

    return( FBE_STATUS_OK);          
}

fbe_status_t emc_encl_stat_diag_page_build_general_info_drive_slot_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *general_info_elem_start_ptr, uint8_t **general_info_elem_end_ptr, uint8_t *num_general_info_elem)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_general_info_elem_array_dev_slot_struct * general_info_elem_p = NULL;
    ses_general_info_elem_array_dev_slot_struct  terminatorEmcGeneralInfoDriveSlotStatus = {0};
     terminator_sp_id_t spid;
    fbe_sas_enclosure_type_t    encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    uint8_t  max_drive_slots=0;
    uint32_t  driveIndex;
    uint8_t  driveStartElemIndex=0;

    status = terminator_virtual_phy_get_enclosure_type(s, &encl_type);
    if (status != FBE_STATUS_OK)
        return status;

    // DRIVE SLOTS
    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots, s->side);
    if (status != FBE_STATUS_OK)
        return status;

    *num_general_info_elem = max_drive_slots;

    status =  fbe_terminator_api_get_sp_id(s, &spid);
    if (status != FBE_STATUS_OK)
        return status;

    status = config_page_get_start_elem_index_in_stat_page(s,
                                                           SES_SUBENCL_TYPE_LCC,
                                                           spid, 
                                                           SES_ELEM_TYPE_ARRAY_DEV_SLOT, 
                                                           FALSE,
                                                           0,
                                                           FALSE,
                                                           NULL,
                                                           &driveStartElemIndex);
    // some enclosures will not report  data, so return no fault
    if (status == FBE_STATUS_COMPONENT_NOT_FOUND)
    {
        return FBE_STATUS_OK;
    }

    if (status != FBE_STATUS_OK)
        return status;

    for (driveIndex = 0; driveIndex < max_drive_slots; driveIndex++)
    {
        general_info_elem_p = (ses_general_info_elem_array_dev_slot_struct *)(general_info_elem_start_ptr) + driveIndex;
        memset(general_info_elem_p, 0, sizeof(ses_general_info_elem_array_dev_slot_struct));

        status = sas_virtual_phy_get_emcGeneralInfoDirveSlotStatus(s, &terminatorEmcGeneralInfoDriveSlotStatus, driveIndex);
        if(status != FBE_STATUS_OK)
        {
           return(status);
        }
        general_info_elem_p->battery_backed = terminatorEmcGeneralInfoDriveSlotStatus.battery_backed;
        general_info_elem_p->elem_index = driveStartElemIndex+driveIndex;    

        *general_info_elem_end_ptr = (uint8_t *)(general_info_elem_p+1);   // next element
    }


    return FBE_STATUS_OK;          
}

fbe_status_t fbe_terminator_sas_enclosure_get_eses_version_desc(fbe_sas_enclosure_type_t encl_type, uint16_t *eses_version_desc_ptr)
{
    terminator_sas_encl_inq_data_t  *inq_data = NULL;

    /*get the correct inq datd for this encl type*/
    sas_enclosure_get_inq_data (encl_type, &inq_data);

    *eses_version_desc_ptr = bswap16(inq_data->eses_version_descriptor);

    return FBE_STATUS_OK;
}

fbe_status_t terminator_initialize_eses_page_info(fbe_sas_enclosure_type_t encl_type, terminator_vp_eses_page_info_t *eses_page_info)
{
    return(eses_initialize_eses_page_info(encl_type, eses_page_info));
}

#ifdef _TEST_MAIN
void main(void)
{
   fbe_status_t status = config_page_init();
}
#endif
