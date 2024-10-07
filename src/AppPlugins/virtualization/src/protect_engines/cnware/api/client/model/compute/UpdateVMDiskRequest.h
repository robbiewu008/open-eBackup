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
#ifndef __UPDATE_DOMAIN_DISK_REQUEST__
#define __UPDATE_DOMAIN_DISK_REQUEST__

#include <string>
#include "protect_engines/cnware/common/Structs.h"
#include "../CNwareRequest.h"

namespace CNwarePlugin {
struct UpdateDomainDiskRequest {
    int32_t bus;
    std::string busDev;
    int32_t cache;
    std::string id;
    int64_t ioHangTimeout = 1800000;
    int64_t readBytesSecond = 0;
    int64_t readIops = 0;
    bool shareable = false;
    int64_t writeBytesSecond = 0;
    int64_t writeIops = 0;

    BEGIN_SERIAL_MEMEBER
    SERIAL_MEMEBER(bus)
    SERIAL_MEMEBER(busDev)
    SERIAL_MEMEBER(cache)
    SERIAL_MEMEBER(id)
    SERIAL_MEMEBER(ioHangTimeout)
    SERIAL_MEMEBER(readBytesSecond)
    SERIAL_MEMEBER(readIops)
    SERIAL_MEMEBER(shareable)
    SERIAL_MEMEBER(writeBytesSecond)
    SERIAL_MEMEBER(writeIops)
    END_SERIAL_MEMEBER
};

class UpdateVMDiskRequest : public CNwareRequest {
public:
    UpdateVMDiskRequest() {}
    ~UpdateVMDiskRequest() {}

    void SetDomainDiskMeta(const UpdateDomainDiskRequest &disk)
    {
        m_disk = disk;
    }

    UpdateDomainDiskRequest GetDomainDiskMeta()
    {
        return m_disk;
    }

    void SetBusDev(const std::string &busDev)
    {
        m_busDev = busDev;
    }

    std::string GetBusDev()
    {
        return m_busDev;
    }

    int32_t BuildRequestBodyString()
    {
        if (!Module::JsonHelper::StructToJsonString(m_disk, m_body)) {
            ERRLOG("Convert snapinfo to json string failed!");
            return VirtPlugin::FAILED;
        }
        DBGLOG("Updata disk metadata: %s", WIPE_SENSITIVE(m_body).c_str());
        return VirtPlugin::SUCCESS;
    }
private:
    UpdateDomainDiskRequest m_disk;
    std::string m_busDev;
};
};
#endif