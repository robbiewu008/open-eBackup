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
#ifndef COMMON_DEFS_H
#define COMMON_DEFS_H


#define BLOCK_SIZE 4194304
#define BLOCK_SIZE_MB 4
#define BLOCK_SIZE_KB 4096
#define HCP_FULL_SNAPSHOT   "HCP_FULL_SNAPSHOT"
#define MB_SIZE 1048576

#define INVALID_PARENT_INDEX 0xFFFFFFFFFFFFFFFF //stands for NULL in tree table

#define NONE_CERT_STRING "none"
#define AUTO_MATCH_CERT_STRING "auto_match"
#define FORCE_AUTO_MATCH_CERT_STRING "force_auto_match"
#define INVALID_CERT_STRING "invalid_cert"
#define STORAGE_UNIT_S3_HTTPS_PORT "443"
#define LOCAL_IP "127.0.0.1"

#define DEFAULT_CUR_PATH "/opt/huawei-data-protection/ebackup/bin"
#define BRICK_PLACE_HOLDER_FILE_NAME "DummyFileForDisasteryRecovery.tmp"

#define SQLITE_S3_TEMP_TILE  "127.0.0.1:/SQLiteTmpFile.txt"

//add by zhangyoupeng 2016-4-14
#define TLV_VERSION_V1 "1.0"
#define TLV_VERSION_V2 "2.0"
#define TLV_VERSION_V2_1 "2.1"
#define TLV_VERSION_V2_2 "2.2"
#define TLV_VERSION_V2_3 "2.3"
//end add by zhangyoupeng 2016-4-14

#endif /*COMMON_DEFS_H*/

