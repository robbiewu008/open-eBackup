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
#ifndef DATACONVERSION_H_
#define DATACONVERSION_H_

#include <iostream>
#include <string>
#include "xbsa/xbsa.h"
#include "xbsaclientcomm/BSAService.h"

class DataConversion {
public:

    DataConversion();
    ~DataConversion();

    static int CopyStrToChar(const std::string &src, char *dst, uint32_t dstSize); // string -> char*

    static void ConvertStrToTime(const std::string &src, struct tm &dst);

    static void ConvertObjectDescriptorIn(BSA_ObjectDescriptor *src, BsaObjectDescriptor &dst);

    static bool ConvertObjectDescriptorOut(BsaObjectDescriptor &src, BSA_ObjectDescriptor *dst);

    static void ConvertdataBlockOut(BsaDataBlock32 &src, BSA_DataBlock32 *dst);

    static void ConvertQueryObjectIn(BSA_QueryDescriptor *src, BsaQueryDescriptor &dst);

    static void ConvertdataBlockIn(BSA_DataBlock32 *src, BsaDataBlock32 &dst);

    static void U64ToBsaU64(unsigned long long u64, BsaUInt64 &b64);
};

#endif