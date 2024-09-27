#include "plugins/host/UpgradeHandle.h"
#include <fstream>
#include <sstream>
#include <chrono>
#include "common/Log.h"
#include "common/Defines.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/ConfigXmlParse.h"
#include "common/CSystemExec.h"
#include "common/Utils.h"
#include "common/Ip.h"
#include "securecom/RootCaller.h"
#include "securecom/CryptAlg.h"
#include "host/CheckConnectStatus.h"
#include "host/host.h"
#include "message/curlclient/CurlHttpClient.h"
#include "servicecenter/thriftservice/JsonToStruct/trjsonandstruct.h"
#include "plugins/host/HostPlugin.h"
#include "message/curlclient/DmeRestClient.h"

using namespace std;
namespace HandleNamespace {
    // 升级状态： 0-failure 1-success 2-intermediate 3-disk check failed 8-abnormal 9-initial
    const mp_string UPGRADE_STATUS_FAILURE = "0";
    const mp_string UPGRADE_STATUS_SUCCESS = "1";
    const mp_string UPGRADE_STATUS_INTERMEDIATE = "2";
    const mp_string UPGRADE_STATUS_DISK_CHECK_FAILED = "3";
    const mp_string UPGRADE_STATUS_ABNORMAL = "8";
    const mp_string UPGRADE_STATUS_INITIAL = "9";
    const mp_string UPGRADE_PKG_NAME = "DataProtect_client.zip";
    const mp_string UPGRADE_PARAM_DOWNLOADLINK = "downloadLink=";
    const mp_string UPGRADE_PARAM_DISPOSITION = "Content-Disposition";
    const mp_string UPGRADE_PARAM_SIGNATURE = "Signature";
    const mp_int32 HTTP_TIME_OUT = 30 * 60;
}

namespace {
    const mp_int32 PROCESS_30_PERCENT = 30;
    const mp_int32 PROCESS_40_PERCENT = 40;
    const mp_int32 PROCESS_70_PERCENT = 70;
    const mp_int32 PROCESS_80_PERCENT = 80;
    const mp_uint32 JOB_STATUS_RUNNING = 2;
    const mp_uint32 JOB_STATUS_SUCCESS = 5;
    const mp_string LABLE_INFO_LEVEL = "info";
    const mp_string UPGRADE_PRECHECK_LABLE = "job_log_agent_register_check_label";
    const mp_string UPGRADE_BEGIN_DOWNLOAD_PKG_LABLE = "job_log_agent_register_download_start_label";
    const mp_string UPGRADE_FINISH_DOWNLOAD_OKG_LABLE = "job_log_agent_register_download_finish_label";
    const mp_string UPGRADE_START_UPGRADE_LABLE = "job_log_agent_register_upgrade_start_label";
    const mp_string UPGRADE_FAIL_PREPARE_LABLE = "job_log_agent_storage_update_prepare_fail_label";
}


/* ------------------------------------------------------------
Description  : 执行agent升级的流程
Input        : req -- 请求消息
Output       : rsp -- 响应消息
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */

std::vector<mp_string> UpgradeHandle::m_vecPMIp;
mp_string UpgradeHandle::m_pmIp;
mp_string UpgradeHandle::m_pmPort;
mp_string UpgradeHandle::m_domainName;
mp_string UpgradeHandle::m_upgradeUrl;
mp_string UpgradeHandle::m_sha256;
mp_string UpgradeHandle::m_signature;
mp_int32 UpgradeHandle::m_upgradePackageSize;

#ifdef WIN32
DWORD WINAPI UpgradeHandle::UpgradeAgentHandle(mp_void* param)
#else
mp_void* UpgradeHandle::UpgradeAgentHandle(mp_void* param)
#endif
{
    HostPlugin* hostPlugin = static_cast<HostPlugin*>(param);
    if (hostPlugin == NULL) {
        ERRLOG("Invalid HostPlugin handler.");
        CMPTHREAD_RETURN;
    }

    mp_string jobId = hostPlugin->GetUpgradeJobId();
    if (jobId.empty()) {
        ERRLOG("Invalid job id.");
        CMPTHREAD_RETURN;
    }

    m_upgradePackageSize = hostPlugin->GetNewPackageSize();
    if (m_upgradePackageSize == 0) {
        ERRLOG("Invalid package size.");
        CMPTHREAD_RETURN;
    }

    mp_int32 iRet = InitRequestCommon();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to do InitRequestCommon.");
        CMPTHREAD_RETURN;
    }

    INFOLOG("Handle the upgrade request step 1: check resource.");
    LabelRepoter(jobId, PROCESS_30_PERCENT, UPGRADE_PRECHECK_LABLE, JOB_STATUS_RUNNING);
    iRet = CheckBeforeUpgrade();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to check resource before upgrade.");
        CMPTHREAD_RETURN;
    }

    INFOLOG("Handle the upgrade request step 2: obtain and verify package.");
    LabelRepoter(jobId, PROCESS_40_PERCENT, UPGRADE_BEGIN_DOWNLOAD_PKG_LABLE, JOB_STATUS_RUNNING);
    iRet = ObtainUpgradePac();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to obtain the upgrade package correctly.");
        CMPTHREAD_RETURN;
    }
    LabelRepoter(jobId, PROCESS_70_PERCENT, UPGRADE_FINISH_DOWNLOAD_OKG_LABLE, JOB_STATUS_RUNNING);

    INFOLOG("Handle the upgrade request step 3: prepare for upgrade.");
    iRet = PrepareForUpgrade();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to prepare for the upgration.");
        CMPTHREAD_RETURN;
    }

    INFOLOG("Handle the upgrade request step 4: call root execute.");
    LabelRepoter(jobId, PROCESS_80_PERCENT, UPGRADE_START_UPGRADE_LABLE, JOB_STATUS_RUNNING);
    iRet = CallUpgradeScript();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to excute upgrade script.");
    }

    CMPTHREAD_RETURN;
}

/* ------------------------------------------------------------
Description  : 升级流程之前检查主机资源：可用空间大小等
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 UpgradeHandle::CheckBeforeUpgrade()
{
    DBGLOG("Begin to check the host's resource before upgrade.");
#ifdef WIN32
    mp_string strScriptPath =  CPath::GetInstance().GetBinFilePath(UPGRADE_CHECK_SCRIPT);
    mp_string strCmd = "cmd.exe /c " + strScriptPath + " " + std::to_string(m_upgradePackageSize);
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
#else
    CRootCaller rootCaller;
    std::ostringstream scriptParam;
    scriptParam << "packageSize=" << m_upgradePackageSize << NODE_COLON;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_CHECKBEFOREUPGRADE, scriptParam.str(), NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_THAW_FAILED);
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Check the resource before upgrade failed, result = %d.", iRet);
        if (iRet == ERROR_AGENT_DISK_NOT_ENOUGH) {
            UpdateUpgradeStatus(HandleNamespace::UPGRADE_STATUS_DISK_CHECK_FAILED);
            return iRet;
        }
        UpdateUpgradeStatus(HandleNamespace::UPGRADE_STATUS_FAILURE);
        return iRet;
    }
    DBGLOG("Succeed to check the host's resource before upgrade.");
    
    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 从下载点获取升级包
Input        : upgradeInfoFile -- 记录升级信息的文件：包路径
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 UpgradeHandle::ObtainUpgradePac()
{
    DBGLOG("Begin to obtain the upgrade package.");

    HttpRequest req;
    mp_int32 iRet = InitRequest(req);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Init http request failed.ret=%d", iRet);
        return iRet;
    }

    IHttpClient* httpClient = IHttpClient::GetInstance();
    if (httpClient == nullptr) {
        ERRLOG("HttpClient create failed when register to PM.");
        return MP_FAILED;
    }

    for (auto iter = m_vecPMIp.begin(); iter != m_vecPMIp.end(); ++iter) {
        mp_string& pmIp = *iter;
        mp_string disposition;
        pmIp = CIP::FormatFullUrl(pmIp);
        SetDomainResolve(req, pmIp);
        iRet = SendRequest(httpClient, req, disposition);
        if (iRet == MP_SUCCESS) {
            INFOLOG("Request for downloading the upgrade package succeeded.");
            size_t start = disposition.find_last_of("_");
            size_t end = disposition.find_last_of("\"");
            if (start != mp_string::npos && end != mp_string::npos && start < end) {
                start++;
                m_sha256 = disposition.substr(start, end - start);
            } else {
                ERRLOG("Failed to obtain sha256.start=%lld, end=%lld", start, end);
                iRet = MP_FAILED;
            }
            break;
        }
        ERRLOG("Failed to send the request for downloading the upgrade package.ip=%s", pmIp.c_str());
    }
    IHttpClient::ReleaseInstance(httpClient);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Obtain the upgrade package failed, result = %d.", iRet);
        UpdateUpgradeStatus(HandleNamespace::UPGRADE_STATUS_FAILURE);
        Json::Value detailsValue;
        detailsValue["logDetail"] = ERROR_AGENT_UPGRADE_FAIL_DOWNLOAD_PACKAGE;
        detailsValue["logInfo"] = UPGRADE_FAIL_PREPARE_LABLE;
        detailsValue["logDetailParam"] = "Failed to download the upgrade package.";
        CHost host;
        host.UpdateUpgradeErrorDetails(detailsValue);
        return MP_FAILED;
    }
    INFOLOG("Succeed to obtain the upgrade package.");
    return iRet;
}

/* ------------------------------------------------------------
Description  : 准备开始升级：校验包的完整性、解压包等
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 UpgradeHandle::PrepareForUpgrade()
{
    DBGLOG("Begin to prepare for the upgrade.");

    // 签名写到文件中
    mp_string signatureStr = CMpString::Base64Decode(m_signature);
#ifdef WIN32
    mp_string signatureFilePath = CPath::GetInstance().GetAgentUpgradePath() + PATH_SEPARATOR + UPGRADE_SIGNATURE_FILE;
#else
    mp_string signatureFilePath = CPath::GetInstance().GetStmpFilePath(UPGRADE_SIGNATURE_FILE);
#endif
    mp_int32 iRet = MP_SUCCESS;
    std::ofstream file(signatureFilePath, std::ios::out | std::ios::trunc | std::ios::binary);
    if (file.is_open()) {
        file << signatureStr;
        file.close();
    } else {
        ERRLOG("Failed to open file: %s, errno[%d]:%s.", signatureFilePath.c_str(), errno, strerror(errno));
        iRet = MP_FAILED;
    }
    if (iRet != MP_SUCCESS) {
        ERRLOG("Write upgrade signature file failed.");
        UpdateUpgradeStatus(HandleNamespace::UPGRADE_STATUS_FAILURE);
        return MP_FAILED;
    }

#ifdef WIN32
    mp_string strScriptPath =  CPath::GetInstance().GetBinFilePath(UPGRADE_PREPARE_SCRIPT);
    mp_string strCmd = "cmd.exe /c echo " + m_sha256 + "|" + strScriptPath;
    iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
#else
    CRootCaller rootCaller;
    iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_PREPAREFORUPGRADE, m_sha256, nullptr);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_THAW_FAILED);
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Prepare for the upgrade failed, result = %d.", iRet);
        UpdateUpgradeStatus(HandleNamespace::UPGRADE_STATUS_FAILURE);
        return MP_FAILED;
    }
    DBGLOG("Succeed to prepare for the upgrade.");

    return MP_SUCCESS;
}

/* ------------------------------------------------------------
Description  : 执行具体升级操作：使用推送升级方式调用升级脚本
Return       : MP_SUCCESS -- 成功
               非MP_SUCCESS -- 失败，返回特定错误码
------------------------------------------------------------- */
mp_int32 UpgradeHandle::CallUpgradeScript()
{
    DBGLOG("Begin to execute the upgrade script.");
#ifdef WIN32
    mp_string strScriptPath =  CPath::GetInstance().GetBinFilePath(UPGRADE_CALLER_SCRIPT);
    mp_string strCmd = "cmd.exe /c " + strScriptPath;
    mp_int32 iRet = CSystemExec::ExecSystemWithoutEcho(strCmd);
#else
    CRootCaller rootCaller;
    mp_int32 iRet = rootCaller.Exec((mp_int32)ROOT_COMMAND_SCRIPT_UPGRADECALLER, "", NULL);
    TRANSFORM_RETURN_CODE(iRet, ERROR_COMMON_APP_THAW_FAILED);
#endif
    if (iRet != MP_SUCCESS) {
        ERRLOG("Excute script failed, iRet = %d.", iRet);
        UpdateUpgradeStatus(HandleNamespace::UPGRADE_STATUS_FAILURE);
        return iRet;
    }
    DBGLOG("Succeed to excute the upgrade script.");

    return MP_SUCCESS;
}

mp_int32 UpgradeHandle::UpdateUpgradeStatus(const mp_string& strUpgradeStatus)
{
    COMMLOG(OS_LOG_DEBUG, "Begin to upgrade status[%s].", strUpgradeStatus.c_str());
    std::string strUpgradeFilePath = CPath::GetInstance().GetConfFilePath(CFG_RUNNING_PARAM);
    std::string strUpgradeFilePathBack = strUpgradeFilePath + "back";
    if (CMpFile::CopyFileCoverDest(strUpgradeFilePath, strUpgradeFilePathBack) != MP_SUCCESS) {
        return MP_FAILED;
    }

    mp_string strText = "UPGRADE_STATUS=" + strUpgradeStatus + STR_CODE_WARP;
    std::ifstream stream;
    stream.open(strUpgradeFilePath.c_str(), std::ifstream::in);
    std::ofstream streamout;
    streamout.open(strUpgradeFilePathBack.c_str(), std::ifstream::out);
    mp_string line;
    mp_int32 upgradeStatus = MP_FAILED;

    if (stream.is_open() && streamout.is_open()) {
        while (getline(stream, line)) {
            if (line.find("UPGRADE_STATUS=") == std::string::npos) {
                streamout << line << STR_CODE_WARP;
            } else {
                streamout << strText;
                upgradeStatus = MP_SUCCESS;
            }
        }
    } else {
        COMMLOG(OS_LOG_ERROR, "Open testcfg.tmp file failed.");
    }

    if (stream.is_open()) {
        stream.close();
    }
    if (streamout.is_open()) {
        streamout.flush();
        streamout.close();
    }

    if (upgradeStatus == MP_SUCCESS) {
        CMpFile::CopyFileCoverDest(strUpgradeFilePathBack, strUpgradeFilePath);
    }
    CMpFile::DelFile(strUpgradeFilePathBack);
    COMMLOG(OS_LOG_DEBUG, "Update upgrade status[%s] successfully.", strUpgradeStatus.c_str());

    return MP_SUCCESS;
}

mp_int32 UpgradeHandle::GetDownloadInfo()
{
    mp_string strUpgradeTmpFile = CPath::GetInstance().GetTmpFilePath("tmpUpgradeInfo");
    std::vector<mp_string> vecRead;
    mp_int32 iRet = CIPCFile::ReadFile(strUpgradeTmpFile, vecRead);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to read the upgrade information file.");
        return MP_FAILED;
    }

    if (vecRead.empty()) {
        ERRLOG("The file is empty.");
        return MP_FAILED;
    }

    mp_string downloadLink;
    for (auto iter = vecRead.begin(); iter != vecRead.end(); iter++) {
        mp_string& line = *iter;
        if (line.find(HandleNamespace::UPGRADE_PARAM_DOWNLOADLINK) == 0) {
            size_t start = HandleNamespace::UPGRADE_PARAM_DOWNLOADLINK.length();
            downloadLink = line.substr(start, line.length() - start);
            break;
        }
    }

    if (downloadLink.empty()) {
        ERRLOG("Failed to obtain the link information.");
        return MP_FAILED;
    }
    INFOLOG("The downloadLink: %s", downloadLink.c_str());
    // [https://[192.168.69.203,192.168.69.202]:25082/v1/host-agent/update/download?uuid=crmdownloadlinkee763a7020164ef29d08f60d91e20a33
    size_t startIp = downloadLink.find("[");
    size_t endIp = downloadLink.find("]");
    if (startIp == mp_string::npos || endIp == mp_string::npos || startIp == endIp) {
        ERRLOG("Failed to obtain the ip address from the link. start=%lld, end=%lld", startIp, endIp);
        return MP_FAILED;
    }
    startIp++;
    m_pmIp = downloadLink.substr(startIp, endIp - startIp);

    size_t startPort = downloadLink.find(":", endIp);
    size_t endPort = downloadLink.find("/", endIp);
    if (startPort == mp_string::npos || endPort == mp_string::npos || startPort >= endPort) {
        ERRLOG("Failed to obtain the port from the link. start=%lld, endPort=%lld", startPort, endPort);
        return MP_FAILED;
    }
    startPort++;
    m_pmPort = downloadLink.substr(startPort, endPort - startPort);
    m_upgradeUrl = downloadLink.substr(endPort, downloadLink.length() - endPort);
    INFOLOG("The m_pmPort: %s, m_pmIp: %s, url: %s", m_pmPort.c_str(), m_pmIp.c_str(), m_upgradeUrl.c_str());
    return MP_SUCCESS;
}

mp_int32 UpgradeHandle::SecurityConfiguration(HttpRequest& req)
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_PM_CA_INFO, req.caInfo);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to get caInfo value");
        return iRet;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_SSL_CERT, req.sslCert);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to get sslCert value");
        return iRet;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_SSL_KEY, req.sslKey);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to get sslKey value");
        return iRet;
    }
    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SECURITY_SECTION, CFG_CERT, req.cert);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to get cert value");
        return iRet;
    }
    return iRet;
}

mp_int32 UpgradeHandle::InitRequestCommon()
{
    mp_int32 iRet = GetDownloadInfo();
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to parse the download information.");
        return MP_FAILED;
    }

    m_vecPMIp.clear();
    CMpString::StrSplit(m_vecPMIp, m_pmIp, ',');
    if (m_vecPMIp.empty()) {
        ERRLOG("Split PM ip failed, PM ip list is empty(%s).", m_pmIp.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}

mp_int32 UpgradeHandle::InitRequest(HttpRequest& req)
{
    mp_int32 iRet = SecurityConfiguration(req);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to set the certificate.");
        return iRet;
    }

    iRet = CConfigXmlParser::GetInstance().GetValueString(CFG_SYSTEM_SECTION, CFG_DOMAIN_NAME_VALUE, m_domainName);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Failed to obtain the domain name..");
        return iRet;
    }

    req.url.append("https://");
    req.url.append(m_domainName);
    req.url.append(":");
    req.url.append(m_pmPort);
    req.url.append(m_upgradeUrl);

    req.domaininfo.append("https://");
    req.domaininfo.append(m_domainName);
    req.fileName = CPath::GetInstance().GetTmpPath() + PATH_SEPARATOR + HandleNamespace::UPGRADE_PKG_NAME;
    req.method = "GET";
    return iRet;
}

void UpgradeHandle::SetDomainResolve(HttpRequest& req, const mp_string& pmIp)
{
    req.hostinfo = "";
    req.hostinfo.append(m_domainName);
    req.hostinfo.append(":");
    req.hostinfo.append(m_pmPort);
    req.hostinfo.append(":");
    req.hostinfo.append(pmIp);
}

mp_int32 UpgradeHandle::SendRequest(IHttpClient* httpClient, const HttpRequest& req, mp_string& disposition)
{
    IHttpResponse* httpRespone = httpClient->SendRequest(req, HandleNamespace::HTTP_TIME_OUT);
    if (httpRespone == nullptr) {
        ERRLOG("Failed to send download request.");
        return MP_FAILED;
    }
    if (!httpRespone->Success()) {
        ERRLOG("Failed to send download request, error code %u.", httpRespone->GetHttpStatusCode());
        delete httpRespone;
        return MP_FAILED;
    }
    std::set<mp_string> tmpSet = httpRespone->GetHeadByName(HandleNamespace::UPGRADE_PARAM_DISPOSITION);
    if (tmpSet.empty()) {
        ERRLOG("Failed to obtain Content-Disposition.");
        delete httpRespone;
        return MP_FAILED;
    }
    // 获取签名
    std::set<mp_string> signatureSet = httpRespone->GetHeadByName(HandleNamespace::UPGRADE_PARAM_SIGNATURE);
    if (signatureSet.empty()) {
        ERRLOG("Failed to obtain signature.");
        delete httpRespone;
        return MP_FAILED;
    }
    m_signature = *signatureSet.begin();
    disposition = *tmpSet.begin();
    INFOLOG("The disposition: %s", disposition.c_str());
    delete httpRespone;
    return MP_SUCCESS;
}

mp_int32 UpgradeHandle::LabelRepoter(
    const mp_string &jobId, mp_int32 process, const mp_string &lableName, mp_int32 status)
{
    Lable label;
    JobLog jobLog;
    jobLog.jobId = jobId;
    jobLog.logInfo = lableName;
    jobLog.level = LABLE_INFO_LEVEL;
    label.jobLogs.push_back(jobLog);
    label.status = status;
    label.progress = process;

    mp_string context;
    if (!JsonHelper::StructToJsonString(label, context)) {
        ERRLOG("Transfer Lable struct to json string failed.");
        return MP_FAILED;
    }
    INFOLOG("Report label: %s", context.c_str());

    // report to DME
    mp_string url = "/v1/internal/jobs/" + jobId + "/action/update";
    DmeRestClient::HttpReqParam param("PUT", url, context);
    param.port = atol(m_pmPort.c_str());
    param.mainJobId = jobId;
    HttpResponse response;
    mp_int32 iRet = DmeRestClient::GetInstance()->SendRequest(param, response);
    if (iRet != MP_SUCCESS) {
        ERRLOG("Report label failed for job(%s) failed.", jobLog.jobId.c_str());
        return MP_FAILED;
    }
    return MP_SUCCESS;
}