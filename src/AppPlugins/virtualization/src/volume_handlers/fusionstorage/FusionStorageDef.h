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
#ifndef FUSIONSTORAGE_DEF_H
#define FUSIONSTORAGE_DEF_H
#include <string>

namespace FusionStorageDef {
const std::string DSWARE_MGR_IP = "IP";
const std::string DSWARE_POOL_ID = "pool_id";
const std::string SNAPSHOT_VOLUMES_ATTACHED = "snapshot_volumes_attached";
const std::string BITMAP_VOLUMES_CREATED = "bitmap_volumes_created";
const std::string BITMAP_VOLUMES_ATTACHED = "bitmap_volumes_attached";
const std::string DATA_VOLUMES_ATTACHED = "data_volumes_attached";
const std::string CACHE_UUID = "cache_uuid";
}
#endif  // FUSIONSTORAGE_DEF_H