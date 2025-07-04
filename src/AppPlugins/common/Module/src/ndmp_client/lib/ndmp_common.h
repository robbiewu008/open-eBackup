/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef _ndmp_common_h
#define _ndmp_common_h

#include "ndmp.h"
#include "log/Log.h"

#ifdef __linux__
typedef long long          longlong_t;
typedef unsigned long long u_longlong_t;
#define USE_SELECT
#endif

#define XDR_AND_SIZE(func) (bool_t(*)(XDR*, ...))xdr_##func, sizeof(func)
#define AUTH_REQUIRED		TRUE
#define AUTH_NOT_REQUIRED	FALSE
#define MAX_RETRY_CNT 5  // 文件打开失败重试次数


// ndmpcopy
#define NDMPCOPY_SUCCESS                   0
#define NDMPCOPY_OTHER_ERR                 -1
#define NDMPCOPY_USERINFO_ERR              -2
#define NDMPCOPY_CONNECT_ERR               -3
#define NDMPCOPY_LOGIC_IP_NOT_EXIST        -4
#define NDMPCOPY_TASK_DELETED              -5
#define NDMPCOPY_FS_PARAMS                 -6
#define NDMPCOPY_CCDB_TIMEOUT             -7

#define UNITY_OS "Unity"
#define DORADO_OS "Huawei"
#define NETAPP_OS "NetApp"
#define ISILON_OS "Isilon"

typedef void* NdmpConnection;

typedef void NdmpConHandlerFunc(NdmpConnection);
	
typedef void NdmpMsgHandlerFunc(NdmpConnection	connection, void* request);

static pthread_mutex_t      g_dataMutex = PTHREAD_MUTEX_INITIALIZER;

void writeProcessMsg(char *msg);
typedef struct NdmpMsgHandler
{
    ndmp_message        message;
    u_short             version;
    NdmpMsgHandlerFunc* func;
    bool_t              auth_required;
    bool_t              (*xdr_request)(XDR* xdrs, ...);
    int                 sizeof_request;
    bool_t              (*xdr_reply)(XDR* xdrs, ...);
    int                 sizeof_reply;
} NdmpMsgHandler;

typedef enum NdmpStatus
{
    NDMP_STATUS_INIT = 1, 						/* 初始状态 */
    NDMP_STATUS_PROCESSING = 2, 				/* 备份/恢复进行中 */
    NDMP_STATUS_ABORTED = 3, 					/* 备份/恢复中断 */
    NDMP_STATUS_FINISHED = 4,					/* 备份/恢复任务上报成功 */
    NDMP_STATUS_CONNECTION_HALTED = 5,			/* 备份/恢复任务异常结束 */
	NDMP_STATUS_COMPLETED = 6,					/* 备份/恢复任务完成 */
    NDMP_STATUS_INTERNAL_ERROR = -1				/* 内部错误 */
} NdmpStatus;

NdmpConnection ndmpCreateConnection(NdmpMsgHandler* msgHandlerTbl);

void ndmpDestroyConnection(NdmpConnection connection);

void ndmpClose(NdmpConnection connectionHandle);

int ndmpConnect(NdmpConnection connectionHandle, char* host, u_long port);

int ndmpPoll(NdmpConnection	connectionHandle, bool_t block);

void ndmpRun(u_long	port, bool_t dofork, NdmpMsgHandler* msgHandlerTbl, NdmpConHandlerFunc* conHandlerFunc);

int ndmpProcessRequests(NdmpConnection connectionHandle);

int ndmpSendReply(NdmpConnection connectionHandle, ndmp_error err, void* data);

int ndmpSendRequest(NdmpConnection connectionHandle, ndmp_message message,
    ndmp_error err, void* request_data, void** reply_data);

void ndmpFreeMessage(NdmpConnection	connectionHandle);

int ndmpGetFd(NdmpConnection connectionHandle);

void ndmpSetClientData(NdmpConnection connectionHandle, void* client_data);

void* ndmpGetClientData(NdmpConnection connectionHandle);

void ndmpSetVersion(NdmpConnection connectionHandle, u_short version);

u_short ndmpGetVersion(NdmpConnection connectionHandle);

void ndmpSetAuthorized(NdmpConnection connectionHandle, bool_t authorized);

u_longlong_t quadToLongLong(ndmp_u_quad	quad);

ndmp_u_quad longLongToQuad(u_longlong_t	ull);

int check_connect_socket(NdmpConnection connectionHandle);

void set_connect_socket_valid(NdmpConnection connectionHandle);

void removeLifIpConn();

void intSaveLifIp();

int NdmpProcessFhAddDir(void *body);

int NdmpProcessFhAddNode(void *body);

int NdmpProcessFhAddFile(void *body);

void NdmpProcessInterrupt();

void set_ndmp_status(NdmpStatus status);

void ndmp_get_connect_stat(NdmpConnection *ndmp_connection, bool is_src);

#endif /* _ndmp_common_h */
