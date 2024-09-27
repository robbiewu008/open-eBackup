#ifndef _KERNEL_SENDER_H
#define _KERNEL_SENDER_H

#include "net_worker.h"

NTSTATUS CreateSocketThread(PDEVICE_EXTENSION pdx);
BOOLEAN DestroySocketThread(GroupInfo *group_info);
BOOLEAN ShutdownBitmapPrepare(GroupInfo *group_info);


#endif