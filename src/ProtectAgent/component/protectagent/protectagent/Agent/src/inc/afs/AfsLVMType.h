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
#ifndef __LVM_TYPE_H__
#define __LVM_TYPE_H__

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include <iostream>
#include <list>
#include <vector>

using namespace std;

// LVM签名长度
#define LVM_SIGLEN 8
// LVM幻数长度
#define LVM_MAGIC_LEN 8
// LVM的UUID长度
#define UUID_LEN 32
// " = "共3个字符
#define LVM_FIX_SPACE_FORMAT 3
// VG的UUID长度，包含两个双引号字符 38 + 2
#define VG_UUID_LEN (38 + 2)
// VG的序列号长度
#define VG_SEQNO_LEN 1
// VG的Format长度
#define VG_FORMAT_LEN 6
// VG的PE长度
#define PV_PE_START_LEN 11

#define LVM2_FORMAT "\"lvm2\""
// PV中ID长度
#define PV_ID_LEN 38
#endif /* INCLUDE_LVM_TYPE_H_ */
