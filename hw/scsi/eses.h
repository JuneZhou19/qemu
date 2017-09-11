#ifndef _ESES_H
#define _ESES_H

#include "fbe_hook.h"
#include "fbe_queue.h"
#include "ses.h"

#define DEFAULT_ENCLOSURE_FIRMWARE_ACTIVATE_TIME_INTERVAL 0
#define DEFAULT_ENCLOSURE_FIRMWARE_RESET_TIME_INTERVAL 0

#define MAX_POSSIBLE_CONNECTOR_ID_COUNT 8

#define MAX_POSSIBLE_DRIVE_SLOTS    60
#define MAX_POSSIBLE_SINGLE_LANE_CONNS_PER_PORT 8
#define MAX_POSSIBLE_ESES_PAGES 20

#define VIPER_VIRTUAL_PHY_SAS_ADDR_MASK 0xFFFFFFFFFFFFFFC0
#define VIPER_VIRTUAL_PHY_SAS_ADDR_LAST_6_BITS 0x3E
#define VIPER_MAX_PHYSICAL_PHY_ID 23
#define SAS_MAX_PHYSICAL_PHY_ID 35
#define BULLET_MAX_PHYSICAL_PHY_ID 23

#define FBE_SAS_ADDRESS_INVALID 0xFFFFFFFFFFFFFFFF

#define MAX_DEVICES_PER_SAS_PORT    0x113
#define INVALID_TMSDT_INDEX         MAX_DEVICES_PER_SAS_PORT
#define INDEX_BIT_MASK  0xFFFF

#define TERMINATOR_EMC_PAGE_NUM_INFO_ELEM_GROUPS        3
#define TERMINATOR_EMC_PAGE_NUM_TRACE_BUF_INFO_ELEMS    2

/*******************************************************************
 *
 *
 *  RESUME DEFENITION
 *
 ******************************************************************/
#define RESUME_PROM_EMC_TLA_PART_NUM_SIZE           16
#define RESUME_PROM_EMC_TLA_ARTWORK_REV_SIZE        3
#define RESUME_PROM_EMC_TLA_ASSEMBLY_REV_SIZE       3
#define RESUME_PROM_EMPTY_SPACE_1_SIZE              2

#define RESUME_PROM_SEMAPHORE_SIZE                  1
#define RESUME_PROM_EMPTY_SPACE_2_SIZE              7

#define RESUME_PROM_EMC_TLA_SERIAL_NUM_SIZE         16
#define RESUME_PROM_EMPTY_SPACE_3_SIZE              3

#define RESUME_PROM_EMC_SYSTEM_HW_PN_SIZE           16
#define RESUME_PROM_EMC_SYSTEM_HW_SN_SIZE           16
#define RESUME_PROM_EMC_SYSTEM_HW_REV_SIZE          3
#define RESUME_PROM_EMPTY_SPACE_18_SIZE             3

#define RESUME_PROM_PRODUCT_PN_SIZE                 16
#define RESUME_PROM_PRODUCT_SN_SIZE                 16
#define RESUME_PROM_PRODUCT_REV_SIZE                3
#define RESUME_PROM_EMPTY_SPACE_19_SIZE             4

#define RESUME_PROM_VENDOR_PART_NUM_SIZE            32
#define RESUME_PROM_VENDOR_ARTWORK_REV_SIZE         3
#define RESUME_PROM_VENDOR_ASSEMBLY_REV_SIZE        3
#define RESUME_PROM_VENDOR_UNIQUE_REV_SIZE          4
#define RESUME_PROM_VENDOR_ACDC_INPUT_TYPE_SIZE     2
#define RESUME_PROM_EMPTY_SPACE_4_SIZE              4

#define RESUME_PROM_VENDOR_SERIAL_NUM_SIZE          32
#define RESUME_PROM_EMPTY_SPACE_5_SIZE              2

#define RESUME_PROM_PCIE_CONFIGURATION_SIZE         1
#define RESUME_PROM_BOARD_POWER_SIZE                2
#define RESUME_PROM_THERMAL_TARGET_SIZE             1
#define RESUME_PROM_THERMAL_SHUTDOWN_LIMIT_SIZE     1
#define RESUME_PROM_EMPTY_SPACE_20_SIZE             41

#define RESUME_PROM_VENDOR_NAME_SIZE                32
#define RESUME_PROM_LOCATION_MANUFACTURE_SIZE       32
#define RESUME_PROM_YEAR_MANUFACTURE_SIZE           4
#define RESUME_PROM_MONTH_MANUFACTURE_SIZE          2
#define RESUME_PROM_DAY_MANUFACTURE_SIZE            2
#define RESUME_PROM_EMPTY_SPACE_6_SIZE              8

#define RESUME_PROM_TLA_ASSEMBLY_NAME_SIZE          32
#define RESUME_PROM_CONTACT_INFORMATION_SIZE        128
#define RESUME_PROM_EMPTY_SPACE_7_SIZE              16

#define RESUME_PROM_ESES_NUM_PROG                   1
#define RESUME_PROM_NUM_PROG_SIZE                   1
#define RESUME_PROM_EMPTY_SPACE_8_SIZE              15

#define RESUME_PROM_PROG_NAME_SIZE                  8
#define RESUME_PROM_PROG_REV_SIZE                   4
#define RESUME_PROM_PROG_CHECKSUM_SIZE              4
#define RESUME_PROM_EMPTY_SPACE_9_SIZE              12

#define RESUME_PROM_WWN_SEED_SIZE                   4
#define RESUME_PROM_SAS_ADDRESS_SIZE                4
#define RESUME_PROM_EMPTY_SPACE_10_SIZE             12

#define RESUME_PROM_RESERVED_REGION_1_SIZE          1
#define RESUME_PROM_EMPTY_SPACE_11_SIZE             4

#define RESUME_PROM_PCBA_PART_NUM_SIZE              16
#define RESUME_PROM_PCBA_ASSEMBLY_REV_SIZE          3
#define RESUME_PROM_PCBA_SERIAL_NUM_SIZE            16
#define RESUME_PROM_EMPTY_SPACE_12_SIZE             2

#define RESUME_PROM_CONFIGURATION_TYPE_SIZE         2
#define RESUME_PROM_EMPTY_SPACE_13_SIZE             2

#define RESUME_PROM_EMC_ALT_MB_PART_SIZE            16
#define RESUME_PROM_EMPTY_SPACE_14_SIZE             4

#define RESUME_PROM_CHANNEL_SPEED_SIZE              2
#define RESUME_PROM_SYSTEM_TYPE_SIZE                2
#define RESUME_PROM_DAE_ENCL_ID_SIZE                1
#define RESUME_PROM_RACK_ID_SIZE                    1
#define RESUME_PROM_SLOT_ID_SIZE                    1
#define RESUME_PROM_EMPTY_SPACE_15_SIZE             5

#define RESUME_PROM_DRIVE_SPIN_UP_SELECT_SIZE       2
#define RESUME_PROM_SP_FAMILY_FRU_ID_SIZE           4
#define RESUME_PROM_FRU_CAPABILITY_SIZE             2
#define RESUME_PROM_EMPTY_SPACE_16_SIZE             2

#define RESUME_PROM_EMC_SUB_ASSY_PART_NUM_SIZE      16
#define RESUME_PROM_EMC_SUB_ASSY_ARTWORK_REV_SIZE   3
#define RESUME_PROM_EMC_SUB_ASSY_REV_SIZE           3
#define RESUME_PROM_EMC_SUB_ASSY_SERIAL_NUM_SIZE    16
#define RESUME_PROM_EMPTY_SPACE_17_SIZE             14

#define RESUME_PROM_CHECKSUM_SIZE                   4

#define RESUME_PROM_PS_MAX_FREQ_SIZE                1


/* Maximum programmables count as defined in the resume PROM Spec. */
#define     RESUME_PROM_MAX_PROG_COUNT              84
#define     IPMI_RESUME_PROM_MAX_PROG_COUNT         84

/* Maximum MAC Addresses count as defined in the resume PROM Spec. */
#define     RESUME_PROM_MAX_MAC_ADDR_COUNT          13

#define     RESUME_PROM_PS_MAX_FREQ_START_ADDR      0x81C

// Margining test mode in Power Supply Information Status Element
typedef enum{
    FBE_ESES_MARGINING_TEST_MODE_STATUS_TEST_RESULTS_INVALID = 0x00,
    FBE_ESES_MARGINING_TEST_MODE_STATUS_TEST_RUNNING         = 0x01,
    FBE_ESES_MARGINING_TEST_MODE_STATUS_TEST_SUCCESSFUL      = 0x02,
    FBE_ESES_MARGINING_TEST_MODE_STATUS_TEST_NOT_SUPPORTED   = 0x03,
    FBE_ESES_MARGINING_TEST_MODE_STATUS_AUTO_TEST_ENABLED    = 0x04,
} fbe_eses_margining_test_mode_status_t;

// Test Results in Power Supply Information Element
typedef enum{
    FBE_ESES_MARGINING_TEST_RESULTS_SUCCESS                = 0x00,
    FBE_ESES_MARGINING_TEST_RESULTS_12V_1_OPEN             = 0x01,
    FBE_ESES_MARGINING_TEST_RESULTS_12V_2_OPEN             = 0x02,
    FBE_ESES_MARGINING_TEST_RESULTS_12V_1_SHORT            = 0x04,
    FBE_ESES_MARGINING_TEST_RESULTS_12V_2_SHORT            = 0x08,
    FBE_ESES_MARGINING_TEST_RESULTS_12V_1_SHORTED_TO_12_V2 = 0x10,
} fbe_eses_margining_test_results_t;

typedef enum fbe_fc_enclosure_type_e {
    FBE_FC_ENCLOSURE_TYPE_INVALID,
    FBE_FC_ENCLOSURE_TYPE_STILLETO,
    FBE_FC_ENCLOSURE_TYPE_KLONDIKE,
    FBE_FC_ENCLOSURE_TYPE_KATANA,
    FBE_FC_ENCLOSURE_TYPE_UNKNOWN,

    FBE_FC_ENCLOSURE_TYPE_LAST
} fbe_fc_enclosure_type_t;


typedef enum fbe_eses_conn_type_e{
    FBE_ESES_CONN_TYPE_UNKNOWN              = 0x0,
    FBE_ESES_CONN_TYPE_SAS_4X               = 0x1,
    FBE_ESES_CONN_TYPE_MINI_SAS_4X          = 0x2,
    FBE_ESES_CONN_TYPE_QSFP_PLUS            = 0x3,
    FBE_ESES_CONN_TYPE_MINI_SAS_4X_ACTIVE   = 0x4,
    FBE_ESES_CONN_TYPE_MINI_SAS_HD_4X       = 0x5,
    FBE_ESES_CONN_TYPE_MINI_SAS_HD_8X       = 0x6,
    FBE_ESES_CONN_TYPE_MINI_SAS_HD_16X      = 0x7,
    FBE_ESES_CONN_TYPE_SAS_DRIVE            = 0x20,
    FBE_ESES_CONN_TYPE_VIRTUAL_CONN         = 0x2F,
    FBE_ESES_CONN_TYPE_INTERNAL_CONN        = 0x3F
}fbe_eses_conn_type_t;

typedef enum fbe_sas_enclosure_protuct_type_e {
    FBE_SAS_ENCLOSURE_PRODUCT_TYPE_INVALID,
    FBE_SAS_ENCLOSURE_PRODUCT_TYPE_ESES,

    FBE_SAS_ENCLOSURE_PRODUCT_TYPE_LAST
} fbe_sas_enclosure_product_type_t;

typedef enum fbe_sas_enclosure_type_e {
    FBE_SAS_ENCLOSURE_TYPE_INVALID,
    FBE_SAS_ENCLOSURE_TYPE_BULLET,
    FBE_SAS_ENCLOSURE_TYPE_VIPER,
    FBE_SAS_ENCLOSURE_TYPE_MAGNUM,
    FBE_SAS_ENCLOSURE_TYPE_CITADEL,
    FBE_SAS_ENCLOSURE_TYPE_BUNKER,
    FBE_SAS_ENCLOSURE_TYPE_DERRINGER,
    FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM,
    FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE,  
    FBE_SAS_ENCLOSURE_TYPE_CAYENNE,
    FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP,
    FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP,  
    FBE_SAS_ENCLOSURE_TYPE_FALLBACK,  
    FBE_SAS_ENCLOSURE_TYPE_BOXWOOD,  
    FBE_SAS_ENCLOSURE_TYPE_KNOT,  
    FBE_SAS_ENCLOSURE_TYPE_PINECONE,  
    FBE_SAS_ENCLOSURE_TYPE_STEELJAW,  
    FBE_SAS_ENCLOSURE_TYPE_RAMHORN,
    FBE_SAS_ENCLOSURE_TYPE_ANCHO,
    FBE_SAS_ENCLOSURE_TYPE_VIKING,
    FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP,
    FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP,
    FBE_SAS_ENCLOSURE_TYPE_NAGA,
    FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP,
    FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP,
    FBE_SAS_ENCLOSURE_TYPE_RHEA,  
    FBE_SAS_ENCLOSURE_TYPE_MIRANDA,  
    FBE_SAS_ENCLOSURE_TYPE_CALYPSO,  
    FBE_SAS_ENCLOSURE_TYPE_TABASCO,
    FBE_SAS_ENCLOSURE_TYPE_NAGA_80,
    FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP,
    FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP,
    FBE_SAS_ENCLOSURE_TYPE_UNKNOWN,
    FBE_SAS_ENCLOSURE_TYPE_LAST
} fbe_sas_enclosure_type_t;


typedef enum fbe_eses_enclosure_board_type_e{
    FBE_ESES_ENCLOSURE_BOARD_TYPE_UNKNOWN       = 0x0000,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_MAGNUM        = 0x0002,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_DERRINGER     = 0x0003,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_VIPER         = 0x0004,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_SENTRY        = 0x0005,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_EVERGREEN     = 0x0006,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_PINECONE      = 0x0007,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_VOYAGER_ICM   = 0x0008,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_VOYAGER_EE    = 0x0009,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_VOYAGER_EE_REV_A  = 0x000A,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_VIKING        = 0x000B,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_VIKING_RSVD0  = 0x000C,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_SKYDIVE       = 0x000D,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_JETFIRE_BEM   = 0x000E,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_BEACHCOMBER   = 0x000F,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_SILVERBOLT    = 0x0010,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_STARDUST      = 0x0011,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_CAYENNE       = 0x0012,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_ANCHO         = 0x0013,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_TABASCO       = 0x0014,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_OBERON        = 0x0015,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_NAGA          = 0x0016,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_LAPETUS       = 0x0017,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_EUROPA        = 0x0018,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_COLOSSUS      = 0x0019,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_NAGA_80       = 0x001A,
    FBE_ESES_ENCLOSURE_BOARD_TYPE_INVALID       = 0xFFFF, 
} fbe_eses_enclosure_board_type_t;       


/* Increased the maximum transfer of inquiry data size 
 * from 120 to 128 to display properly the 8-character firmware 
 * revision number by navi.(#153573).
 */
enum fbe_scsi_data_e {
    FBE_SCSI_INQUIRY_DATA_SIZE = 136, /* Requested bytes (Max of 136 from ESES) */
    FBE_SCSI_INQUIRY_DATA_MINIMUM_SIZE = 36, /* Requested bytes (minimum of 36) */
    FBE_SCSI_INQUIRY_VENDOR_ID_SIZE = 8, /* Size of VENDOR IDENTIFICATION field in inquiry response */
    FBE_SCSI_INQUIRY_VENDOR_ID_OFFSET = 8, /* VENDOR IDENTIFICATION field offset in inquiry response */
    FBE_SCSI_INQUIRY_PRODUCT_ID_SIZE = 16, /* Size of PRODUCT IDENTIFICATION field in inquiry response */
    FBE_SCSI_INQUIRY_PRODUCT_ID_OFFSET = 16, /* PRODUCT IDENTIFICATION field offset in inquiry response */
    FBE_SCSI_INQUIRY_BOARD_TYPE_SIZE = 2, /* Size of PRODUCT IDENTIFICATION field in inquiry response */
    FBE_SCSI_INQUIRY_BOARD_TYPE_OFFSET = 114, /* PRODUCT IDENTIFICATION field offset in inquiry response */
    FBE_SCSI_INQUIRY_ENCLOSURE_PLATFORM_TYPE_SIZE = 2,
    FBE_SCSI_INQUIRY_ENCLOSURE_PLATFORM_TYPE_OFFSET = 112,
//  FBE_SCSI_INQUIRY_ENCLOSURE_UNIQUE_ID_SIZE = 4,
//  FBE_SCSI_INQUIRY_ENCLOSURE_UNIQUE_ID_OFFSET = 114,
    FBE_SCSI_INQUIRY_ESES_VERSION_DESCRIPTOR_SIZE = 2,
    FBE_SCSI_INQUIRY_ESES_VERSION_DESCRIPTOR_OFFSET = 118,
    FBE_SCSI_INQUIRY_ENCLOSURE_SERIAL_NUMBER_SIZE = 16,
    FBE_SCSI_INQUIRY_ENCLOSURE_SERIAL_NUMBER_OFFSET = 120,

    /* Read Capacity:
     * This is the number of bytes we expect back from a Read Capacity command.
     * The data indicates the last valid lba and the block length.
     */
    FBE_SCSI_READ_CAPACITY_DATA_SIZE = 8,  
    FBE_SCSI_READ_CAPACITY_DATA_SIZE_16 = 12,

    FBE_SCSI_MAX_MODE_SENSE_SIZE = 0xFF,
    FBE_SCSI_INQUIRY_REVISION_OFFSET = 32,
    FBE_SCSI_INQUIRY_REVISION_SIZE = 4,
    FBE_SCSI_INQUIRY_SERIAL_NUMBER_OFFSET = 36,
    FBE_SCSI_INQUIRY_SERIAL_NUMBER_SIZE = 20,
    FBE_SCSI_INQUIRY_PART_NUMBER_OFFSET = 56,
    FBE_SCSI_INQUIRY_PART_NUMBER_SIZE = 13,
    FBE_SCSI_INQUIRY_TLA_OFFSET = 69,
    FBE_SCSI_INQUIRY_TLA_SIZE = 12,
    FBE_SCSI_DESCRIPTION_BUFF_SIZE = 32,
    FBE_SCSI_RECEIVE_DIAGNOSTIC_RESULTS_CDB_SIZE = 6,
    FBE_SCSI_SEND_DIAGNOSTIC_CDB_SIZE = 6,
    FBE_SCSI_SEND_DIAGNOSTIC_QUICK_SELF_TEST_BITS = 0x06,
    FBE_SCSI_READ_BUFFER_CDB_SIZE = 10,
    FBE_SCSI_WRITE_BUFFER_CDB_SIZE = 10,
    FBE_SCSI_MODE_SENSE_10_CDB_SIZE = 10,
    FBE_SCSI_MODE_SELECT_10_CDB_SIZE = 10,
    /* Parameters for requesting and parsing the long form
     * of mode page 0x19 with SAS PHY information*/
    FBE_SCSI_MODE_SENSE_RETURN_NO_BLOCK_DESCRIPTOR = 0x08,
    FBE_SCSI_MODE_SENSE_REQUEST_PAGE_0x19 = 0x19,
    FBE_SCSI_MODE_SENSE_SUBPAGE_0x01 = 0x01,
    FBE_SCSI_MODE_SENSE_HEADER_LENGTH = 4,    
    FBE_SCSI_MODE_SENSE_PHY_DESCRIPTOR_OFFSET = 8,
    FBE_SCSI_MODE_SENSE_PHY_DESCRIPTOR_LENGTH = 48,    
    FBE_SCSI_MODE_SENSE_HARDWARE_MAXIMUM_LINK_RATE_OFFSET = 33,
    FBE_SCSI_MODE_SENSE_HARDWARE_MAXIMUM_LINK_RATE_MASK = 0x0F,    
    /* Mode Page 0x19 long format PHY Physical link rate field parameters */
    FBE_SCSI_MODE_PAGE_0x19_PHY_SPEED_FIELD_1_5_GBPS = 0x08,
    FBE_SCSI_MODE_PAGE_0x19_PHY_SPEED_FIELD_3_GBPS = 0x09,    
    FBE_SCSI_MODE_PAGE_0x19_PHY_SPEED_FIELD_6_GBPS = 0x0A,
    FBE_SCSI_MODE_PAGE_0x19_PHY_SPEED_FIELD_12_GBPS = 0x0B,
    FBE_SCSI_INQUIRY_VPDC0_MIN_SIZE = 4,   /* Min. expected for VPD pg. C0 inquiry */
    FBE_SCSI_INQUIRY_VPD_PROD_REV_SIZE = 9,
    FBE_SCSI_INQUIRY_VPD_BRIDGE_HW_REV_SIZE = 7,
    FBE_SCSI_FORMAT_UNIT_SANITIZE_HEADER_LENGTH = 8,
};

typedef struct terminator_sas_encl_eses_info_s {
    fbe_sas_enclosure_type_t encl_type;
    fbe_u8_t max_drive_count;
    fbe_u8_t max_phy_count;
    fbe_u8_t max_encl_conn_count; // not used currently
    fbe_u8_t max_lcc_conn_count;  // max sas coonnector count per LCC.
    fbe_u8_t max_port_conn_count; // max connectors per port ( the number of single lanes + one entire connector)
    fbe_u8_t max_single_lane_port_conn_count; // The number of single lanes
    fbe_u8_t max_ps_count;
    fbe_u8_t max_cooling_elem_count;
    fbe_u8_t max_temp_sensor_elems_count;  // Per LCC
    fbe_u8_t drive_slot_to_phy_map[MAX_POSSIBLE_DRIVE_SLOTS];
    fbe_u8_t max_diplay_character_count;
    fbe_u8_t max_lcc_count;
    fbe_u8_t max_ee_lcc_count;
    fbe_u8_t max_ext_cooling_elem_count;
    fbe_u8_t max_bem_cooling_elem_count;  //cuurently only jetfire has this type of fan
    fbe_u8_t max_conn_id_count;
    fbe_u8_t individual_conn_to_phy_map[MAX_POSSIBLE_CONNECTOR_ID_COUNT][MAX_POSSIBLE_SINGLE_LANE_CONNS_PER_PORT];
    fbe_u8_t conn_id_to_drive_start_slot[MAX_POSSIBLE_CONNECTOR_ID_COUNT];
} terminator_sas_encl_eses_info_t;

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

/* POWER SUPPLY Related structures */

// Each power supply (PSA, PSB--etc) in an encl. can be
// addressed by using these enums
typedef enum terminator_eses_ps_id
{
    PS_0 = 0,
    PS_1 = 1,
    PS_2 = 2,
    PS_3 = 3,
}terminator_eses_ps_id;

typedef enum terminator_eses_display_character_id
{
    DISPLAY_CHARACTER_0 = 0,
    DISPLAY_CHARACTER_1,
    DISPLAY_CHARACTER_2,
    MAX_DISPLAY_CHARACTERS
}terminator_eses_display_character_id;

// Each Cooling element in an encl. can be
// addressed by using these enums
typedef enum terminator_eses_cooling_id
{
    COOLING_0 = 0,
    COOLING_1 = 1,
    COOLING_2 = 2,
    COOLING_3 = 3,
    COOLING_4 = 4,
    COOLING_5 = 5,
    COOLING_6 = 6,
    COOLING_7 = 7,
    COOLING_8 = 8,
    COOLING_9 = 9,
}terminator_eses_cooling_id;
/* End of POWER SUPPLY related strcutures */

// Each Connector element in an encl. can be
// addressed by using these enums
typedef enum terminator_eses_sas_conn_id
{
    LOCAL_ENTIRE_CONNECTOR_0 = 0,
    LOCAL_ENTIRE_CONNECTOR_1 = 5,
    PEER_ENTIRE_CONNECTOR_0 = 10,
    PEER_ENTIRE_CONNECTOR_1 = 15,
}terminator_eses_sas_conn_id;

typedef enum terminator_eses_temp_sensor_id
{
    TEMP_SENSOR_0 = 0,
    TEMP_SENSOR_1 = 1,
    TEMP_SENSOR_2 = 2,
    TEMP_SENSOR_3 = 3,
    TEMP_SENSOR_4 = 4,
    TEMP_SENSOR_5 = 5
}terminator_eses_temp_sensor_id;

// This is used when mapping a connector into a range
// We really do care about the values of these functions.  In
// particular they are assigned to connector_ids. Don't change
// the order/values unless you are clear on the usage within
// the physical package.
typedef enum terminator_conn_map_range_e
{
    CONN_IS_ILLEGAL = -1,
    CONN_IS_DOWNSTREAM = 0,
    CONN_IS_UPSTREAM,
    CONN_IS_RANGE0,
    CONN_IS_INTERNAL_RANGE1,  
    CONN_IS_INTERNAL_RANGE2,  // Not really used currently.
} terminator_conn_map_range_t;

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

/******************************************************************************//**
 * Download Microcode Mode
 *
 * This code appears in the \ref fbe_eses_download_control_page_header_t 
 * "mode" byte.
 *******************************************************************************/
typedef enum {
    FBE_ESES_DL_CONTROL_MODE_DOWNLOAD = 0x0E, // download, defer activate
    FBE_ESES_DL_CONTROL_MODE_ACTIVATE = 0x0F, // activate deferred ucode
} fbe_eses_download_microcode_mode;

/******************************************************************************//**
 * Element status codes.
 *
 * This code appears in the \ref ses_cmn_stat_struct "common status byte" in each \ref
 * ses_stat_elem_struct "status element" in the \ref ses_pg_encl_stat_struct "Enclosure Status"
 * diagnostic page.
 *******************************************************************************/
typedef enum {
    SES_STAT_CODE_UNSUPP        = 0x0, ///< 0h, unable to detect status.
    SES_STAT_CODE_OK            = 0x1, ///< 1h, element present and has no errors.
    SES_STAT_CODE_CRITICAL      = 0x2, ///< 2h, fatal condition for element.
    SES_STAT_CODE_NONCRITICAL   = 0x3, ///< 3h, warning condition detected for element.
    SES_STAT_CODE_UNRECOV       = 0x4, ///< 4h, unrecoverable condition detected for element.
    SES_STAT_CODE_NOT_INSTALLED = 0x5, ///< 5h, element not installed.
    SES_STAT_CODE_UNKNOWN       = 0x6, ///< 6h, sensor failed or status unavailable.
    SES_STAT_CODE_UNAVAILABLE   = 0x7, ///< 7h, installed with no errors, but not turned on.
} ses_elem_stat_code_enum;

/****************************************************************************/
/** SES Element Types.  These values appear in the \ref ses_type_desc_hdr_struct.elem_type field.
 ****************************************************************************/
typedef enum {
    SES_ELEM_TYPE_PS             = 0x02, ///< Type 02h. Power Supply element.
    SES_ELEM_TYPE_COOLING        = 0x03, ///< Type 03h. Cooling element.
    SES_ELEM_TYPE_TEMP_SENSOR    = 0x04, ///< Type 04h. Temperature Sensor element.
    SES_ELEM_TYPE_ALARM          = 0x06, ///< Type 06h. Alarm element (unused in ESES).
    SES_ELEM_TYPE_ESC_ELEC       = 0x07, ///< Type 07h. Enclosure Serivces Controller Electronics element.
    SES_ELEM_TYPE_UPS            = 0x0B, ///< Type 0Bh. Uninterruptible Power Supply element.
    SES_ELEM_TYPE_DISPLAY        = 0x0C, ///< Type 0Ch. Display element.
    SES_ELEM_TYPE_ENCL           = 0x0E, ///< Type 0Eh. Enclosure element.
    SES_ELEM_TYPE_LANG           = 0x10, ///< Type 10h. Language element.
    SES_ELEM_TYPE_ARRAY_DEV_SLOT = 0x17, ///< Type 17h. Array Device Slot element.
    SES_ELEM_TYPE_SAS_EXP        = 0x18, ///< Type 18h. SAS Expander element.
    SES_ELEM_TYPE_SAS_CONN       = 0x19, ///< Type 19h. SAS Connector element.
    SES_ELEM_TYPE_EXP_PHY        = 0x81, ///< Type 81h. Expander phy element (EMC-specific).
    SES_ELEM_TYPE_INVALID        = 0xFF, /// used to an initialization value.
} ses_elem_type_enum;

/****************************************************************************/
/** SES Buffer Types.  These values appear in the
 * \ref ses_buf_desc_struct.buf_type field.
 ****************************************************************************/
typedef enum ses_buf_type_enum {
    SES_BUF_TYPE_EEPROM       = 0,
    SES_BUF_TYPE_ACTIVE_TRACE = 1,
    SES_BUF_TYPE_SAVED_TRACE  = 2,
    SES_BUF_TYPE_EVENT_LOG    = 3,
    SES_BUF_TYPE_SAVED_DUMP   = 4,
    SES_BUF_TYPE_ACTIVE_RAM   = 5,
    SES_BUF_TYPE_REGISTERS    = 6,
    SES_BUF_TYPE_INVALID      = 0xFF
} ses_buf_type_enum;

/****************************************************************************/
/** SES Buffer Indexes.  These values appear in the
 * \ref ses_buf_desc_struct.buf_index field.
 ****************************************************************************/
typedef enum ses_buf_index_eeprom_enum {
    SES_BUF_INDEX_LOCAL_SXP_EEPROM   = 0,
    SES_BUF_INDEX_BASEBOARD_EEPROM   = 1,
    SES_BUF_INDEX_LOCAL_LCC_EEPROM   = 2,
    SES_BUF_INDEX_PEER_LCC_EEPROM    = 3,
    SES_BUF_INDEX_CONN_0_EEPROM      = 4,
    SES_BUF_INDEX_CONN_1_EEPROM      = 5,
    SES_BUF_INDEX_CONN_2_EEPROM      = 6,
    SES_BUF_INDEX_CONN_3_EEPROM      = 7,
    SES_BUF_INDEX_LOCAL_PS0_EEPROM   = 8,
    SES_BUF_INDEX_LOCAL_PS1_EEPROM   = 9,
    SES_BUF_INDEX_PEER_PS0_EEPROM    = 10,
    SES_BUF_INDEX_PEER_PS1_EEPROM    = 11,
    SES_BUF_INDEX_SSC_EEPROM         = 12,
    SES_BUF_INDEX_INVALID_EEPROM     = 0xFF
} ses_buf_index_eeprom_enum;

/****************************************************************************/
/** SES Cooling fan speed codes.  These values appear in the Cooling
 * \ref ses_ctrl_elem_cooling_struct "control" and
 * \ref ses_stat_elem_cooling_struct "status" elements.
 ****************************************************************************/
typedef enum {
    SES_FAN_SPEED_STOPPED     = 0,
    SES_FAN_SPEED_LOWEST      = 1,
    SES_FAN_SPEED_2NDLOWEST   = 2,
    SES_FAN_SPEED_3RDLOWEST   = 3,
    SES_FAN_SPEED_MEDIUM      = 4,
    SES_FAN_SPEED_3RDHIGHEST  = 5,
    SES_FAN_SPEED_2NDHIGHEST  = 6,
    SES_FAN_SPEED_HIGHEST     = 7
} ses_fan_speed_code_enum;

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

/****************************************************************************/
/** Component type field in \ref ses_ver_desc_struct "version descriptor"
 *  in the \ref ses_subencl_desc_struct "subenclosure descriptor"
 *  in the \ref ses_pg_config_struct "Configuration diagnostic page".
 ****************************************************************************/

typedef enum {
    SES_COMP_TYPE_EXPANDER_FW_EMA         = 0, ///< Type  0, expander firmware 
    SES_COMP_TYPE_EXPANDER_BOOT_LOADER_FW = 1, ///< Type  1, expander boot loader
    SES_COMP_TYPE_INIT_STR                = 2, ///< Type  2, init string
    SES_COMP_TYPE_FPGA_IMAGE              = 3, ///< Type  3, FPGA image
    SES_COMP_TYPE_PS_FW                   = 4, ///< Type  4, power supply firmware
    SES_COMP_TYPE_OTHER_EXPANDER          = 5, ///< Type  5, other expander image
    SES_COMP_TYPE_LCC_MAIN                = 6, ///< Type  6, main (combined) image
    SES_COMP_TYPE_SPS_FW                  = 7, ///< Type  7, SPS image
    SES_COMP_TYPE_COOLING_FW              = 8, ///< Type  8, Cooling Module firmware
    SES_COMP_TYPE_BBU_FW                  = 9, ///< Type  9, BBU firmware
    SES_COMP_TYPE_SPS_SEC_FW              = 10,///< Type 10, Secondary SPS firmware
    SES_COMP_TYPE_SPS_BAT_FW              = 11,///< Type 11, SPS Battery firmware
    SES_COMP_TYPE_OTHER_HW_VER            = 30,///< Type 30, hardware version
    SES_COMP_TYPE_OTHER_FW                = 31,///< Type 31, other firwmare image
    SES_COMP_TYPE_UNKNOWN                 = 255,
} ses_comp_type_enum;

/****************************************************************************/
/** Identifier for a subenclosure, defined in the
 *  \ref ses_pg_config_struct "Configuration" diagnostic page and appearing in
 *  a number of other SES pages.  This identifier is unique within the environment
 *  for a given EMA, but different EMAs may use different identifiers for the same
 *  subenclosure.  
 *
 * If not \ref SES_SUBENCL_ID_NONE, an item of this type is guaranteed to
 * fit into a \p fbe_u8_t.
 * The first ID is \ref SES_SUBENCL_ID_PRIMARY and the number of them (not including
 * the primary) is in ses_pg_config_struct.num_secondary_subencls.
 * These IDs are assigned contiguously.
 ****************************************************************************/
typedef enum {
    SES_SUBENCL_ID_FIRST   = 0,       ///< First subenclosure ID
    SES_SUBENCL_ID_PRIMARY = 0,       ///< Primary subenclosure = local LCC
    SES_NUM_SUBENCLS_0     = 0,       ///< Count of zero subenclosures
    SES_SUBENCL_ID_NONE    = 0xFF,      ///< An ID indicating no subenclosure (can't appear SES page)
} ses_subencl_id;

/****************************************************************************/
/** We use to assign side id on the basis of component index and number of 
 *  components for respective subenclosure. This identifier is unique within the environment
 *  for a given EMA.
 *
 * If not \ref FBE_ESES_SUBENCL_SIDE_ID_INVALID, an item of this type is guaranteed to
 * fit into a \p fbe_u8_t.
 * These IDs are assigned while processing configuration page.
 ****************************************************************************/
typedef enum {
    FBE_ESES_SUBENCL_SIDE_ID_A         = 0,       
    FBE_ESES_SUBENCL_SIDE_ID_B         = 1,       
    FBE_ESES_SUBENCL_SIDE_ID_C         = 2, 
    FBE_ESES_SUBENCL_SIDE_ID_MIDPLANE  = 0x1F,  // Midplane.
    FBE_ESES_SUBENCL_SIDE_ID_INVALID   = 0xFF,  
} fbe_eses_subencl_side_id_t;

/****************************************************************************/
/** Index of a \ref ses_stat_elem_struct "status" or \ref ses_ctrl_elem_struct "control" element
 * in the \ref ses_pg_encl_stat_struct "Enclosure Status" or
 * \ref ses_pg_encl_ctrl_struct "Enclosure Control" diagnostic page.  This index
 * is used frequently in SES to identify an individual element and is unique across all elements.
 * The index of an individual element is the position of the element in the page,
 * relative to all other individual elements, not including overall status or control elements.
 *
 * It is guaranteed that this number will fit into a \p fbe_u8_t.
 * The first index is \ref SES_ELEM_INDEX_FIRST and the number of them is equal to the total
 * of the \ref ses_type_desc_hdr_struct.num_possible_elems "num_possible_elems" fields in the
 * \ref ses_type_desc_hdr_struct "type descriptor headers" in the 
 * \ref ses_pg_config_struct "Configuration" diagnostic page.  The indexes are contiguous, but,
 * because they do not count overall status elements, they cannot be used directly to index
 * into the array of status or control elements in the Enclosure Status or Control pages.
 * \see ses_pg_encl_stat_struct.stat_elem
 ****************************************************************************/
typedef enum {
    SES_ELEM_INDEX_FIRST = 0,      ///< First possible element index
    SES_NUM_ELEMS_0      = 0,      ///< Count of zero elements
    SES_ELEM_INDEX_NONE  = 0xFF,   ///< An index indicating no or unknown element
} ses_elem_index;

typedef enum{
    FBE_ESES_TRACE_BUF_ACTION_CTRL_NO_CHANGE = 0,
    FBE_ESES_TRACE_BUF_ACTION_CTRL_CLEAR_BUF = 2,
    FBE_ESES_TRACE_BUF_ACTION_CTRL_SAVE_BUF = 4,
}fbe_eses_trace_buf_action_ctrl_t;

typedef enum{
    FBE_ESES_TRACE_BUF_ACTION_STATUS_ACTIVE = 0,
    FBE_ESES_TRACE_BUF_ACTION_STATUS_UNEXPECTED_EXCEPTION = 1,
    FBE_ESES_TRACE_BUF_ACTION_STATUS_ASSERTION_FAILURE_RESET = 2,
    FBE_ESES_TRACE_BUF_ACTION_STATUS_RESET_ISSUED = 3,
    FBE_ESES_TRACE_BUF_ACTION_STATUS_CLIENT_INIT_SAVE_BUF = 4,
    FBE_ESES_TRACE_BUF_ACTION_STATUS_WATCHDOG_FIRED = 5,
    FBE_ESES_TRACE_BUF_ACTION_STATUS_NMI_OCCURRED = 6,
    FBE_ESES_TRACE_BUF_ACTION_STATUS_ASSERTION_FAILURE_NO_RESET = 7,
    FBE_ESES_TRACE_BUF_ACTION_STATUS_UNUSED_BUF = 0xFF,
}fbe_eses_trace_buf_action_status_t;

typedef enum{
    FBE_ESES_READ_BUF_MODE_DATA = 0x02,
    FBE_ESES_READ_BUF_MODE_DESC = 0x03,
    FBE_ESES_READ_BUF_MODE_INVALID = 0x1F,    
}fbe_eses_read_buf_mode_t;

typedef enum{
    FBE_ESES_WRITE_BUF_MODE_DATA = 0x02,
    FBE_ESES_WRITE_BUF_MODE_INVALID = 0x1F,    
}fbe_eses_write_buf_mode_t;

// Margining test mode in Power Supply Information Control Element
typedef enum{
    FBE_ESES_MARGINING_TEST_MODE_CTRL_NO_CHANGE          = 0x00,
    FBE_ESES_MARGINING_TEST_MODE_CTRL_START_TEST         = 0x01,
    FBE_ESES_MARGINING_TEST_MODE_CTRL_STOP_TEST          = 0x02,
    FBE_ESES_MARGINING_TEST_MODE_CTRL_CLEAR_TEST_RESULTS = 0x03,
    FBE_ESES_MARGINING_TEST_MODE_CTRL_DISABLE_AUTO_TEST  = 0x04,
    FBE_ESES_MARGINING_TEST_MODE_CTRL_ENABLE_AUTO_TEST   = 0x05,
}fbe_eses_margining_test_mode_ctrl_t;

typedef enum{
    FBE_ESES_EXP_PORT_UNKNOWN  = 0x00,
    FBE_ESES_EXP_PORT_UPSTREAM = 0x01, // primary port.
    FBE_ESES_EXP_PORT_DOWNSTREAM = 0x02,  // expansion port.
    FBE_ESES_EXP_PORT_UNIVERSAL = 0x03,
}fbe_eses_exp_port_t;

// defines for PowerCycleRequest field
typedef enum
{
    FBE_ESES_ENCL_POWER_CYCLE_REQ_NONE = 0,
    FBE_ESES_ENCL_POWER_CYCLE_REQ_BEGIN,
    FBE_ESES_ENCL_POWER_CYCLE_REQ_CANCEL,
    FBE_ESES_ENCL_POWER_CYCLE_REQ_RETURN_CC
} fbe_eses_power_cycle_request_t;

// defines for Information Element Type.
typedef enum
{
    FBE_ESES_INFO_ELEM_TYPE_SAS_CONN = 0,
    FBE_ESES_INFO_ELEM_TYPE_TRACE_BUF,
    FBE_ESES_INFO_ELEM_TYPE_ENCL_TIME,
    FBE_ESES_INFO_ELEM_TYPE_GENERAL,
    FBE_ESES_INFO_ELEM_TYPE_DRIVE_POWER,
    FBE_ESES_INFO_ELEM_TYPE_PS,
    FBE_ESES_INFO_ELEM_TYPE_SPS
} fbe_eses_info_elem_type_t;

// defines for expander reset reason.
typedef enum
{
    FBE_ESES_RESET_REASON_UNKNOWN          = 0,
    FBE_ESES_RESET_REASON_WATCHDOG         = 1,
    FBE_ESES_RESET_REASON_SOFTRESET        = 2,
    FBE_ESES_RESET_REASON_EXTERNAL         = 3,
} fbe_eses_reset_reason_t;

// defines for enclosure shutdown reason.
typedef enum
{
    FBE_ESES_SHUTDOWN_REASON_NOT_SCHEDULED                  = 0,
    FBE_ESES_SHUTDOWN_REASON_CLIENT_REQUESTED_POWER_CYCLE   = 1,
    FBE_ESES_SHUTDOWN_REASON_CRITICAL_TEMP_FAULT            = 2,
    FBE_ESES_SHUTDOWN_REASON_CRITICAL_COOLIG_FAULT          = 3,
    FBE_ESES_SHUTDOWN_REASON_PS_NOT_INSTALLED               = 4,
    FBE_ESES_SHUTDOWN_REASON_UNSPECIFIED_HW_NOT_INSTALLED   = 5,
    FBE_ESES_SHUTDOWN_REASON_UNSPECIFIED_REASON             = 127,
} fbe_eses_shutdown_reason_t;

#define FBE_ESES_EXPANDER_ACTION_CLEAR_RESET  0xf

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
 
/************************************
 * General Information Element Expander
 ************************************/
typedef struct ses_general_info_elem_expander_s
{
    fbe_u8_t elem_index;        // Byte 0
    fbe_u8_t reset_reason : 4;  // Byte 1, bit 0-3
    fbe_u8_t reserved1 : 3;     // Byte 1, bit 4-6
    fbe_u8_t fru : 1;           // Byte 1, bit 7, reserved in a control element.
    fbe_u8_t reserved2;         // Byte 2
    fbe_u8_t reserved3;         // Byte 3
}ses_general_info_elem_expander_struct;

SIZE_CHECK(ses_general_info_elem_expander_struct, 4);

/************************************
 * SAS Connector Information Element 
 ************************************/
typedef struct ses_sas_conn_inf_elem_s{
    fbe_u8_t conn_elem_index;       // Byte 0;
    fbe_u64_t attached_sas_address; // Byte 1-8; Reserved in a control element.
    fbe_u8_t           : 4;         // Byte 9, bit 0-3;
    fbe_u8_t port_use  : 2;         // Byte 9, bit 4-5; 
    fbe_u8_t port_type : 2;         // Byte 9, bit 6-7; Reserved in a control element.
    fbe_u8_t wide_port_id : 4;      // Byte 10, bit 0-3;
    fbe_u8_t recovery     : 1;      // Byte 10, bit 4;  Reserved in a control element.
    fbe_u8_t enable       : 1;      // Byte 10, bit 5;
    fbe_u8_t              : 2;      // Byte 10, bit 6-7;
    fbe_u8_t conn_id;               // Byte 11; Reserved in a control element.
    fbe_u8_t attached_phy_id;       // Byte 12; 
    // In newer versions of CDES (version 0.21) we also have the next field.
    fbe_u8_t attached_sub_encl_id;  // Byte 13; Used in Voyager but not in Viper/Derringer
} ses_sas_conn_info_elem_struct;

SIZE_CHECK(ses_sas_conn_info_elem_struct, 14);

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

#define RESUME_PROM_ALL_FIELDS  0xFE

#define RESUME_PROM_CHECKSUM_SEED 0x64656573

/* 
 * EEPROM Common Area, Version Zero
 */

typedef struct 
{
    uint32_t   version;                     
    uint32_t   ffid;                        
    uint8_t    emc_serialnum[16];           
    uint8_t    emc_partnum[16];             
    uint8_t    emc_assembly_revision[3];             
    uint8_t    mfg_date[10];                
    uint8_t    vendor_revision[3];          
    uint8_t    vendor_part_number[32];      
    uint8_t    vendor_model_number[16];     
    uint8_t    mfg_location[32];           
    uint8_t    vendor_serialnum[32];        
    uint8_t    vendor_name[32];
} dev_eeprom_rev0_info_struct;

typedef struct _RESUME_PROM_PROG_DETAILS
{
    int8_t        prog_name       [RESUME_PROM_PROG_NAME_SIZE];
    int8_t        prog_rev        [RESUME_PROM_PROG_REV_SIZE];
    uint32_t    prog_checksum;
} RESUME_PROM_PROG_DETAILS;

typedef struct _RESUME_PROM_STRUCTURE
{
    int8_t            emc_tla_part_num            [RESUME_PROM_EMC_TLA_PART_NUM_SIZE];
    int8_t            emc_tla_artwork_rev         [RESUME_PROM_EMC_TLA_ARTWORK_REV_SIZE];
    int8_t            emc_tla_assembly_rev        [RESUME_PROM_EMC_TLA_ASSEMBLY_REV_SIZE];
    uint8_t     empty_space_1               [RESUME_PROM_EMPTY_SPACE_1_SIZE];

    uint8_t         rp_semaphore                [RESUME_PROM_SEMAPHORE_SIZE];
    uint8_t     empty_space_2               [RESUME_PROM_EMPTY_SPACE_2_SIZE];

    int8_t            emc_tla_serial_num          [RESUME_PROM_EMC_TLA_SERIAL_NUM_SIZE];
    uint8_t     empty_space_3               [RESUME_PROM_EMPTY_SPACE_3_SIZE];

    int8_t            emc_system_hw_pn            [RESUME_PROM_EMC_SYSTEM_HW_PN_SIZE];
    int8_t            emc_system_hw_sn            [RESUME_PROM_EMC_SYSTEM_HW_SN_SIZE];
    int8_t            emc_system_hw_rev           [RESUME_PROM_EMC_SYSTEM_HW_REV_SIZE];
    uint8_t     empty_space_18              [RESUME_PROM_EMPTY_SPACE_18_SIZE];

    int8_t            product_part_number         [RESUME_PROM_PRODUCT_PN_SIZE];
    int8_t            product_serial_number       [RESUME_PROM_PRODUCT_SN_SIZE];
    int8_t            product_revision            [RESUME_PROM_PRODUCT_REV_SIZE];
    uint8_t     empty_space_19              [RESUME_PROM_EMPTY_SPACE_19_SIZE];

    int8_t            vendor_part_num             [RESUME_PROM_VENDOR_PART_NUM_SIZE];
    int8_t            vendor_artwork_rev          [RESUME_PROM_VENDOR_ARTWORK_REV_SIZE];
    int8_t            vendor_assembly_rev         [RESUME_PROM_VENDOR_ASSEMBLY_REV_SIZE];
    int8_t            vendor_unique_rev           [RESUME_PROM_VENDOR_UNIQUE_REV_SIZE];
    int8_t            vendor_acdc_input_type      [RESUME_PROM_VENDOR_ACDC_INPUT_TYPE_SIZE];
    uint8_t     empty_space_4               [RESUME_PROM_EMPTY_SPACE_4_SIZE];

    int8_t            vendor_serial_num           [RESUME_PROM_VENDOR_SERIAL_NUM_SIZE];
    uint8_t     empty_space_5               [RESUME_PROM_EMPTY_SPACE_5_SIZE];

    uint8_t         pcie_configuration          [RESUME_PROM_PCIE_CONFIGURATION_SIZE];
    uint8_t         board_power                 [RESUME_PROM_BOARD_POWER_SIZE];
    uint8_t         thermal_target              [RESUME_PROM_THERMAL_TARGET_SIZE];
    uint8_t         thermal_shutdown_limit      [RESUME_PROM_THERMAL_SHUTDOWN_LIMIT_SIZE];
    uint8_t     empty_space_20              [RESUME_PROM_EMPTY_SPACE_20_SIZE];

    int8_t            vendor_name                 [RESUME_PROM_VENDOR_NAME_SIZE];
    int8_t            loc_mft                     [RESUME_PROM_LOCATION_MANUFACTURE_SIZE];
    int8_t            year_mft                    [RESUME_PROM_YEAR_MANUFACTURE_SIZE];
    int8_t            month_mft                   [RESUME_PROM_MONTH_MANUFACTURE_SIZE];
    int8_t            day_mft                     [RESUME_PROM_DAY_MANUFACTURE_SIZE];
    uint8_t     empty_space_6               [RESUME_PROM_EMPTY_SPACE_6_SIZE];

    int8_t            tla_assembly_name           [RESUME_PROM_TLA_ASSEMBLY_NAME_SIZE];
    int8_t            contact_information         [RESUME_PROM_CONTACT_INFORMATION_SIZE];
    uint8_t     empty_space_7               [RESUME_PROM_EMPTY_SPACE_7_SIZE];

    uint8_t         num_prog                    [RESUME_PROM_NUM_PROG_SIZE];
    uint8_t     empty_space_8               [RESUME_PROM_EMPTY_SPACE_8_SIZE];

    RESUME_PROM_PROG_DETAILS    prog_details    [RESUME_PROM_MAX_PROG_COUNT];
    uint8_t     empty_space_9               [RESUME_PROM_EMPTY_SPACE_9_SIZE];

    uint32_t        wwn_seed;
    uint8_t         sas_address                 [RESUME_PROM_SAS_ADDRESS_SIZE];
    uint8_t     empty_space_10              [RESUME_PROM_EMPTY_SPACE_10_SIZE];

    uint8_t     reserved_region_1           [RESUME_PROM_RESERVED_REGION_1_SIZE];
    uint8_t     empty_space_11              [RESUME_PROM_EMPTY_SPACE_11_SIZE];

    int8_t            pcba_part_num               [RESUME_PROM_PCBA_PART_NUM_SIZE];
    int8_t            pcba_assembly_rev           [RESUME_PROM_PCBA_ASSEMBLY_REV_SIZE];
    int8_t            pcba_serial_num             [RESUME_PROM_PCBA_SERIAL_NUM_SIZE];
    uint8_t     empty_space_12              [RESUME_PROM_EMPTY_SPACE_12_SIZE];

    uint8_t         configuration_type          [RESUME_PROM_CONFIGURATION_TYPE_SIZE];
    uint8_t     empty_space_13              [RESUME_PROM_EMPTY_SPACE_13_SIZE];

    int8_t            emc_alt_mb_part_num         [RESUME_PROM_EMC_ALT_MB_PART_SIZE];
    uint8_t     empty_space_14              [RESUME_PROM_EMPTY_SPACE_14_SIZE];

    uint8_t         channel_speed               [RESUME_PROM_CHANNEL_SPEED_SIZE];
    uint8_t         system_type                 [RESUME_PROM_SYSTEM_TYPE_SIZE];
    uint8_t         dae_encl_id                 [RESUME_PROM_DAE_ENCL_ID_SIZE];
    uint8_t         rack_id                     [RESUME_PROM_RACK_ID_SIZE];
    uint8_t         slot_id                     [RESUME_PROM_SLOT_ID_SIZE];
    uint8_t     empty_space_15              [RESUME_PROM_EMPTY_SPACE_15_SIZE];

    uint8_t         drive_spin_up_select        [RESUME_PROM_DRIVE_SPIN_UP_SELECT_SIZE];
    uint8_t         sp_family_fru_id            [RESUME_PROM_SP_FAMILY_FRU_ID_SIZE];
    uint8_t         fru_capability              [RESUME_PROM_FRU_CAPABILITY_SIZE];
    uint8_t     empty_space_16              [RESUME_PROM_EMPTY_SPACE_16_SIZE];

    int8_t            emc_sub_assy_part_num       [RESUME_PROM_EMC_SUB_ASSY_PART_NUM_SIZE];
    int8_t            emc_sub_assy_artwork_rev    [RESUME_PROM_EMC_SUB_ASSY_ARTWORK_REV_SIZE];
    int8_t            emc_sub_assy_rev            [RESUME_PROM_EMC_SUB_ASSY_REV_SIZE];
    int8_t            emc_sub_assy_serial_num     [RESUME_PROM_EMC_SUB_ASSY_SERIAL_NUM_SIZE];
    uint8_t     empty_space_17              [RESUME_PROM_EMPTY_SPACE_17_SIZE];

    uint32_t        resume_prom_checksum;

} RESUME_PROM_STRUCTURE;

// Pack the structures to prevent padding.
#pragma pack(push, 1) 

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

typedef struct {
    uint8_t elem_stat_code : 4; ///< Bit 0-3, element status code,\ref ses_elem_stat_code_enum
    uint8_t swap           : 1; ///< Bit 4,   element was swapped
    uint8_t                  : 1; //   Bit 5
    uint8_t prdfail        : 1; ///< Bit 6,   predicted failure
    uint8_t                  : 1; //   Bit 7
} ses_cmn_stat_struct;

typedef struct {
    ses_cmn_stat_struct  cmn_stat;                  ///< byte 0,         common status byte
    uint8_t r_r_abort             : 1; ///< byte 1, bit 0,  copied from <code>rqst_r_r_abort</code> in \ref ses_ctrl_elem_esc_elec_struct "control" element
    uint8_t rebuild_remap         : 1; ///< byte 1, bit 1,  copied from <code>rqst_rebuild_remap</code> in \ref ses_ctrl_elem_esc_elec_struct "control" element
    uint8_t in_failed_array       : 1; ///< byte 1, bit 2,  copied from <code>rqst_in_failed_array</code> in \ref ses_ctrl_elem_esc_elec_struct "control" element
    uint8_t in_crit_array         : 1; ///< byte 1, bit 3,  copied from <code>rqst_in_crit_array</code> in \ref ses_ctrl_elem_esc_elec_struct "control" element
    uint8_t cons_check            : 1; ///< byte 1, bit 4,  copied from <code>rqst_cons_check</code> in \ref ses_ctrl_elem_esc_elec_struct "control" element
    uint8_t hot_spare             : 1; ///< byte 1, bit 5,  copied from <code>rqst_hot_spare</code> in \ref ses_ctrl_elem_esc_elec_struct "control" element
    uint8_t rsvd_dev              : 1; ///< byte 1, bit 6,  copied from <code>rqst_rsvd_dev</code> in \ref ses_ctrl_elem_esc_elec_struct "control" element
    uint8_t ok                    : 1; ///< byte 1, bit 7,  online pattern active (not displayed by Viper)
    uint8_t report                : 1; ///< byte 2, bit 0,  unused
    uint8_t ident                 : 1; ///< byte 2, bit 1,  identify pattern active
    uint8_t rmv                   : 1; ///< byte 2, bit 2,  copied from <code>rqst_rmv</code> in \ref ses_ctrl_elem_esc_elec_struct "control" element
    uint8_t rdy_to_insert         : 1; ///< byte 2, bit 3,  copied from <code>rqst_insert</code> in \ref ses_ctrl_elem_esc_elec_struct "control" element
    uint8_t encl_bypassed_b       : 1; ///< byte 2, bit 4,  unused
    uint8_t encl_bypassed_a       : 1; ///< byte 2, bit 5,  unused
    uint8_t do_not_remove         : 1; ///< byte 2, bit 6,  copied from <code>do_not_remove</code> in \ref ses_ctrl_elem_esc_elec_struct "control" element
    uint8_t app_client_bypassed_a : 1; ///< byte 2, bit 7,  unused
    uint8_t dev_bypassed_b        : 1; ///< byte 3, bit 0,  unused
    uint8_t dev_bypassed_a        : 1; ///< byte 3, bit 1,  unused
    uint8_t bypassed_b            : 1; ///< byte 3, bit 2,  unused
    uint8_t bypassed_a            : 1; ///< byte 3, bit 3,  unused
    uint8_t dev_off               : 1; ///< byte 3, bit 4,  slot powered off
    uint8_t fault_requested       : 1; ///< byte 3, bit 5,  failure pattern active due to local rqst_fault
    uint8_t fault_sensed          : 1; ///< byte 3, bit 6,  failure pattern active due to peer client request or EMA-detected fault
    uint8_t app_client_bypassed_b : 1; ///< byte 3, bit 7,  unused
} ses_stat_elem_array_dev_slot_struct;

/*SAS expander status element in the \ref ses_pg_encl_stat_struct "Enclosure Status" diagnostic page. */
typedef struct {
    ses_cmn_stat_struct        cmn_stat;  ///< byte 0, common status byte
    fbe_u8_t                        : 6;  ///< byte 1, bit 0-5,
    fbe_u8_t fail                   : 1;  ///< byte 1, bit 6,
    fbe_u8_t ident                  : 1;  ///< byte 1, bit 7, 
    fbe_u16_t reserved;                   ///< byte 2-3
} ses_stat_elem_sas_exp_struct;

SIZE_CHECK(ses_stat_elem_sas_exp_struct, 4);


/*!*************************************************************************
 *  @enum fbe_enclosure_type_t
 *  @brief 
 *    This defines all the valid enclosure types.
 *  FBE_ENCLOSURE_TYPE_LAST is always the last in the list.
 ***************************************************************************/ 
typedef enum 
{
    FBE_ENCLOSURE_TYPE_INVALID,
    /*! Fibre channel enclosure */
    FBE_ENCLOSURE_TYPE_FIBRE,
    /*! SAS backend enclosure */
    FBE_ENCLOSURE_TYPE_SAS,

    FBE_ENCLOSURE_TYPE_LAST
}fbe_enclosure_type_t;


typedef struct terminator_eses_buf_info_s
{
    uint8_t buf_id;

    // This is the alignment within the buffer
    // that dictates what offsets are allowed
    // into the buffer
    uint8_t buf_offset_boundary;
    // Indicates if the buffer is writable using
    // ESES commands like write buffer scsi cdb.
    bool writable;
    // buffer length in bytes.
    uint32_t buf_len;
    uint8_t *buf;
    uint8_t buf_type;
}terminator_eses_buf_info_t;

/* vp is short for virtual phy as the TAG is getting rather long*/
typedef struct vp_config_diag_page_info_s
{
    uint32_t gen_code;
    // pointer to config page being used by the Encl/Vphy
    uint8_t *config_page;
    uint32_t config_page_size;
    // pointer to the config page info being used by Encl/VPhy
    terminator_eses_config_page_info_t *config_page_info;
    ses_ver_desc_struct *ver_desc;
    uint8_t num_bufs;
    terminator_eses_buf_info_t *buf_info;
}vp_config_diag_page_info_t;

/*****************************************************************************/
/** Download Microcode Status Descriptor
 *  This is the status descriptor that's returned with the Download 
 *  Microcode Status Page.
 *  
 *****************************************************************************/
typedef struct fbe_download_status_desc_s
{
    uint8_t    rsvd;                   // [0]
    uint8_t    subencl_id;             // [1]          
    uint8_t    status;                 // [2]
    uint8_t    addl_status;            // [3]
    uint32_t   max_size;               // [4-7]
    uint8_t    rsvd1[3];               // [8-10]
    uint8_t    expected_buffer_id;     // [11]
    uint32_t   expected_buffer_offset; // [12-15]
} fbe_download_status_desc_t;


/* Note: Put prefix "terminator_eses" before this structure type name in future*/
typedef struct vp_download_microcode_stat_diag_page_info_s
{
    fbe_download_status_desc_t download_status_desc;
}vp_download_microcode_stat_diag_page_info_t;

typedef struct vp_download_microcode_ctrl_diag_page_info_s
{
     bool  image_download_in_progress;
     uint8_t    subenclosure_id; 
     uint8_t    buffer_id;
     uint32_t   image_length;
     uint8_t    *image_buffer;

}vp_download_microcode_ctrl_diag_page_info_t;

typedef enum
{
    TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_CURRENT,        // current mode parameter values
    TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_CHANGEABLE,     // changeable mode parameter values
    TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_DEFAULT,        // default mode parameter values
    TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_SAVED,          // saved mode parameter values
    TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_MAX
}terminator_eses_mode_param_value_type;

/************************************************
 * COMMON MODE PAGE HEADER.
 ************************************************/
typedef struct fbe_eses_pg_common_mode_pg_hdr_s 
{    
    uint8_t pg_code          :6;  // Byte 0, bit 0-5;
    uint8_t spf              :1;  // Byte 0, bit 6;
    uint8_t ps               :1;  // Byte 0, bit 7; 
                                   // In mode sense, set to 1 to indicate that parameter in this page can be saved in non-volatile memory (persistent). 
                                   // set to 0 to indicate that parameter in this page can be saved in non-volatile memory (non-persistent); 
                                   // In mode select, reserved.
    uint8_t pg_len;               // Byte 1, the number of bytes to follow.
}fbe_eses_mode_pg_common_hdr_t;


/************************************************
 * EMC ESES Persistent(eep) Mode Page(20h).
 ************************************************/
typedef struct fbe_eses_pg_eep_mode_pg_s 
{    
    fbe_eses_mode_pg_common_hdr_t mode_pg_common_hdr; // Byte 0-1
    uint8_t rsvd1            :4;  // Byte 2, bit 0-3;
    uint8_t bad_exp_recovery_enabled : 1; // Byte 2, bit 4;
    uint8_t ha_mode          :1;  // Byte 2, bit 5;
    uint8_t ssu_disable      :1;  // Byte 2, bit 6;
    uint8_t disable_indicator_ctrl :1; // Byte 2, bit 7;
    uint8_t rsvd2[13];                 // Byte 3-15;
} fbe_eses_pg_eep_mode_pg_t;

/************************************************
 * EMC ESES Non Persistent(eenp) Mode Page(21h).
 ************************************************/
typedef struct fbe_eses_pg_eenp_mode_pg_s
{    
    fbe_eses_mode_pg_common_hdr_t mode_pg_common_hdr; // Byte 0-1
    uint8_t sps_dev_supported                :1;  // Byte 2, bit 0;
    uint8_t rsvd1                            :2;  // Byte 2, bit 1-2;
    uint8_t include_drive_connectors         :1;  // Byte 2, bit 3;
    uint8_t disable_auto_shutdown            :1;  // Byte 2, bit 4;
    uint8_t disable_auto_cooling_ctrl        :1;  // Byte 2, bit 5;
    uint8_t activity_led_ctrl                :1;  // Byte 2, bit 6;
    uint8_t test_mode                        :1;  // Byte 2, bit 7;
    uint8_t rsvd2[13];            // Byte 3-15;
} fbe_eses_pg_eenp_mode_pg_t;

typedef struct ses_common_statistics_field_s
{
    uint8_t elem_offset;  // Byte 0
    uint8_t stats_len;    // Byte 1. (n-1)
} ses_common_statistics_field_t;

typedef struct fbe_eses_power_supply_stats_s
{
    ses_common_statistics_field_t common_stats; // Bytes 0-1
    uint8_t   dc_over_voltage_count;
    uint8_t   dc_under_voltage_count;
    uint8_t   fail_count;
    uint8_t   over_tempe_fail_count;
    uint8_t   ac_fail_count;
    uint8_t   dc_fail_count;
}fbe_eses_power_supply_stats_t;


typedef struct fbe_eses_cooling_stats_s
{
    ses_common_statistics_field_t common_stats; // Bytes 0-1
    uint8_t    fail_count;
}fbe_eses_cooling_stats_t;


typedef struct fbe_eses_temperature_stats_s
{
    ses_common_statistics_field_t common_stats; // Bytes 0-1
    uint8_t    ot_fail_count;
    uint8_t    ot_warning_count;
}fbe_eses_temperature_stats_t;

typedef struct fbe_eses_exp_stats_s
{
    ses_common_statistics_field_t common_stats; // Bytes 0-1
    uint16_t   exp_change_count;
}fbe_eses_exp_stats_t;

typedef struct fbe_eses_device_slot_stats_s
{
    ses_common_statistics_field_t common_stats; // Bytes 0-1
    uint8_t    insert_count;                   // Byte 2
    uint8_t    power_down_count;               // Byte 3
} fbe_eses_device_slot_stats_t;

typedef struct fbe_eses_exp_phy_stats_s
{
    ses_common_statistics_field_t common_stats; // Bytes 0-1
    uint8_t    rsvd;                           // Byte 2
    uint32_t   invalid_dword;                  // 3-6
    uint32_t   disparity_error;                // 7-10
    uint32_t   loss_dword_sync;                // 11-14
    uint32_t   phy_reset_fail;                 // 15-18
    uint32_t   code_violation;                 // 19-22
    uint8_t    phy_change;                     // 23
    uint16_t   crc_pmon_accum;                 // 24-25
    uint16_t   in_connect_crc;                 // 26-27
} fbe_eses_exp_phy_stats_t;


// vp stands for virtual phy
typedef struct terminator_eses_vp_mode_page_info_s
{
    fbe_eses_pg_eep_mode_pg_t   eep_mode_pg[TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_MAX];
    fbe_eses_pg_eenp_mode_pg_t  eenp_mode_pg[TERMINATOR_ESES_MODE_PARAMETER_VALUE_TYPE_MAX];
} terminator_eses_vp_mode_page_info_t;

typedef struct vp_eses_page_info_s
{
    vp_config_diag_page_info_t                   vp_config_diag_page_info;
    vp_download_microcode_stat_diag_page_info_t  vp_download_micrcode_stat_diag_page_info;
    vp_download_microcode_ctrl_diag_page_info_t  vp_download_microcode_ctrl_diag_page_info;
    terminator_eses_vp_mode_page_info_t          vp_mode_page_info;
} terminator_vp_eses_page_info_t;

#pragma pack(pop) /* Go back to default alignment.*/

/**********************************/
/*     enclosure inquiry local variables         */
/**********************************/

typedef struct terminator_sas_encl_inq_data_s {
    fbe_sas_enclosure_type_t    encl_type;
    fbe_u16_t                   board_type;
    const char                  *type_str;
    const char                  *vendor;
    const char                  *product_id;
    const char                  *product_revision_level;
    const char                  *component_vendor_id;
    fbe_u16_t                   component_id;
    fbe_u8_t                    component_revision_level;
    const char                  *enclosure_unique_id;
    fbe_u16_t                   eses_version_descriptor;
    fbe_u16_t                   enclosure_platform_type;
} terminator_sas_encl_inq_data_t;

typedef struct terminator_sas_unsupported_eses_page_info_s{
    fbe_u8_t unsupported_receive_diag_pages[MAX_POSSIBLE_ESES_PAGES];
    fbe_u32_t unsupported_receive_diag_page_count;
    fbe_u8_t unsupported_send_diag_pages[MAX_POSSIBLE_ESES_PAGES];
    fbe_u32_t unsupported_send_diag_page_count;
} terminator_sas_unsupported_eses_page_info_t;

#pragma pack(push, ses, 1)
/*Expander Phy status element in the \ref ses_pg_encl_stat_struct "Enclosure Status" diagnostic page. */
typedef struct {
    ses_cmn_stat_struct   cmn_stat;                        ///< byte 0, common status byte
    fbe_u8_t exp_index;              ///< byte 1,index of sas expander or esc electronics element representing the expander containing the PHY    
    fbe_u8_t phy_id                 : 7;  ///< byte 2, bit 0-6, Logical identifier of the PHY
    fbe_u8_t force_disabled         : 1;  ///< byte 2, bit 7,  indicates EMA disabled PHY that it self-diagnoised
    fbe_u8_t reserved               : 3;  ///< byte 3, bit 0-2, 
    fbe_u8_t carrier_detect         : 1;  ///< byte 3, bit 3, set if something other than storage device inserted in slot
    fbe_u8_t sata_spinup_hold       : 1;  ///< byte 3, bit 4, set if SATA drive inserted in a slot that is in spinup hold state
    fbe_u8_t spinup_enabled         : 1;  ///< byte 3, bit 5, set 
    fbe_u8_t link_rdy               : 1;  ///< byte 3, bit 6, set if the link is ready on the Phy
    fbe_u8_t phy_rdy                : 1;  ///< byte 3, bit 7, corresponds to the SAS_PHY_Ready state
} ses_stat_elem_exp_phy_struct;

SIZE_CHECK(ses_stat_elem_exp_phy_struct, 4);

/** Power Supply status element in the \ref ses_pg_encl_stat_struct "Enclosure Status" diagnostic page. */
typedef struct {
    ses_cmn_stat_struct     cmn_stat;           ///< byte 0,         common status byte
    PAD                 : 7; ///< byte 1, bit 0-6
    fbe_u8_t ident         : 1; ///< byte 1, bit 7,  identify pattern active
    PAD                 : 1; ///< byte 2, bit 0
    fbe_u8_t dc_over_curr  : 1; ///< byte 2, bit 1,  DC over current
    fbe_u8_t dc_under_vltg : 1; ///< byte 2, bit 2,  DC under voltage
    fbe_u8_t dc_over_vltg  : 1; ///< byte 2, bit 3,  DC over voltage
    PAD                 : 4; ///< byte 2, bit 4-7
    fbe_u8_t dc_fail       : 1; ///< byte 3, bit 0,  at least one DC power failed
    fbe_u8_t ac_fail       : 1; ///< byte 3, bit 1,  input power failed
    fbe_u8_t temp_warn     : 1; ///< byte 3, bit 2,  over temperature warning
    fbe_u8_t overtmp_fail  : 1; ///< byte 3, bit 3,  over temperature failure
    fbe_u8_t off           : 1; ///< byte 3, bit 4,  no DC power at all
    fbe_u8_t rqsted_on     : 1; ///< byte 3, bit 5,  installed and turned on
    fbe_u8_t fail          : 1; ///< byte 3, bit 6,  failure pattern active
    fbe_u8_t hot_swap      : 1; ///< byte 3, bit 7,  hot swap doesn't remove power
} ses_stat_elem_ps_struct;

SIZE_CHECK(ses_stat_elem_ps_struct, 4);


/*SAS connector status element in the \ref ses_pg_encl_stat_struct "Enclosure Status" diagnostic page. */
typedef struct {
    ses_cmn_stat_struct        cmn_stat;  ///< byte 0, common status byte
    fbe_u8_t conn_type              : 7;  ///< byte 1, bit 0-6    
    fbe_u8_t ident                  : 1;  ///< byte 1, bit 7, 
    fbe_u8_t conn_physical_link;          ///< byte 2, 
    fbe_u8_t                        : 6;  ///< byte 3, bit 0-5, 
    fbe_u8_t fail                   : 1;  ///< byte 3, bit 6, 
    fbe_u8_t                        : 1;  ///< byte 3, bit 4, set if SATA drive inserted in a slot that is in spinup hold state
} ses_stat_elem_sas_conn_struct;


/** Cooling status element in the \ref ses_pg_encl_stat_struct "Enclosure Status" diagnostic page. */
typedef struct {
    ses_cmn_stat_struct
           cmn_stat;                   ///< byte 0,            common status byte
    fbe_u8_t  actual_fan_speed_high  : 2; ///< byte 1,  bit 0-1 , speed in 10's of rpms
    PAD                           : 5; ///< byte 1,  bit 2-6
    fbe_u8_t   ident                 : 1; ///< byte 1,  bit 7,  identify pattern active
    fbe_u8_t  actual_fan_speed_low      ; ///< byte 2,  speed in 10's of rpms
    fbe_u8_t  actual_speed_code      : 3; ///< byte 3,  bit 0-2, actual speed code, \ref ses_fan_speed_code_enum
    PAD                           : 1; ///< byte 3,  bit 3
    fbe_u8_t   off                   : 1; ///< byte 3,  bit 4,  no cooling
    fbe_u8_t   rqsted_on             : 1; ///< byte 3,  bit 5,  installed and turned on
    fbe_u8_t   fail                  : 1; ///< byte 3,  bit 6,  failure pattern active
    PAD                           : 1; ///< byte 3,  bit 7
} ses_stat_elem_cooling_struct;


/** Temperature Sensor status element in the \ref ses_pg_encl_stat_struct "Enclosure Status" diagnostic page. */
typedef struct {
    ses_cmn_stat_struct        cmn_stat;        ///< byte 0,         common status byte
    PAD               : 7; ///< byte 1, bit 0-6
    fbe_u8_t  ident      : 1; ///< byte 1, bit 7,  identify pattern active
    fbe_u8_t  temp       : 8; ///< byte 2,         temperature + 20C
    fbe_u8_t  ut_warning : 1; ///< byte 3, bit 0,  unused
    fbe_u8_t  ut_failure : 1; ///< byte 3, bit 1,  unused
    fbe_u8_t  ot_warning : 1; ///< byte 3, bit 2,  Exceed high warning threshold
    fbe_u8_t  ot_failure : 1; ///< byte 3, bit 3,  Exceed high critical threshold
    PAD               : 4; ///< byte 3, bit 4-7
} ses_stat_elem_temp_sensor_struct;

/*Display element in the \ref ses_pg_encl_ctrl_struct "Display Control" diagnostic page. */
typedef struct {
    ses_cmn_stat_struct        cmn_stat; ///< byte 0, common status byte
    fbe_u8_t display_mode_status    : 2; ///< byte 1, bit 0-1
    PAD                             : 4; ///< byte 1, bit 2-5
    fbe_u8_t fail                   : 1; ///< byte 1, bit 6,
    fbe_u8_t ident                  : 1; ///< byte 1, bit 7,  activate identify pattern
    fbe_u8_t display_char_stat;          ///< byte 2
    PAD                             : 8; ///< byte 3
} ses_stat_elem_display_struct;

/*Enclosure element in the \ref ses_pg_encl_stat_struct "Enclosure Status" diagnostic page. */
typedef struct {
    ses_cmn_stat_struct        cmn_stat;  ///< byte 0, common status byte
    fbe_u8_t                        : 7;  ///< byte 1, bit 0-6,
    fbe_u8_t ident                  : 1;  ///< byte 1, bit 7, 
    fbe_u8_t warning_indicaton      : 1;  ///< byte 2, bit 0, 
    fbe_u8_t failure_indicaton      : 1;  ///< byte 2, bit 1, 
    fbe_u8_t time_until_power_cycle : 6;  ///< byte 2, bit 2-7, 
    fbe_u8_t warning_rqsted         : 1;  ///< byte 3, bit 1, 
    fbe_u8_t failure_rqsted         : 1;  ///< byte 3, bit 1, 
    fbe_u8_t rqsted_power_off_duration : 6;  ///< byte 3, bit 2-7,
} ses_stat_elem_encl_struct;

/************************************************
 * EMC Enclosure Control/Status Diagnostic Page.
 ************************************************/
typedef struct ses_pg_emc_encl_stat_s 
{    
    ses_common_pg_hdr_struct hdr;  // Byte 0-7, the common page header;
    fbe_u8_t reserved[2];          // Byte 8-9;
    fbe_u8_t shutdown_reason : 7;  // Byte 10, bit 0-6, reserved in a control page;
    fbe_u8_t partial         : 1;  // Byte 10, bit 7, reserved in a control page;
    fbe_u8_t num_info_elem_groups; // Byte 11, num of # information element groups;
    // information element groups
} ses_pg_emc_encl_stat_struct;

typedef struct fbe_eses_info_elem_group_hdr_s
{
    uint8_t info_elem_type;  // Byte 0
    uint8_t num_info_elems;  // Byte 1
    uint8_t info_elem_size;  // Byte 2     
}fbe_eses_info_elem_group_hdr_t;

typedef struct ses_trace_buf_info_elem_s
{
    uint8_t buf_id;      // Byte 0
    uint8_t buf_action;  // Byte 1
    uint8_t elem_index;   // Byte 2
    uint8_t rev_level[5];    // Byte 3-7, used in a status element only and reserved in a control element
    uint8_t timestamp[21];    // Byte 8-28, used in a status element only and reserved in a control element
}ses_trace_buf_info_elem_struct;


typedef struct ses_addl_elem_stat_hdr_desc_s
{
    uint8_t protocol_id : 4;   //Byte 0, bit 0-3
    uint8_t eip     : 1;       //Byte 0, bit 4
    uint8_t         : 2;       //Byte 0, bit 6,5
    uint8_t invalid : 1;       //Byte 0, bit 7
    uint8_t desc_len;          //Byte 1
    uint8_t reserved;          //Byte 2
    uint8_t elem_index;        //Byte 3
    /* Protocol_specific information */
    
}ses_addl_elem_stat_desc_hdr_struct;

typedef struct ses_sas_exp_prot_spec_info_s
{
    uint8_t num_exp_phy_descs; // Byte 0   
    uint8_t           : 6;     // Byte 1, bit 0-5
    uint8_t desc_type : 2;     // Byte 1, bit 6-7
    uint16_t reserved;         // Byte 2-3
    uint64_t sas_address;      // Byte 4-11
    /* Expander Phy descriptor */
}ses_sas_exp_prot_spec_info_struct;

typedef struct ses_sas_exp_phy_desc_s{
    uint8_t conn_elem_index;     // Byte 0
    uint8_t other_elem_index;    // Byte 1
}ses_sas_exp_phy_desc_struct;

typedef struct ses_array_dev_slot_prot_spec_info_s
{
    uint8_t num_phy_descs;  // Byte 0   
    uint8_t not_all_phys : 1;   // Byte 1, bit 0
    uint8_t           : 5;   // Byte 1, bit 1-5
    uint8_t desc_type : 2;   // Byte 1, bit 6-7
    uint8_t reserved;       // byte 2
    uint8_t dev_slot_num;   // Byte 3
    /* Array Device Phy descriptor */
}ses_array_dev_slot_prot_spec_info_struct;

typedef struct ses_array_dev_phy_desc_s{
    uint8_t           :4;  
    uint8_t device_type:3;
    uint8_t           :1; // Byte 7
                    
    uint8_t reserved0; // Byte 8
    uint8_t           :1;  
    uint8_t smp_initiator_port:1;
    uint8_t stp_initiator_port:1;
    uint8_t ssp_initiator_port:1;
    uint8_t           :4; // Byte 9

    uint8_t sata_device:1;
    uint8_t smp_target_port:1;
    uint8_t stp_target_port:1;
    uint8_t ssp_target_port:1;
    uint8_t                :3;  
    uint8_t sata_port_selector:1; // Byte 10

    uint64_t attached_sas_address; // Byte 11 
    uint64_t sas_address;   // Byte 12-19
    uint8_t phy_id;         // Byte 20
    uint8_t reserved1[7];    // Byte 21-27
}ses_array_dev_phy_desc_struct;

/******************************************************************************
 * ESC ELectronics Protocol Specific Info  
 * in the additional element status descriptor of the additional status page.
 ******************************************************************************/
typedef struct ses_esc_elec_prot_spec_info_s
{
    fbe_u8_t num_exp_phy_descs;  // Byte 0   
    fbe_u8_t           : 6;     // Byte 1, bit 0-5
    fbe_u8_t desc_type : 2;     // Byte 1, bit 6-7
    fbe_u16_t reserved;         // byte 2-3
    /* Expander Phy descriptor */
}ses_esc_elec_prot_spec_info_struct;

/******************************************************************************
 * Expander Phy Descriptor in ESC ELectronics Protocol Specific Info  
 * in the additional element status descriptor of the additional status page.
 ******************************************************************************/
typedef struct ses_esc_elec_exp_phy_desc_s{
    fbe_u8_t phy_id;    // Byte 0
    fbe_u8_t reserved;   // Byte 1
    fbe_u8_t conn_elem_index;     // Byte 2
    fbe_u8_t other_elem_index;    // Byte 3
    fbe_u64_t sas_address; // Byte 4-11   
}ses_esc_elec_exp_phy_desc_struct;


/************************************
 * Power Supply Information Element
 ************************************/
typedef struct ses_ps_info_elem_s
{
    fbe_u8_t    ps_elem_index;           // Byte 0
    fbe_u8_t    input_power_valid : 1;   // Byte 0, bit 0, reserved in control element
    fbe_u8_t    low_power_mode_b : 1;    // Byte 0, bit 1, reserved in control element
    fbe_u8_t    low_power_mode_a : 1;    // Byte 0, bit 2, reserved in control element
    fbe_u8_t    margining_test_mode : 4; // Byte 0, bit 3-6, reserved in control element
    fbe_u8_t    reserved : 1;            // Byte 1, bit 7
    fbe_u8_t    margining_test_results;  // Byte 2,
    fbe_u16_t   input_power;             // Byte 3-4.
} ses_ps_info_elem_struct;

/************************************
 * General Information Element Array device slot
 ************************************/
typedef struct ses_general_info_elem_array_dev_slot_s
{
    fbe_u8_t elem_index;        // Byte 0
    fbe_u8_t reserved1 : 6;     // Byte 1, bit 0-5
    fbe_u8_t battery_backed : 1;     // Byte 1, bit 6
    fbe_u8_t fru : 1;           // Byte 1, bit 7, reserved in a control element.
    fbe_u8_t duration : 7;      // Byte 2, bit 0-6
    fbe_u8_t power_cycle : 1;   // Byte 2, bit 7
    fbe_u8_t reserved2;         // Byte 3
} ses_general_info_elem_array_dev_slot_struct;

/****************************************************************************
 ****************************************************************************//**
 ** Enclosure Status diagnostic page (02h).
 **
 ** This is a status page that reports overall health of the enclosure and
 ** contains a 4-byte status element for each element within
 ** each subenclosure defined in the
 ** \ref ses_pg_config_struct "Configuration diagnostic page".
 ****************************************************************************
 ****************************************************************************/

typedef struct {
    fbe_u8_t pg_code      : 8; ///< Byte 0            \ref SES_PG_CODE_ENCL_STAT, \ref ses_pg_code_enum
    fbe_u8_t unrecov      : 1; ///< Byte 1, bit 0, unrecoverable condition active; see \ref ses_elem_stat_code_enum
    fbe_u8_t crit         : 1; ///< Byte 1, bit 1, critical condition active; see \ref ses_elem_stat_code_enum
    fbe_u8_t non_crit     : 1; ///< Byte 1, bit 2, non-critical condition active; see \ref ses_elem_stat_code_enum
    fbe_u8_t info         : 1; ///< Byte 1, bit 3,
                            ///< set once on most changes to this page per I_T nexus
    fbe_u8_t invop        : 1; ///< Byte 1, bit 4,    always 0 in ESES
    PAD                : 3; //   Byte 1, bits 7-5
    fbe_u16_t pg_len;        ///< Byte 2-3,length of rest of page
    fbe_u32_t gen_code;      ///< Byte 4-7,generation code; see ses_pg_config_struct.gen_code
    ///                                         Byte 8-11+,    status elements.
    /// For each \ref ses_type_desc_hdr_struct "type descriptor header" in each
    /// \ref ses_subencl_desc_struct "subenclosure descriptor" in the
    /// \ref ses_pg_config_struct "Configuration diagnostic page", 
    /// there is one overall status elements for that type, followed by
    /// one individual status element for each element, as indicated by
    /// \ref ses_type_desc_hdr_struct.num_possible_elems.
    /// The sequence of individual status elements (not including overall
    /// status elements) defines the \ref SES_ELEM_INDEX "element index"
    /// for each element.  Because elements are fixed-length, you can
    /// directly index into the i'th status element by referencing \p stat_elem[i].
    /// but, because of the overall status elements, the index into this array is not
    /// equal to the element's \ref ses_elem_index.
    fbe_u8_t stat_elem[1]; ///// \ref ses_stat_elem_struct    
} ses_pg_encl_stat_struct;

#pragma pack(pop, ses)

/* definition of a terminator_eses_buf_node for holding buf memory to avoid the memory leak */
typedef struct terminator_eses_free_mem_node_s 
{
   fbe_u8_t *buf;
   struct terminator_eses_free_mem_node_s *next;
} terminator_eses_free_mem_node_t;

typedef struct terminator_enclosure_firmware_new_rev_record_s
{
    fbe_queue_element_t queue_element;
    ses_subencl_type_enum subencl_type;
    terminator_eses_subencl_side side;
    ses_comp_type_enum  comp_type;
    fbe_u32_t slot_num;
    char new_rev_number[16];
} terminator_enclosure_firmware_new_rev_record_t;

typedef struct terminator_sas_virtual_phy_info_s{
    fbe_sas_address_t        device_address;
    fbe_sas_enclosure_type_t enclosure_type;
    fbe_u8_t side;

    ses_stat_elem_array_dev_slot_struct *drive_slot_status;
    fbe_u8_t *drive_slot_insert_count;
    fbe_u8_t *drive_power_down_count;

    ses_stat_elem_exp_phy_struct *phy_status;

    ses_stat_elem_ps_struct *ps_status;
    ses_stat_elem_sas_conn_struct *sas_conn_status;

    ses_stat_elem_cooling_struct *cooling_status;

    ses_stat_elem_temp_sensor_struct *temp_sensor_status;
    ses_stat_elem_temp_sensor_struct *overall_temp_sensor_status;

    ses_stat_elem_display_struct *display_status;

    // There is one enclosure element for each LCC subenclosure
    // and chassis subenclosure. There is no ability to control
    // indicators in the peer LCC's, so no structure defined for
    // peer for now.
    ses_stat_elem_encl_struct encl_status;

    ses_stat_elem_encl_struct chassis_encl_status;

    ses_pg_emc_encl_stat_struct emcEnclStatus;

    ses_ps_info_elem_struct emcPsInfoStatus;

    ses_general_info_elem_array_dev_slot_struct * general_info_elem_drive_slot;   

    // dont declare as bool , as in future we may want to set
    // unit attention N number of times--etc.
    fbe_u8_t unit_attention;

    // All the eses info related to ESES pages other
    // than "Status elements"(status page) is
    // is stored here
    terminator_vp_eses_page_info_t eses_page_info;
    fbe_u32_t           miniport_sas_device_table_index;
    // queue holds new firmware revision numbers for different components
    fbe_queue_head_t    new_firmware_rev_queue_head;
    // time intervals to specify how long we need to wait before
    // starting logging out and logging in LCC subenclosure in ms
    fbe_u32_t activate_time_intervel;
    fbe_u32_t reset_time_intervel;
    void *ses_dev;
} terminator_sas_virtual_phy_info_t;


#define SWP16(d) ((((d) & 0xFF) << 8) | (((d) >> 8) & 0xFF))
#define BYTE_SWAP_16(p) ((((p) >> 8) & 0x00ff) | (((p) << 8) & 0xff00))
#define BYTE_SWAP_32(p) \
    ((((p) & 0xff000000) >> 24) |   \
     (((p) & 0x00ff0000) >>  8) |   \
     (((p) & 0x0000ff00) <<  8) |   \
     (((p) & 0x000000ff) << 24))
#define BYTE_SWAP_64(p) \
    ((((p) & 0xff00000000000000) >> 56) |   \
     (((p) & 0x00ff000000000000) >> 40) |   \
     (((p) & 0x0000ff0000000000) >> 24) |   \
     (((p) & 0x000000ff00000000) >>  8) |   \
     (((p) & 0x00000000ff000000) <<  8) |   \
     (((p) & 0x0000000000ff0000) << 24) |   \
     (((p) & 0x000000000000ff00) << 40) |   \
     (((p) & 0x00000000000000ff) << 56))

extern fbe_u8_t tabasco_config_page_with_ps[][1200];
extern terminator_eses_config_page_info_t tabasco_config_page_info_with_ps[2];

fbe_status_t config_page_init(fbe_u8_t side);
fbe_status_t sanity_check_cdb_receive_diagnostic_results(fbe_u8_t *cdb_page_ptr, fbe_u8_t *page_code);
fbe_status_t sas_enclosure_get_inq_data (fbe_sas_enclosure_type_t encl_type, terminator_sas_encl_inq_data_t **inq_data);
fbe_status_t sas_virtual_phy_check_enclosure_type(fbe_sas_enclosure_type_t encl_type, terminator_sp_id_t spid);
fbe_status_t fbe_terminator_api_get_sp_id(terminator_sas_virtual_phy_info_t *info, terminator_sp_id_t *sp_id);
fbe_u32_t enclosure_status_diagnostic_page_size(fbe_sas_enclosure_type_t encl_type);
fbe_sas_address_t sas_enclosure_calculate_virtual_phy_sas_address(SCSISESState *s);

fbe_status_t sas_virtual_phy_max_phys(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_phys, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_drive_slots(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_drive_slots, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_display_characters(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_diplay_characters, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_temp_sensor_elems(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_temp_sensor_elems, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_ps_elems(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_ps_elems, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_cooling_elems(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_cooling_elems, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_conns_per_lcc(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_conns, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_conns_per_port(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_conns, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_ext_cooling_elems(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_ext_cooling_elems, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_bem_cooling_elems(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_bem_cooling_elems, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_lccs(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_lccs, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_ee_lccs(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_ee_lccs, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_get_drive_slot_to_phy_mapping(fbe_u8_t drive_slot, fbe_u8_t *phy_id, fbe_sas_enclosure_type_t encl_type, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_get_drive_power_down_count(terminator_sas_virtual_phy_info_t * info, fbe_u32_t slot_number, fbe_u8_t *power_down_count);
fbe_status_t sas_virtual_phy_get_drive_slot_insert_count(terminator_sas_virtual_phy_info_t * info, fbe_u32_t slot_number, fbe_u8_t *insert_count);
fbe_status_t sas_virtual_phy_max_single_lane_conns_per_port(fbe_sas_enclosure_type_t encl_type, uint8_t *max_conns, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_max_conn_id_count(fbe_sas_enclosure_type_t encl_type, uint8_t *max_conn_id_count, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_get_individual_conn_to_phy_mapping(uint8_t individual_lane, uint8_t connector_id, uint8_t *phy_id, fbe_sas_enclosure_type_t encl_type, terminator_sp_id_t spid);
fbe_status_t sas_virtual_phy_get_emcEnclStatus(terminator_sas_virtual_phy_info_t *info, ses_pg_emc_encl_stat_struct *emcEnclStatusPtr);
fbe_status_t sas_virtual_phy_get_emcPsInfoStatus(terminator_sas_virtual_phy_info_t *info, ses_ps_info_elem_struct *emcPsInfoStatusPtr);
fbe_status_t sas_virtual_phy_get_emcGeneralInfoDirveSlotStatus(terminator_sas_virtual_phy_info_t *info, ses_general_info_elem_array_dev_slot_struct *emcGeneralInfoDirveSlotStatusPtr, fbe_u8_t drive_slot);


terminator_sas_virtual_phy_info_t * sas_virtual_phy_info_new(fbe_sas_enclosure_type_t encl_type, fbe_sas_address_t sas_address, fbe_u8_t side);

fbe_status_t terminator_initialize_eses_page_info(fbe_sas_enclosure_type_t encl_type, terminator_vp_eses_page_info_t *eses_page_info);

fbe_status_t config_page_get_gen_code(terminator_sas_virtual_phy_info_t *virtual_phy_handle, fbe_u32_t *gen_code);
fbe_status_t config_page_set_gen_code_by_config_page(fbe_u8_t *config_page, fbe_u32_t gen_code);
fbe_status_t config_page_set_all_ver_descs_in_config_page(fbe_u8_t *config_page, terminator_eses_config_page_info_t *config_page_info, ses_ver_desc_struct *ver_desc, fbe_u16_t eses_version);

fbe_status_t enclosure_status_diagnostic_page_build_status_elements( fbe_u8_t *encl_stat_diag_page_start_ptr, fbe_u8_t **stat_elem_end_ptr, terminator_sas_virtual_phy_info_t *info); 

fbe_status_t emc_statistics_stat_page_build_device_slot_stats(terminator_sas_virtual_phy_info_t *info, uint8_t *device_slot_stats_start_ptr, uint8_t **device_slot_stats_end_ptr);
fbe_status_t emc_statistics_stat_page_build_power_supply_stats(terminator_sas_virtual_phy_info_t *s, uint8_t *device_slot_stats_start_ptr, uint8_t **device_slot_stats_end_ptr);
fbe_status_t emc_statistics_stat_page_build_cooling_stats(terminator_sas_virtual_phy_info_t *s, uint8_t *device_slot_stats_start_ptr, uint8_t **device_slot_stats_end_ptr);
fbe_status_t emc_statistics_stat_page_build_temp_sensor_stats(terminator_sas_virtual_phy_info_t *s, uint8_t *device_slot_stats_start_ptr, uint8_t **device_slot_stats_end_ptr);
fbe_status_t emc_statistics_stat_page_build_sas_exp_stats(terminator_sas_virtual_phy_info_t *s, uint8_t *device_slot_stats_start_ptr, uint8_t **device_slot_stats_end_ptr);
fbe_status_t emc_statistics_stat_page_build_exp_phy_stats(terminator_sas_virtual_phy_info_t *s, uint8_t *device_slot_stats_start_ptr, uint8_t **device_slot_stats_end_ptr);
fbe_status_t emc_encl_stat_diag_page_build_sas_conn_inf_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *sas_conn_elem_start_ptr, uint8_t **sas_conn_elem_end_ptr, uint8_t *num_sas_conn_info_elem);
fbe_status_t emc_encl_stat_diag_page_build_trace_buffer_inf_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *trace_buffer_elem_start_ptr, uint8_t **trace_buffer_elem_end_ptr, uint8_t *num_trace_buffer_info_elem);
fbe_status_t emc_encl_stat_diag_page_build_general_info_expander_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *general_info_elem_start_ptr, uint8_t **general_info_elem_end_ptr, uint8_t *num_general_info_elem);
fbe_status_t emc_encl_stat_diag_page_build_general_info_drive_slot_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *general_info_elem_start_ptr, uint8_t **general_info_elem_end_ptr, uint8_t *num_general_info_elem);
fbe_status_t emc_encl_stat_diag_page_build_ps_info_elems(terminator_sas_virtual_phy_info_t *s, uint8_t *ps_info_elem_start_ptr, uint8_t **ps_info_elem_end_ptr, uint8_t *num_ps_info_elem); 
fbe_status_t addl_elem_stat_page_build_stat_descriptors(terminator_sas_virtual_phy_info_t *info, uint8_t *status_elements_start_ptr, uint8_t **status_elements_end_ptr);
fbe_status_t fbe_terminator_sas_enclosure_get_eses_version_desc(fbe_sas_enclosure_type_t encl_type, uint16_t *eses_version_desc_ptr);


#endif /* _ESES_H */
