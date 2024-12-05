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
#ifndef FUSION_STORAGE_H
#define FUSION_STORAGE_H

#include "define/Types.h"
#include "log/Log.h"
#include "curl_http/HttpClientInterface.h"
#include "device_access/fusionstorage/Const.h"

namespace Module {
    const std::string FUSION_STORAGE_MODULE_NAME = "FusionStorage";
    const int MP_NOTEXIST = 2;

    struct LunParams {
        std::string volumeName = "";
        bool Compress = false;
        bool Dedup = false;
        int poolid = -1;
        unsigned long long Size = -1;
        unsigned long long usedSize = -1;

        LunParams(std::string vName, bool cpress, bool dd, int pd, unsigned long long sz)
            : volumeName(vName), Compress(cpress), Dedup(dd), poolid(pd), Size(sz) {}

        LunParams() {}
    };

    struct FSSnapshotInfo {
        std::string snapshotName;
        std::string volumeName;
    };

    class FusionStorage {
    public:
        explicit FusionStorage(Json::Value fsJson)
        {
            fs_pHttpCLient = IHttpClient::GetInstance();
            GET_JSON_STRING(fsJson, FUSION_STORAGE_TYPE, Type);
            GET_JSON_STRING(fsJson, FUSION_STORAGE_IP, FusionStorageIP);
            GET_JSON_STRING(fsJson, FUSION_STORAGE_PORT, FusionStoragePort);
            GET_JSON_STRING(fsJson, FUSION_STORAGE_USERNAME, FusionStorageUsername);
            GET_JSON_STRING(fsJson, FUSION_STORAGE_PASSWORD, FusionStoragePassword);
            GET_JSON_INT32(fsJson, FUSION_STORAGE_POOL, fsPoolid);
            HCP_Log(DEBUG, FUSION_STORAGE_MODULE_NAME)
                    << Type << "," << FusionStorageIP << "," << FusionStoragePort << ","
                    << fsPoolid << HCPENDLOG;
            Login();
        }

        explicit FusionStorage() {}

        virtual ~FusionStorage()
        {
            Logout();
            IHttpClient::ReleaseInstance(fs_pHttpCLient);
        }

        /*
        Create fusionstorage Lun
        Date : 2020/03/03
        out params:id -> LUN id.
                  :WWN  -> LUN WWN.
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                     1.Create fusionstorage Lun
        */
        int CreateLUN(LunParams params, int &id, std::string &WWN);

        /*
        query fusionstorage Lun by lun name
        Date : 2020/03/03
        out params:id -> LUN id.
                :WWN  -> LUN WWN.
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.query fusionstorage Lun by lun name
        */
        int QueryLUN(std::string volumeName, int &id, std::string &WWN,
            unsigned long long &size, unsigned long long &usedSize);

        /*
        delete volume with name
        Date : 2020/03/03
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                     1.delete volume with name
        */
        int DeleteLUN(std::string volumeName);

        /*
        create fusionstorage host by host name and host ip address
        Date : 2020/03/03
        out params:HOST -> host name.equal with UUID
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.create fusionstorage host by host name and host ip address
        */
        int CreateHost(const std::string hostName, const std::string ip, std::string &HOST);

        /*
        query fusionstorage host by host name
        Date : 2020/03/03
        out params:HOST -> host name.equal with UUID
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.query fusionstorage host by host name
        */
        int QueryHost(const std::string hostName, const std::string ip, std::string &HOST);

        /*
        create fusionstorage port with iscsi connector
        Date : 2020/03/03
        out params:iscsi -> port name.equal with InitiatorName
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.create fusionstorage port with iscsi connector
        */
        int CreateISCSIPort(const std::string iscsiConnector, std::string &iscsi);

        /*
        query fusionstorage port with iscsi connector
        Date : 2020/03/03
        out params:iscsi -> port name.equal with InitiatorName
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.query fusionstorage port with iscsi connector
        */
        int QueryISCSIPort(const std::string iscsiConnector, std::string &iscsi);

        /*
        bind fusionstorage port to host
        Date : 2020/03/03
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.bind fusionstorage port to host
        */
        int BindHostISCSIPort(std::string hostName, std::string iscsiPort);

        /*
        query is fusionstorage port bind to this host
        Date : 2020/03/03
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.query is fusionstorage port bind to this host
        */
        int QueryHostISCSIPort(std::string hostName, std::string iscsiPort);

        /*
        attach lun to special host
        Date : 2020/03/03
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.attach lun to special host
        */
        int CreateHostMapping(const std::string hostName, const std::string lunName);

        int DeleteHostMapping(const std::string hostName, const std::string lunName);

        /*
        query is lun attached to special host
        Date : 2020/03/03
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                    1.query is lun attached to special host
        */
        int QueryHostMapping(const std::string hostName, const std::string volumeName);

        /*
        create link clone volume with special snapshot/volume
        Date : 2020/03/03
        out params:id -> link clone volume ID
                  :WWN -> link clone volume WWN
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                     1.create link clone volume with special snapshot
        */
        int CreateCloneVolume(CLONE_SOURCE type, std::string source, std::string volumeName, int &id, std::string &WWN);

        /*
        create snapshot for special volume
        Date : 2020/03/03
        out params:id -> snapshot ID
                  :WWN -> snapshot WWN
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                     1.create snapshot for special volume
        */
        int CreateSnapshot(std::string VolumeName, std::string SnapshotName, int &id, std::string &WWN);

        /*
        delete snapshot by name
        Date : 2020/03/05
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                     1.delete snapshot by name
        */
        int DeleteSnapshot(std::string SnapshotName);

        /*
        query snapshot by name
        Date : 2020/03/03
        out params:id -> snapshot ID
                  :WWN -> snapshot WWN
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                     1.query snapshot by name
        */
        int QuerySnapshot(std::string SnapshotName, int &id, std::string &WWN);

        /*
        query all iscsi host,need Manual open on DeviceManager.
        Date : 2020/03/03
        out params:iscsiList -> iscsi ip and port List,value like ["1.1.1.1:8000","1.1.1.2:8000"]
        return : Success.SUCCESS, failed:FAILED or HTTP ERROR CODE.
        Description:
                     1.query all iscsi host,need Manual open on DeviceManager.
        */
        int QueryIscsiHost(std::vector<std::string> &iscsiList);

        int ExtendVolumeSize(std::string volumeName, int volumeSize);

        int QueryVBSVolumeAttachNode(std::string volumeName, std::vector<std::string> &attachNodeIPs);

        int QueryAllVBSClient(std::vector<std::string> &mgrIPs);

        int AttachVBSVolume(std::string volumeName, const std::vector<std::string> &nodeIPs);

        int DetachVBSVolume(std::string volumeName, const std::vector<std::string> &nodeIPs);

        int CheckLunExisis(std::string volumeName, std::string &WWN, bool &exists);

        int RevertSnapshot(std::string SnapshotName, std::string targetVolumeName);

        int QuerySnapshotEx(std::string SnapshotName, int &id, std::string &WWN, int &status);

        int QuerySnapshotList(std::string queryCond, std::vector<FSSnapshotInfo> &snapshots);

    public:
        std::string GetType()
        {
            return Type;
        }

        void SetType(std::string type)
        {
            Type = type;
        }

        std::string GetFusionStorageIP()
        {
            return FusionStorageIP;
        }

        void SetFusionStorageIP(std::string fusionStorageIP)
        {
            FusionStorageIP = fusionStorageIP;
        }

        std::string GetFusionStoragePort()
        {
            return FusionStoragePort;
        }

        void SetFusionStoragePort(std::string fusionStoragePort)
        {
            FusionStoragePort = fusionStoragePort;
        }

        std::string GetFusionStorageUsername()
        {
            return FusionStorageUsername;
        }

        void SetFusionStorageUsername(std::string fusionStorageUsername)
        {
            FusionStorageUsername = fusionStorageUsername;
        }

        std::string GetFusionStoragePassword()
        {
            return FusionStoragePassword;
        }

        void SetFusionStoragePassword(std::string fusionStoragePassword)
        {
            FusionStoragePassword = fusionStoragePassword;
        }

        int GetPoolid()
        {
            return fsPoolid;
        }

        void SetPoolid(int poolid)
        {
            fsPoolid = poolid;
        }

        int Login();

        int Logout();

        int ParseBodyV1(const std::string &json_data, Json::Value &data, std::string &errorDes, int &errorCode);

        int ParseBodyV2(const std::string &json_data, Json::Value &data, std::string &errorDes, int &errorCode);

        virtual int SendRequest(HttpRequest &req, Json::Value &data, std::string &errorDes, int &errorCode);

        bool GetJsonValue(const Json::Value &jsValue, std::string strKey, std::string &strValue);

    protected:
        std::string fs_token = "";
        IHttpClient *fs_pHttpCLient = NULL;

    private:
        std::string Type = "";
        std::string FusionStorageIP = "";
        std::string FusionStoragePort = "";
        std::string FusionStorageUsername = "";
        std::string FusionStoragePassword = "";
        int fsPoolid = -1;
    };
}
#endif  // FUSION_STORAGE_H