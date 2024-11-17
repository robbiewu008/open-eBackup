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
#ifndef BSA_OBJ_MANAGER_H
#define BSA_OBJ_MANAGER_H

#include <mutex>
#include "common/Types.h"
#include "apps/dws/XBSAServer/DwsTaskCommonDef.h"

struct BsaQueryPageInfo {
    BsaQueryPageInfo() {}
    BsaQueryPageInfo(mp_uint32 _limit, mp_uint32 _offset) : limit(_limit), offset(_offset) {}

    mp_uint32 limit{0};
    mp_uint32 offset{0};
};

class BsaObjManager {
public:
    static BsaObjManager &GetInstance()
    {
        static BsaObjManager instance;
        return instance;
    }
    mp_int32 CreateObject(BsaObjInfo &objInfo);
    mp_int32 QueryObject(const BsaObjInfo &queryCond, const BsaQueryPageInfo &pageInfo,
        std::vector<BsaObjInfo> &result, mp_long bsaHandle);
    mp_int32 SaveObjects(const std::map<mp_uint64, BsaObjInfo> &objList, mp_long bsaHandle);

private:
    mp_int32 DwsQueryObject(const BsaObjInfo &queryCond, const BsaQueryPageInfo &pageInfo,
        std::vector<BsaObjInfo> &result, mp_long bsaHandle);
    mp_int32 HcsQueryObject(const BsaObjInfo &queryCond, const BsaQueryPageInfo &pageInfo,
        std::vector<BsaObjInfo> &result, mp_long bsaHandle);
    mp_int32 InformixQueryObject(const BsaObjInfo &queryCond, const BsaQueryPageInfo &pageInfo,
                                       std::vector<BsaObjInfo> &result, mp_long bsaHandle);

private:
    BsaObjManager() {};
    std::mutex m_mutexTrans; // DB事务锁
};

#endif // BSA_OBJ_MANAGER_H