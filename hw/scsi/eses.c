#ifndef _ESES_H
#define _ESES_H

#include <stddef.h>
#include <stdint.h>
#include <string.h>

// TODO
#define CSX_ASSERT_AT_COMPILE_TIME_GLOBAL_SCOPE(x)

#if !defined (SIZE_CHECK)
#define SIZE_CHECK(name, length) CSX_ASSERT_AT_COMPILE_TIME_GLOBAL_SCOPE((sizeof(name)) == length)
#endif

typedef uint8_t fbe_u8_t;
typedef char fbe_char_t;
typedef uint16_t fbe_u16_t;
typedef uint32_t fbe_u32_t;
typedef fbe_u32_t fbe_boot_t;

#define FBE_TRUE 1
#define FBE_FALSE 0
#define FBE_IS_TRUE(val) ((val) == FBE_TRUE)
#define FBE_IS_FALSE(val) ((val) == FBE_FALSE)

typedef fbe_u8_t PAD;
typedef char CHAR;

#define CONFIG_PAGE_MAX_SUB_ENCLS 15
#define CONFIG_PAGE_MAX_TYPE_DESC_HEADERS 40
#define CONFIG_PAGE_MAX_VER_DESCS 25
#define MAX_BUF_DESC_PER_SUBENCL 20

#define FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_A1             "Power Supply A1"
#define FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_B1             "Power Supply B1"
#define FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_A0             "Power Supply A0"
#define FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_B0             "Power Supply B0"
#define FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_A              "Power Supply A"
#define FBE_ESES_SUBENCL_TEXT_POWER_SUPPLY_B              "Power Supply B"


typedef enum terminator_sp_id_e{
    TERMINATOR_SP_A,
    TERMINATOR_SP_B
} terminator_sp_id_t;

/****************************************************************************/
/** Subenclosure type field in \ref ses_subencl_desc_struct "subenclosure descriptor"
 *  in the \ref ses_pg_config_struct "Configuration diagnostic page".
 ****************************************************************************/

typedef enum ses_subencl_type_enum {
    SES_SUBENCL_TYPE_PS      = 0x02, ///< Type 02h, Power Supply
    SES_SUBENCL_TYPE_COOLING = 0x03, ///< Type 03h, Cooling
    SES_SUBENCL_TYPE_LCC     = 0x07, ///< Type 07h, LCC
    SES_SUBENCL_TYPE_UPS     = 0x0B, ///< Type 0Bh, UPS
    SES_SUBENCL_TYPE_CHASSIS = 0x0E, ///< Type 0Eh, Chassis
    SES_SUBENCL_TYPE_INVALID = 0xFF  /// 
} ses_subencl_type_enum;

typedef enum fbe_status_e{
    FBE_STATUS_INVALID = 0,
    FBE_STATUS_OK,
    FBE_STATUS_MORE_PROCESSING_REQUIRED,
    FBE_STATUS_PENDING,
    FBE_STATUS_CANCEL_PENDING,
    FBE_STATUS_CANCELED,
    FBE_STATUS_TIMEOUT,
    FBE_STATUS_INSUFFICIENT_RESOURCES,
    FBE_STATUS_NO_DEVICE,
    FBE_STATUS_NO_OBJECT,

    FBE_STATUS_SERVICE_MODE, /* used for database enter service mode */

    FBE_STATUS_CONTINUE, /* May be used for edge tap functianality ONLY */

    FBE_STATUS_BUSY,   /* The object is in activate or activate pending state */
    FBE_STATUS_SLUMBER,/* The object is in hibernate or hibernate pending state */
    FBE_STATUS_FAILED, /* The object is in fail or fail pending state */
    FBE_STATUS_DEAD,   /* The object is in destroy or destroy pending state */

    FBE_STATUS_NOT_INITIALIZED,
    FBE_STATUS_TRAVERSE, /* Base object returns this status when packet needs to be traversed */

    FBE_STATUS_EDGE_NOT_ENABLED, /* This status returned when packet sent to not enabled edge */
    FBE_STATUS_COMPONENT_NOT_FOUND,
    FBE_STATUS_ATTRIBUTE_NOT_FOUND,
    FBE_STATUS_HIBERNATION_EXIT_PENDING,/*this packet is an IO waking up the object, don't drain it*/
    FBE_STATUS_IO_FAILED_RETRYABLE,
    FBE_STATUS_IO_FAILED_NOT_RETRYABLE,

    FBE_STATUS_COMPLETE_JOB_DESTROY, /* This is used by Job service to complete the packet during queue destroy */
    FBE_STATUS_QUIESCED, /* Used by block transport server to error monitor ops that come to a held server. */
    FBE_STATUS_GENERIC_FAILURE,
    FBE_STATUS_NO_ACTION /* This will indicate that requested action is already performed */
} fbe_status_t;


/*
 *  constants related to the pages
 */
typedef enum fbe_eses_const_e
{   
    /* Diplay element */
    FBE_ESES_NUM_ELEMS_PER_TWO_DIGIT_DISPLAY = 2, 
    FBE_ESES_NUM_ELEMS_PER_ONE_DIGIT_DISPLAY = 1,     
    /* All the pages with the common header. */
    FBE_ESES_PAGE_LENGTH_OFFSET = 3,
    FBE_ESES_PAGE_SIZE_ADJUST   = 4,  // num bytes:page code + subencl id + page length
    FBE_ESES_PAGE_HEADER_SIZE   = 8,
    FBE_ESES_PAGE_MAX_SIZE      = 2500,/*changed on 10/11/11 by Chang, Rui.*/
    FBE_ESES_GENERATION_CODE_SIZE = 4,
    /*status/control page*/
    FBE_ESES_CTRL_STAT_ELEM_SIZE =  4,
    /*status page*/
    ESES_ENCLOSURE_STATUS_PAGE_MAX_SIZE = 400,
    FBE_ESES_ENTIRE_CONNECTOR_PHYSICAL_LINK = 0xFF,
    /*additional status page*/
    FBE_ESES_ADDL_ELEM_STATUS_DESC_HDR_SIZE = 4,
    FBE_ESES_ARRAY_DEV_SLOT_PROT_SPEC_INFO_HEADER_SIZE = 4,
    FBE_ESES_SAS_EXP_PROT_SPEC_INFO_HEADER_SIZE = 12,
    FBE_ESES_ESC_ELEC_PROT_SPEC_INFO_HEADER_SIZE = 4,
    ESES_ENCLOSURE_ADDL_STATUS_PAGE_MAX_SIZE = 950,

    /*EMC Enclosure status/control page */
    FBE_ESES_EMC_CTRL_STAT_FIRST_INFO_ELEM_GROUP_OFFSET = 12,
    FBE_ESES_EMC_CONTROL_SAS_CONN_INFO_ELEM_SIZE = 14,
    FBE_ESES_EMC_CONTROL_TRACE_BUF_INFO_ELEM_SIZE = 29,
    FBE_ESES_EMC_CONTROL_ENCL_TIME_INFO_ELEM_SIZE = 8,
    FBE_ESES_EMC_CONTROL_GENERAL_INFO_ELEM_SIZE = 4,
    FBE_ESES_EMC_CTRL_STAT_PS_INFO_ELEM_SIZE = 5,
    FBE_ESES_EMC_CTRL_STAT_SPS_INFO_ELEM_SIZE = 6,
    FBE_ESES_EMC_CTRL_STAT_ENCL_TIME_ZONE_UNSPECIFIED = 96,
    FBE_ESES_EMC_CTRL_STAT_INFO_ELEM_GROUP_HEADER_SIZE = 3,
#if 0
    /*EMC Enclosure status/control page */
    FBE_ESES_EMC_CTRL_STAT_FIRST_INFO_ELEM_GROUP_OFFSET = 12,
    FBE_ESES_EMC_CTRL_STAT_SAS_CONN_INFO_ELEM_SIZE = 13,
    FBE_ESES_EMC_CTRL_STAT_TRACE_BUF_INFO_ELEM_SIZE = 29,
    FBE_ESES_EMC_CTRL_STAT_ENCL_TIME_INFO_ELEM_SIZE = 8,
    FBE_ESES_EMC_CTRL_STAT_GENERAL_INFO_ELEM_SIZE = 4,
    FBE_ESES_EMC_CTRL_STAT_PS_INFO_ELEM_SIZE = 5,
    FBE_ESES_EMC_CTRL_STAT_ENCL_TIME_ZONE_UNSPECIFIED = 96,
    FBE_ESES_EMC_CTRL_STAT_INFO_ELEM_GROUP_HEADER_SIZE = 3,
#endif
    FBE_ESES_DRIVE_POWER_CYCLE_DURATION_MAX     = 127,  // 7 bits.
    FBE_ESES_DRIVE_POWER_CYCLE_DURATION_DEFAULT = 0,  // The duration for the power cycle in 0.5 second increments.  
                                                      // A value of 0 will result in the default time of 5 seconds.  
    /*download control page*/
    FBE_ESES_DL_CONTROL_UCODE_DATA_LENGTH_OFFSET =  23, // last byte (offset) before image data begins
    FBE_ESES_DL_CONTROL_UCODE_DATA_OFFSET       =   24, // offset where the image data begins
    FBE_ESES_DL_CONTROL_MAX_PAGE_LENGTH         = 4096, // max page length in the send diag page
    FBE_ESES_DL_CONTROL_MAX_UCODE_DATA_LENGTH   = 4072, // max ucode data length in the send diag page
    FBE_ESES_MICROCODE_STATUS_PAGE_MAX_SIZE     =   23, // max page length for microcode status page
    FBE_ESES_DL_CONTROL_TUNNEL_UCODE_DATA_OFFSET = 46,        // offset where the image data begins
    FBE_ESES_DL_CONTROL_MAX_TUNNEL_UCODE_DATA_LENGTH = 2000,  // max ucode data length in the tunnel send diag page
    FBE_ESES_DL_CONTROL_TUNNEL_GET_DOWNLOAD_STATUS_PG_LEN = 22, // Size of the tunneled receive diag status page. 

    /* EMC Statistics status/control page size */
    FBE_ESES_EMC_STATS_CTRL_STAT_COMM_FIELD_LENGTH          = 2,
    FBE_ESES_EMC_STATS_CTRL_STAT_FIRST_ELEM_OFFSET          = 8,

    /*config page*/
    FBE_ESES_SUBENCL_DESC_LENGTH_OFFSET  = 3,
    FBE_ESES_VER_DESC_SIZE               = 20,
    FBE_ESES_VER_DESC_REVISION_SIZE      = 5,
    FBE_ESES_VER_DESC_IDENTIFIER_SIZE    = 12,
    FBE_ESES_BUF_DESC_SIZE               = 4,
    FBE_ESES_ELEM_TYPE_DESC_HEADER_SIZE  = 4,
    FBE_ESES_ELEM_TYPE_NAME_MAX_LEN      = 23,
    FBE_ESES_CONFIG_SUBENCL_PRODUCT_ID_SIZE  = 16,

    /* FW */
    FBE_ESES_FW_REVISION_1_0_SIZE  = 5,
    FBE_ESES_FW_REVISION_2_0_SIZE  = 16,
    FBE_ESES_FW_REVISION_SIZE      = 16,        // this should be max value of the various CDES-x FW Rev size

    FBE_ESES_FW_IDENTIFIER_SIZE    = 12,

    /* CDES-1 Microcode file format */
    FBE_ESES_MCODE_IMAGE_HEADER_SIZE_OFFSET      = 0,
    FBE_ESES_MCODE_IMAGE_HEADER_SIZE_BYTES       = 4,

    FBE_ESES_MCODE_IMAGE_SIZE_OFFSET             = 16,
    FBE_ESES_MCODE_IMAGE_SIZE_BYTES              = 4,

    FBE_ESES_MCODE_IMAGE_COMPONENT_TYPE_OFFSET   = 24,
    FBE_ESES_MCODE_IMAGE_COMPONENT_TYPE_SIZE_BYTES   = 1,
     
    FBE_ESES_MCODE_IMAGE_REV_OFFSET              = 40,
    FBE_ESES_MCODE_IMAGE_REV_SIZE_BYTES          = 5,

    FBE_ESES_MCODE_NUM_SUBENCL_PRODUCT_ID_OFFSET = 69,
    FBE_ESES_MCODE_SUBENCL_PRODUCT_ID_OFFSET     = 70,

    /* CDES-2 Microcode file format */
    FBE_ESES_CDES2_MCODE_IMAGE_HEADER_SIZE_OFFSET  = 8,
    FBE_ESES_CDES2_MCODE_IMAGE_HEADER_SIZE_BYTES   = 4,

    FBE_ESES_CDES2_MCODE_DATA_SIZE_OFFSET  = 20,
    FBE_ESES_CDES2_MCODE_DATA_SIZE_BYTES   = 4,

    FBE_ESES_CDES2_MCODE_IMAGE_REV_OFFSET          = 48,
    /* The image rev size is actually 32 bytes. But we only use 16 bytes.*/
    FBE_ESES_CDES2_MCODE_IMAGE_REV_SIZE_BYTES      = 16,

    FBE_ESES_CDES2_MCODE_IMAGE_COMPONENT_TYPE_OFFSET   = 12,
    FBE_ESES_CDES2_MCODE_IMAGE_COMPONENT_TYPE_SIZE_BYTES   = 4,

    FBE_ESES_CDES2_MCODE_NUM_SUBENCL_PRODUCT_ID_OFFSET = 104,
    FBE_ESES_CDES2_MCODE_SUBENCL_PRODUCT_ID_OFFSET     = 108,

    /* string out page */
    FBE_ESES_STR_OUT_PAGE_HEADER_SIZE    = 4,
    FBE_ESES_STR_OUT_PAGE_STR_OUT_DATA_MAX_LEN = 80,
    FBE_ESES_STR_OUT_PAGE_DEBUG_CMD_NUM_ELEMS_PER_BYTE = 4,
    FBE_ESES_STR_OUT_PAGE_DEBUG_CMD_NUM_BITS_PER_ELEM = 2,

    /* string in page */
    FBE_ESES_STR_IN_PAGE_HEADER_SIZE    = 4,

    /* threshold in/out page */
    FBE_ESES_THRESH_IN_PAGE_OVERALL_STATUS_ELEM_SIZE    = 4,    
    FBE_ESES_THRESH_IN_PAGE_INDIVIDUAL_STATUS_ELEM_SIZE    = 4,
    FBE_ESES_THRESH_IN_PAGE_STATUS_ELEM_SIZE    = 4,
    FBE_ESES_THRESHOLD_OUT_PAGE_OVERALL_CONTROL_ELEMENT_SIZE    = 4,
    FBE_ESES_THRESHOLD_OUT_PAGE_INDIVIDUAL_CONTROL_ELEMENT_SIZE    = 4,
    FBE_ESES_TEMP_THRESHOLD_OFFSET = 20,
    
    /* mode parameter list */
    FBE_ESES_MODE_PARAM_LIST_HEADER_SIZE = 8,
    FBE_ESES_MODE_PAGE_COMMON_HEADER_SIZE = 2,
    /* EMC ESES persistent mode page */
    FBE_ESES_EMC_ESES_PERSISTENT_MODE_PAGE_SIZE = 16,
    FBE_ESES_EMC_ESES_PERSISTENT_MODE_PAGE_LEN  = 0x0E, // The number of bytes to following the page lenth The page length is different from page size. 
                                                        // It does not include the bytes before the page_length bytes and the page_length bytes.
    /* EMC ESES non persistent mode page */
    FBE_ESES_EMC_ESES_NON_PERSISTENT_MODE_PAGE_SIZE = 16,
    FBE_ESES_EMC_ESES_NON_PERSISTENT_MODE_PAGE_LEN  = 0x0E, // The page length is different from page size. 
                                                            // It does not include the bytes before the page_length bytes and the page_length bytes.
    /* Read Buffer */
    FBE_ESES_READ_BUF_DESC_SIZE  = 4,
    /* Trace Buffer */
    FBE_ESES_TRACE_BUF_MAX_SIZE = 12288,   // 12KB.

    FBE_ESES_TRACE_BUF_MIN_TRANSFER_SIZE = 2048, //2KB.
    FBE_ESES_TRACE_BUF_MAX_TRANSFER_SIZE = 4096, //4KB.

    /* Sense Data for Illegal Request */
    FBE_ESES_SENSE_DATA_ILLEGAL_REQUEST_START_OFFSET = 15,
    FBE_ESES_SENSE_DATA_ILLEGAL_REQUEST_FILED_POINTER_FIRST_BYTE_OFFSET = 16,
    FBE_ESES_SENSE_DATA_ILLEGAL_REQUEST_FILED_POINTER_SECOND_BYTE_OFFSET = 17,
} fbe_eses_const_t;

/******************************************************************************//**
 * Supported SES page codes.
 *
 * 00h - 0Fh Standard SES pages          <br>
 * 10h - 1Fh Vendor-specific (EMC) pages <br>
 * 20h - 2Fh Reserved for SES-2          <br>
 * 80h - FFh Vendor-specific for SPC-4 
 *******************************************************************************/
typedef enum {
    // Standard SES pages 0x00 - 0x0F
    SES_PG_CODE_SUPPORTED_DIAGS_PGS         = 0x00, ///< 00h
    SES_PG_CODE_CONFIG                      = 0x01, ///< 01h
    SES_PG_CODE_ENCL_CTRL                   = 0x02, ///< 02h
    SES_PG_CODE_ENCL_STAT                   = 0x02, ///< 02h
    SES_PG_CODE_HELP_TEXT                   = 0x03, ///< 03h
    SES_PG_CODE_STR_IN                      = 0x04, ///< 04h
    SES_PG_CODE_STR_OUT                     = 0x04, ///< 04h
    SES_PG_CODE_THRESHOLD_IN                = 0x05, ///< 05h
    SES_PG_CODE_THRESHOLD_OUT               = 0x05, ///< 05h
    SES_PG_CODE_ELEM_DESC                   = 0x07, ///< 07h
    SES_PG_CODE_ENCL_BUSY                   = 0x09, ///< 09h
    SES_PG_CODE_ADDL_ELEM_STAT              = 0x0A, ///< 0Ah
    SES_PG_CODE_DOWNLOAD_MICROCODE_CTRL     = 0x0E, ///< 0Eh
    SES_PG_CODE_DOWNLOAD_MICROCODE_STAT     = 0x0E, ///< 0Eh
    // Vendor-specific SES pages (EMC) 0x10-0x1E
    SES_PG_CODE_EMC_ENCL_CTRL               = 0x10, ///< 10h
    SES_PG_CODE_EMC_ENCL_STAT               = 0x10, ///< 10h
    SES_PG_CODE_EMC_STATS_CTRL              = 0x11, ///< 11h
    SES_PG_CODE_EMC_STATS_STAT              = 0x11, ///< 11h
    //                                        0x12-0x1F available
    SES_PG_CODE_EMC_ESES_PLATFORM_SPECIFIC  = 0x1F, ///< 1Fh
    // Reserved for SES-2              0x20 - 0x2F
    SES_PG_CODE_EMC_ESES_PERSISTENT_MODE_PG = 0x20,
    SES_PG_CODE_EMC_ESES_NON_PERSISTENT_MODE_PG = 0x21,
    SES_PG_CODE_ALL_SUPPORTED_MODE_PGS      = 0x3F,
    // Vendor-specific SPC-4 pages     0x80 - 0xFF
    SES_PG_CODE_EMM_SYS_LOG_PG              = 0x80, ///< 80h PMC
    SES_PG_CODE_DELAY_TEST_PG               = 0x81, ///< 81h PMC
    SES_PG_CODE_FW_STAT_PG                  = 0x82, ///< 82h PMC
    SES_PG_CODE_TUNNEL_DIAG_CTRL            = 0x83, ///< 83h PMC
    SES_PG_CODE_TUNNEL_DIAG_STATUS          = 0x83, ///< 83h PMC

    /// The maximum page code for status pages we store persistently
    SES_PG_CODE_INVALID                     = 0xFF
} ses_pg_code_enum;

static terminator_sp_id_t terminator_sp_id = TERMINATOR_SP_A;

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

#pragma pack(push, 1)
   
/**  SES page header.
 * This describes the first 8 bytes common to most SES pages, used for functions
 * that process this common information.
 ****************************************************************************/
typedef struct {
    fbe_u8_t pg_code            : 8; ///< Byte 0,   the page code, \ref ses_pg_code_enum
    PAD                      : 8; ///< Byte 1,   page code-specific field
    fbe_u16_t pg_len;              ///< Byte 2-3, length of rest of page
    fbe_u32_t gen_code;            ///< Byte 4-7, [expected] generation code
                                            ///<           (not used in all pages)
} ses_common_pg_hdr_struct;

SIZE_CHECK(ses_common_pg_hdr_struct, 8);

/*****************************************************************************/ 
/** NAA identifier for a piece of hardware, used in SES and other places.
 * The value is specific to the vendor of the hardware.
 * For EMC, the values are defined below.  
 *****************************************************************************/

typedef struct {
    fbe_u8_t vendor_specific_id_A_high : 4;   ///< Byte 0, bits 0-3,      always 0x2
    fbe_u8_t NAA                       : 4;   ///< Byte 0, bits 4-7 
    fbe_u8_t vendor_specific_id_A_low;        ///< Byte 1
    fbe_u8_t IEEE_company_id[3];              ///<     Byte 2-4,              0x000097 for EMC.      
    fbe_u8_t vendor_specific_id_B[3];         ///< Byte 5-7,              zero if hardware not present.
} naa_id_struct;

/*****************************************************************************/
/** Subenclosure product revision level in the 
 *  \ref ses_subencl_desc_struct "subenclosure descriptor" in the
 *  \ref ses_pg_config_struct "Configuration diagnostic page".
 * This number contains 4 bytes in vendor-specific format.  If the subenclosure
 * contains a "main" downloadable image this is 4 ASCII hex digits representing a
 * 16-bit binary number.  Bits 15-0 of the number map to:
 *
 * bits 15-11 major number 0-31<br>
 * bits 10-4  minor number 00-99<br>
 * bits 3-0   debug version 0,a-o
 ****************************************************************************/
#define SES_SUBENCL_REV_LEVEL_DIGITS  4
typedef struct {
    fbe_u8_t digits[SES_SUBENCL_REV_LEVEL_DIGITS]; 
} ses_subencl_prod_rev_level_struct;

SIZE_CHECK(ses_subencl_prod_rev_level_struct, 4);

/****************************************************************************/
/** Version descriptor in \ref ses_subencl_desc_struct "subenclosure descriptor"
 * in the \ref ses_pg_config_struct "Configuration diagnostic page".
 * Each descriptor describes the version of some hardware or firmware in the
 * subenclosure, usually associated with a particular element.
 ****************************************************************************/

typedef struct
{
    CHAR   comp_rev_level[5];     ///< Byte 2-6
    CHAR   comp_id[12];           ///< Byte 7-18
} ses_cdes_1_0_revision_struct;

typedef struct
{
    CHAR   comp_rev_level[16];     ///< Byte 2-17
    CHAR   reserved;               ///< Byte 18
} ses_cdes_2_0_revision_struct;

typedef union
{
    ses_cdes_1_0_revision_struct    cdes_1_0_rev;
    ses_cdes_2_0_revision_struct    cdes_2_0_rev;
} ses_cdes_revision_union;

typedef struct {
    fbe_u8_t  elem_index         : 8; ///< Byte 0, element in \ref ses_pg_encl_stat_struct
                                                ///<   "Enclosure Status diagnostic page" to which
                                                ///<   this descriptor pertains
    fbe_u8_t  comp_type          : 5; ///< Byte 1,\ref ses_comp_type_enum bits 0-4
    fbe_u8_t  downloadable       : 1; ///< Byte 1, bit 5
    fbe_u8_t  updated            : 1; ///< Byte 1, bit 6
    fbe_u8_t  main               : 1; ///< Byte 1, bit 7
    ses_cdes_revision_union cdes_rev; ///< Bytes 2-18
    fbe_u8_t  buf_id;                 ///< Byte 19
} ses_ver_desc_struct;

SIZE_CHECK(ses_ver_desc_struct, 20);

/****************************************************************************/
/** Subenclosure descriptor in the \ref ses_pg_config_struct "Configuration diagnostic page".
 *
 * There is one of these in the page for each subenclosure, as reported by
 * \ref ses_pg_config_struct.num_secondary_subencls, plus 1 for the primary subenclosure.
 *
 * The descriptor is variable-length.
 * This structure defines only the first part of the descriptor.  The remainder contains
 * the following fields.  Comments below name the functions you can use to get pointers to
 * these fields.
 *
 * <pre>
    fbe_u8_t           num_buf_descs;                   // \ref fbe_eses_fbe_eses_get_ses_num_buf_descs_p()
    ses_buf_desc    buf_desc[num_buf_descs];         // \ref fbe_eses_get_ses_buf_desc_p()
    fbe_u8_t           num_vpd_pgs;                     // \ref fbe_eses_get_ses_num_vpd_pgs_p()
    fbe_u8_t           vpd_pg_list[num_vpd_pgs];        // \ref fbe_eses_get_ses_vpd_pg_list_p()
    fbe_u8_t           subencl_text_len;                // \ref fbe_eses_get_ses_subencl_text_len_p()
    CHAR            subencl_text[subencl_text_len];  // \ref fbe_eses_get_ses_subencl_text_p()
 * </pre>
 *
 * To get from one descriptor to the next one, use
 * \ref fbe_eses_get_next_ses_subencl_desc_p().
 *
 * This descriptor may appear as a placeholder in the page
 * even if the subenclosure is not present.
 * If not present, \ref subencl_vendor_id is blank (ASCII spaces). Otherwise, unless specified,
 * fields are valid even if the subenclosure is not present.
 ****************************************************************************/
typedef struct {
    fbe_u8_t num_encl_svc_procs    : 3; ///< Byte  0, bit 0-2
    PAD                         : 1; //   Byte  0, bit 3
    fbe_u8_t rel_encl_svcs_proc_id : 3; ///< Byte  0, bit 4-6
    PAD                         : 1; //   Byte  0, bit 7
    fbe_u8_t subencl_id            : 8; ///< Byte  1,           
    fbe_u8_t num_elem_type_desc_hdrs;   ///< Byte  2,          0 if not present
    fbe_u8_t subencl_desc_len;          ///< Byte  3,           
    naa_id_struct   subencl_log_id;            ///< Byte  4-11,       see \ref naa_id_struct
    CHAR  subencl_vendor_id[8];      ///< Byte 12-19,       all spaces if subenclosure not present
    CHAR  subencl_prod_id[16];       ///< Byte 20-35
    ses_subencl_prod_rev_level_struct subencl_prod_rev_level;     ///< Byte 36-39, all spaces if subenclosure not present
    fbe_u8_t subencl_type          : 8; ///< Byte 40,\ref ses_subencl_type_enum 
    fbe_u8_t side                  : 5; ///< Byte 41, bit 0-4, side number or 0x1F
    fbe_u8_t reserved              : 2; ///< Byte 41, bit 5-6, reserved
    fbe_u8_t FRU                   : 1; ///< Byte 41, bit 7,   subenclosure is a FRU
    CHAR  side_name;                 ///< Byte 42,          side name
    fbe_u8_t container_subencl_id;      ///< Byte 43,          same as \p subencl_id if no container
    fbe_u8_t manager_subencl_id;        ///< Byte 44,          same as \p subencl_id if no other manager
    fbe_u8_t peer_subencl_id;           ///< Byte 45,          0xFF if no peer
    fbe_u8_t encl_rel_subencl_uid;      ///< Byte 46,          unique ID across current EMAs 
    CHAR  subencl_ser_num[16];       ///< Byte 47-62,       undefined if not present
    fbe_u8_t num_ver_descs;             ///< Byte 63,           
    ///                                             Byte 64+,
    /// version descriptors.  There are \ref num_ver_descs of these, one for
    /// each version of hardware or software described.
    ses_ver_desc_struct ver_desc[1];               
    
 /* The remaining fields appear after the above array.  Use macros below
    to get pointers to these fields. 

    fbe_u8_t           num_buf_descs;
    ses_buf_desc    buf_desc[num_buf_descs];
    fbe_u8_t           num_vpd_pgs;
    fbe_u8_t           vpd_pg_list[num_vpd_pgs];
    fbe_u8_t           subencl_text_len;
    CHAR            subencl_text[subencl_text_len];
 */
    
} ses_subencl_desc_struct;

SIZE_CHECK(ses_subencl_desc_struct, 64+sizeof(ses_ver_desc_struct));


/****************************************************************************
 ****************************************************************************//**
 ** Configuration diagnostic page (01h).
 **
 ** This is a status page that returns the configuration of the enclosure,
 ** including a list of subenclosures and the types and numbers of elements within them.
 **
 ** This structure defines only the first part of the page up to the start of the first \ref
 ** ses_subencl_desc_struct "subenclosure descriptor". To get to each subsequent subenclosure
 ** descriptor, use \ref fbe_eses_get_next_ses_subencl_desc_p(). After all the subenclosure descriptors,
 ** the following appear:
 **
 ** <pre>
 **    ses_type_desc_hdr_struct type_desc_hdr[num_hdrs]; // Type descriptor headers
 **    CHAR                    type_desc_text[];        // Type descriptor texts
 ** </pre>
 **
 ** The value of <code>num_hdrs</code> can be determined by adding up the values of the \ref
 ** ses_subencl_desc_struct.num_elem_type_desc_hdrs "num_elem_type_desc_hdrs" in all the \ref
 ** ses_subencl_desc_struct "subenclosure descriptors".
 **
 ** The <tt>type_desc_text</tt> field is a series of <tt>num_hdrs</tt> variable-length ASCII
 ** strings, one for each <tt>type_desc_hdr</tt>. The length of the <tt>i</tt>th string is in
 ** <tt>type_desc_hdr[i].\ref ses_type_desc_hdr_struct.type_desc_text_len "type_desc_text_len"</tt>.
 ***************************************************************************
 ****************************************************************************/

typedef struct {
    fbe_u8_t  pg_code : 8;            ///< Byte 0    \ref SES_PG_CODE_CONFIG, \ref ses_pg_code_enum
    fbe_u8_t  num_secondary_subencls; ///< Byte 1,   number does not include primary
    fbe_u16_t pg_len;              ///< Byte 2-3, length of rest of page
    fbe_u32_t  gen_code;            ///< Byte 4-7, generation code bumped on any change in this page
    ///                                                  Byte 8+,  subenclosure descriptors.
    /// There is always one descriptor for the primary subenclosure (always an LCC)
    /// plus one for each secondary subenclosure (\ref num_secondary_subencls).  They
    /// are variable length, so you have to go through them in order to get to the next
    /// one, using \ref fbe_eses_get_next_ses_subencl_desc_p().
    ses_subencl_desc_struct subencl_desc;
    
    // Following this:
    //    ses_type_desc_hdr_struct type_desc_hdr[num_hdrs]; // Type descriptor headers
    //   CHAR                    type_desc_text[];        // Type descriptor texts
    // See comments above.
} ses_pg_config_struct;


/****************************************************************************/
/** Type descriptor header in the 
 * \ref ses_pg_config_struct "Configuration diagnostic page".
 *
 * There is one entry per element type per type descriptor text per subenclosure. 
 *
 * These headers appear in the page immediately following the last subenclosure descriptor.
 * To get a pointer to the array of these, you must iterate beyond the last
 * subenclosure descriptor using \ref fbe_eses_get_next_ses_subencl_desc_p().
 ****************************************************************************/
typedef struct {
    fbe_u8_t elem_type             : 8; ///< Byte 0, element type, \ref ses_elem_type_enum
    fbe_u8_t num_possible_elems;        ///< Byte 1, number of possible elements of this type in the subenclosure
    fbe_u8_t subencl_id            : 8; ///< Byte 2, subenclosure ID
    fbe_u8_t type_desc_text_len;        ///< Byte 3, length of the type descriptor text
} ses_type_desc_hdr_struct;

SIZE_CHECK(ses_type_desc_hdr_struct, 4);

/****************************************************************************/
/** Buffer descriptor field in \ref ses_subencl_desc_struct "subenclosure descriptor" in the
 * \ref ses_pg_config_struct "Configuration diagnostic page".
 *
 * To obtain a pointer to the array of these, use fbe_eses_get_ses_buf_desc_p().
 * To get a pointer to the number of array elements use fbe_eses_get_ses_num_buf_descs_p().
 ****************************************************************************/
typedef struct {
    fbe_u8_t buf_id;            ///< Byte 0,           ID used in READ/WRITE BUFER
    fbe_u8_t buf_type      : 7; ///< Byte 1, bits 0-6, type of the buffer \ref ses_buf_type_enum
    fbe_u8_t writable      : 1; ///< Byte 1, bit 7,    WRITE BUFER allowed     
    fbe_u8_t buf_index;         ///< Byte 2,           meaning depends on buf_type
    fbe_u8_t buf_spec_info;     ///< Byte 3,           not used at this time
} ses_buf_desc_struct;

SIZE_CHECK(ses_buf_desc_struct, 4);
 
#pragma pack(pop)

// This contains information about the buffer descriptors
// in a particular SubEnclosure.
typedef struct terminator_eses_subencl_buf_desc_info_s
{
    // This corresponds to number of buf descs in
    // this subencl
    fbe_u8_t num_buf_descs;
    ses_buf_desc_struct buf_desc[MAX_BUF_DESC_PER_SUBENCL];
} terminator_eses_subencl_buf_desc_info_t;

typedef struct terminator_eses_config_page_info_s{
    fbe_u8_t num_subencls;
    ses_subencl_desc_struct subencl_desc_array[CONFIG_PAGE_MAX_SUB_ENCLS];
    fbe_u8_t subencl_slot[CONFIG_PAGE_MAX_SUB_ENCLS];
    fbe_u8_t num_type_desc_headers;
    ses_type_desc_hdr_struct type_desc_array[CONFIG_PAGE_MAX_TYPE_DESC_HEADERS];
    fbe_u8_t  elem_index_of_first_elem_array[CONFIG_PAGE_MAX_TYPE_DESC_HEADERS];
    fbe_u16_t offset_of_first_elem_array[CONFIG_PAGE_MAX_TYPE_DESC_HEADERS];
    fbe_u8_t num_ver_descs;
    ses_ver_desc_struct ver_desc_array[CONFIG_PAGE_MAX_VER_DESCS];
    // This is the total number of buffer descs in all the Subenclosures.
    fbe_u8_t num_buf_descs;
    // Elements in the below subencl_buf_desc_info[] have one to one
    // mapping to the elements in subencl_desc_array[]. So each
    // element in subencl_buf_desc_info[] gives information about
    // the buffer descs of corresponding subenclosure in
    // subencl_desc_array. As we can assume Subenclosure descs
    // in current config page are arranged consecutively by
    // Subenclosure ID, To get the buffer desc information
    // in a particular subenclosure the below array can be
    // indexed by the "Subenclosure ID" of that particular subenclosure.
    terminator_eses_subencl_buf_desc_info_t subencl_buf_desc_info[CONFIG_PAGE_MAX_SUB_ENCLS];
    // This gives the complete size of the status page.
    fbe_u16_t encl_stat_diag_page_size;
} terminator_eses_config_page_info_t;

typedef enum
{
    SIDE_A = 0,
    SIDE_B = 1,
    SIDE_C = 2,
    SIDE_D = 3,
    SIDE_UNDEFINED = 0x1f,
} terminator_eses_subencl_side;

// Pack the structures to prevent padding.
#pragma pack(push, fbe_eses, 1) 

/**************************************
 * RECEIVE_DIAGNOSTIC_RESULTS command.
 **************************************/
typedef struct fbe_eses_receive_diagnostic_results_cdb_s
{
    /* Byte 0 */
    fbe_u8_t operation_code;
    /* Byte 1 */
    fbe_u8_t page_code_valid : 1; // Byte 1, bit 0
    fbe_u8_t rsvd : 7;   // Byte 1, bit 1-7
    /* Byte 2 */
    fbe_u8_t page_code;
    /* Byte 3 */
    fbe_u8_t alloc_length_msbyte;
    /* Byte 4 */
    fbe_u8_t alloc_length_lsbyte;
    /* Byte 5 */
    fbe_u8_t control;
} fbe_eses_receive_diagnostic_results_cdb_t;

/**************************************
 * SEND_DIAGNOSTIC command.
 **************************************/
typedef struct fbe_eses_send_diagnostic_cdb_s
{
    /* Byte 0 */
    fbe_u8_t operation_code;    // SCSI operation code (1dh)
    /* Byte 1 */
    fbe_u8_t unit_offline :1;   // ignored
    fbe_u8_t device_offline :1; // ignored
    fbe_u8_t self_test :1;      // Perform the Diag Op in the SelfTestcode
    fbe_u8_t rsvd :1;           // Reserved bit
    fbe_u8_t page_format :1;    // Page Format for Single Diag Page
    fbe_u8_t self_test_code : 3;// not supported, must be 0
    /* Byte 2 */
    fbe_u8_t rsvd1;             // Reserved byte
    /* Byte 3 */
    fbe_u8_t param_list_length_msbyte; // The number of bytes to be written to the EMA (MSB)
    /* Byte 4 */
    fbe_u8_t param_list_length_lsbyte; // The number of bytes to be written to the EMA (LSB)
    /* Byte 5 */
    fbe_u8_t control;
} fbe_eses_send_diagnostic_cdb_t;

/**************************************
 * READ_BUFFER command.
 **************************************/
typedef struct fbe_eses_read_buf_cdb_s
{
    fbe_u8_t operation_code;    // Byte 0, SCSI operation code (3Ch)
    fbe_u8_t mode           :5; // Byte 1, bit 0-4, mode
    fbe_u8_t reserved       :3; // Byte 1, bit 5-7, ignored
    fbe_u8_t buf_id;            // Byte 2, buffer id
    fbe_u8_t buf_offset_msbyte;           // Byte 3, buffer offset (MSB)  
    fbe_u8_t buf_offset_midbyte;          // Byte 4, buffer offset (The byte between MSB and LSB) 
    fbe_u8_t buf_offset_lsbyte;           // Byte 5, buffer offset (LSB) 
    fbe_u8_t alloc_length_msbyte; // Byte 6, the maximum number of bytes to read from the EMA (MSB)
    fbe_u8_t alloc_length_midbyte; // Byte 7, the maximum number of bytes to read from the EMA (The byte between MSB and LSB)
    fbe_u8_t alloc_length_lsbyte; // Byte 8, the maximum number of bytes to read from the EMA (LSB)
    fbe_u8_t control; // Byte 9
} fbe_eses_read_buf_cdb_t;

/**************************************
 * WRITE_BUFFER command.
 **************************************/
typedef struct fbe_eses_write_buf_cdb_s
{
    fbe_u8_t operation_code;    // Byte 0, SCSI operation code (3Ch)
    fbe_u8_t mode           :5; // Byte 1, bit 0-4, mode
    fbe_u8_t reserved       :3; // Byte 1, bit 5-7, ignored
    fbe_u8_t buf_id;            // Byte 2, buffer id
    fbe_u8_t buf_offset_msbyte;           // Byte 3, buffer offset (MSB)  
    fbe_u8_t buf_offset_midbyte;          // Byte 4, buffer offset (The byte between MSB and LSB) 
    fbe_u8_t buf_offset_lsbyte;           // Byte 5, buffer offset (LSB) 
    fbe_u8_t param_list_length_msbyte; // Byte 6, the number of bytes to be written to the EMA (MSB)
    fbe_u8_t param_list_length_midbyte; // Byte 7, the number of bytes to be written to the EMA (The byte between MSB and LSB)
    fbe_u8_t param_list_length_lsbyte; // Byte 8, the number of bytes to be written to the EMA (LSB)
    fbe_u8_t control; // Byte 9
} fbe_eses_write_buf_cdb_t;


/**************************************
 * MODE SENSE (10) command.
 **************************************/
typedef struct fbe_eses_mode_sense_10_cdb_s
{
    fbe_u8_t operation_code;    // Byte 0, SCSI operation code (5Ah)
    fbe_u8_t rsvd1          :3; // Byte 1, bit 0-2, reserved
    fbe_u8_t dbd            :1; // Byte 1, bit 3
    fbe_u8_t llbaa          :1; // Byte 1, bit 4
    fbe_u8_t rsvd2          :3; // Byte 1, bit 5-7, reserved
    fbe_u8_t pg_code        :6; // Byte 2, bit 0-5, the mode page to be returned.
    fbe_u8_t pc             :2; // Byte 2, bit 6-7 
    fbe_u8_t subpg_code;        // Byte 3, the mode subpage code to be returned.
    fbe_u8_t rsvd3[3];       // Byte 4-6,  
    fbe_u8_t alloc_length_msbyte; // Byte 7, the number of bytes to be written to the EMA (MSB)
    fbe_u8_t alloc_length_lsbyte; // Byte 8, the number of bytes to be written to the EMA (LSB)
    fbe_u8_t control;             // Byte 9
} fbe_eses_mode_sense_10_cdb_t;

/**************************************
 * MODE SELECT (10) command.
 **************************************/
typedef struct fbe_eses_mode_select_10_cdb_s
{
    fbe_u8_t operation_code;    // Byte 0, SCSI operation code (55h)
    fbe_u8_t sp             :1; // Byte 1, bit 0
    fbe_u8_t rsvd1          :3; // Byte 1, bit 1-3, reserved
    fbe_u8_t pf             :1; // Byte 1, bit 4
    fbe_u8_t rsvd2          :3; // Byte 1, bit 5-7, reserved
    fbe_u8_t rsvd3[5];          // Byte 2-6    
    fbe_u8_t param_list_length_msbyte; // Byte 7, the number of bytes to be written to the EMA (MSB)
    fbe_u8_t param_list_length_lsbyte; // Byte 8, the number of bytes to be written to the EMA (LSB)
    fbe_u8_t control;             // Byte 9
} fbe_eses_mode_select_10_cdb_t;

#pragma pack(pop, fbe_eses) /* Go back to default alignment.*/

// Tabasco configuration info
static fbe_u8_t tabasco_config_page_with_ps[][1200] = TABASCO_CONFIGURATION_PAGE_CONTENTS;
static terminator_eses_config_page_info_t tabasco_config_page_info_with_ps;

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

static __inline ses_subencl_desc_struct *fbe_eses_get_next_ses_subencl_desc_p(ses_subencl_desc_struct *d) {
    return (ses_subencl_desc_struct *)(((fbe_u8_t *) d) + d->subencl_desc_len + 4);
}

fbe_status_t fbe_terminator_api_get_sp_id(terminator_sp_id_t *sp_id)
{
    *sp_id = terminator_sp_id;
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
fbe_status_t config_page_parse_given_config_page(fbe_u8_t *config_page,
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

    terminator_sp_id_t spid;
    
    status =  fbe_terminator_api_get_sp_id(&spid);

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

fbe_status_t config_page_init(void)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sp_id_t spid;
    
    status =  fbe_terminator_api_get_sp_id(&spid);

    // Parse and build tabasco configuration page related structures.
    status = config_page_parse_given_config_page(tabasco_config_page_with_ps[spid],
                                                tabasco_config_page_info_with_ps.subencl_desc_array,
                                                &tabasco_config_page_info_with_ps.subencl_slot[0],
                                                tabasco_config_page_info_with_ps.type_desc_array,
                                                tabasco_config_page_info_with_ps.elem_index_of_first_elem_array,
                                                tabasco_config_page_info_with_ps.offset_of_first_elem_array,
                                                tabasco_config_page_info_with_ps.ver_desc_array,
                                                tabasco_config_page_info_with_ps.subencl_buf_desc_info,
                                                &tabasco_config_page_info_with_ps.num_subencls,
                                                &tabasco_config_page_info_with_ps.num_type_desc_headers,
                                                &tabasco_config_page_info_with_ps.num_ver_descs,
                                                &tabasco_config_page_info_with_ps.num_buf_descs,
                                                &tabasco_config_page_info_with_ps.encl_stat_diag_page_size);

    return status;
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


void main(void)
{
   fbe_status_t status = config_page_init();
}

#endif
