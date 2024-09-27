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
#ifndef DEVICE_MANAGER_H
#define DEVICE_MANAGER_H
#include "define/Types.h"
#include <vector>
#include <memory>
#include "device_access/netapp/NetAppNas.h"
#include "device_access/netapp/NetAppNasNFS.h"
#include "device_access/netapp/NetAppNasCifs.h"
#include "device_access/netapp/NetAppNasSnapshot.h"
#include "device_access/dorado/DoradoBlock.h"
#include "device_access/dorado/DoradoBlockSnapshot.h"
#include "device_access/dorado/DoradoNas.h"
#include "device_access/dorado/DoradoNasSnapshot.h"
#include "device_access/fusionstorage/FusionStorageBlock.h"
#include "device_access/fusionstorage/FSBlockSnapshot.h"
#include "device_access/dorado/DoradoNasNFS.h"
#include "device_access/dorado/DoradoNasCifs.h"
#include "device_access/oceanstor/OceanstorNas.h"
#include "device_access/oceanstor/OceanstorNasSnapshot.h"
#include "device_access/oceanstor/OceanstorNasNFS.h"
#include "device_access/oceanstor/OceanstorNasCifs.h"
#include "device_access/fusionstorage/FusionStorageNas.h"
#include "device_access/fusionstorage/FusionStorageNasNFS.h"
#include "device_access/fusionstorage/FusionStorageNasCifs.h"
#include "device_access/fusionstorage/FSNasSnapshot.h"

namespace Module {
    class DeviceManager {
    public:
        static std::unique_ptr<ControlDevice> CreateDeviceInst(ControlDeviceInfo info, STORAGE_ENUM type, PROTOCOL pro,
                                                               bool readFromK8s = true) {
            if (type == FUSIONSTORAGE) {
                return CreateFusionstorDeviceInst(info, pro);
            } else if (type == DORADO) {
                return CreateDoradoDeviceInst(info, pro, readFromK8s);
            } else if (type == OCEANSTOR) {
                return CreateOceanstorDeviceInst(info, pro);
            } else if (type == NETAPP) {
                return CreateNetAppDeviceInst(info, pro);
            } else {
                return nullptr;
            }
        }

        static std::unique_ptr<ControlDevice> CreateFusionstorDeviceInst(ControlDeviceInfo info, PROTOCOL pro) {
#ifndef JOBMANAGER_COMPILE
            if (pro == SAN) {
                return std::make_unique<FusionStorageBlock>(info);
            } else if (pro == NAS) {
                return std::make_unique<FusionStorageNas>(info);
            } else if (pro == NFS) {
                return std::make_unique<FusionStorageNasNFS>(info);
            } else if (pro == CIFS) {
                return std::make_unique<FusionStorageNasCIFS>(info);
            } else {
                return nullptr;
            }
#endif
        }

        static std::unique_ptr<ControlDevice> CreateDoradoDeviceInst(ControlDeviceInfo info, PROTOCOL pro,
                                                                     bool readFromK8s = true) {
            if (pro == SAN) {
                return std::make_unique<DoradoBlock>(info, readFromK8s);
            } else if (pro == NAS) {
                return std::make_unique<DoradoNas>(info, readFromK8s);
            } else if (pro == NFS) {
                return std::make_unique<DoradoNasNFS>(info, readFromK8s);
            } else if (pro == CIFS) {
                return std::make_unique<DoradoNasCIFS>(info, readFromK8s);
            } else {
                return nullptr;
            }
        }

        static std::unique_ptr<ControlDevice> CreateOceanstorDeviceInst(ControlDeviceInfo info, PROTOCOL pro) {
            if (pro == NAS) {
                return std::make_unique<OceanstorNas>(info);
            } else if (pro == NFS) {
                return std::make_unique<OceanstorNasNFS>(info);
            } else if (pro == CIFS) {
                return std::make_unique<OceanstorNasCIFS>(info);
            } else {
                return nullptr;
            }
        }

        static std::unique_ptr<ControlDevice> CreateNetAppDeviceInst(ControlDeviceInfo info, PROTOCOL pro) {
            if (pro == SAN) {
                return nullptr;
            } else if (pro == NAS) {
                return std::make_unique<NetAppNas>(info);
            } else if (pro == NFS) {
                return std::make_unique<NetAppNasNFS>(info);
            } else if (pro == CIFS) {
                return std::make_unique<NetAppNasCIFS>(info);
            } else {
                return nullptr;
            }
        }

        static std::unique_ptr<ControlDevice> CreateSnapshotDeviceInst(ControlDeviceInfo info, STORAGE_ENUM type,
                                                                       PROTOCOL pro, int id, std::string uniquePath) {
            if (type == FUSIONSTORAGE) {
                return CreateFusionstorSnapshotDevInst(info, pro, id, uniquePath);
            } else if (type == DORADO) {
                return CreateDoradoSnapshotDevInst(info, pro, id, uniquePath);
            } else if (type == OCEANSTOR) {
                return CreateOceanstorSnapshotDevInst(info, pro, id, uniquePath);
            } else if (type == NETAPP) {
                return CreateNetAppSnapshotDeviInst(info, pro, id, uniquePath);
            } else {
                return nullptr;
            }
        }

        static std::unique_ptr<ControlDevice> CreateFusionstorSnapshotDevInst(ControlDeviceInfo info, PROTOCOL pro,
                                                                              int id, std::string uniquePath) {
            if (pro == SAN) {
                return std::make_unique<FSBlockSnapshot>(info, id, uniquePath);
            } else if (pro == NAS) {
                return nullptr;
            } else {
                return nullptr;
            }
        }

        static std::unique_ptr<ControlDevice> CreateDoradoSnapshotDevInst(ControlDeviceInfo info, PROTOCOL pro,
                                                                          int id, std::string uniquePath) {
            if (pro == SAN) {
                return std::make_unique<DoradoBlockSnapshot>(info, id, uniquePath);
            } else if (pro == NAS) {
                return std::make_unique<DoradoNasSnapshot>(info, std::to_string(id), uniquePath);
            } else {
                return nullptr;
            }
        }

        static std::unique_ptr<ControlDevice> CreateOceanstorSnapshotDevInst(ControlDeviceInfo info, PROTOCOL pro,
                                                                             int id, std::string uniquePath) {
            if (pro == NAS) {
                return std::make_unique<OceanstorNasSnapshot>(info, std::to_string(id), uniquePath);
            } else {
                return nullptr;
            }
        }

        static std::unique_ptr<ControlDevice> CreateNetAppSnapshotDeviInst(ControlDeviceInfo info, PROTOCOL pro,
                                                                           int id, std::string uniquePath) {
            if (pro == NAS) {
                return std::make_unique<NetAppNasSnapshot>(info, std::to_string(id), uniquePath);
            } else {
                return nullptr;
            }
        }

        static void DestoryDeviceInst(std::shared_ptr<ControlDevice> dev) {
            if (dev != nullptr) {
                dev->Clean();
            }
        }
    };
}
#endif  // DEVICE_MANAGER_H
