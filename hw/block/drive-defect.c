#include "qemu/osdep.h"
#include "qemu/module.h"
#include "block/drive-defect.h"

static const TypeInfo drive_defect_handler_info = {
    .name          = TYPE_DRIVE_DEFECT_HANDLER,
    .parent        = TYPE_INTERFACE,
    .class_size    = sizeof(DriveDefectHandlerClass),
};

static void drive_defect_handler_register_types(void)
{
    type_register_static(&drive_defect_handler_info);
}

type_init(drive_defect_handler_register_types)

