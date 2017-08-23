#include "eses.h"
#include "trace.h"
#include "trace/control.h"

/**********************************/
/*        local variables         */
/**********************************/

/****************************************************
The order of arrangement of ESES enclosure attributes
for each enclosure type.
1. encl_type
2. max_drive_count
3. max_phy_count
4. max_encl_conn_count
5. max_lcc_conn_count
6. max_port_conn_count
7. max_single_lane_port_conn_count
8. max_ps_count
9. max_cooling_elem_count
10. max_temp_sensor_elems_count
11. drive_slot_to_phy_map
12. max display character_count
13. max lccs
14. max ee lccs
15. max ext cooling elements 
16. max_conn_id_count 
17. Invidiual connector to phy mapping 
NOTE: 255 is used in the mapping array if the mapping is
      undefined. (Ex: There are no expansion port connectors
      for Magnum).
*****************************************************/

#define SELECT_ENCL_ESES_INFO_TABLE(spid)     ((!(spid)) ? a_encl_eses_info_table : b_encl_eses_info_table)

#define DEFAULT_ENCLOSURE_FIRMWARE_ACTIVATE_TIME_INTERVAL 0
#define DEFAULT_ENCLOSURE_FIRMWARE_RESET_TIME_INTERVAL 0

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-braces"

terminator_sas_encl_eses_info_t a_encl_eses_info_table[] = {
    {FBE_SAS_ENCLOSURE_TYPE_BULLET, 15,    36,     20,     10,      5,      4,      2,      4,      1, 0, 0, 0, 0, 2, 0, 0,0, 2, 0, 0},

    {FBE_SAS_ENCLOSURE_TYPE_VIPER,  15,    36,     20,     10,      5,      4,      2,      4,      1,
        {20, 22, 23, 21, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9},  //drive_slot_to_phy_map[]
        3, 2, 0, 0, 0, 2,
        {   
            {4, 5, 6, 7},  // Connector Id 0 individual connector to phy map
            {0, 1, 2, 3},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_PINECONE,  12,    24,     20,     10,      5,      4,      2,      4,      0,
        {19, 20, 21, 22, 15, 18, 23, 16, 17, 14, 13, 12},  //drive_slot_to_phy_map[]
        3, 2, 0, 0, 0, 2,
        {   
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_MAGNUM, 15,    24,     10,     5,      5,      4,      0,      0,      0,
        {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18}, 0, 2, 0 ,0, 0, 2,
        {   
            {0, 1, 2, 3},          // Connector Id 0 individual connector to phy map
            {255, 255, 255, 255},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
        
    {FBE_SAS_ENCLOSURE_TYPE_BUNKER, 15,    36,     20,     10,      5,      4,      0,      0,      0,
        {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26}, 0, 2, 0 ,0,0, 2,
        {
            {1, 0, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    }, 
        
    {FBE_SAS_ENCLOSURE_TYPE_CITADEL, 25,    36,     20,     10,      5,      4,      0,      0,      0,
        {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}, 0, 2, 0, 0,0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
        
    {FBE_SAS_ENCLOSURE_TYPE_DERRINGER, 25,    36,     20,     10,      5,      4,      2,      4,      1,
        {11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35},      
         3, 2, 0, 0, 0, 2,
         {
            {4,5,6,7},    // Connector Id 0 individual connector to phy map
            {0, 1, 2, 3}, // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },     
         
    {FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM,  0,    24,     60,     30,      5,      4,      2,      
    7,      1,
        {0},              //drive_slot_to_phy_map[]
        3, 4 , 2, 3, 0, 6,
        {
            {20, 21, 22, 23},  // Connector Id 0 individual connector to phy map
            {12, 13, 14, 15},  // Connector Id 1 individual connector to phy map
            {16, 17, 18, 19},  // Connector Id 2 individual connector to phy map
            {8, 9, 10, 11},    // Connector Id 3 individual connector to phy map
            {0, 1, 2, 7},      // Connector Id 4 individual connector to phy map
            {3, 4, 5, 6}       // Connector Id 5 individual connector to phy map
        },
        {0, 0, 0, 0, 0, 30}    // Connector id to drive start slot
    },
    
    /* Below mapping is from EE at A side only as 2 EE have different mapping for simulation purpose */ 
    {FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE,  30,    36,     10,     5,      5,      4,      0,      0,      0,
        // Below is drive_slot_to_phy_map[]
        {6,  1,  2,  5, 9, 8, 3,  7,  15, 18, 22, 27, 0,  4, 12, 20, 23, 28, 11, 10, 13, 19, 24, 30, 14, 16, 17, 21, 26, 25},
        0, 2, 0, 0, 0, 1,
        {
            {31, 32, 33, 34},  // Connector Id 0 individual connector to phy map
            {0},
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },        
        
    {FBE_SAS_ENCLOSURE_TYPE_FALLBACK, 25,    36,     20,     10,      5,      4,      0,     3,      1,
        {32, 31, 29, 25, 22, 18, 17, 19, 20, 21, 23, 24, 26, 27, 28, 30, 16, 15, 14, 13, 12, 11, 10, 9, 8}, 0, 2, 0, 0, 3, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
        
    {FBE_SAS_ENCLOSURE_TYPE_BOXWOOD, 12,    36,     20,     10,      5,      4,      0,      0,      0,
        {15, 16, 17, 18, 11, 14, 19, 12, 13, 10, 9, 8}, 0, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_KNOT, 25,    36,     20,     10,      5,      4,      0,      0,      0,
        {8, 9, 10, 11, 12, 13, 14, 15, 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26, 27, 28, 29, 30, 31, 32}, 0, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_STEELJAW, 12,    36,     20,     10,      5,      4,      0,      0,      0,
        {19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8}, 0, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_RAMHORN, 25,    36,     20,     10,      5,      4,      0,      0,      0,
        {8, 9, 10, 11, 12, 13, 14, 15, 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26, 27, 28, 29, 30, 31, 32}, 0, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
    {FBE_SAS_ENCLOSURE_TYPE_ANCHO,	15,    36,	   20,	   10,		5,		4,		2,		4,		1,
        {23, 22, 21, 20, 19, 18, 17, 16, 15, 14, 13, 12, 11, 10,  5},  //drive_slot_to_phy_map[]
         3, 2, 0, 0, 0, 2,
         {	
					{0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
					{6, 7, 8, 9},  // Connector Id 1 individual connector to phy map
					{0},
					{0}, 
					{0},
					{0}
			},
		{0}
    },
    
    {FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP,  0,    36,     88,     44,      6,      5,      4,      
    10,      2,
        {0},              //drive_slot_to_phy_map[]
        3, 2, 1, 10, 0, MAX_POSSIBLE_CONNECTOR_ID_COUNT,
        {
            {3, 2, 1, 0},           // Connector Id 0 individual connector to phy map
            {7, 6, 5, 4},           // Connector Id 1 individual connector to phy map
            {8, 9, 10, 11, 12},     // Connector Id 2 individual connector to phy map
            {13, 14, 15, 16, 17},   // Connector Id 3 individual connector to phy map
            {18, 19, 20, 21, 22},   // Connector Id 4 individual connector to phy map
            {23, 24, 25, 26, 27},   // Connector Id 5 individual connector to phy map
            {31, 30, 29, 28},       // Connector Id 6 individual connector to phy map
            {35, 34, 33, 32}        // Connector Id 7 individual connector to phy map
        },
        {0, 0, 0, 30, 60, 90, 0, 0}    // Connector id to drive start slot
    },

    /* Below mapping is from EE at A side only. B side has different mapping for simulation purpose */ 
    {FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP,  30,    36,     12,     6,      6,      5,      0,      0,      0,
        // Below is drive_slot_to_phy_map[]
        {12, 14, 16, 18, 20, 13, 15, 17, 19, 21, 7, 9, 11, 23, 25, 6, 8, 10, 22, 24, 1, 3, 5, 27, 29, 0, 2, 4, 26, 28},
        0, 2, 0, 0, 0, 1,
        {
            {31, 32, 33, 34, 35},  // Connector Id 0 individual connector to phy map
            {0},
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },   
    
    {FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP,  0,    24,     56,     28,      9,      8,      2,      
    10,      1,
        {0},              //drive_slot_to_phy_map[]
        3, 2, 1, 3, 0, MAX_POSSIBLE_CONNECTOR_ID_COUNT,
        {
            {2, 3, 1, 0},           // Connector Id 0 individual connector to phy map
            {6, 7, 5, 4},           // Connector Id 1 individual connector to phy map
            {10, 11, 9, 8},     // Connector Id 2 individual connector to phy map
            {22, 23, 21, 20},   // Connector Id 3 individual connector to phy map
            {12, 13, 14, 15, 16, 17, 18, 19},  // Connector Id 4 individual connector to phy map
            {0, 0, 0, 0},   // Connector Id 5 individual connector to phy map
        },
        {0, 0, 0, 0, 0, 0, 0, 0}    // Connector id to drive start slot
    },

    /* Below mapping is from EE at A side only. B side has different mapping for simulation purpose */ 
    {FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP,  60,    68,     16,     8,      8,      8,      0,      0,      0,
        // Below is drive_slot_to_phy_map[]
        {46, 47, 42, 43, 38, 39, 32, 33, 26, 27, 22, 23, 66, 67, 62, 63, 58, 59, 14, 15, 10, 11, 6, 7, 54, 55, 50, 40, 41, 34, 35, 28, 29, 51, 18, 19, 64, 60, 56, 52, 48, 44, 24, 20, 16, 12, 8, 4, 65, 61, 57, 53, 49, 45, 25, 21, 17, 13, 9, 5},
        0, 2, 0, 0, 0, 1,
        {
            {3, 2, 1, 0, 30, 31, 36, 37},  // Connector Id 0 individual connector to phy map
            {0},
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },        
    {FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP,  0,    68,     76,     38,      9,      8,      4,      
    10,      2,
        {0},              //drive_slot_to_phy_map[]
        3, 2, 1, 10, 0, 6,
        {
            {3, 2, 1, 0},           // Connector Id 0 individual connector to phy map
            {7, 6, 5, 4},           // Connector Id 1 individual connector to phy map
            {31, 30, 29, 28},       // Connector Id 2 individual connector to phy map
            {35, 34, 33, 32},        // Connector Id 3 individual connector to phy map
            {8, 9, 10, 11, 12,13, 14, 15}, // Connector Id 4 individual connector to phy map
            {16, 17, 18, 19, 20, 21, 22, 23}   // Connector Id 5 individual connector to phy map
        },
        {0, 0, 0, 0, 0, 60}    // Connector id to drive start slot
    },

    /* Below mapping is from EE at A side only. B side has different mapping for simulation purpose */ 
    {FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP,  60,    68,     12,     6,      9,      8,      0,      0,      0,
        // Below is drive_slot_to_phy_map[]
        {62, 56, 50, 44, 38, 32, 26, 20, 14, 8, 63, 57, 51, 45, 39, 33, 27, 21, 15, 9, 64, 58, 52, 46, 40, 34, 28, 22, 16, 10, 
         65, 59, 53, 47, 41, 35, 29, 23, 17, 11, 66, 60, 54, 48, 42, 36, 30, 24, 18, 12, 67, 61, 55, 49, 43, 37, 31, 25, 19, 13},
        0, 2, 0, 0, 0, 1,
        {
            {0, 1, 2, 3, 4, 5, 6, 7},  // Connector Id 0 individual connector to phy map
            {0},
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },    

    {FBE_SAS_ENCLOSURE_TYPE_RHEA, 12,    36,     20,     10,      5,      4,      0,      0,      0,
        {15, 2, 5, 32, 0, 3, 6, 31, 1, 4, 7, 30}, 0, 2, 0, 0, 0, 2,
        {
            {16, 17, 18, 19},  // Connector Id 0 individual connector to phy map
            {20, 21, 22, 23},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_MIRANDA, 25,    36,     20,     10,      5,      4,      0,      0,      0,
        {8, 9, 10, 11, 12, 13, 14, 15, 0, 1, 2, 3, 4, 5, 6, 7, 24, 25, 26, 27, 28, 29, 30, 31, 32}, 0, 2, 0, 0, 0, 2,
        {
            {16, 17, 18, 19},  // Connector Id 0 individual connector to phy map
            {20, 21, 22, 23},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
// ENCL_CLEANUP - Moon DPE need real values        
    {FBE_SAS_ENCLOSURE_TYPE_CALYPSO, 25,    36,     20,     10,      5,      4,      0,      6,      4,
        {8, 9, 10, 11, 12, 13, 14, 15, 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26, 27, 28, 29, 30, 31, 32}, 0, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_TABASCO, 25,    36,     20,     10,      5,      4,      2,      4,      1,
        {11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35},      
         3, 2, 0, 0, 0, 2,
         {
            {0, 1, 2, 3},    // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},    // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    // JJB NAGA_80 cleanup
    {FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP,  0,    68,     76,     38,      9,      8,      4,      
    10,      1,
        {0},              //drive_slot_to_phy_map[]
        3, 2, 1, 10, 0, 6,
        {
            {3, 2, 1, 0},           // Connector Id 0 individual connector to phy map
            {7, 6, 5, 4},           // Connector Id 1 individual connector to phy map
            {31, 30, 29, 28},       // Connector Id 2 individual connector to phy map
            {35, 34, 33, 32},        // Connector Id 3 individual connector to phy map
            {8, 9, 10, 11, 12,13, 14, 15}, // Connector Id 4 individual connector to phy map
            {16, 17, 18, 19, 20, 21, 22, 23}   // Connector Id 5 individual connector to phy map
        },
        {0, 0, 0, 0, 0, 40}    // Connector id to drive start slot
    },

    /* Below mapping is from EE at A side only. B side has different mapping for simulation purpose */ 
    {FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP,  40,    68,     12,     6,      9,      8,      0,      0,      1,
        // Below is drive_slot_to_phy_map[]
        {62, 56, 50, 44, 38, 32, 26, 20, 14, 8, 64, 58, 52, 46, 40, 34, 28, 22, 16, 10, 
         66, 60, 54, 48, 42, 36, 30, 24, 18, 12, 67, 61, 55, 49, 43, 37, 31, 25, 19, 13},
        0, 2, 0, 0, 0, 1,
        {
            {0, 1, 2, 3, 4, 5, 6, 7},  // Connector Id 0 individual connector to phy map
            {0},
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },    

    {FBE_SAS_ENCLOSURE_TYPE_LAST,   0,     0,      0,      0,      0,      0,      0,      0 }
};
// Duplicated for B side for viper enclosure other enclosure might not be correct
terminator_sas_encl_eses_info_t b_encl_eses_info_table[] = {
    {FBE_SAS_ENCLOSURE_TYPE_BULLET, 15,    36,     20,     10,      5,      4,      2,      4,      1, 0, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0},

    {FBE_SAS_ENCLOSURE_TYPE_PINECONE,  12,    24,     20,     10,      5,      4,      2,      4,      0,
        {12, 13, 14, 17, 16, 23, 18, 15, 22, 21, 20, 19},  //drive_slot_to_phy_map[]
        3, 2, 0, 0, 0, 2,
        {   
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },


    {FBE_SAS_ENCLOSURE_TYPE_VIPER,  15,    36,     20,     10,      5,      4,      2,      4,      1,
        {9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 21, 23, 22, 20}, 3, 2, 0, 0, 0, 2,
        {
            {4, 5, 6, 7}, // Connector Id 0 individual connector to phy map
            {0, 1, 2, 3}, // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
 
    {FBE_SAS_ENCLOSURE_TYPE_MAGNUM, 15,    24,     10,     5,      5,      4,      0,      0,      0,
        {4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18}, 0, 2, 0, 0, 0, 2,
        {   
            {0, 1, 2, 3},   // Connector Id 0 individual connector to phy map
            {255, 255, 255, 255},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
        
    {FBE_SAS_ENCLOSURE_TYPE_BUNKER, 15,    36,     20,     10,      5,      4,      0,      0,      0,
        {12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26}, 0, 2, 0, 0, 0, 2,
        {
            {1, 0, 2, 3}, // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7}, // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    }, 
        
    {FBE_SAS_ENCLOSURE_TYPE_CITADEL, 25,    36,     20,     10,      5,      4,      0,      0,      0,
        {8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32}, 0, 2, 0, 0, 0, 2,
        {
            {1, 0, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
        
    {FBE_SAS_ENCLOSURE_TYPE_DERRINGER, 25,    36,     20,     10,      5,      4,      2,      4,      1,
        {11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35},      
         3, 2, 0, 0, 0, 2,
         {
            {4,5,6,7},     // Connector Id 0 individual connector to phy map
            {0, 1, 2, 3},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    }, 

    {FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM,  0,    24,     60,     30,      5,      4,      2,      
    7,      1,
        {0},              //drive_slot_to_phy_map[]
        3, 4 , 2, 3, 0, 6,
        {
            {20, 21, 22, 23},  // Connector Id 0 individual connector to phy map
            {12, 13, 14, 15},  // Connector Id 1 individual connector to phy map
            {16, 17, 18, 19},  // Connector Id 2 individual connector to phy map
            {8, 9, 10, 11},    // Connector Id 3 individual connector to phy map
            {0, 1, 2, 7},      // Connector Id 4 individual connector to phy map
            {3, 4, 5, 6}       // Connector Id 5 individual connector to phy map
        },
        {0, 0, 0, 0, 0, 30}    // Connector id to drive start slot
    },
    
    /* Below mapping is from EE at B side only as 2 EE have different mapping for simulation purpose */ 
    {FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE,  30,    36,     10,     5,      5,      4,      0,      0,      0,
        // Below is drive_slot_to_phy_map[]
        {12, 11, 10, 5, 0, 6, 20, 17, 14, 7,  2,  3,  19, 18, 9, 8,  4,  1,  23, 22, 21, 13, 15, 16, 27, 28, 24, 26, 25, 30},
        0, 2, 0, 0, 0, 1,
        {
            {31, 32, 33, 34},  // Connector Id 0 individual connector to phy map
            {0},
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
         
    {FBE_SAS_ENCLOSURE_TYPE_FALLBACK, 25,    36,     20,     10,      5,      4,      0,      3,      1,
        {32, 31, 30, 29, 28, 27, 26, 25, 16, 17, 19, 20, 22, 23, 24, 21, 18, 15, 14, 13, 12, 11, 10, 9, 8}, 0, 2, 0, 0, 3, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
        
    {FBE_SAS_ENCLOSURE_TYPE_BOXWOOD, 12,    36,     20,     10,      5,      4,      0,      0,      0,
        {15, 16, 17, 18, 11, 14, 19, 12, 13, 10, 9, 8}, 0, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_KNOT, 25,    36,     20,     10,      5,      4,      0,      0,      0,
        {8, 9, 10, 11, 12, 13, 14, 15, 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26, 27, 28, 29, 30, 31, 32}, 0, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_STEELJAW, 12,    36,     20,     10,      5,      4,      0,      0,      0,
        {19, 18, 17, 16, 15, 14, 13, 12, 11, 10, 9, 8}, 0, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_RAMHORN, 25,    36,     20,     10,      5,      4,      0,      0,      0,
        {8, 9, 10, 11, 12, 13, 14, 15, 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26, 27, 28, 29, 30, 31, 32}, 0, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_ANCHO,  15,    36,     20,     10,      5,      4,      2,      4,      1,
        {5,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23}, 3, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},    // Connector Id 0 individual connector to phy map
            {6, 7, 8, 9},    // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP,  0,    36,     88,     44,      6,      5,      4,      
    10,      2,
        {0},              //drive_slot_to_phy_map[]
        3, 2, 1, 10, 0, MAX_POSSIBLE_CONNECTOR_ID_COUNT,
        {
            {3, 2, 1, 0},           // Connector Id 0 individual connector to phy map
            {7, 6, 5, 4},           // Connector Id 1 individual connector to phy map
            {8, 9, 10, 11, 12},     // Connector Id 2 individual connector to phy map
            {13, 14, 15, 16, 17},   // Connector Id 3 individual connector to phy map
            {18, 19, 20, 21, 22},   // Connector Id 4 individual connector to phy map
            {23, 24, 25, 26, 27},   // Connector Id 5 individual connector to phy map
            {31, 30, 29, 28},       // Connector Id 6 individual connector to phy map
            {35, 34, 33, 32}        // Connector Id 7 individual connector to phy map
        },
        {0, 0, 0, 30, 60, 90, 0, 0}    // Connector id to drive start slot
    },

    {FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP,  30,    36,     12,     6,      6,      5,      0,      0,      0,
        // Below is drive_slot_to_phy_map[]
        {12, 14, 16, 18, 20, 13, 15, 17, 19, 21, 11, 22, 23, 25, 27, 9, 10, 24, 26, 28, 7, 8, 29, 31, 33, 5, 6, 30, 32, 34},
        0, 2, 0, 0, 0, 1,
        {
            {0, 1, 2, 3, 4},  // Connector Id 0 individual connector to phy map
            {0},
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },       

    {FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP,  0,    24,     56,     28,      9,      8,      2,      
    10,      1,
        {0},              //drive_slot_to_phy_map[]
        3, 2, 1, 3, 0, MAX_POSSIBLE_CONNECTOR_ID_COUNT,
        {
            {2, 3, 1, 0},           // Connector Id 0 individual connector to phy map
            {6, 7, 5, 4},           // Connector Id 1 individual connector to phy map
            {10, 11, 9, 8},     // Connector Id 2 individual connector to phy map
            {22, 23, 21, 20},   // Connector Id 3 individual connector to phy map
            {12, 13, 14, 15, 16, 17, 18, 19},  // Connector Id 4 individual connector to phy map
            {0, 0, 0, 0},   // Connector Id 5 individual connector to phy map
        },
        {0, 0, 0, 0, 0, 0, 0, 0}    // Connector id to drive start slot
    },

    {FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP,  60,    68,     16,     8,      8,      8,      0,      0,      0,
        // Below is drive_slot_to_phy_map[]
        {46, 47, 42, 43, 38, 39, 32, 33, 26, 27, 22, 23, 66, 67, 62, 63, 58, 59, 14, 15, 10, 11, 6, 7, 54, 55, 50, 40, 41, 34, 35, 28, 29, 51, 18, 19, 64, 60, 56, 52, 48, 44, 24, 20, 16, 12, 8, 4, 65, 61, 57, 53, 49, 45, 25, 21, 17, 13, 9, 5},
        0, 2, 0, 0, 0, 1,
        {
            {3, 2, 1, 0, 30, 31, 36, 37},  // Connector Id 0 individual connector to phy map
            {0},
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },        

    {FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP,  0,    68,     76,     38,      9,      8,      4,      
    10,      2,
        {0},              //drive_slot_to_phy_map[]
        3, 2, 1, 10, 0, 6,
        {
            {3, 2, 1, 0},           // Connector Id 0 individual connector to phy map
            {7, 6, 5, 4},           // Connector Id 1 individual connector to phy map
            {31, 30, 29, 28},       // Connector Id 2 individual connector to phy map
            {35, 34, 33, 32},        // Connector Id 3 individual connector to phy map
            {8, 9, 10, 11, 12,13, 14, 15}, // Connector Id 4 individual connector to phy map
            {16, 17, 18, 19, 20, 21, 22, 23}   // Connector Id 5 individual connector to phy map
        },
        {0, 0, 0, 0, 0, 60}    // Connector id to drive start slot
    },

    /* Below mapping is from EE at A side only. B side has different mapping for simulation purpose */ 
    {FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP,  60,    68,     12,     6,      9,      8,      0,      0,      0,
        // Below is drive_slot_to_phy_map[]
        {62, 56, 50, 44, 38, 32, 26, 20, 14, 8, 63, 57, 51, 45, 39, 33, 27, 21, 15, 9, 64, 58, 52, 46, 40, 34, 28, 22, 16, 10, 
         65, 59, 53, 47, 41, 35, 29, 23, 17, 11, 66, 60, 54, 48, 42, 36, 30, 24, 18, 12, 67, 61, 55, 49, 43, 37, 31, 25, 19, 13},
        0, 2, 0, 0, 0, 1,
        {
            {0, 1, 2, 3, 4, 5, 6, 7},  // Connector Id 0 individual connector to phy map
            {0},
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },    

    {FBE_SAS_ENCLOSURE_TYPE_RHEA, 12,    36,     20,     10,      5,      4,      0,      0,      0,
        {32, 7, 4, 15, 31, 6, 3, 0, 30, 5, 2, 1}, 0, 2, 0, 0, 0, 2,
        {
            {16, 17, 18, 19},  // Connector Id 0 individual connector to phy map
            {20, 21, 22, 23},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

    {FBE_SAS_ENCLOSURE_TYPE_MIRANDA, 25,    36,     20,     10,      5,      4,      0,      0,      0,
        {32, 31, 30, 29, 28, 27, 26, 25, 24, 7, 6, 5, 4, 3, 2, 1, 0, 15, 14, 13, 12, 11, 10, 9, 8}, 0, 2, 0, 0, 0, 2,
        {
            {16, 17, 18, 19},  // Connector Id 0 individual connector to phy map
            {20, 21, 22, 23},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },

// ENCL_CLEANUP - Moon DPE need real values        
    {FBE_SAS_ENCLOSURE_TYPE_CALYPSO, 25,    36,     20,     10,      5,      4,      0,      0,      4,
        {8, 9, 10, 11, 12, 13, 14, 15, 16,  17,  18,  19,  20,  21,  22,  23,  24,  25,  26, 27, 28, 29, 30, 31, 32}, 0, 2, 0, 0, 0, 2,
        {
            {0, 1, 2, 3},  // Connector Id 0 individual connector to phy map
            {4, 5, 6, 7},  // Connector Id 1 individual connector to phy map
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },
    {FBE_SAS_ENCLOSURE_TYPE_TABASCO, 25,	  36,	  20,	  10,	   5,	   4,	   2,	   4,	   1,
			{11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35},	   
			 3, 2, 0, 0, 0, 2,
			 {
				{0, 1, 2, 3},	   // Connector Id 0 individual connector to phy map
				{4, 5, 6, 7},      // Connector Id 1 individual connector to phy map
				{0},
				{0}, 
				{0},
				{0}
			},
			{0}
    }, 

    // JJB NAGA_80 cleanup
    {FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP,  0,    68,     76,     38,      9,      8,      4,      
    10,      1,
        {0},              //drive_slot_to_phy_map[]
        3, 2, 1, 10, 0, 6,
        {
            {3, 2, 1, 0},           // Connector Id 0 individual connector to phy map
            {7, 6, 5, 4},           // Connector Id 1 individual connector to phy map
            {31, 30, 29, 28},       // Connector Id 2 individual connector to phy map
            {35, 34, 33, 32},        // Connector Id 3 individual connector to phy map
            {8, 9, 10, 11, 12,13, 14, 15}, // Connector Id 4 individual connector to phy map
            {16, 17, 18, 19, 20, 21, 22, 23}   // Connector Id 5 individual connector to phy map
        },
        {0, 0, 0, 0, 0, 40}    // Connector id to drive start slot
    },

    /* Below mapping is from EE at A side only. B side has different mapping for simulation purpose */ 
    {FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP,  40,    68,     12,     6,      9,      8,      0,      0,      1,
        // Below is drive_slot_to_phy_map[]
        {62, 56, 50, 44, 38, 32, 26, 20, 14, 8, 64, 58, 52, 46, 40, 34, 28, 22, 16, 10, 
         66, 60, 54, 48, 42, 36, 30, 24, 18, 12, 67, 61, 55, 49, 43, 37, 31, 25, 19, 13},
        0, 2, 0, 0, 0, 1,
        {
            {0, 1, 2, 3, 4, 5, 6, 7},  // Connector Id 0 individual connector to phy map
            {0},
            {0},
            {0}, 
            {0},
            {0}
        },
        {0}
    },    

    {FBE_SAS_ENCLOSURE_TYPE_LAST,   0,     0,      0,      0,      0,      0,      0,      0}
};

#pragma GCC diagnostic pop

//static terminator_sas_unsupported_eses_page_info_t unsupported_eses_page_info;

fbe_status_t sas_virtual_phy_check_enclosure_type(fbe_sas_enclosure_type_t encl_type, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            return(FBE_STATUS_OK);
        }
        encl_eses_info ++;
    }

    return(status);
}


/* the sas address of the virtual phy depends on the enclosure type
 * Bullet encl, decrement 1 from the enclosure SAS address
 * Viper encl, the last 6 bits is 0x3E.
 */
fbe_sas_address_t sas_enclosure_calculate_virtual_phy_sas_address(SCSISESState *s)
{
    fbe_sas_address_t enclosure_sas_address;
    fbe_sas_address_t v_phy_sas_address;
    fbe_sas_enclosure_type_t encl_type = (fbe_sas_enclosure_type_t)s->dae_type;

    enclosure_sas_address = s->qdev.wwn;
    if (enclosure_sas_address == FBE_SAS_ADDRESS_INVALID)
        return FBE_SAS_ADDRESS_INVALID;

    switch(encl_type) {
        case FBE_SAS_ENCLOSURE_TYPE_BULLET:
            v_phy_sas_address = enclosure_sas_address - 1;
            break;
        case FBE_SAS_ENCLOSURE_TYPE_MAGNUM:
        case FBE_SAS_ENCLOSURE_TYPE_VIPER:
        case FBE_SAS_ENCLOSURE_TYPE_BUNKER:
        case FBE_SAS_ENCLOSURE_TYPE_CITADEL:
        case FBE_SAS_ENCLOSURE_TYPE_DERRINGER:
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM:
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE:
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP:
        case FBE_SAS_ENCLOSURE_TYPE_FALLBACK:
        case FBE_SAS_ENCLOSURE_TYPE_BOXWOOD:
        case FBE_SAS_ENCLOSURE_TYPE_KNOT:
        case FBE_SAS_ENCLOSURE_TYPE_PINECONE:
        case FBE_SAS_ENCLOSURE_TYPE_STEELJAW:
        case FBE_SAS_ENCLOSURE_TYPE_RAMHORN:
        case FBE_SAS_ENCLOSURE_TYPE_ANCHO:
        case FBE_SAS_ENCLOSURE_TYPE_CALYPSO:
        case FBE_SAS_ENCLOSURE_TYPE_RHEA:
        case FBE_SAS_ENCLOSURE_TYPE_MIRANDA:
        case FBE_SAS_ENCLOSURE_TYPE_TABASCO:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP:
            v_phy_sas_address = (enclosure_sas_address & VIPER_VIRTUAL_PHY_SAS_ADDR_MASK)
                                 + VIPER_VIRTUAL_PHY_SAS_ADDR_LAST_6_BITS;
            break;

        default:
            // not sure if this is the best return value here.
            return FBE_SAS_ADDRESS_INVALID;
            break;
    }
    return v_phy_sas_address;
}

/* This function frees the dynamically allocated
 * memory pointed to by the given pointer if the
 * pointer is not NULL
 */
static void terminator_free_mem_on_not_null(void *ptr)
{
    if(ptr != NULL)
    {
        free(ptr);
    }
    return;
}

/* this initialises a node, allocates memory for the node, and returns   */
/* a pointer to the new node. Must pass it the node details */
static terminator_eses_free_mem_node_t * initnode(void)
{
   terminator_eses_free_mem_node_t *ptr;
   ptr = (terminator_eses_free_mem_node_t *) fbe_terminator_allocate_memory(sizeof(terminator_eses_free_mem_node_t));
   if(ptr == NULL )         /* error allocating node?         */
       return NULL;         /* then return NULL, else        */
   else 
   {                               /* allocated node successfully */
       ptr->buf = 0;
       ptr->next = NULL;       
       return ptr;            /* return pointer to new node  */
   }
}

/* this adds a node to the end of the list. You must allocate a node and  */
/* then pass its address to this function                                 */
static void add_to_free_mem_list( terminator_eses_free_mem_node_t ** head_ptr, fbe_u8_t *buf )  /* adding to end of list */
{
   terminator_eses_free_mem_node_t *new_node;

   new_node = initnode();
   if(new_node != NULL)
   {
       new_node->buf = buf;
   }
   else
   {
    return;
   }
   if( *head_ptr == NULL )
   {
       *head_ptr = new_node;
       (*head_ptr)->next = NULL;
       return;
   }
   else
   {
        new_node->next = (*head_ptr)->next;
        (*head_ptr)->next= new_node;
   }
}

/* search the list, and return a pointer to the found node     */
/* accepts a data(buf memory) to search for, and a pointer from which to start. If    */
/* you pass the pointer as 'head', it searches from the start of the list */
static terminator_eses_free_mem_node_t * find_free_mem(terminator_eses_free_mem_node_t *head_ptr, fbe_u8_t *buf)
{
    while((head_ptr) && (head_ptr ->buf != buf ))
    {   
        head_ptr = head_ptr->next; 
    }
    return head_ptr;
}

/*Destroy the complet free memory link list for*/
static void destroy_free_mem_list(terminator_eses_free_mem_node_t *head)  
{ 
    terminator_eses_free_mem_node_t *tmp; 
    if(head == NULL)
    {
        return;
    }
    while(head != NULL)
    { 
        tmp = head->next; 
        fbe_terminator_free_memory(head); 
        head = tmp; 
    } 
}

static void sas_virtual_phy_free_config_page_info_memory(vp_config_diag_page_info_t *vp_config_page_info)
{
    fbe_u8_t i;
/* head points to first terminator_eses_buf_node in list, end points to last terminator_eses_buf_node in list */
/* initialise both to NULL, meaning no nodes in list yet */
    terminator_eses_free_mem_node_t *head =NULL;

    // Free memory related to version descriptor data.
    terminator_free_mem_on_not_null(vp_config_page_info->ver_desc);

    // Free memory related to buffer information.
    for(i=0; i < vp_config_page_info->num_bufs; i++)
    {
             if(vp_config_page_info->buf_info[i].buf != NULL && //only find the memoru is free or not if buf is not NULL
            !(find_free_mem(head, vp_config_page_info->buf_info[i].buf ) ))
            {
               terminator_free_mem_on_not_null(vp_config_page_info->buf_info[i].buf);
               add_to_free_mem_list(&head,vp_config_page_info->buf_info[i].buf);
            }
    }
    destroy_free_mem_list(head);
    terminator_free_mem_on_not_null(vp_config_page_info->buf_info);
}

/* This functions frees ANY dynamically allocated memory
 * in the Virtual Phy Object data, if any exists.
 */
static void sas_virtual_phy_free_allocated_memory(terminator_sas_virtual_phy_info_t *attributes)
{
    fbe_queue_element_t * queue_element;
    terminator_enclosure_firmware_new_rev_record_t *new_rev = NULL;
    if(attributes != NULL)
    {
        terminator_free_mem_on_not_null(attributes->cooling_status);
        terminator_free_mem_on_not_null(attributes->drive_slot_status);
        terminator_free_mem_on_not_null(attributes->drive_slot_insert_count);
        terminator_free_mem_on_not_null(attributes->drive_power_down_count);
        terminator_free_mem_on_not_null(attributes->phy_status);
        terminator_free_mem_on_not_null(attributes->ps_status);
        terminator_free_mem_on_not_null(attributes->temp_sensor_status);
        terminator_free_mem_on_not_null(attributes->overall_temp_sensor_status);
        terminator_free_mem_on_not_null(attributes->display_status);
        sas_virtual_phy_free_config_page_info_memory(&attributes->eses_page_info.vp_config_diag_page_info);
        // destroy new revision record queue
        queue_element = fbe_queue_pop(&attributes->new_firmware_rev_queue_head);
        while(queue_element != NULL){
            new_rev = (terminator_enclosure_firmware_new_rev_record_t *)queue_element;
            fbe_terminator_free_memory(new_rev);
            queue_element = fbe_queue_pop(&attributes->new_firmware_rev_queue_head);
        }
        fbe_queue_destroy(&attributes->new_firmware_rev_queue_head);
    }
    terminator_free_mem_on_not_null(attributes);

    return;
}

fbe_status_t sas_virtual_phy_max_drive_slots(fbe_sas_enclosure_type_t encl_type, uint8_t *max_drive_slots, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_drive_slots = encl_eses_info->max_drive_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}


fbe_status_t sas_virtual_phy_max_phys(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_phys, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_phys = encl_eses_info->max_phy_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}

fbe_status_t sas_virtual_phy_max_ps_elems(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_ps_elems, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_ps_elems = encl_eses_info->max_ps_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}

fbe_status_t sas_virtual_phy_max_conns_per_lcc(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_conns, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_conns = encl_eses_info->max_lcc_conn_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}


fbe_status_t sas_virtual_phy_max_cooling_elems(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_cooling_elems, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_cooling_elems = encl_eses_info->max_cooling_elem_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}


fbe_status_t sas_virtual_phy_max_display_characters(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_diplay_characters, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_diplay_characters = encl_eses_info->max_diplay_character_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }
    return(status);
}

fbe_status_t sas_virtual_phy_max_temp_sensor_elems(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_temp_sensor_elems, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_temp_sensor_elems = encl_eses_info->max_temp_sensor_elems_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }
    return(status);
}

fbe_status_t sas_virtual_phy_max_conns_per_port(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_conns, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_conns = encl_eses_info->max_port_conn_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}

/*********************************************************************
*   sas_virtual_phy_max_ext_cooling_elems ()
*********************************************************************
*
*  Description:
*   This gets count of ext cooling elements.
*
*  Inputs:
*   enclosure type
*   pointer containing count of ext cooling elements
*
*  Return Value:
*   success or failure.
*
*  History:
*    06-06-2011  created
*
*********************************************************************/
fbe_status_t sas_virtual_phy_max_ext_cooling_elems(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_ext_cooling_elems, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_ext_cooling_elems = encl_eses_info->max_ext_cooling_elem_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}


/*********************************************************************
*   sas_virtual_phy_max_bem_cooling_elems ()
*********************************************************************
*
*  Description:
*   This gets count of BEM cooling elements.
*
*  Inputs:
*   enclosure type
*   pointer containing count of ext cooling elements
*
*  Return Value:
*   success or failure.
*
*  History:
*    18-07-2012  created. Rui Chang
*
*********************************************************************/
fbe_status_t sas_virtual_phy_max_bem_cooling_elems(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_bem_cooling_elems, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_bem_cooling_elems = encl_eses_info->max_bem_cooling_elem_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}

/*********************************************************************
*   sas_virtual_phy_max_lccs ()
*********************************************************************
*
*  Description:
*   This gets count of lccs.
*
*  Inputs:
*   enclosure type
*   pointer containing count of max lccs
*
*  Return Value:
*   success or failure.
*
*  History:
*    06-06-2011  created
*
*********************************************************************/
fbe_status_t sas_virtual_phy_max_lccs(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_lccs, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_lccs = encl_eses_info->max_lcc_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}

/*********************************************************************
*   sas_virtual_phy_max_ee_lccs ()
*********************************************************************
*
*  Description:
*   This gets count of ee lccs.
*
*  Inputs:
*   enclosure type
*   pointer containing count of ee lccs
*
*  Return Value:
*   success or failure.
*
*  History:
*    06-06-2011  created
*
*********************************************************************/
fbe_status_t sas_virtual_phy_max_ee_lccs(fbe_sas_enclosure_type_t encl_type, fbe_u8_t *max_ee_lccs, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_ee_lccs = encl_eses_info->max_ee_lcc_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}

/*
 * End of functions for getting and setting drive slot power cycle count
 */

/*********************************************************************
*   sas_virtual_phy_get_drive_slot_to_phy_mapping ()
*********************************************************************
*
*  Description:
*   This returns the phy identifier corresponding to given
*   physical drive slot.
*
*  Inputs:
*   drive_slot: physical drive slot on encl.
*   phy_id: Phy identifier
*
*  Return Value:
*   success or failure.
*
*  History:
*    Sep08  created
*
*********************************************************************/
fbe_status_t sas_virtual_phy_get_drive_slot_to_phy_mapping(
    fbe_u8_t drive_slot,
    fbe_u8_t *phy_id,
    fbe_sas_enclosure_type_t encl_type,
    terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;
    fbe_u8_t max_drive_slots;

    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots, spid);
    RETURN_ON_ERROR_STATUS;

    // to be completed based on the encl type.

    if(drive_slot < max_drive_slots)
    {
        encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
        while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
        {
            if(encl_eses_info->encl_type == encl_type)
            {
                *phy_id = encl_eses_info->drive_slot_to_phy_map[drive_slot];
                status = FBE_STATUS_OK;
                return(status);
            }
            encl_eses_info ++;
        }
    }

    status = FBE_STATUS_GENERIC_FAILURE;
    return(status);
}


/*********************************************************************
*            terminator_update_drive_parent_device ()
*********************************************************************
*
*  Description:
*    This function gets the parent device which manages the specified slot. 
*    virtual_phy only manages local slot number, ICM virtual phy does not own any slot.
*
*    NOTICE: since the function also resets the passed-in device pointer and slot_number if necessary,
*    device_ptr and slot_number should be passed in as pointer and their original value may change.
*
*  Inputs:
*   parent_ptr   - pointer to the device pointer where the seeking starts. It may be reset to another value
*                      if it is not the parent of the slot. NOTE that it cannot be the drive itself, should be enclosure
*                      or virtual phy.
*   slot_number - pointer to the slot number. It may be reset to the local slot number in the parent device.
*
*  Return Value:
*       success or failure
*
*  History:
*      March-23-2010: Bo Gao created.
*
*********************************************************************/
static fbe_status_t terminator_update_drive_parent_device(fbe_terminator_device_ptr_t *parent_ptr, fbe_u32_t *slot_number)
{
#if 0
    fbe_status_t status = FBE_STATUS_OK;
    fbe_u8_t max_drive_slots;
    fbe_sas_enclosure_type_t encl_type;
    terminator_component_type_t device_type = base_component_get_component_type(*parent_ptr);
    base_component_t *parent = NULL, *child = NULL;

    /* get the current enclosure type and max drive slots */
    switch(device_type)
    {
    case TERMINATOR_COMPONENT_TYPE_VIRTUAL_PHY:
        status = sas_virtual_phy_get_enclosure_type(*parent_ptr, &encl_type);
        RETURN_ON_ERROR_STATUS;
        status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots);
        RETURN_ON_ERROR_STATUS;
        break;
    case TERMINATOR_COMPONENT_TYPE_ENCLOSURE:
        encl_type = sas_enclosure_get_enclosure_type(*parent_ptr);
        status = terminator_max_drive_slots(encl_type, &max_drive_slots);
        RETURN_ON_ERROR_STATUS;
        break;
    default:
        return FBE_STATUS_GENERIC_FAILURE;
    }

    if (*slot_number >= max_drive_slots)
    {
        /* for the ICM case, seek for the actual EE which owns the slot */
        if(max_drive_slots == 0)
        {
            if (device_type == TERMINATOR_COMPONENT_TYPE_VIRTUAL_PHY)
            {
                /* ICM virtual phy */
                status = base_component_get_parent(*parent_ptr, &parent);
                RETURN_ON_ERROR_STATUS;
            }
            else
            {
                /* device_ptr is the ICM enclosure */
                parent = (base_component_t *)*parent_ptr;
            }
            child = base_component_get_first_child(parent);
            while (child != NULL)
            {
                if(child->component_type == TERMINATOR_COMPONENT_TYPE_ENCLOSURE)
                {
                    status = terminator_update_enclosure_local_drive_slot_number(child, slot_number);
                    if (status == FBE_STATUS_OK)
                    {
                        if (device_type == TERMINATOR_COMPONENT_TYPE_VIRTUAL_PHY)
                        {
                            // make sure we return the same type
                            (*parent_ptr) = (terminator_virtual_phy_t *)base_component_get_child_by_type(child, TERMINATOR_COMPONENT_TYPE_VIRTUAL_PHY);
                        }
                        else
                        {
                            (*parent_ptr) = child;
                        }
                        return status;
                    }
                }
                child = base_component_get_next_child(parent, child);
            }
        }
        // we need to adjust the slot number for EEs
        else if ((device_type == TERMINATOR_COMPONENT_TYPE_VIRTUAL_PHY)||
                 (sas_enclosure_get_logical_parent_type(*parent_ptr)==TERMINATOR_COMPONENT_TYPE_ENCLOSURE))
        {
            if (device_type == TERMINATOR_COMPONENT_TYPE_VIRTUAL_PHY)
            {
                base_component_get_parent(*parent_ptr, &child);
            }
            else
            {
                child = (base_component_t *)*parent_ptr;
            }
            status = terminator_update_enclosure_local_drive_slot_number(child, slot_number);
            return status;
        }

        return FBE_STATUS_GENERIC_FAILURE;
    }
#endif
    return FBE_STATUS_OK;
}


fbe_status_t sas_virtual_phy_get_drive_slot_insert_count(terminator_sas_virtual_phy_info_t * info,
                                                         fbe_u32_t slot_number, 
                                                         fbe_u8_t *insert_count)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    //terminator_sas_virtual_phy_info_t * info = NULL;
    void *self = NULL;

    status = terminator_update_drive_parent_device((fbe_terminator_device_ptr_t*)&self, &slot_number);
    RETURN_ON_ERROR_STATUS;
#if 0
    info = base_component_get_attributes(&self->base);
    if (info == NULL)
    {
        return status;
    }
#endif
    fbe_copy_memory(insert_count, &info->drive_slot_insert_count[slot_number], sizeof(fbe_u8_t));
    status = FBE_STATUS_OK;
    return status;
}

fbe_status_t sas_virtual_phy_get_drive_power_down_count(terminator_sas_virtual_phy_info_t * info,
                                                        fbe_u32_t slot_number, 
                                                        fbe_u8_t *power_down_count)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    //terminator_sas_virtual_phy_info_t * info = NULL;
    void *self = NULL;

    status = terminator_update_drive_parent_device((fbe_terminator_device_ptr_t*)&self, &slot_number);
    RETURN_ON_ERROR_STATUS;
#if 0
    info = base_component_get_attributes(&self->base);
    if (info == NULL)
    {
        return status;
    }
#endif
    fbe_copy_memory(power_down_count, &info->drive_power_down_count[slot_number], sizeof(fbe_u8_t));
    status = FBE_STATUS_OK;
    return status;
}

fbe_status_t sas_virtual_phy_max_single_lane_conns_per_port(fbe_sas_enclosure_type_t encl_type, uint8_t *max_conns, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_conns = encl_eses_info->max_single_lane_port_conn_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(status);
}

fbe_status_t sas_virtual_phy_max_conn_id_count(fbe_sas_enclosure_type_t encl_type, uint8_t *max_conn_id_count, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;

    // to be completed based on the encl type.
    encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
    while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
    {
        if(encl_eses_info->encl_type == encl_type)
        {
            *max_conn_id_count = encl_eses_info->max_conn_id_count;
            status = FBE_STATUS_OK;
            return(status);
        }
        encl_eses_info ++;
    }

    return(FBE_STATUS_GENERIC_FAILURE);
}

fbe_status_t sas_virtual_phy_get_individual_conn_to_phy_mapping(uint8_t individual_lane, uint8_t connector_id, uint8_t *phy_id, fbe_sas_enclosure_type_t encl_type, terminator_sp_id_t spid)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_encl_eses_info_t *encl_eses_info;
    uint8_t max_single_lane_port_conns;

    status = sas_virtual_phy_max_single_lane_conns_per_port(encl_type, &max_single_lane_port_conns, spid);
    if (status != FBE_STATUS_OK)
        return status;

    if(individual_lane < max_single_lane_port_conns)
    {
        encl_eses_info = SELECT_ENCL_ESES_INFO_TABLE(spid);
        while(encl_eses_info->encl_type != FBE_SAS_ENCLOSURE_TYPE_LAST)
        {
            if(encl_eses_info->encl_type == encl_type) 
            {
                *phy_id = encl_eses_info->individual_conn_to_phy_map[connector_id][individual_lane];
                trace_eses_vphy_get_individual_conn_to_phy_mapping(encl_type, connector_id, individual_lane, *phy_id);
                status = FBE_STATUS_OK;
                return(status);
            }
            encl_eses_info ++;
        }
    }

    status = FBE_STATUS_GENERIC_FAILURE;
    return(status);
}

fbe_status_t sas_virtual_phy_get_emcEnclStatus(terminator_sas_virtual_phy_info_t *info,
                                               ses_pg_emc_encl_stat_struct *emcEnclStatusPtr)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;

    fbe_copy_memory(emcEnclStatusPtr, &info->emcEnclStatus, sizeof(ses_pg_emc_encl_stat_struct));

    status = FBE_STATUS_OK;
    return(status);
}

fbe_status_t sas_virtual_phy_get_emcPsInfoStatus(terminator_sas_virtual_phy_info_t *info,
                                                 ses_ps_info_elem_struct *emcPsInfoStatusPtr)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;

    fbe_copy_memory(emcPsInfoStatusPtr, &info->emcPsInfoStatus, sizeof(ses_ps_info_elem_struct));

    status = FBE_STATUS_OK;
    return(status);
}

fbe_status_t sas_virtual_phy_get_emcGeneralInfoDirveSlotStatus(terminator_sas_virtual_phy_info_t *info,
                                                 ses_general_info_elem_array_dev_slot_struct *emcGeneralInfoDirveSlotStatusPtr,
                                                 fbe_u8_t drive_slot)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;

    fbe_copy_memory(emcGeneralInfoDirveSlotStatusPtr, &info->general_info_elem_drive_slot[drive_slot], sizeof(ses_general_info_elem_array_dev_slot_struct));

    status = FBE_STATUS_OK;
    return(status);
}

/**************************************************************************
 *  sas_virtual_phy_info_new()
 **************************************************************************
 *
 *  DESCRIPTION:
 *    This interface(constructor) creates and initializes a virtual PHY object.
 *
 *  PARAMETERS:
 *      enclosure type and SAS address of the enclosure, with which the new
 *      virtual phy is to be associated.
 *
 *  RETURN VALUES/ERRORS:
 *      success or failure.
 *
 *  NOTES:
 *      When Terminator Initializes--->
 *       All drive slots will be "POWERED ON"
 *           // i.e. "DEVICE OFF" bit in Array device slot status element is NOT SET.
 *       ALL Drive Slot Status will be "NOT INSTALLED".
 *       ALL PHY STATUS WILL BE "OK".
 *       Phy ready bit in PHY status is set to false
 *
 *  HISTORY:
 *      Created
 **************************************************************************/
terminator_sas_virtual_phy_info_t * sas_virtual_phy_info_new(
    fbe_sas_enclosure_type_t encl_type,
    fbe_sas_address_t sas_address, fbe_u8_t side)
{
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    terminator_sas_virtual_phy_info_t *new_info = NULL;
    fbe_u32_t drive_index, phy_index, ps_index, cooling_index,
              temp_sensor_index, display_index, sas_conn_index;
    fbe_u8_t  max_drive_slots = 0, max_phys = 0, max_ps_elems = 0, max_cooling_elems = 0,
              max_temp_sensor_elems = 0, max_diplay_characters = 0, max_sas_conn_elems_per_side = 0;

    /* SAFEBUG - panic on uninitialized data in sas_virtual_phy_free_allocated_memory */
    new_info = (terminator_sas_virtual_phy_info_t *)calloc(1, sizeof(terminator_sas_virtual_phy_info_t));
    if (new_info == NULL)
        return new_info;

    /* we don't need login data in QEMU */
    new_info->device_address = sas_address;

    // Give virtual phy the enclosure type
    new_info->enclosure_type = encl_type;
    new_info->side = side;

    //First initialize the member pointers to NULL
    new_info->cooling_status = NULL;
    new_info->drive_slot_status = NULL;
    new_info->phy_status = NULL;
    new_info->ps_status = NULL;
    new_info->temp_sensor_status = NULL;

    // Allocate memory and initialize the drive, phy, cooling--etc eses
    // statuses as we now have the virtual phy specialized to the encl
    // type.
    // DRIVE SLOTS
    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_slots, side);
    if(status != FBE_STATUS_OK)
    {
       printf("%s:sas_virtual_phy_max_drive_slots failed, encl_type=%d\n", __FUNCTION__, encl_type);
       sas_virtual_phy_free_allocated_memory(new_info);
       return(NULL);
    }

    if (max_drive_slots == 0) { goto skip_stuff; } /* SAFEBUG - prevent allocation of 0 bytes */
    // Drive Slot ESES Status
    new_info->drive_slot_status = (ses_stat_elem_array_dev_slot_struct *)fbe_terminator_allocate_memory(max_drive_slots * sizeof(ses_stat_elem_array_dev_slot_struct));
    if (new_info->drive_slot_status == NULL)
    {
        printf( "%s failed to allocate memory for drive slot status at line %d.\n", __FUNCTION__, __LINE__); 
        sas_virtual_phy_free_allocated_memory(new_info);
        return NULL;
    }
    fbe_zero_memory(new_info->drive_slot_status, max_drive_slots * sizeof(ses_stat_elem_array_dev_slot_struct));

    for (drive_index = 0; drive_index < max_drive_slots; drive_index ++)
    {
        new_info->drive_slot_status[drive_index].cmn_stat.elem_stat_code = SES_STAT_CODE_NOT_INSTALLED;
        // All drive slots are powered ON intially.
        new_info->drive_slot_status[drive_index].dev_off = FBE_FALSE;
    }

    //Drive Slot insert Count
    new_info->drive_slot_insert_count = (fbe_u8_t *)fbe_terminator_allocate_memory(max_drive_slots * sizeof(fbe_u8_t));
    if (new_info->drive_slot_insert_count == NULL)
    {
        printf("%s failed to allocate memory for drive slot insert count at line %d.\n", __FUNCTION__, __LINE__);
        sas_virtual_phy_free_allocated_memory(new_info);

        return NULL;
    }
    fbe_zero_memory(new_info->drive_slot_insert_count, max_drive_slots * sizeof(fbe_u8_t));

    //Drive Slot power down count
    new_info->drive_power_down_count = (fbe_u8_t *)fbe_terminator_allocate_memory(max_drive_slots * sizeof(fbe_u8_t));
    if (new_info->drive_power_down_count == NULL)
    {
        printf("%s failed to allocate memory for drive slot power down count at line %d.\n", __FUNCTION__, __LINE__);
        sas_virtual_phy_free_allocated_memory(new_info);

        return NULL;
    }
    fbe_zero_memory(new_info->drive_power_down_count, max_drive_slots * sizeof(fbe_u8_t));

    new_info->general_info_elem_drive_slot = 
        (ses_general_info_elem_array_dev_slot_struct *)fbe_terminator_allocate_memory(max_drive_slots * sizeof(ses_general_info_elem_array_dev_slot_struct));
    if (new_info->general_info_elem_drive_slot == NULL)
    {
        printf("%s failed to allocate memory for general info element drive slot at line %d.\n", __FUNCTION__, __LINE__); 
        sas_virtual_phy_free_allocated_memory(new_info);

        return NULL;
    }
    fbe_zero_memory(new_info->general_info_elem_drive_slot, max_drive_slots * sizeof(ses_general_info_elem_array_dev_slot_struct));

    for (drive_index = 0; drive_index < max_drive_slots; drive_index ++)
    {
      // currently we only have jetfire using this battery backed info. 
        if (new_info->enclosure_type == FBE_SAS_ENCLOSURE_TYPE_FALLBACK) 
        {
            new_info->general_info_elem_drive_slot[drive_index].battery_backed = 1;
        }
        else
        {
            new_info->general_info_elem_drive_slot[drive_index].battery_backed = 0;
        }
    }


    skip_stuff:;


    // PHY'S
    status = sas_virtual_phy_max_phys(encl_type, &max_phys, side);
    if(status != FBE_STATUS_OK)
    {
        sas_virtual_phy_free_allocated_memory(new_info);
        return(NULL);
    }
    new_info->phy_status = (ses_stat_elem_exp_phy_struct *)fbe_terminator_allocate_memory(max_phys * sizeof(ses_stat_elem_exp_phy_struct));
    if (new_info->phy_status == NULL)
    {
        printf( "%s failed to allocate memory for physical status at line %d.\n", __FUNCTION__, __LINE__); 
        sas_virtual_phy_free_allocated_memory(new_info);

        return NULL;
    }
    fbe_zero_memory(new_info->phy_status, max_phys * sizeof(ses_stat_elem_exp_phy_struct));

    for(phy_index = 0; phy_index < max_phys; phy_index ++)
    {
        // As per ESES 0.16 all PHY's initially have their status code set to OK
        new_info->phy_status[phy_index].cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
        new_info->phy_status[phy_index].phy_id = phy_index;
        new_info->phy_status[phy_index].phy_rdy = FBE_TRUE;
        new_info->phy_status[phy_index].link_rdy = FBE_TRUE;
    }

    // POWER SUPPLIES
    status = sas_virtual_phy_max_ps_elems(encl_type, &max_ps_elems, side);
    if(status != FBE_STATUS_OK)
    {
        sas_virtual_phy_free_allocated_memory(new_info);
        return(NULL);
    }
    if ( max_ps_elems > 0 )
    {
        new_info->ps_status = (ses_stat_elem_ps_struct *)fbe_terminator_allocate_memory(max_ps_elems * sizeof(ses_stat_elem_ps_struct));
        if (new_info->ps_status == NULL)
        {
            printf( "%s failed to allocate memory for ps status at line %d.\n", __FUNCTION__, __LINE__); 
            sas_virtual_phy_free_allocated_memory(new_info);

            return NULL;
        }
        fbe_zero_memory(new_info->ps_status, max_ps_elems * sizeof(ses_stat_elem_ps_struct));

        // Initialize the Power supplies. Assume all are installed & working properly.
        for(ps_index = 0; ps_index < max_ps_elems; ps_index ++)
        {
            new_info->ps_status[ps_index].cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
            new_info->ps_status[ps_index].off = 0;
            new_info->ps_status[ps_index].dc_fail = 0;
            new_info->ps_status[ps_index].rqsted_on = 1;
            new_info->ps_status[ps_index].ac_fail = 0;
        }
    }
    else
    {
        new_info->ps_status = NULL;
    }

    // SAS CONNECTOR
    status = sas_virtual_phy_max_conns_per_lcc(encl_type, &max_sas_conn_elems_per_side, side);
    if(status != FBE_STATUS_OK)
    {
        sas_virtual_phy_free_allocated_memory(new_info);
        return(NULL);
    }
    if ( 2 * max_sas_conn_elems_per_side > 0 )
    {
        new_info->sas_conn_status = (ses_stat_elem_sas_conn_struct *)fbe_terminator_allocate_memory(2 * max_sas_conn_elems_per_side * sizeof(ses_stat_elem_sas_conn_struct));
        if (new_info->sas_conn_status == NULL)
        {
            printf( "%s failed to allocate memory for sas connection status at line %d.\n", __FUNCTION__, __LINE__); 
            sas_virtual_phy_free_allocated_memory(new_info);

            return NULL;
        }
        fbe_zero_memory (new_info->sas_conn_status, 2 * max_sas_conn_elems_per_side * sizeof(ses_stat_elem_sas_conn_struct));

        // Initialize the Power supplies. Assume all are installed & working properly.
        for(sas_conn_index = 0; sas_conn_index < 2*max_sas_conn_elems_per_side; sas_conn_index ++)
        {
            new_info->sas_conn_status[sas_conn_index].cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
        }
    }
    else
    {
        new_info->sas_conn_status = NULL;
    }

    // COOLING
    status = sas_virtual_phy_max_cooling_elems(encl_type, &max_cooling_elems, side);
    if(status != FBE_STATUS_OK)
    {
        sas_virtual_phy_free_allocated_memory(new_info);
        return(NULL);
    }
    if ( max_cooling_elems > 0 )
    {
        new_info->cooling_status = (ses_stat_elem_cooling_struct *)fbe_terminator_allocate_memory(max_cooling_elems * sizeof(ses_stat_elem_cooling_struct));
        if (new_info->cooling_status == NULL)
        {
            printf( "%s failed to allocate memory for cooling status at line %d.\n", __FUNCTION__, __LINE__); 
            sas_virtual_phy_free_allocated_memory(new_info);

            return NULL;
        }

        fbe_zero_memory(new_info->cooling_status, max_cooling_elems * sizeof(ses_stat_elem_cooling_struct));

        // Initialize the Cooling elements(fans). Assume all are installed & working properly.
        // Internally in terminator there will be no assumption on which fan belongs to which
        // power supply. We consider them as independent . Upto the user to make the disinction.
        for(cooling_index = 0; cooling_index < max_cooling_elems; cooling_index ++)
        {
            new_info->cooling_status[cooling_index].cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
            // Fan speed not yet decided... Put to some thing for now.
            new_info->cooling_status[cooling_index].actual_fan_speed_high = 0;
            new_info->cooling_status[cooling_index].actual_fan_speed_low = 0x10;
            new_info->cooling_status[cooling_index].rqsted_on = 1;
            new_info->cooling_status[cooling_index].off = 0;
            // Relative speed code of 7 indicates the fan is at its highest speed.
            new_info->cooling_status[cooling_index].actual_speed_code = 7;
        }
    }
    else
    {
        new_info->cooling_status = NULL;
    }


    // TEMPERATURE SENSOR
    status = sas_virtual_phy_max_temp_sensor_elems(encl_type, &max_temp_sensor_elems, side);
    if(status != FBE_STATUS_OK)
    {
        sas_virtual_phy_free_allocated_memory(new_info);
        return(NULL);
    }
    if ( max_temp_sensor_elems > 0 )
    {
        new_info->overall_temp_sensor_status = 
            (ses_stat_elem_temp_sensor_struct *)fbe_terminator_allocate_memory(max_temp_sensor_elems * sizeof(ses_stat_elem_temp_sensor_struct));
        if (new_info->overall_temp_sensor_status == NULL)
        {
            printf( "%s failed to allocate memory for overall temp sensor status at line %d.\n", __FUNCTION__, __LINE__); 
            sas_virtual_phy_free_allocated_memory(new_info);

            return NULL;
        }

        fbe_zero_memory(new_info->overall_temp_sensor_status, max_temp_sensor_elems * sizeof(ses_stat_elem_temp_sensor_struct));

        new_info->temp_sensor_status = 
            (ses_stat_elem_temp_sensor_struct *)fbe_terminator_allocate_memory(max_temp_sensor_elems * sizeof(ses_stat_elem_temp_sensor_struct));
        if (new_info->temp_sensor_status == NULL)
        {
            printf( "%s failed to allocate memory for temp sensor status at line %d.\n", __FUNCTION__, __LINE__); 
            sas_virtual_phy_free_allocated_memory(new_info);

            return NULL;
        }

        fbe_zero_memory(new_info->temp_sensor_status, max_temp_sensor_elems * sizeof(ses_stat_elem_temp_sensor_struct));

        for(temp_sensor_index = 0; temp_sensor_index < max_temp_sensor_elems; temp_sensor_index ++)
        {
            // overall temperature status element.
            new_info->overall_temp_sensor_status[temp_sensor_index].cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
            new_info->overall_temp_sensor_status[temp_sensor_index].ot_failure = 0;
            new_info->overall_temp_sensor_status[temp_sensor_index].ot_warning = 0;
            new_info->overall_temp_sensor_status[temp_sensor_index].ident = 0;
            // it was not yet decided what should be the temperature. put to room temp
            // of 22. It should be offset by 20.
            new_info->overall_temp_sensor_status[temp_sensor_index].temp = 22+42;

            new_info->temp_sensor_status[temp_sensor_index].cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
            new_info->temp_sensor_status[temp_sensor_index].ot_failure = 0;
            new_info->temp_sensor_status[temp_sensor_index].ot_warning = 0;
            new_info->temp_sensor_status[temp_sensor_index].ident = 0;
            // it was not yet decided what should be the temperature. put to room temp
            // of 22. It should be offset by 20.
            new_info->temp_sensor_status[temp_sensor_index].temp = 22+42;
        }
    }
    else
    {
        new_info->overall_temp_sensor_status = NULL;
        new_info->temp_sensor_status = NULL;
    }

    //ENCLOSURE ELEMENTS for LOCAL LCC & CHASSIS
    fbe_zero_memory(&new_info->encl_status, sizeof(ses_stat_elem_encl_struct));
    fbe_zero_memory(&new_info->chassis_encl_status, sizeof(ses_stat_elem_encl_struct));
    new_info->encl_status.cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
    new_info->chassis_encl_status.cmn_stat.elem_stat_code = SES_STAT_CODE_OK;

    // Initially unit attention is not set on the Unit.
    new_info->unit_attention = FBE_FALSE;

    {
        switch(encl_type)
        {
            case FBE_SAS_ENCLOSURE_TYPE_TABASCO:
                new_info->eses_page_info.vp_config_diag_page_info.config_page = tabasco_config_page_with_ps[side];
                new_info->eses_page_info.vp_config_diag_page_info.config_page_info = &tabasco_config_page_info_with_ps[side];
                new_info->eses_page_info.vp_config_diag_page_info.config_page_size = sizeof(tabasco_config_page_with_ps[side]);
                break;
            default:
                return (NULL);
        }
    }

    // Initialize the eses related page information stored per VPhy.
    status = terminator_initialize_eses_page_info(encl_type, &new_info->eses_page_info);
    if(status != FBE_STATUS_OK)
    {
        sas_virtual_phy_free_allocated_memory(new_info);
        return(NULL);
    }

    //DISPLAY CHARACTERS
    status = sas_virtual_phy_max_display_characters(encl_type, &max_diplay_characters, side);
    if(status != FBE_STATUS_OK)
    {
        sas_virtual_phy_free_allocated_memory(new_info);
        return(NULL);
    }
    if ( max_diplay_characters > 0 )
    {
        // There is one display status element for each display character
        new_info->display_status = 
            (ses_stat_elem_display_struct *)fbe_terminator_allocate_memory(max_diplay_characters * sizeof(ses_stat_elem_display_struct));
        if (new_info->display_status == NULL)
        {
            printf( "%s failed to allocate memory for display status at line %d.\n", __FUNCTION__, __LINE__); 
            sas_virtual_phy_free_allocated_memory(new_info);

            return NULL;
        }
        fbe_zero_memory(new_info->display_status , max_diplay_characters * sizeof(ses_stat_elem_display_struct));

        // Initialize all the display characters.
        for(display_index = 0; display_index < max_diplay_characters; display_index++)
        {
            new_info->display_status[display_index].cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
            new_info->display_status[display_index].display_mode_status = 0x01;
            new_info->display_status[display_index].display_char_stat = '-';
        }
    }
    else
    {
        new_info->display_status = NULL;
    }

    // EMC Enclosure Status
    new_info->emcEnclStatus.shutdown_reason = FBE_ESES_SHUTDOWN_REASON_NOT_SCHEDULED;

    // EMC Power Supply Status
    new_info->emcPsInfoStatus.margining_test_mode = FBE_ESES_MARGINING_TEST_MODE_STATUS_TEST_SUCCESSFUL;
    new_info->emcPsInfoStatus.margining_test_results = FBE_ESES_MARGINING_TEST_RESULTS_SUCCESS;

    switch(new_info->enclosure_type)
    {
        case FBE_SAS_ENCLOSURE_TYPE_VIPER:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_MAGNUM:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_BUNKER:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_CITADEL:
            break;      
        case FBE_SAS_ENCLOSURE_TYPE_DERRINGER:
            break;    
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_ICM:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_VOYAGER_EE:
            break;     
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_DRVSXP:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_VIKING_IOSXP:
            break;     
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_DRVSXP:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_CAYENNE_IOSXP:
            break;     
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_DRVSXP:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_IOSXP:
            break;    
        case FBE_SAS_ENCLOSURE_TYPE_FALLBACK:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_BOXWOOD:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_KNOT:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_PINECONE:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_STEELJAW:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_RAMHORN:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_ANCHO:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_CALYPSO:
        case FBE_SAS_ENCLOSURE_TYPE_RHEA:
        case FBE_SAS_ENCLOSURE_TYPE_MIRANDA:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_TABASCO:
            break;  
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_DRVSXP:
            break;
        case FBE_SAS_ENCLOSURE_TYPE_NAGA_80_IOSXP:
            break;
        default:
            break;
    }

    new_info->miniport_sas_device_table_index = INVALID_TMSDT_INDEX;

    // Initialize new firmware revision queue
    fbe_queue_init(&new_info->new_firmware_rev_queue_head);

    // set time intervals to default value
    new_info->activate_time_intervel = DEFAULT_ENCLOSURE_FIRMWARE_ACTIVATE_TIME_INTERVAL;
    new_info->reset_time_intervel = DEFAULT_ENCLOSURE_FIRMWARE_RESET_TIME_INTERVAL;

    return new_info;
}

