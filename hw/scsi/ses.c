/*
 * SCSI Enclosure Service emulation
 *
 */

#define DEBUG_SCSI

#ifdef DEBUG_SCSI
#define DPRINTF_TS(fmt, ...) \
do { struct timeval _now; gettimeofday(&_now, NULL); \
    qemu_log_mask(LOG_TRACE, "[%zd.%06zd] scsi-enclosure: " fmt, (size_t)_now.tv_sec, (size_t)_now.tv_usec, ##__VA_ARGS__); \
} while (0)

#define DPRINTF(fmt, ...) \
do {  qemu_log_mask(LOG_TRACE, fmt, ##__VA_ARGS__); \
} while (0)
#else
#define DPRINTF_TS(fmt, ...) do {} while(0)
#define DPRINTF(fmt, ...) do {} while(0)
#endif

#include "ses.h"
#include "eses.h"

#ifdef __linux
#include <scsi/sg.h>
#include <unistd.h>
#include "trace/control.h"
#include "trace.h"
#endif

static void scsi_free_request(SCSIRequest *req)
{
}

/* Helper function for command completion with sense.  */
static void scsi_check_condition(SCSISESReq *r, SCSISense sense)
{
    DPRINTF("Command complete tag=0x%x sense=%d/%d/%d\n",
            r->req.tag, sense.key, sense.asc, sense.ascq);
    scsi_req_build_sense(&r->req, sense);
    scsi_req_complete(&r->req, CHECK_CONDITION);
}

static void eses_sas_info_array_device_slot_status(SCSISESState *s)
{
    // Get current valid scsi-id range
    SCSIDevice *d = (SCSIDevice *)s;
    terminator_sas_virtual_phy_info_t *info = (terminator_sas_virtual_phy_info_t *)s->eses_sas_info;
    fbe_sas_enclosure_type_t encl_type = info->enclosure_type;
    fbe_u32_t exp_scsi_id = d->id;
    fbe_u8_t downstream_phys;

    // Init all device status to "NOT_INSTALLED"
    fbe_u32_t i;
    sas_virtual_phy_max_drive_slots(encl_type, &downstream_phys);
    fbe_u32_t valid_scsi_id_start = exp_scsi_id - downstream_phys;
    for (i = 0; i < downstream_phys; i++) {
        (info->drive_slot_status+i)->cmn_stat.elem_stat_code = SES_STAT_CODE_NOT_INSTALLED;
    }

    // Scan all device, and set status to OK if drive device is inserted
    SCSIBus *bus = scsi_bus_from_device((SCSIDevice *)s);
    BusChild *kid;
    QTAILQ_FOREACH_REVERSE(kid, &bus->qbus.children, ChildrenHead, sibling) {
        DeviceState *qdev = kid->child;
        SCSIDevice *dev = SCSI_DEVICE(qdev);

        //trace_eses_encl_status_diag_page_array_device_slot(dev, dev->channel, dev->id, dev->lun);
        if (dev->type == TYPE_DISK) {
            if (dev->id >= valid_scsi_id_start && dev->id < exp_scsi_id) {
                // get slot from scsi-id
                fbe_u8_t slot;
                slot = dev->id - valid_scsi_id_start;
                // set the status to OK
                (info->drive_slot_status + slot)->cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
            }
        }
    }
}

static void eses_sas_info_exp_phy_status(SCSISESState *s)
{
    // Prepare info
    terminator_sas_virtual_phy_info_t *info = (terminator_sas_virtual_phy_info_t *)s->eses_sas_info;
    fbe_sas_enclosure_type_t encl_type = info->enclosure_type;
    fbe_u8_t max_conn_id_count = 0;
    fbe_u8_t max_single_lane_port_conn_count = 0;
    fbe_u8_t max_phy_count = 0;
    fbe_u8_t max_drive_count = 0;
    fbe_status_t status = FBE_STATUS_OK;

    // Get mapping based on encl_type
    status = sas_virtual_phy_max_conn_id_count(encl_type, &max_conn_id_count);
    if (status != FBE_STATUS_OK) {
        trace_eses_sas_info_exp_phy_status("Get max_conn_id_count", status);
    }

    status = sas_virtual_phy_max_single_lane_conns_per_port(encl_type, &max_single_lane_port_conn_count);
    if (status != FBE_STATUS_OK) {
        trace_eses_sas_info_exp_phy_status("Get max_single_lane_conns_per_port", status);
    }

    status = sas_virtual_phy_max_phys(encl_type, &max_phy_count);
    if (status != FBE_STATUS_OK) {
        trace_eses_sas_info_exp_phy_status("Get max_phys", status);
    }

    status = sas_virtual_phy_max_drive_slots(encl_type, &max_drive_count);
    if (status != FBE_STATUS_OK) {
        trace_eses_sas_info_exp_phy_status("Get max_drive_slots", status);
    }
    //Init all phy status to NOT_AVAILABLE
    fbe_u32_t i, j;
    for (i = 0; i < max_phy_count; i++){
        info->phy_status[i].cmn_stat.elem_stat_code = SES_STAT_CODE_UNAVAILABLE;
        info->phy_status[i].phy_rdy = 0x0;
        info->phy_status[i].link_rdy = 0x0;
    }

    // For phy in drive_slot_to_phy_map,
    // and individual_conn_phy_mapping, (depending on max_conn_id_count and max_single_lane_port_conn_count)
    // mark stat code of phy in upstream connector as OK, otherwise, mark as NOT_AVAILABLE
    fbe_u8_t phy_id;
    for (i = 0; i < max_drive_count; i++) {
        status = sas_virtual_phy_get_drive_slot_to_phy_mapping(i, &phy_id, encl_type);
        info->phy_status[phy_id].cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
    }

    // If phy of upstream and downstream port are connected, set LINK_RDY and PHY_RDY to ready
    for (j = 0; j < max_conn_id_count; j++) {
        for (i = 0; i < max_single_lane_port_conn_count; i++) {
            status = sas_virtual_phy_get_individual_conn_to_phy_mapping(i, j, &phy_id, encl_type);
            info->phy_status[phy_id].cmn_stat.elem_stat_code = SES_STAT_CODE_OK;
        }
    }

    // If drive is inserted on this phy, set LINK_RDY and PHY_RDY to ready, otherwise, set it to not ready
    SCSIDevice *d = (SCSIDevice *)s;
    fbe_u32_t exp_scsi_id = d->id;
    fbe_u8_t downstream_phys;
    sas_virtual_phy_max_drive_slots(encl_type, &downstream_phys);
    fbe_u32_t valid_scsi_id_start = exp_scsi_id - downstream_phys;
    SCSIBus *bus = scsi_bus_from_device((SCSIDevice *)s);
    BusChild *kid;
    fbe_u8_t slot;
    QTAILQ_FOREACH_REVERSE(kid, &bus->qbus.children, ChildrenHead, sibling){
        DeviceState *qdev = kid->child;
        SCSIDevice *dev = SCSI_DEVICE(qdev);

        //trace_eses_encl_status_diag_page_array_device_slot(dev, dev->channel, dev->id, dev->lun);
        if (dev->type == TYPE_DISK)
        {
            if (dev->id >= valid_scsi_id_start && dev->id < exp_scsi_id){
                // get slot from scsi-id
                slot = dev->id - valid_scsi_id_start;
                // set PHY_RDY to ready
                info->phy_status[slot].phy_rdy = 0x1;
                // FIXME
                // set LINK_RDY to ready
                info->phy_status[slot].link_rdy = info->phy_status[slot].phy_rdy;
            }
        }
    }

}

static void *eses_sas_info_handler(void *args)
{
    SCSISESState *s = args;
    qemu_thread_get_self(&s->eses_sas_info_thread);

    while (true) {
        // Maintain array device slot information
        sleep(3);

        // Set expander phy status
        eses_sas_info_exp_phy_status(s);

        // Set array device slot status according to SAS_PHY_RDY
        eses_sas_info_array_device_slot_status(s);

    }

    return NULL;
}

static int build_enclosure_status_diagnostic_page(SCSIRequest *req, uint8_t *outbuf)
{
    SCSISESState *s = DO_UPCAST(SCSISESState, qdev, req->dev);
    terminator_sas_virtual_phy_info_t * info = s->eses_sas_info;
    SCSISESReq *r = DO_UPCAST(SCSISESReq, req, req);
    fbe_sas_enclosure_type_t  encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    ses_pg_encl_stat_struct  *encl_status_diag_page_hdr = NULL;
    int buflen = -1;
    fbe_u8_t *encl_stat_diag_page_start_ptr = NULL; // point to first overall status element
    fbe_u8_t *stat_elem_end_ptr = NULL;
    fbe_u16_t encl_stat_page_size = 0;
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;

    encl_type = (fbe_sas_enclosure_type_t)s->dae_type;
    if (r->buflen < enclosure_status_diagnostic_page_size(encl_type)) {
        DPRINTF("outbuf too small to hold response\n");
        return -1;
    }

    encl_status_diag_page_hdr = (ses_pg_encl_stat_struct *)outbuf;
    encl_status_diag_page_hdr->pg_code = SES_PG_CODE_ENCL_STAT;
    // ignore these for now.
    encl_status_diag_page_hdr->unrecov = 0;
    encl_status_diag_page_hdr->crit = 0;
    encl_status_diag_page_hdr->non_crit = 0;
    // for now ignore info, as anyhow the client compares each poll response
    // and takes action in casae of a change. if this is ignored generation
    // code is also ignored.
    encl_status_diag_page_hdr->info = 0;

    status = config_page_get_gen_code(info, &encl_status_diag_page_hdr->gen_code);
    RETURN_ON_ERROR_STATUS;
    encl_status_diag_page_hdr->gen_code = BYTE_SWAP_32(encl_status_diag_page_hdr->gen_code);

    // ignore for now,, he he ;-) I like these words
    encl_status_diag_page_hdr->invop = 0;

    // Get the page size from the config page being used.
    encl_stat_page_size = info->eses_page_info.vp_config_diag_page_info.config_page_info->encl_stat_diag_page_size;
    encl_status_diag_page_hdr->pg_len = encl_stat_page_size - 4;
    encl_status_diag_page_hdr->pg_len = BYTE_SWAP_16(encl_status_diag_page_hdr->pg_len);

    encl_stat_diag_page_start_ptr = ((fbe_u8_t *)encl_status_diag_page_hdr);

    enclosure_status_diagnostic_page_build_status_elements(encl_stat_diag_page_start_ptr, 
                                                           &stat_elem_end_ptr, 
                                                           info);
    // page length can be obtained by subtracting status_elements_end_ptr from
    // status_elements_start_ptr. keep for future.
    buflen = stat_elem_end_ptr - encl_stat_diag_page_start_ptr + 1;

    return buflen;
}

static fbe_status_t build_configuration_diagnostic_page(SCSIRequest *req, uint8_t *outbuf)
{
    SCSISESState *s = DO_UPCAST(SCSISESState, qdev, req->dev);
    terminator_sas_virtual_phy_info_t * info = s->eses_sas_info;
    fbe_sas_enclosure_type_t  encl_type = info->enclosure_type;
    fbe_status_t status;
    vp_config_diag_page_info_t *config_diag_page_info;
    uint8_t *buffer = NULL;
    uint16_t eses_version = 0;


    buffer = outbuf;

    fbe_terminator_sas_enclosure_get_eses_version_desc(encl_type, &eses_version);
    config_diag_page_info = &info->eses_page_info.vp_config_diag_page_info;

    memcpy(buffer, config_diag_page_info->config_page, (config_diag_page_info->config_page_size) * sizeof(uint8_t));
    status = config_page_set_all_ver_descs_in_config_page(buffer,
                                                          config_diag_page_info->config_page_info,
                                                          config_diag_page_info->ver_desc,
                                                          eses_version);

    if (status != FBE_STATUS_OK)
        return -1;


    status = config_page_set_gen_code_by_config_page(buffer, config_diag_page_info->gen_code);
    //encl_status_diag_page_hdr->gen_code = BYTE_SWAP_32(encl_status_diag_page_hdr->gen_code);
    if (status != FBE_STATUS_OK)
        return -1;

    return status;
}

static fbe_status_t build_emc_statistics_status_diagnostic_page(SCSIRequest *req, uint8_t *outbuf)
{
    SCSISESState *s = DO_UPCAST(SCSISESState, qdev, req->dev);
    terminator_sas_virtual_phy_info_t * info = s->eses_sas_info;
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_common_pg_hdr_struct *emc_statistics_stat_page_hdr;
    uint8_t *elem_stats_start_ptr = NULL; 
    uint8_t *elem_stats_end_ptr = NULL;
    //fbe_sas_enclosure_type_t  encl_type = info->enclosure_type;

    emc_statistics_stat_page_hdr = (ses_common_pg_hdr_struct *)outbuf;
    memset(emc_statistics_stat_page_hdr, 0, sizeof(ses_common_pg_hdr_struct));

    emc_statistics_stat_page_hdr->pg_code = SES_PG_CODE_EMC_STATS_STAT;

    // Gen code returned in bigEndian format.
    status = config_page_get_gen_code(info, 
                                      &emc_statistics_stat_page_hdr->gen_code);
    emc_statistics_stat_page_hdr->gen_code = 
        bswap32(emc_statistics_stat_page_hdr->gen_code);


    elem_stats_start_ptr = (uint8_t *)(emc_statistics_stat_page_hdr + 1);
    status = emc_statistics_stat_page_build_device_slot_stats(info, elem_stats_start_ptr, 
                                                              &elem_stats_end_ptr);

    if (status != FBE_STATUS_OK)
        return status;

    emc_statistics_stat_page_hdr->pg_len = 
        (uint16_t)((elem_stats_end_ptr - ((uint8_t *)emc_statistics_stat_page_hdr)) - 4);
    // Always return page length in big endian format as actual expander firmware does that.
    emc_statistics_stat_page_hdr->pg_len = bswap16(emc_statistics_stat_page_hdr->pg_len);

    return(status);     
}

static fbe_status_t build_additional_element_status_diagnostic_page(SCSIRequest *req, uint8_t *outbuf)
{
    SCSISESState *s = DO_UPCAST(SCSISESState, qdev, req->dev);
    terminator_sas_virtual_phy_info_t * info = s->eses_sas_info;
    //fbe_sas_enclosure_type_t  encl_type = info->enclosure_type;
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_common_pg_hdr_struct *addl_elem_stat_page_hdr;
    uint8_t *status_elements_start_ptr = NULL; // point to first overall status element
    uint8_t *status_elements_end_ptr = NULL;

    addl_elem_stat_page_hdr = (ses_common_pg_hdr_struct *)outbuf;
    memset(addl_elem_stat_page_hdr, 0, sizeof(ses_common_pg_hdr_struct));
    addl_elem_stat_page_hdr->pg_code = SES_PG_CODE_ADDL_ELEM_STAT;
    // Gen code returned in bigendian format.
    status = config_page_get_gen_code(info, &addl_elem_stat_page_hdr->gen_code); 
    if (status != FBE_STATUS_OK)
        return status;

    addl_elem_stat_page_hdr->gen_code = bswap32(addl_elem_stat_page_hdr->gen_code);

    status_elements_start_ptr = (uint8_t *) (((uint8_t *)addl_elem_stat_page_hdr) + FBE_ESES_PAGE_HEADER_SIZE);
    status = addl_elem_stat_page_build_stat_descriptors(info, status_elements_start_ptr, &status_elements_end_ptr);
    if (status != FBE_STATUS_OK) {
        DPRINTF("%s: addl_elem_stat_page_build_stat_descriptors() failed: %x\n", __FUNCTION__, status);
        return status;
    }
    addl_elem_stat_page_hdr->pg_len = (uint16_t)(status_elements_end_ptr - (uint8_t *)(addl_elem_stat_page_hdr) - 4);
    // Always return page length in big endian format as actual expander firmware does that.
    addl_elem_stat_page_hdr->pg_len = bswap16(addl_elem_stat_page_hdr->pg_len);

    status = FBE_STATUS_OK;
    return(status); 
}

static fbe_status_t build_emc_enclosure_status_diagnostic_page(SCSIRequest *req, uint8_t *outbuf)
{
    SCSISESState *s = DO_UPCAST(SCSISESState, qdev, req->dev);
    terminator_sas_virtual_phy_info_t * info = s->eses_sas_info;
    //fbe_sas_enclosure_type_t  encl_type = info->enclosure_type;
    fbe_status_t status = FBE_STATUS_GENERIC_FAILURE;
    ses_pg_emc_encl_stat_struct *emc_encl_stat_diag_page_hdr;
    ses_pg_emc_encl_stat_struct terminatorEmcEnclStatus;
    uint8_t *elements_start_ptr = NULL; 
    uint8_t *elements_end_ptr = NULL;
    fbe_eses_info_elem_group_hdr_t *info_elem_group_hdr = NULL;
    uint8_t num_of_expander_elem=0;
    uint8_t num_of_drive_elem=0;

    emc_encl_stat_diag_page_hdr = (ses_pg_emc_encl_stat_struct *)outbuf;
    memset(emc_encl_stat_diag_page_hdr, 0, sizeof(ses_pg_emc_encl_stat_struct));

    // ignore other fields.
    emc_encl_stat_diag_page_hdr->hdr.pg_code = SES_PG_CODE_EMC_ENCL_STAT;
    // Gen code returned in bigEndian format.
    status = config_page_get_gen_code(info, &emc_encl_stat_diag_page_hdr->hdr.gen_code);
    if (status != FBE_STATUS_OK)
        return status;

    emc_encl_stat_diag_page_hdr->hdr.gen_code = bswap32(emc_encl_stat_diag_page_hdr->hdr.gen_code);

    emc_encl_stat_diag_page_hdr->num_info_elem_groups = TERMINATOR_EMC_PAGE_NUM_INFO_ELEM_GROUPS;

    info_elem_group_hdr = (fbe_eses_info_elem_group_hdr_t *)(emc_encl_stat_diag_page_hdr + 1);
    info_elem_group_hdr->info_elem_type = FBE_ESES_INFO_ELEM_TYPE_SAS_CONN;
    info_elem_group_hdr->info_elem_size = FBE_ESES_EMC_CONTROL_SAS_CONN_INFO_ELEM_SIZE;
    elements_start_ptr = (uint8_t *)(info_elem_group_hdr + 1);
    status = emc_encl_stat_diag_page_build_sas_conn_inf_elems(info, elements_start_ptr, 
                                                             &elements_end_ptr, 
                                                             &info_elem_group_hdr->num_info_elems);
    if (status != FBE_STATUS_OK)
        return status;

    info_elem_group_hdr = (fbe_eses_info_elem_group_hdr_t *)(elements_end_ptr);
    info_elem_group_hdr->info_elem_type = FBE_ESES_INFO_ELEM_TYPE_TRACE_BUF;
    info_elem_group_hdr->info_elem_size = FBE_ESES_EMC_CONTROL_TRACE_BUF_INFO_ELEM_SIZE;
    elements_start_ptr = (uint8_t *)(info_elem_group_hdr + 1);
    status = emc_encl_stat_diag_page_build_trace_buffer_inf_elems(info, elements_start_ptr, 
                                                                &elements_end_ptr, 
                                                                &info_elem_group_hdr->num_info_elems);   
    if (status != FBE_STATUS_OK)
        return status;

    info_elem_group_hdr = (fbe_eses_info_elem_group_hdr_t *)(elements_end_ptr);
    info_elem_group_hdr->info_elem_type = FBE_ESES_INFO_ELEM_TYPE_GENERAL;
    info_elem_group_hdr->info_elem_size = FBE_ESES_EMC_CONTROL_GENERAL_INFO_ELEM_SIZE;
    elements_start_ptr = (uint8_t *)(info_elem_group_hdr + 1);
    status = emc_encl_stat_diag_page_build_general_info_expander_elems(info, elements_start_ptr, 
                                                                &elements_end_ptr, 
                                                                &num_of_expander_elem);   
    if (status != FBE_STATUS_OK)
        return status;

    info_elem_group_hdr->num_info_elems = num_of_expander_elem;

    status = emc_encl_stat_diag_page_build_general_info_drive_slot_elems(info, elements_end_ptr, 
                                                                &elements_end_ptr, 
                                                                &num_of_drive_elem);   
    if (status != FBE_STATUS_OK)
        return status;

    info_elem_group_hdr->num_info_elems += num_of_drive_elem;

    info_elem_group_hdr = (fbe_eses_info_elem_group_hdr_t *)(elements_end_ptr);
    info_elem_group_hdr->info_elem_type = FBE_ESES_INFO_ELEM_TYPE_PS;
    info_elem_group_hdr->info_elem_size = FBE_ESES_EMC_CTRL_STAT_PS_INFO_ELEM_SIZE;
    elements_start_ptr = (uint8_t *)(info_elem_group_hdr + 1);
    status = emc_encl_stat_diag_page_build_ps_info_elems(info, elements_start_ptr, 
                                                         &elements_end_ptr, 
                                                        &info_elem_group_hdr->num_info_elems);   
    if (status != FBE_STATUS_OK)
        return status;

    emc_encl_stat_diag_page_hdr->hdr.pg_len = (uint16_t)((elements_end_ptr - ((uint8_t *)emc_encl_stat_diag_page_hdr)) - 4);
    // Always return page length in big endian format as actual expander firmware does that.
    emc_encl_stat_diag_page_hdr->hdr.pg_len = bswap16(emc_encl_stat_diag_page_hdr->hdr.pg_len);

    status = sas_virtual_phy_get_emcEnclStatus(info, &terminatorEmcEnclStatus);
    if (status != FBE_STATUS_OK)
    emc_encl_stat_diag_page_hdr->shutdown_reason = terminatorEmcEnclStatus.shutdown_reason;

    return(status);     
}

static int scsi_ses_emulate_recv_diag(SCSIRequest *req,  uint8_t *outbuf, struct SCSISense **senseCode)
{
    SCSISESState *s = DO_UPCAST(SCSISESState, qdev, req->dev);
    int buflen = 0;
    fbe_status_t                        status = FBE_STATUS_GENERIC_FAILURE;
    fbe_sas_enclosure_type_t            encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    fbe_bool_t not_ready = FBE_TRUE;
    
    encl_type = (fbe_sas_enclosure_type_t)s->dae_type;
    status = sas_virtual_phy_check_enclosure_type(encl_type);
    if (status != FBE_STATUS_OK) {
       DPRINTF("Unknown DAE type: %x\n", encl_type);
       return -1;
    }
    /* PCV must be 1 */
    if (req->cmd.buf[1] != 1) {
        *senseCode = (struct SCSISense *)&sense_code_INVALID_FIELD;
        return -1;
    }

#if 0
    // Check if the user marked the page as unsupported
    if(!terminator_is_eses_page_supported(FBE_SCSI_RECEIVE_DIAGNOSTIC, receive_diagnostic_page_code))
    {
        //set up sense data here.
        build_sense_info_buffer(sense_buffer, 
            FBE_SCSI_SENSE_KEY_ILLEGAL_REQUEST, 
            FBE_SCSI_ASC_UNSUPPORTED_ENCLOSURE_FUNCTION,
            FBE_SCSI_ASCQ_UNSUPPORTED_ENCLOSURE_FUNCTION);
        fbe_payload_cdb_set_scsi_status(payload_cdb_operation, FBE_PAYLOAD_CDB_SCSI_STATUS_CHECK_CONDITION);
        fbe_payload_cdb_set_request_status(payload_cdb_operation, FBE_PORT_REQUEST_STATUS_ERROR);
        return(FBE_STATUS_OK);
    }
#endif

    switch(req->cmd.buf[2])
    {
        case SES_PG_CODE_ENCL_STAT:
            buflen = build_enclosure_status_diagnostic_page(req, outbuf);
            break;
        case SES_PG_CODE_CONFIG:
            status = build_configuration_diagnostic_page(req, outbuf);
            break;
        case SES_PG_CODE_EMC_STATS_STAT:
            status = build_emc_statistics_status_diagnostic_page(req, outbuf);
            break;
        case SES_PG_CODE_ADDL_ELEM_STAT:
            status = build_additional_element_status_diagnostic_page(req, outbuf);
            break;
        case SES_PG_CODE_EMC_ENCL_STAT:
            status = build_emc_enclosure_status_diagnostic_page(req, outbuf);
            break;
        case SES_PG_CODE_DOWNLOAD_MICROCODE_STAT:
            //status = build_download_microcode_status_diagnostic_page(sg_list, virtual_phy_handle);
            break;
        default:
            status = FBE_STATUS_GENERIC_FAILURE;
            not_ready = FBE_FALSE;
            *senseCode = (struct SCSISense *)&sense_code_ILLEGAL_REQ_ENCL_FUNC_NOT_SUPPORTED;
            break;
    }

    if (status != FBE_STATUS_OK)
    {
        if(not_ready) {
            // As the status page building failed, this indicates some bug 
            // (possible) in code. So put "NOT READY" in  sense data and return.
            DPRINTF("%s Building status pages failed, possible BUG in terminator?\n", __FUNCTION__);
            *senseCode = (struct SCSISense *)&sense_code_KEY_NOT_READY_ENCL_SRVC_NA;
        }
        return -1;
    }

    return buflen;
}

static int scsi_ses_emulate_inquiry(SCSIRequest *req, uint8_t *outbuf)
{
    SCSISESState *s = DO_UPCAST(SCSISESState, qdev, req->dev);
    int buflen = 0;
    int start;
    terminator_sas_encl_inq_data_t *    inq_data = NULL;
    fbe_status_t                        status = FBE_STATUS_GENERIC_FAILURE;
    fbe_sas_enclosure_type_t            encl_type = FBE_SAS_ENCLOSURE_TYPE_INVALID;
    
    encl_type = (fbe_sas_enclosure_type_t)s->dae_type;
    status = sas_virtual_phy_check_enclosure_type(encl_type);
    if (status != FBE_STATUS_OK) {
        DPRINTF("Unknown DAE type: 0x%x.\n", encl_type);
        return -1;
    }
    sas_enclosure_get_inq_data(encl_type, &inq_data);

    if (req->cmd.buf[1] & 0x1) {
        /* Vital product data */
        uint8_t page_code = req->cmd.buf[2];

        outbuf[buflen++] = s->qdev.type & 0x1f;
        outbuf[buflen++] = page_code ; // this page
        outbuf[buflen++] = 0x00;
        outbuf[buflen++] = 0x00;
        start = buflen;

        switch (page_code) {
        case 0x00: /* Supported page codes, mandatory */
        {
            DPRINTF("Inquiry EVPD[Supported pages] buffer size %zd\n", req->cmd.xfer);
            outbuf[buflen++] = 0x00; // list of supported pages (this page)
            if (s->serial) {
                outbuf[buflen++] = 0x80; // unit serial number
            }
            outbuf[buflen++] = 0x83; // device identification
            outbuf[buflen++] = 0x86; 
            outbuf[buflen++] = 0xc0;
            outbuf[buflen++] = 0xc1;
            outbuf[buflen++] = 0xc2;
            outbuf[buflen++] = 0xc3;
            outbuf[buflen++] = 0xc4;
            outbuf[buflen++] = 0xc5;
            outbuf[buflen++] = 0xc6;
            outbuf[buflen++] = 0xc7;
            break;
        }
        case 0x80: /* Device serial number, optional */
        {
            int l;

            if (!s->serial) {
                DPRINTF("Inquiry (EVPD[Serial number] not supported\n");
                return -1;
            }

            l = strlen(s->serial);
            if (l > 36) {
                l = 36;
            }

            DPRINTF("Inquiry EVPD[Serial number] "
                    "buffer size %zd\n", req->cmd.xfer);
            memcpy(outbuf + buflen, s->serial, l);
            buflen += l;
            break;
        }
        case 0x83: /* Device identification page, mandatory */
        {
            const uint64_t sas_addr_0 = 0x50060480E3FAAA09;
            const uint64_t sas_addr_1 = 0x500604810B18F57E;

            DPRINTF("Inquiry EVPD[Device identification] buffer size %zd\n", req->cmd.xfer);

            // first designation descriptor: logical unit id
            outbuf[buflen++] = 0x01; // Binary
            outbuf[buflen++] = 0x03; // PIV: 0b, ASSOC: 00b, desigtype: 03h
            outbuf[buflen++] = 0x00; // reserved
            outbuf[buflen++] = 8;
            *(uint64_t*)&outbuf[buflen] = cpu_to_be64(sas_addr_0);
            buflen += 8;

            // second designation descriptor: target port id
            outbuf[buflen++] = 0x61; // binary, proto id 6h
            outbuf[buflen++] = 0x93; // PIV: 1b, ASSOC: 01b, desigtype: 03h
            outbuf[buflen++] = 0x00; // reserved
            outbuf[buflen++] = 8;
            *(uint64_t*)&outbuf[buflen] = cpu_to_be64(sas_addr_1);
            buflen += 8;

            // third designation descriptor: relative target port id
            outbuf[buflen++] = 0x61; // binary, proto id 6h
            outbuf[buflen++] = 0x94; // PIV: 1b, ASSOC: 01b, desigtype: 03h
            outbuf[buflen++] = 0x00; // reserved
            outbuf[buflen++] = 4;
            outbuf[buflen++] = 0;
            outbuf[buflen++] = 0;
            outbuf[buflen++] = 0;
            outbuf[buflen++] = 1;

            break;
        }
        case 0x86:
        {
            //outbuf[3] = 0x3C;
            outbuf[5] = 0x01; // SIMPSUP: 1b
            buflen = 64;
            break;
        }
        default:
            DPRINTF("Inquiry EVPD[page 0x%x]: not supported\n", page_code);
            return -1;
        }

        /* done with EVPD */
        assert(buflen - start <= 255);
        outbuf[start - 1] = buflen - start;
        return buflen;
    }

    /* Standard INQUIRY data */
    if (req->cmd.buf[2] != 0) {
        return -1;
    }

    /* PAGE CODE == 0 */
    buflen = req->cmd.xfer;
    if (buflen > SCSI_MAX_INQUIRY_LEN) {
        buflen = SCSI_MAX_INQUIRY_LEN;
    }

    outbuf[0] = s->qdev.type & 0x1f;
    outbuf[1] = 0;

    /*
     * We claim conformance to SPC-3, which is required for guests
     * to ask for modern features like READ CAPACITY(16) or the
     * block characteristics VPD page by default.  Not all of SPC-3
     * is actually implemented, but we're good enough.
     */
    outbuf[2] = 6;
    outbuf[3] = 2 | 0x10; /* Format 2, HiSup */

    outbuf[6] = 0xD0; /* BQue | EncServ | Multip */
    /* Sync data transfer and TCQ.  */
    outbuf[7] = 0x02; /* CMDQue */

    /* 8-15: T10 vendor ID */
    //strpadcpy((char *) &outbuf[8], 8, "EMC", ' ');
    strpadcpy((char *) &outbuf[8], 8, (const char *)inq_data->vendor, ' ');
    /* 16-31: product ID */
    //strpadcpy((char *) &outbuf[16], 16, "ESES Enclosure", ' ');
    strpadcpy((char *) &outbuf[16], 16, (const char *)inq_data->product_id, ' ');
    /* 32-35: product revision level, in ascii */
    strpadcpy((char *) &outbuf[32], 4, (const char *)inq_data->product_revision_level, '0');

    if (buflen > 36) {
        outbuf[4] = buflen - 5; /* Additional Length = (Len - 1) - 4 */
    } else {
        /* If the allocation length of CDB is too small,
           the additional length is not adjusted */
        outbuf[4] = 36 - 5;
        return buflen;
    }

    /*  component vendor ID: 'PMCSIERA' */
    strpadcpy((char*)&outbuf[36], 8, (const char *)inq_data->component_vendor_id, '\0');

    /*  component ID: 0x8054, SXP36X12G chip */
     *(uint16_t*)&outbuf[44] = inq_data->component_id;

    /* component revision level */
    outbuf[46] = inq_data->component_revision_level;

    *(uint16_t*)&outbuf[58] = cpu_to_be16(0x00A0); /* SAM-5 */
    *(uint16_t*)&outbuf[60] = cpu_to_be16(0x0C20); /* SAS-2 */
    *(uint16_t*)&outbuf[62] = cpu_to_be16(0x0460); /* SPC-4 */
    *(uint16_t*)&outbuf[64] = cpu_to_be16(0x0580); /* SES-3 */

    *(uint16_t*)&outbuf[112] = inq_data->enclosure_platform_type; /* enclosure platform type: Tabasco */
    *(uint16_t*)&outbuf[114] = inq_data->board_type; /* enclosure board type: Tabasco */
    *(uint16_t*)&outbuf[118] = inq_data->eses_version_descriptor; /* eses version descriptor */

    if (s->serial) {
        strpadcpy((char*)&outbuf[120], 16, s->serial, '\0'); /* enclosure unique id */
    } else {
        strpadcpy((char*)&outbuf[120], 16, (const char *)inq_data->enclosure_unique_id, '\0'); /* enclosure unique id */
    }

    return buflen;
}

static int scsi_ses_emulate_mode_sense(SCSISESReq *r, uint8_t *outbuf)
{
    return 0;
}

static void scsi_ses_emulate_read_data(SCSIRequest *req)
{
    SCSISESReq *r = DO_UPCAST(SCSISESReq, req, req);
    int buflen = r->iov.iov_len;

    if (buflen) {
        DPRINTF("Read buf_len=%d\n", buflen);
        r->iov.iov_len = 0;
        scsi_req_data(&r->req, buflen);
        return;
    }

    /* This also clears the sense buffer for REQUEST SENSE.  */
    scsi_req_complete(&r->req, GOOD);
}

static void scsi_ses_emulate_mode_select(SCSISESReq *r, uint8_t *inbuf)
{
}

static void scsi_ses_emulate_write_data(SCSIRequest *req)
{
    SCSISESReq *r = DO_UPCAST(SCSISESReq, req, req);

    if (r->iov.iov_len) {
        int buflen = r->iov.iov_len;
        DPRINTF("Write buf_len=%d\n", buflen);
        r->iov.iov_len = 0;
        scsi_req_data(&r->req, buflen);
        return;
    }

    switch (req->cmd.buf[0]) {
    case MODE_SELECT:
    case MODE_SELECT_10:
        /* This also clears the sense buffer for REQUEST SENSE.  */
        scsi_ses_emulate_mode_select(r, r->iov.iov_base);
        break;

    default:
        abort();
    }
}

static int32_t scsi_ses_emulate_command(SCSIRequest *req, uint8_t *buf)
{
    SCSISESReq *r = DO_UPCAST(SCSISESReq, req, req);
    SCSISESState *s = DO_UPCAST(SCSISESState, qdev, req->dev);
    uint8_t *outbuf = NULL;
    int buflen;
    struct SCSISense *senseCode = NULL;

    if (req->cmd.xfer > 65536) {
        goto illegal_request;
    }

    r->buflen = MAX(4096, req->cmd.xfer);

    if (!r->iov.iov_base) {
        r->iov.iov_base = blk_blockalign(s->qdev.conf.blk, r->buflen);
        buflen = req->cmd.xfer;
        outbuf = r->iov.iov_base;
        memset(outbuf, 0, r->buflen);
    }

    switch (req->cmd.buf[0]) {
    case TEST_UNIT_READY:
        break;
    case INQUIRY:
	buflen = scsi_ses_emulate_inquiry(req, outbuf);
        if (buflen < 0) {
            goto illegal_request;
        }
        break;
    case MODE_SENSE:
    case MODE_SENSE_10:
        buflen = scsi_ses_emulate_mode_sense(r, outbuf);
        if (buflen < 0) {
            goto illegal_request;
        }
        break;
    case REQUEST_SENSE:
        /* Just return "NO SENSE".  */
        buflen = scsi_build_sense(NULL, 0, outbuf, r->buflen,
                                  (req->cmd.buf[1] & 1) == 0);
        if (buflen < 0) {
            goto illegal_request;
        }
        break;
    case MODE_SELECT:
        DPRINTF("Mode Select(6) (len %lu)\n", (long)r->req.cmd.xfer);
        break;
    case MODE_SELECT_10:
        DPRINTF("Mode Select(10) (len %lu)\n", (long)r->req.cmd.xfer);
        break;
    case RECEIVE_DIAGNOSTIC:
	buflen = scsi_ses_emulate_recv_diag(req, outbuf, &senseCode);
        if (buflen < 0) {
            goto illegal_request;
        }
        break;
    case SEND_DIAGNOSTIC:
        break;
    default:
        DPRINTF("Unknown SCSI command (%2.2x=%s)\n", buf[0],
                scsi_command_name(buf[0]));
        scsi_check_condition(r, SENSE_CODE(INVALID_OPCODE));
        return 0;
    }
    assert(!r->req.aiocb);
    r->iov.iov_len = MIN(r->buflen, req->cmd.xfer);
    if (r->iov.iov_len == 0) {
        scsi_req_complete(&r->req, GOOD);
    }
    if (r->req.cmd.mode == SCSI_XFER_TO_DEV) {
        assert(r->iov.iov_len == req->cmd.xfer);
        return -r->iov.iov_len;
    } else {
        return r->iov.iov_len;
    }

illegal_request:
    if (senseCode != NULL) {
        scsi_check_condition(r, *senseCode);
    } else if (r->req.status == -1) {
        scsi_check_condition(r, SENSE_CODE(INVALID_FIELD));
    }

    return 0;
}

static void scsi_ses_reset(DeviceState *dev)
{
}

static void scsi_ses_realize(SCSIDevice *dev, Error **errp)
{
    SCSISESState *s = DO_UPCAST(SCSISESState, qdev, dev);

    s->qdev.type = TYPE_ENCLOSURE;
    if (!s->product) {
        s->product = g_strdup("Virtual SCSI Enclosure Device");
    }

    if (!s->version) {
        s->version = g_strdup(qemu_hw_version());
    }

    if (!s->vendor)  {
        s->vendor = g_strdup("QEMU");
    }

    /* initialize ESES, put here for current_machine is ready */
    config_page_init();

    /* initialize VPHY SAS Info */
    s->eses_sas_info = sas_virtual_phy_info_new(s->dae_type, s->qdev.wwn);

    // Create a thread to maintain eses sas info
    qemu_thread_create(&s->eses_sas_info_thread, "ESES thread", eses_sas_info_handler, s, QEMU_THREAD_DETACHED);

}

static uint8_t *scsi_get_buf(SCSIRequest *req)
{
    SCSISESReq *r = DO_UPCAST(SCSISESReq, req, req);

    return (uint8_t *)r->iov.iov_base;
}

static const SCSIReqOps scsi_ses_emulate_reqops = {
    .size         = sizeof(SCSISESReq),
    .free_req     = scsi_free_request,
    .send_command = scsi_ses_emulate_command,
    .read_data    = scsi_ses_emulate_read_data,
    .write_data   = scsi_ses_emulate_write_data,
    .get_buf      = scsi_get_buf,
};

static const SCSIReqOps *const scsi_ses_reqops_dispatch[256] = {
    [REQUEST_SENSE]                   = &scsi_ses_emulate_reqops,
    [REPORT_LUNS]                     = &scsi_ses_emulate_reqops,
    [TEST_UNIT_READY]                 = &scsi_ses_emulate_reqops,
    [INQUIRY]                         = &scsi_ses_emulate_reqops,
    [WRITE_BUFFER]                    = &scsi_ses_emulate_reqops,
    [READ_BUFFER]                     = &scsi_ses_emulate_reqops,
    [MODE_SENSE]                      = &scsi_ses_emulate_reqops,
    [MODE_SENSE_10]                   = &scsi_ses_emulate_reqops,
    [MODE_SELECT]                     = &scsi_ses_emulate_reqops,
    [MODE_SELECT_10]                  = &scsi_ses_emulate_reqops,
    [RECEIVE_DIAGNOSTIC]              = &scsi_ses_emulate_reqops,
    [SEND_DIAGNOSTIC]                 = &scsi_ses_emulate_reqops,
};

static SCSIRequest *scsi_new_request(SCSIDevice *d, uint32_t tag, uint32_t lun,
                                     uint8_t *buf, void *hba_private)
{
    SCSISESState *s = DO_UPCAST(SCSISESState, qdev, d);
    SCSIRequest *req;
    const SCSIReqOps *ops;
    uint8_t command;

    command = buf[0];
    ops = scsi_ses_reqops_dispatch[command];
    if (!ops) {
        ops = &scsi_ses_emulate_reqops;
    }
    req = scsi_req_alloc(ops, &s->qdev, tag, lun, hba_private);

#ifdef DEBUG_SCSI
    DPRINTF_TS("Command: lun=%d tag=0x%x data=%02X", lun, tag, buf[0]);
    {
        int i;
        for (i = 1; i < scsi_cdb_length(buf); i++) {
            DPRINTF(" %02X", buf[i]);
        }
        DPRINTF("\n");
    }
#endif

    return req;
}

#define DEFINE_SCSI_SES_PROPERTIES()                                \
    DEFINE_PROP_STRING("ver", SCSISESState, version),               \
    DEFINE_PROP_STRING("serial", SCSISESState, serial),             \
    DEFINE_PROP_STRING("vendor", SCSISESState, vendor),             \
    DEFINE_PROP_STRING("product", SCSISESState, product)            

static Property scsi_ses_properties[] = {
    DEFINE_SCSI_SES_PROPERTIES(),
    DEFINE_PROP_UINT8("dae_type", SCSISESState, dae_type, 0),
    DEFINE_PROP_UINT64("wwn", SCSISESState, qdev.wwn, 0),
    DEFINE_PROP_UINT64("port_wwn", SCSISESState, qdev.port_wwn, 0),
    DEFINE_PROP_UINT16("port_index", SCSISESState, port_index, 0),
    DEFINE_PROP_END_OF_LIST(),
};

static void scsi_ses_class_initfn(ObjectClass *klass, void *data)
{
    DeviceClass *dc = DEVICE_CLASS(klass);
    SCSIDeviceClass *sc = SCSI_DEVICE_CLASS(klass);

    sc->realize      = scsi_ses_realize;
    sc->alloc_req    = scsi_new_request;
    dc->fw_name = "ses";
    dc->desc = "Virtual SCSI Enclosure Service";
    dc->reset = scsi_ses_reset;
    dc->props = scsi_ses_properties;
}

static const TypeInfo scsi_ses_info = {
    .name          = "ses",
    .parent        = TYPE_SCSI_DEVICE,
    .instance_size = sizeof(SCSISESState),
    .class_init    = scsi_ses_class_initfn,
};

static void scsi_enclosure_register_types(void)
{
    type_register_static(&scsi_ses_info);
}

type_init(scsi_enclosure_register_types)
