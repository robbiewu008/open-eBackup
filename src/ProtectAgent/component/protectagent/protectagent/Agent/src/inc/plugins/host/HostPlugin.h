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
#ifndef __AGENT_HOST_PLUGIN_H__
#define __AGENT_HOST_PLUGIN_H__

#include "common/Types.h"
#include "plugins/ServicePlugin.h"
#include "host/host.h"
#include "message/rest/interfaces.h"
#include "plugins/host/UpdateCertHandle.h"


static const mp_string REST_PARAM_HOST_OS = "os";
static const mp_string REST_PARAM_HOST_SN = "sn";
static const mp_string REST_PARAM_HOST_UUID = "uuid";
static const mp_string REST_PARAM_HOST_NAME = "name";
static const mp_string REST_PARAM_HOST_TYPE = "type";
static const mp_string REST_PARAM_HOST_SUBTYPE = "subType";
static const mp_string REST_PARAM_HOST_ENDPOINT = "endpoint";
static const mp_string REST_PARAM_HOST_USERNAME = "username";
static const mp_string REST_PARAM_HOST_PASSWORD = "password";
static const mp_string REST_PARAM_HOST_OSTYPE = "osType";
static const mp_string REST_PARAM_HOST_VERSION = "version";
static const mp_string REST_PARAM_HOST_EXTENDINFO = "extendInfo";
static const mp_string REST_PARAM_HOST_BUILD_NUM = "buildNum";
static const mp_string REST_PARAM_HOST_DISK_NUM = "diskNumber";
static const mp_string REST_PARAM_HOST_DISK_NUMS = "diskNumbers";
static const mp_string REST_PARAM_HOST_INIT_ISCSI = "iscsis";
static const mp_string REST_PARAM_HOST_INIT_FC = "fcs";
static const mp_string REST_PARAM_HOST_LUN_ID = "lunId";
static const mp_string REST_PARAM_HOST_WWN = "wwn";
static const mp_string REST_PARAM_HOST_ARRAY_SN = "arraySn";
static const mp_string REST_PARAM_HOST_ARRAY_VENDOR = "arrayVendor";
static const mp_string REST_PARAM_HOST_ARRAY_MODEL = "arrayModel";
static const mp_string REST_RARAM_HOST_DEVICE_NAME = "deviceName";
static const mp_string REST_PARAM_HOST_DEVICE_DISKNUM = "diskNumber";
static const mp_string REST_PARAM_HOST_PARTISIONNAME = "partitionName";
static const mp_string REST_PARAM_HOST_CAPACITY = "capacity";
static const mp_string REST_PARAM_HOST_DISKNAME = "diskName";
static const mp_string REST_PARAM_HOST_LBA_ADDR = "LBA";
static const mp_string REST_PARAM_HOST_FUSION_STORAGE_IP = "dsware";
static const mp_string REST_PARAM_AGENT_CURRENT_VERSION = "curVersion";
static const mp_string REST_PARAM_AGENT_VERSION_TIMESTAMP = "versionTimestamp";
static const mp_string REST_PARAM_AGENT_UPGRADE_DOWNLOADLINK = "downloadLink";
static const mp_string REST_PARAM_AGENT_UPGRADE_AGENTID = "agentId";
static const mp_string REST_PARAM_AGENT_UPGRADE_AGENTNAME = "agentName";
static const mp_string REST_PARAM_AGENT_UPGRADE_PACKAGESIZE = "newPackageSize";
static const mp_string REST_PARAM_AGENT_UPGRADE_REVSTATUS = "revStatus";
static const mp_string REST_PARAM_AGENT_UPGRADE_STATUS = "upgradeStatus";
static const mp_string REST_PARAM_AGENT_UPGRADE_JOBID = "jobId";
static const mp_string REST_PARAM_AGENT_PACKAGE_TYPE = "packageType";
static const mp_string REST_PARAM_AGENT_MODFIY_STATUS = "modifyStatus";

static const mp_string REST_PARAM_AGENT_ERROR_CODE = "errorCode";
static const mp_string REST_PARAM_AGENT_ERROR_MSG = "errorMessage";

static const mp_string REST_PARAM_HOST_FILENAME = "fileName";
static const mp_string REST_PARAM_HOST_PARAMS  = "params";
static const mp_string REST_PARAM_HOST_IP = "ip";
static const mp_string REST_PARAM_HOST_PORT = "port";
static const mp_string REST_PARAM_HOST_SNMPTYPE = "snmpType";
static const mp_string REST_PARAM_HOST_SNMP_AUTHTYPE = "authType";
static const mp_string REST_PARAM_HOST_SNMP_ENCRYPTYPE = "encryptType";

static const mp_string REST_PARAM_HOST_TIMEZONE_BIAS = "tzBias";
static const mp_string REST_PARAM_HOST_TIMEZONE_ISDST = "isDST";

static const mp_string REST_PARAM_HOST_THIRDPARTY_STATE = "state";
static const mp_string REST_PARAM_HOST_LOG_EXPORT_ID = "id";
static const mp_string REST_PARAM_HOST_LOG_EXPORT_MAXSIZE = "maxSize";
static const mp_string REST_PARAM_HOST_LOG_PACK_FORMAT = "format";
static const mp_string REST_PARAM_HOST_UPGRAGE_TYPE = "upgradeProtocolType";

static const mp_string SNMP_PROTOCOL_USER = "HTTP_SNMPUSERNAME";
static const mp_string SNMP_AUTH_PW = "HTTP_AUTHPASSWORD";
static const mp_string SNMP_ENCRYPT_PW = "HTTP_ENCRYPTPASSWORD";

static const mp_string SNMP_CONNECT_DME_TYPE = "type";
static const mp_string SNMP_CONNECT_DME_IP = "dmeIP";
static const mp_string SNMP_CONNECT_DME_PORT = "dmePort";

static const mp_string ADD_CONTROLLER_RESPONDS = "revStatus";

static const mp_string SNMP_CONNECT_DME_INVALID_IPV4 = "127.0.0.1";
static const mp_string SNMP_CONNECT_DME_INVALID_IPV6 = ":::1";

class HostPlugin : public CServicePlugin {
public:
    HostPlugin();
    ~HostPlugin();

    mp_int32 Init(std::vector<mp_uint32>& cmds);
    mp_int32 DoAction(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 DoAction(CDppMessage& reqMsg, CDppMessage& rspMsg);

    mp_string GetUpgradeJobId() const
    {
        return upgradeJobId;
    }

    mp_string GetCompressType() const
    {
        return compressType;
    }

    mp_string GetModifyJobId() const
    {
        return modifyJobId;
    }

    mp_string GetUpdateCertJobId() const
    {
        return updateCertJobId;
    }

    mp_int32 GetNewPackageSize() const
    {
        return m_newPackageSize;
    }

private:
    void CallAutoStartFunction();
    CHost m_host;
    thread_id_t m_upgradThread;
    static const mp_uchar HOST_PLUGIN_NUM_3   = 3;
    static const mp_uchar HOST_PLUGIN_NUM_4   = 4;
    static const mp_uchar HOST_PLUGIN_NUM_5   = 5;
    static const mp_int32 HOST_PLUGIN_NUM_255 = 255;
    static const mp_int32 HOST_PLUGIN_NUM_65535 = 65535;

private:
    mp_int32 QueryAgentVersion(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 QueryFusionStorageIP(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 QueryHostInfo(CRequestMsg& req, CResponseMsg& rsp);
    mp_void InitApi();
#ifdef FRAME_SIGN
    EXTER_ATTACK mp_int32 QueryHostV1Info(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 QueryHostV1AppPlugins(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 QueryWwpns(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 QueryWwpnsV2(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 QueryIqns(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 ScanIqns(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 DataturboRescan(CRequestMsg& req, CResponseMsg& rsp);
#endif
    mp_int32 QueryDiskInfo(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 QueryTimeZone(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 QueryInitiators(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 QueryPartisions(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 QueryAgentInfo(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 QueryUpgradeStatus(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 QueryModifyStatus(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 ScanDisk(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 QueryThirdPartyScripts(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 ExecThirdPartyScript(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 GenerateTrapInfo(CRequestMsg& req,
        std::vector<trap_server>& vecTrapServer, snmp_v3_param& stParam);
    EXTER_ATTACK mp_int32 UpdateTrapServer(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 UpgradeAgent(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 RegTrapServer(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 PushNewCert(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 RequestUpdateCert(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 RequestCleanCert(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 RequestRollBackCert(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 CheckConnectPmToAgent(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 UnRegTrapServer(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 VerifySnmp(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 ExecFreezeScript(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 ExecThawScript(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 QueryFreezeStatusScript(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 CollectAgentLog(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 CollectAgentLogStauts(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 ExportAgentLog(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 UpdateAgentLogLevel(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 CleanAgentExportedLog(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 UpdateLinksInfo(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 NotifyManagerServer(CRequestMsg& reqMsg, CResponseMsg& rspMsg);
    EXTER_ATTACK mp_int32 ModifyPlugin(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 CheckCompressTool(CRequestMsg& req, CResponseMsg& rsp);
    mp_int32 CheckExportLogParams(const mp_string &strLogId, const mp_string &strMaxSize, CResponseMsg& rsp);

    mp_string GetLogName()
    {
        return m_host.GetLogName();
    }
#ifdef WIN32
    EXTER_ATTACK mp_int32 DeviceOnline(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 DeviceBatchOnline(CRequestMsg& req, CResponseMsg& rsp);
#endif
    mp_int32 CheckValidDMEIpAddr(const mp_string& dmeIp);
    EXTER_ATTACK mp_int32 ConnectDME(CRequestMsg& req, CResponseMsg& rsp);
    EXTER_ATTACK mp_int32 GetInitiators(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 ScanDiskByDpp(CDppMessage& reqMsg, CDppMessage& rspMsg);
    EXTER_ATTACK mp_int32 GetHostIps(CDppMessage& reqMsg, CDppMessage& rspMsg);

    mp_string upgradeJobId;
    mp_string modifyJobId;
    mp_string updateCertJobId;
    mp_int32 m_newPackageSize;
    mp_string compressType;
};

#endif  // __AGENT_HOST_PLUGIN_H__
