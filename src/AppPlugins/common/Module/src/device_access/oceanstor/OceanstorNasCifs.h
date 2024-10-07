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
#ifndef OCEANSTOR_NAS_CIFS_H
#define OCEANSTOR_NAS_CIFS_H

#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "device_access/Const.h"
#include "device_access/oceanstor/OceanstorNas.h"

namespace Module {
    enum DOMAINTYPE_ENUM {
        AD_USER = 0, LDAP_USER = 1, LOCAL_USER = 2, NIS_USER = 3
    };
    enum PERMISSION_ENUM {
        READ_ONLY = 0, FULL_CONTROL = 1, FORBIDDEN = 2, READ_AND_WRITE = 3
    };

    class OceanstorNasCIFS : public OceanstorNas {
    public:
        explicit OceanstorNasCIFS(ControlDeviceInfo deviceInfo) : OceanstorNas(deviceInfo) {}

        OceanstorNasCIFS(ControlDeviceInfo deviceInfo, std::string fsId) : OceanstorNas(deviceInfo, fsId) {}

        OceanstorNasCIFS(ControlDeviceInfo deviceInfo, std::string id, std::string path)
                : OceanstorNas(deviceInfo) {
            fileSystemId = id;
            uniquePath = path;
        }

        virtual ~OceanstorNasCIFS();

        int Bind(HostInfo &host, const std::string &shareId = "") override;

        int UnBind(HostInfo host, const std::string &shareId = "") override;

        int Query(DeviceDetails &info) override;

        int DeleteWindowsUser(std::string userName);

        int CreateShare();

        int Delete() override;

        std::unique_ptr <ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

    protected:
        int QueryFileSystem(DeviceDetails &info) override;

        int CIFSShareAddClient(std::string name, int ID, const std::string &domainName = "");

        int DeleteCIFSShare(DeviceDetails info);

        int CreateCifsShare(std::string fileSystemName, std::string FsId);

        int QueryCifsShare(DeviceDetails &info, std::string fsId);

        int CreateWindowUser(std::string userName, std::string password);

        std::unique_ptr <ControlDevice> CreateClone(std::string volumeName, int &errorCode);

        std::string uniquePath;

        int GetFsNameFromShareName();

        std::string fileSystemName{};
    };
}
#endif