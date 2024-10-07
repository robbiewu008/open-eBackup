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
#ifndef __MODULE_NDMPD_H
#define __MODULE_NDMPD_H

#define NDMPD_DEFAULT_MIN_PORT 10100
#define NDMPD_DEFALUT_MAX_PORT 10163
#define NDMPD_DATA_PORT_MAX_NUM 64
#define NDMPD_DATA_MAX_FILE_PATH 1024
#define NDMPD_DATA_DEFAULT_BUFFER_SIZE 4096
#define NDMPD_DATA_AGGR_BUFFER_SIZE 1024 *1024
#define NDMP_CACHE_NODE_DEFAULE_INFO_LINE_SIZE 4090
#define NDMP_CACHE_DEFAULE_INFO_LINE_SIZE 1050
#define NDMPD_DEFAULT_RESTORE_FILE_NUM 20
#define DEFAULT_DATA_SOCKET_BUFFER_SZIE  2048 * 1024 
#define DEFAULT_DATA_SOCKET_TIMEOUT_VALUE 900 //15min
#define NDMPD_DEFAULT_SOCKET_TIMEOUT_SECONDS 10
#define NDMPD_DEFAULT_FULL_BACKUP_FILE_NAME "fs_full_back.file"
#define NDMPD_DEFAULT_BACKUP_PROCESS_MSG_FILE_NAME "BACKUP_LOG_MESSAGE"
#define NDMPD_DEFAULT_RESTORE_PROCESS_MSG_FILE_NAME "RESTORE_LOG_MESSAGE"
#define NDMPD_DEFAULT_RESTORE_FILE_TMP "restore_files.tmp"

typedef struct NdmpDataConnection {
    int conn_sock;
    int listen_sock;
    unsigned short port;
    char *backFilePath;
    char *fspath;
    char *dstPath;
    int level;
} NdmpDataConnection_t;

extern NdmpDataConnection_t g_dataConnection;

void* NdmpStartDataReceive(void* args);
void* NdmpStartRestoreData(void* args);
int NdmpClientCreateDataIpv4Listener(char *srcIp, unsigned short *port, int *fd, unsigned long *addr);
int NdmpClientCreateDataIpv6Listener(char *srcIp, unsigned short *port, int *fd, char *addr);
int NdmpCLientCreateDataConnect(unsigned short port, int fd);
#endif