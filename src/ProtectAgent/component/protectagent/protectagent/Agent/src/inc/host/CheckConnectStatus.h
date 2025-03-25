#ifndef _AGENTCLI_CHECK_CONNECT_STATUS_H_
#define _AGENTCLI_CHECK_CONNECT_STATUS_H_

#include <atomic>
#include <vector>
#include <thread>
#include <atomic>
#include <functional>
#include "message/curlclient/CurlHttpClient.h"
#include "common/Defines.h"
#include "common/JsonUtils.h"
#include "common/CMpThread.h"
#include "host/host.h"

namespace {
// Json字段
const mp_string ITEMS = "items";
const mp_string PAGES = "pages";
// PM业务IP
const mp_string REST_MANAGER_SERVER_LIST = "managerServerList";
// 服务器IP
const mp_string ENDPOINT = "endpoint";
// nodes ipv4 ip
const mp_string CLUSTERS_NODES_V4 = "managementIPv4";
// nodes ipv6 ip
const mp_string CLUSTERS_NODES_V6 = "managementIPv6";
// 节点名
const mp_string NODENAME = "nodeName";
// 服务器类型
const mp_string TYPE = "type";
// 服务器类型
const mp_string SUBTYPE = "sub_type";
// controller node
const mp_string CONTROLLER = "controller";
// 服务器端口
const mp_string PORT = "port";
// 连接状态
const mp_string LINKSTATUS = "link_status";
}


class CheckConnectStatus;
typedef mp_int32 (CheckConnectStatus::*pFuncGetComList)();

class CheckConnectStatus {
public:
    mp_int32 Handle(const mp_string& actionType);
    static CheckConnectStatus& GetInstance()
    {
        return m_Instance;
    }
    CheckConnectStatus()
    {
        m_bNeedExit = MP_FALSE;
        (mp_void) memset_s(&m_CheckConnectivityThread, sizeof(m_CheckConnectivityThread),
            0, sizeof(m_CheckConnectivityThread));
    }
    ~CheckConnectStatus();
    mp_int32 Init();
    mp_int32 UpdateVsphereConnectivity();
    mp_void CheckVsphereConnectivity();
    mp_bool GetExitFlag()
    {
        return m_bNeedExit;
    }

private:
    mp_bool VerifyLinkStatusBody(const Json::Value& body);
    bool CheckIfIpsVecEmpty(std::vector<std::string>& VecIp);
    mp_int32 ReportHost();
    mp_int32 CheckAgentRole();
    mp_int32 DeleteHost();
    mp_int32 RegisterHost2PM();
    mp_int32 DeleteHostFromPM();
    mp_int32 InitRegisterReq(HttpRequest& req);
    mp_int32 GetHostInfo(Json::Value& hostInfo);
    mp_void PrintResult(const mp_string& actionType, const mp_int32& ret);
    mp_void PrintActionType(const mp_string& actionType);
    mp_void SecurityConfiguration(HttpRequest& req, const mp_string& actionType);

    // 解析Vcenter IP和服务器类型
    mp_int32 GetvSphereIp();
    mp_int32 GetPmControllerIp();
    // 配置文件中获取PM控制器IP
    mp_int32 GetPmControllerIpFromConfig();
    mp_int32 GetvSphereJsonValue(mp_int32 page_no, const mp_string& requrl, std::vector<Json::Value>& vecJsonValue);
    mp_int32 DoSendRequestClusterNodes(const mp_string& requrl, std::vector<Json::Value>& vecJsonValue);
    // 解析DME IP和服务器类型
    mp_int32 GetUpdateInterval(mp_uint32& intervaltime);
    mp_int32 GetPMIPandPort();
#ifdef LINUX
    mp_int32 GetDpcNodeConfig();
    mp_bool  CheckDpcProcess();
    mp_void  ConfigDpcFlowControl();
#endif
    mp_bool  IsInstallDataTurbo();
    mp_int32 UpdatePMInfo();
    mp_int32 UpdatePMInfo(Json::Value registerReq);
    mp_int32 InitRequest(const mp_string& reqmethod, const mp_string& requrl, const mp_string& ip, HttpRequest& req);
    mp_int32 InitDeleteHostReq(const mp_string hostid, HttpRequest& req);
    mp_int32 BuildPMBody(Json::Value& PMMsg);
    // 检查连通性
    mp_int32 CheckConnect();
    mp_int32 CheckVDDK();
    // 发送请求获取IP和返回连通状态码
    mp_int32 SendRequest(const HttpRequest& req, mp_string& responseBody);
    // 线程启动函数
#ifdef WIN32
    static DWORD WINAPI CheckConnectivity(LPVOID param);
#else
    static mp_void* CheckConnectivity(mp_void* param);
#endif
    std::vector<Json::Value> m_vecValue;
    thread_lock_t m_Mutex;

    // 跟PM之间的心跳上报
    bool CheckEnabledReportHeartBeat();
    void GetReportHeartBeatConfig();
    void ReportHeartBeatToPM();
    void BuildHeatBeatBody(host_info_t &hostInfo, std::vector<std::string> &connectedIps, Json::Value &jIpList,
        Json::Value &jsonBody);
    mp_int32 SendHeatBeatRequest(const std::vector<mp_string>& ipList, const Json::Value& jsonBody);
    mp_int32 ReportHeartBeatToPMInner();

    // 请求PM所有节点IP列表
    mp_int32 RequestPmIpList();
    mp_int32 UpdatePmIpList(const std::vector<mp_string>& oldIpList, const mp_string& newIpList);
    // 资源利用率上报
    mp_int32 GetResourceAlarmThreshouldValue(mp_double& value, const mp_string& key);
    void GetReportResourceUsageConfig();
    void GetSysReourceRestPrintConfig();
    void ReportResourceUsageToPM();
    mp_int32 ReportResourceUsageToPMInner();
    // 系统剩余资源过低打印
    void GetSysResourceUsage();
    void GetSysResourceUsageInner();

private:
    struct Componet {
        mp_string mainType;
        mp_string destRole;
        mp_string destAddr;
        mp_string destAddr6;
        mp_string destName;
        mp_int32 state;
        mp_string port;
        Componet()
        {
            mainType = "";
            destRole = "";
            destAddr = "";
            destAddr6 = "";
            destName = "";
            state = 0;
            port = "";
        }
    };

    static mp_void* CheckConnectivitySub(CheckConnectStatus* pthis, std::vector<pFuncGetComList>& getComList);
    mp_int32 CheckComponentConnect(Componet& comp, const mp_string& MP_PORT);
    std::vector<Componet> m_ComList;
    static CheckConnectStatus m_Instance;
    // 发送check请求线程id
    volatile mp_bool m_bNeedExit;
    thread_id_t m_CheckConnectivityThread;
    std::unique_ptr<std::thread> m_heartBeatToPMThread;
    std::unique_ptr<std::thread> m_ResourceUsageToPMThread;
    std::unique_ptr<std::thread> m_GetSysResourceUsageThread;
    mp_string m_AgentRole;
    std::vector<mp_string> m_PMIpVec;
    mp_string m_AgentIp;
    mp_string m_AgentPort;
    mp_string m_PMIp;
    mp_string m_PMPort;
    mp_string m_outputStr;
    mp_bool m_isDpcNode;
    mp_bool m_isInstalledDpcClient;
    std::vector<std::string> m_connectedIps;   // 资源占用量上报使用可连接IP列表

    std::atomic<bool> m_stopFlag { false };
    int m_heartbeatToPmInterval { 300 };
    int m_resourceUsageToPmInterval { 3000 };
    int m_sysCpuMemRestPrint { 300 };
    std::vector<std::string> m_connectedPMInLastRound;
    mp_uint64 m_lastUpdateVsphereConnectivityTime = 0;
};

#endif
