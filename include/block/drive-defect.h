/*
 * Disk defect intreface. 
 *
 * Authors:
 *
 * Chen, Conghui <conghui.chen@emc.com>
 * Gu, Forrest <forrest.gu@emc.com>
 *
 */
#ifndef DRIVE_DEFECT_H
#define DRIVE_DEFECT_H

#include "qom/object.h"
#include "qmp-commands.h"
#include "qapi/error.h"

#define TYPE_DRIVE_DEFECT_HANDLER "drive-defect-handler"

#define DRIVE_DEFECT_HANDLER_CLASS(klass) \
     OBJECT_CLASS_CHECK(DriveDefectHandlerClass, (klass), TYPE_DRIVE_DEFECT_HANDLER)
#define DRIVE_DEFECT_HANDLER_GET_CLASS(obj) \
     OBJECT_GET_CLASS(DriveDefectHandlerClass, (obj), TYPE_DRIVE_DEFECT_HANDLER)
#define DRIVE_DEFECT_HANDLER(obj) \
     INTERFACE_CHECK(DriveDefectHandler, (obj), TYPE_DRIVE_DEFECT_HANDLER)

struct DriveDefectDescriptor
{
    uint32_t glist_size;
    DriveDefect *glist;
    uint32_t plist_size;
    DriveDefect *plist;
};

typedef struct DriveDefectHandler {
    /* <private> */
    Object Parent;
} DriveDefectHandler;

/**
 * DriveDefectHandlerClass:
 *
 * Interface to be implemented by a device performing
 * drive defect operation functions.
 *
 * @parent: Opaque parent interface.
 * @get_drive_defect: get drive defect callback.
 * @set_drive_defect: set drive defect callback.
 */
typedef struct DriveDefectHandlerClass {
    /* <private> */
    InterfaceClass parent;

    /* <public> */
    DriveDefectList* (*get_drive_defect)(DeviceState *dev, const char *type, Error **errp);
    void (*set_drive_defect)(DeviceState *dev, const char *type, uint32_t defect_count, Error **errp);
} DriveDefectHandlerClass;

#define ADDR_FMT_SHORT  0b000
#define ADDR_FMT_LONG   0b011
#define ADDR_FMT_CHO    0b100
#define ADDR_FMT_CHS    0b101
#define ADDR_FMT_VENDOR 0b110
#define ADDR_LENGTH(fmt) ((fmt)?8:4)

#endif
