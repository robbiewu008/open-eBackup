#ifndef _NET_WORKER_H
#define _NET_WORKER_H

#include "group_mem.h"

VOID CreateSocket(SocketInfo *socket_info, GroupInfo *group_info);

NTSTATUS ImSendSync(GroupInfo *group_info, SocketInfo *socket_info, IOMirrorCmd *cmd, PVOID free_data);
NTSTATUS ImSendBufferSync(SocketInfo *socket_info, PVOID buffer, uint32_t size);

NTSTATUS ImReceiveSync(SocketInfo *socket_info, char *data, uint32_t size);
NTSTATUS ImReceiveAsync(SocketInfo *socket_info);
NTSTATUS ImReceiveAsyncEx(SocketInfo *socket_info);
NTSTATUS ImReceive(SocketInfo *socket_info);

VOID ProcessErrorState(GroupInfo *group_info);
VOID ProcessDisconnect(GroupInfo *group_info);

#endif