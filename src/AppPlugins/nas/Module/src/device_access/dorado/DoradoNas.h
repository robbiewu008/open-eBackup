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
#ifndef DORADO_NAS_H
#define DORADO_NAS_H

#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/CurlHttpClient.h"
#include "device_access/Const.h"
#include "device_access/dorado/DoradoBlock.h"

namespace Module {
    enum ALLOCTYPE {
        Thick = 0, Thin = 1
    };

    enum DTREEPARENTTYPE {
        DTREEFS = 40
    };
    enum DISTALG {
        PERFORMANCE_MODE = 0,
        CAPACITY_BALANCE_MODE,
        DIRECTORY_BALANCE_MODE,
        DIRECTORY_SHUFFLE_MODE,
        CONTROLLER_AFFINITY_MODE
    };

    class DoradoNas : public DoradoBlock {
    public:
        explicit DoradoNas(ControlDeviceInfo deviceInfo, bool readFromK8s = true) : DoradoBlock(deviceInfo,
                                                                                                readFromK8s) {
            fileSystemId = "";
            readK8s = readFromK8s;
        }

        DoradoNas(ControlDeviceInfo deviceInfo, std::string fsId, bool readFromK8s = true)
                : DoradoBlock(deviceInfo, readFromK8s),
                  fileSystemId(fsId),
                  readK8s(readFromK8s) {}

        virtual ~DoradoNas();

        int Bind(HostInfo &host, const std::string &shareId = "") override;

        int UnBind(HostInfo host, const std::string &shareId = "") override;

        int Create(unsigned long long size) override;

        int Delete() override;

        int Query(DeviceDetails &info) override;

        std::unique_ptr<ControlDevice> CreateSnapshot(std::string snapshotName, int &errorCode) override;

        std::unique_ptr<ControlDevice> CreateClone(std::string cloneName, int &errorCode) override;

        int QueryServiceHost(std::vector<std::string> &ipList, IP_TYPE ipType = IP_TYPE::IP_V4) override;

        int QueryServiceHost(std::vector<LogicPortInfo> &logicPorts, IP_TYPE ipType);

        int QueryServiceIpController(
                std::vector<std::pair<std::string, std::string>> &ipControllerList,
                IP_TYPE ipType = IP_TYPE::IP_V4) override;

        int ExtendSize(unsigned long long size) override;

        int QuerySnapshotList(std::vector<FSSnapshotInfo> &snapshots) override;

        int Revert(std::string SnapshotName) override;

        int QueryRevertInfo(const std::string &resourceName, std::string &rollbackRate,
                                 std::string &rollbackStatus) override;

        int QueryRevertSnapshotId(const std::string &resourceName, std::string &snapshotId, std::string &rollbackRate,
                                       std::string &rollbackStatus);

        int QuerySnapshotRollBackStatus(const std::string &fileSystemId, std::string &snapshotId,
            std::string &rollbackRate, std::string &rollbackStatus, std::string &endTime);

        int QuerySnapshotRollBackStatusV2(const std::string &fileSystemId, std::string &snapshotId,
            std::string &rollbackRate, std::string &rollbackStatus, std::string &endTime);

        int QuerySnapshotRollBackStatusV3(const std::string &fileSystemId, const std::string &snapshotName,
            std::string &rollbackStatus, std::string &endTime);

        int QuerySnapshotRollBackStatusV4(const std::string &fileSystemId, std::string &snapshotId,
            std::string &rollbackStatus, std::string &endTime);

        int SwitchOverPrimaryAndSecondResource(const std::string &pairId);

        int SecondaryResourceProtectEnable(const std::string &pairId);

        int SecondaryResourceProtectDisable(const std::string &pairId);

        int CheckReplicationPair(int lunId, int &rfsId, std::string devId, std::string &pairId);

        int CreateReplication(
                int fsId, int &rfsId, std::string rDevId, int bandwidth, std::string &pairId);

        int DeleteRemoteDevice(std::string devicdID, std::string &errDes);

        int QueryRemoteDevice(std::string devicdID, std::string &localGroupId, std::string &remoteGroupId,
                                   std::string &remoteESN);

        int BatchQueryRemoteDevice(std::string esn, std::string &devicdID);

        int
        GetLifPort(std::vector<std::string> &ownCtlIP, std::vector<std::string> &otherCtlIP, IP_TYPE ipType) override;

        int QueryContorllerCnt(int &outCnt) override;

        int BatchQueryRemoteDevice(std::string &devicdID);

        int BatchQueryRemoteDevice(std::string esn, std::string &devicdID, int &healthStatus,
                                        int &runStatus);

        int RelinkRemoteDevice(std::string localPort, std::string remoteIP, std::string remoteUser,
                                    std::string remotePassWord, std::string deviceID);

        int BatchQueryAllRemoteDevice(std::vector<std::string> &devicdIDList);

        int AddReplicationRepportgroupMember(std::string groupId, const std::vector<std::string> &iscsiList);

        int RemoveReplicationRepportgroupMember(std::string groupId, const std::vector<std::string> &iscsiList);

        int QueryReplicationPortIpInfo(std::vector<LogicalPorts> &ipList);

        int QueryReplicationRepportgroup(const std::string &groupName, std::string &groupId);

        int CreateReplicationRepportgroup(
                const std::vector<std::string> &iscsiList, const std::string &groupName, std::string &groupId);

        int CreateShare() override;

        int GetUsablePoolID(unsigned long long size, std::string &poolId);

        int
        CreateSnapshotWithSnapTag(const std::string &snapshotName, const std::string &snapTag, std::string &snapshotId);

        int CreateSnapshotWithSnapTagAndVstoreId(const std::string &snapshotName, const std::string &snapTag,
                                                      std::string &snapshotID, const std::string &vstoreId);

        int DeleteSnapshot(std::string SnapshotName, std::string id);

        int DeleteSnapshot(std::string SnapshotName) override;

        int
        DeleteSnapshotWithVstoreId(const std::string &SnapshotName, const std::string &id, const std::string &vstoreId);

        int GetReplicationPairID(const std::string &fsID, const std::string &remoteDeviceEsn, std::string &pairID);

        int QueryFileSystemByID(const std::string &fileName, const std::string &fsID, DeviceDetails &info);

        int VerifyDoradoIP(std::string doradoIP);


        virtual int QueryFileSystem(DeviceDetails &info);

        int QueryNasSnapShotByName(std::string snapShotName, std::string parentId, std::string &snapShotId);

        int GetFileSystemNameByID(DeviceDetails &info, std::string &errDes);

        int QuerySnapshot(std::string SnapshotName, std::string &id);

        int CheckPerformanceStatisticSwitch();

        int OpenPerformanceStatisticSwitch();

        int QueryReplicationPerformance(std::string pairID, uint64_t &bandwidth);

        int QueryFileSystemSnapShotNum(std::string fileSystemId, unsigned long long &count);

        int QueryRemoteDeviceUserByName(const std::string &userName, std::string &remoteDevUserID);

        int DeleteRemoteDeviceUserByID(const std::string &remoteDevUserID);

        int RollBackBySnapShotId(const std::string &SnapshotName, const std::string &snapshotId);

        int RollBackBySnapShotIdWithVstoreId(const std::string &snapshotName, const std::string &snapshotId,
                                                  const std::string &vstoreId, Module::RestResult& result);

        int CreateReplicationWithSnapTag(const int &fsId, const std::string &rDevId, const int &bandwidth,
                                              const std::string &snapTag, int &rfsId, std::string &pairId);

        int GetESN(std::string &esn);

        int DeleteFileSystemAndParentSnapshot();

        int QueryDTree(const std::string &dtreeName);

        int DeleteDTree(const std::string &dtreeName);

        int CreateDTree(const std::string &dtreeName);

        std::string GetFileSystemId() {
            return fileSystemId;
        }

        int QueryReplicationPairIds(std::string lunId, std::vector<std::string> &pairIdList);

        void EncodeUrlFileName(std::string &fileName);

        int GetDeviceInfo(const Json::Value &data, DeviceDetails &info);

    protected:
        int CreateFileSystem(unsigned long long size, SecurityStyle secStyle);

        void IterateNFSHost(Json::Value data, std::vector<std::string> &nfsIPList, IP_TYPE ipType);

        int CreateCloneFileSystem(std::string volumeName, std::string &fsid);

        int QueryLIFPortList(std::vector<std::string> &ipList, Json::Value &data);

        int DeleteFileSystem();

        int QueryFileSystem(std::string fileName, DeviceDetails &info);

        void GetLogicPortAndControllerAction(
                const Json::Value &oneNode, std::vector<std::pair<std::string, std::string>> &ipControllerList,
                IP_TYPE ipType);

        int GetLogicPortAndController(
                const Json::Value &data, std::vector<std::pair<std::string, std::string>> &ipControllerList,
                IP_TYPE ipType);

        int QueryNasShareClient(NasSharedInfo &info, std::string url, std::string type);

        int QueryNasShare(std::vector<NasSharedInfo> &infos, std::string url, std::string type);

    protected:
        std::string fileSystemId{};
        unsigned int usedSpace = 0;
        unsigned int totalSpace = 0;
        bool readK8s = true;

    private:
        bool HandleHostData(Json::Value oneNode, IP_TYPE ipType, std::vector<LogicPortInfo> &logicPorts);

    };
}
#endif  // DORADO_NAS_H
