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
#ifndef HCS_DISTRIBUTED_STORAGE_MGR_H
#define HCS_DISTRIBUTED_STORAGE_MGR_H

#include <string>
#include "define/Types.h"
#include "json/json.h"
#include "curl_http/HttpClientInterface.h"
#include "volume_handlers/oceanstor/ApiOperator.h"
#include "protect_engines/hcs/common/HcsMacros.h"
#include "volume_handlers/fusionstorage/client/FusionStorageClient.h"
#include "common/model/ResponseModel.h"
#include "common/client/RestClient.h"
#include "protect_engines/hcs/api/evs/EvsClient.h"
#include "protect_engines/hcs/common/HcsHttpStatus.h"

using namespace VirtPlugin;


namespace HcsPlugin {
    class DistributedStorageMgr {
    public:
        static int CheckDistributedConnection(const ControlDeviceInfo &info, std::string &errorStr);
        static int DoCheckDistributedConnection(GetFusionStorageRequest& request, AuthObj& storageAuth);
    };
}

#endif // HCS_DISTRIBUTED_STORAGE_MGR_H