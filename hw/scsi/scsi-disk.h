#ifndef _SCSI_DISK_H
#define _SCSI_DISK_H


#include "hw/qdev.h"
#include "hw/block/block.h"
#include "sysemu/sysemu.h"
#include "sysemu/block-backend.h"
#include "sysemu/blockdev.h"
#include "qemu/notify.h"
#include "qemu/osdep.h"
#include "qapi/error.h"
#include "hw/scsi/scsi.h"
#include "block/scsi.h"
#include "block/drive-defect.h"
#include "qapi-types.h"

typedef struct SCSIDiskReq {
    SCSIRequest req;
    /* Both sector and sector_count are in terms of qemu 512 byte blocks.  */
    uint64_t sector;
    uint32_t sector_count;
    uint32_t buflen;
    bool started;
    bool need_fua_emulation;
    struct iovec iov;
    QEMUIOVector qiov;
    BlockAcctCookie acct;
    unsigned char *status;
} SCSIDiskReq;

typedef struct SCSIStatusError
{
    ErrorType error_type; //BUSY CHECK_CONDITION ABORT ACA
    uint32_t count;
}SCSIStatusError;

typedef struct SCSIDiskState
{
    SCSIDevice qdev;
    uint32_t features;
    bool media_changed;
    bool media_event;
    bool eject_request;
    uint16_t port_index;
    uint64_t max_unmap_size;
    uint64_t max_io_size;
    QEMUBH *bh;
    char *version;
    char *serial;
    char *vendor;
    char *product;
    bool tray_open;
    bool tray_locked;
    uint32_t rotation;
    uint64_t format_time_emulation;
    int progress;
    bool format_in_progress;
    char *page_file;
    uint8_t* page_buffer;
    struct DriveDefectDescriptor drive_defect_desc;
    uint8_t  attached_phy_id;
    uint64_t attached_wwn;
    uint8_t* log_page;
    SCSIStatusError scsi_status_error;
} SCSIDiskState;

SCSIDiskState * get_drive_peer_port(SCSIDiskState *s);
void dispatch_error_inject_request(uint8_t *s_log_page, const char *type, ActionMode action, bool has_parameter, uint16_t parameter, bool has_parameter_length, uint8_t parameter_length, bool has_val, uint64_t val, Error **error);
uint8_t *prepare_log_sense_page(SCSIDiskState *s);
int get_page_data(uint8_t *s_log_page, uint8_t *buffer, uint8_t page_code, uint8_t subpage_code, uint16_t parameter_pointer);
#endif
