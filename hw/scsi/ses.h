#ifndef _SES_H
#define _SES_H

#include "qemu/osdep.h"
#include "qapi/error.h"
#include "qemu/error-report.h"
#include "hw/scsi/scsi.h"
#include "block/scsi.h"
#include "sysemu/sysemu.h"
#include "sysemu/block-backend.h"
#include "sysemu/blockdev.h"
#include "hw/block/block.h"
#include "sysemu/dma.h"
#include "qemu/cutils.h"

#define SCSI_MAX_INQUIRY_LEN        256
#define SCSI_MAX_MODE_LEN           256

typedef struct SCSISESState SCSISESState;

typedef struct SCSISESReq {
    SCSIRequest req;
    /* Both sector and sector_count are in terms of qemu 512 byte blocks.  */
    uint32_t buflen;
    struct iovec iov;
    QEMUIOVector qiov;
} SCSISESReq;

struct SCSISESState {
    SCSIDevice qdev;
    uint32_t features;
    uint16_t port_index;
    QEMUBH *bh;
    char *version;
    char *serial;
    char *vendor;
    char *product;
    uint8_t dae_type;
    QemuThread eses_sas_info_thread;
    void *eses_sas_info; // should be terminator_sas_virtual_phy_info_t
};

#endif /* _SES_H */
