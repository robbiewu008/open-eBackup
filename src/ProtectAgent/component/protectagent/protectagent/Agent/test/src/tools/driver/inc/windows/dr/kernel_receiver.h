#ifndef _KERNEL_RECEIVER_H
#define _KERNEL_RECEIVER_H

#include "net_worker.h"

VOID ReceiveAndProcessCmd(SocketInfo *socket_info, GroupInfo *group_info);

#endif