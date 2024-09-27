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
#ifndef __MODULE_INTERFACE_H__
#define __MODULE_INTERFACE_H__

#include <stdio.h>
#include <errno.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {

typedef struct NdmpClientInterface_S {
    char *srcAuth;
    char *dstAuth;
    char *srcIp;
    char *dstIp;
    char *level; // 1 digit num
    char *srcPwd;
    char *dstPwd;
    char *srcPath;
    char *dstPath;
    char *backFilePath;
    char *exclude;
    int dumpType;
    int port;
} NdmpClientInterface;

typedef struct FSInfo {
    uint32_t invalid;
    char *fs_type;
    char *fs_logical_device;
    char *fs_physical_device;
    char *fs_status;
} FSInfo;

typedef struct NdmpGetFsInfo {
    int error;
    uint32_t fs_info_len;
    FSInfo *fs_info_val;
} NdmpFsInfo;

typedef struct NdmpGetStat {
    int status; // 0 - init connect, 1 - processing, 3 - abort, 4 - finish, 5 - server error <0 - client error
    uint64_t process_bytes;
    uint64_t remain_bytes;
    uint64_t files_cnt;
} NdmpStat;

typedef struct NdmpDirs {
    unsigned int pathLen;
    char *path;
    unsigned long long node;
    unsigned long long parent;
} NdmpDirs;
typedef struct NdmpGetFhAddDir {
    unsigned int dir_len;
    NdmpDirs *dirs;
} NdmpGetFhAddDir;
 
typedef struct NdmpFileStats {
    unsigned long long node;
    unsigned long long FhInfo;    
    unsigned int pathLen;
    char *path;
    unsigned long invalid;
    ndmp_fs_type fsType;
    ndmp_file_type ftype;
    unsigned long mtime;
    unsigned long atime;
    unsigned long ctime;
    unsigned long owner;
    unsigned long group;
    unsigned long fattr;
    unsigned long long size;
    unsigned long links;    
} NdmpFileStats;
 
typedef struct NdmpGetFhAddNode {
    int error;
    unsigned int files_len;
    NdmpFileStats *files;
} NdmpGetFhAddNode;

/* use to init ndmp client */
extern int NdmpClientAuth(NdmpClientInterface *interface);
/* use to init scanner log */
extern void InitLog(const char* fullLogPath, int logLevel, const char* rootPath = "");
/* get fs info, all filesystem with logicIp(associate with tenentId) */
extern int NdmpGetFsInfo(NdmpFsInfo* reply);
/* start backup*/
extern int NdmpStartBackup(NdmpClientInterface *interface);
/* start restore */
extern int NdmpStartRestore(NdmpClientInterface *interface);
/* abort backup*/
extern int NdmpAbortBackup(NdmpClientInterface *interface);
/* get stat */
extern NdmpStat NdmpGetStatFunc();
/* destroy ndmp resource*/
extern void NdmpDestroy();
}
#endif
#endif
