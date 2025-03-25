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
#include "common/Log.h"
#include "common/ConfigXmlParse.h"
#include "common/Utils.h"
#include "message/curlclient/CurlHttpClient.h"
#include "host/ConnectivityManager.h"

namespace {
const uint32_t MAX_RETRY_TIMES = 3;
const mp_uint32 ONE_SECOND = 1 * 1000;  // 1000 ms
const int64_t DEFAULT_THREAD_NUM = 30;
const int64_t SECOND_TO_WAIT_RESULT_BE_GET = 10;
const int64_t SECOND_TO_WAIT_RESULT = 1;
}

ConnectivityManager& ConnectivityManager::GetInstance()
{
    static ConnectivityManager instance;
    instance.Init();
    return instance;
}

ConnectivityManager::~ConnectivityManager()
{
    UnInit();
}

void ConnectivityManager::Init()
{
    std::lock_guard<std::mutex> lock(m_initMtx);
    if (m_initFlag) {
        return;
    }
    std::string numStr;
    mp_int32 iRet =
        CConfigXmlParser::GetInstance().GetValueString(CFG_BACKUP_SECTION, CFG_CHECK_CONN_THREAD_NUM, numStr);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get check connnect thread num failed.");
        m_workerNum = DEFAULT_THREAD_NUM;
    }
    m_workerNum = CMpString::SafeStoll(numStr, DEFAULT_THREAD_NUM);
    COMMLOG(OS_LOG_INFO, "Enter Init ConnectivityManager, workeNum:%d.", m_workerNum);
    for (uint32_t i = 0; i < m_workerNum; ++i) {
        std::shared_ptr<std::thread> woker = std::make_shared<std::thread>(
            std::bind(&ConnectivityManager::CheckThreadRun, this));
        if (woker == nullptr) {
            COMMLOG(OS_LOG_INFO, "Init ConnectivityManager fail.");
            return;
        }
        m_wokers.push_back(woker);
    }
    m_initFlag = true;
    COMMLOG(OS_LOG_INFO, "Init ConnectivityManager, workeNum:%d.", m_wokers.size());
}

void ConnectivityManager::UnInit()
{
    COMMLOG(OS_LOG_INFO, "Enter UnInit ConnectivityManager");
    m_exitFlag = true;
    for (int i = 0; i < m_workerNum; ++i) {
        IpCheckRequest request;
        AddCheckRequest(request);      // wakeup all worker thread to exit.
    }

    for (std::shared_ptr<std::thread> worker : m_wokers) {
        if (worker) {
            worker->join();
        }
    }

    COMMLOG(OS_LOG_INFO, "UnInit ConnectivityManager");
}

bool ConnectivityManager::IfIpVectorsSame(const std::vector<std::string>& v1, const std::vector<std::string>& v2)
{
    if (v1.size() != v2.size()) {
        COMMLOG(OS_LOG_DEBUG, "sizes are not equal");
        return false;
    }
    if (v1.size() == 0) {
        COMMLOG(OS_LOG_WARN, "empty vec");
        return true;
    }
    std::vector<std::string> t1 = v1;
    std::vector<std::string> t2 = v2;
    std::sort(t1.begin(), t1.end());
    std::sort(t2.begin(), t2.end());

    for (uint64_t i = 0; i < v1.size(); i++) {
        if (t1[i] != t2[i]) {
            COMMLOG(OS_LOG_DEBUG, "values are not equal: %d vs %d", t1[i], t2[i]);
            return false;
        }
    }
    return true;
}

bool ConnectivityManager::IfDuplicatedCheckingIsRunning(std::vector<std::string>& runningIps, uint32_t port)
{
    std::unique_lock<std::mutex> lock(m_runningCheckingMtx);
    bool isRunningChecking = false;
    if (m_runningCheckings.count(port) == 0) {
        COMMLOG(OS_LOG_DEBUG, "port %d is not checking, will start checking", port);
        std::set<std::vector<std::string>> temp = {runningIps};
        // set running state
        m_runningCheckings[port] = temp;
    } else {
        for (const std::vector<std::string>& v : m_runningCheckings[port]) {
            if (IfIpVectorsSame(v, runningIps)) {
                isRunningChecking = true;
                runningIps = v;
            }
        }
        if (!isRunningChecking) {
            COMMLOG(OS_LOG_DEBUG, "port %d is checking, but ips are different, will start checking", port);
            // set running state
            m_runningCheckings[port].insert(runningIps);
            isRunningChecking = true;
        }
    }
    return isRunningChecking;
}

std::vector<std::string> ConnectivityManager::DoGetConnectedIps(const std::vector<std::string>& runningIps,
    uint32_t port)
{
    std::vector<std::string> connectIps;
    std::shared_ptr<IpConnectChecker> checker = std::make_shared<IpConnectChecker>();
    if (checker == nullptr) {
        COMMLOG(OS_LOG_INFO, "Create Ip Connect Checker failed.");
        {
            std::unique_lock<std::mutex> lock(m_runningCheckingMtx);
            m_runningCheckings[port].erase(runningIps);
            if (m_runningCheckings[port].empty()) {
                m_runningCheckings.erase(port);
            }
        }
        return connectIps;
    }
    std::set<std::string> dstIpSet(runningIps.begin(), runningIps.end());
    checker->SetCheckIpCount(dstIpSet.size());
    COMMLOG(OS_LOG_INFO, "There are %d ip need to be checked", dstIpSet.size());

    for (const std::string& dstIp : dstIpSet) {
        IpCheckRequest request;
        request.checker = checker;
        request.ipPair.dstIp = dstIp;
        request.ipPair.dstPort = port;
        AddCheckRequest(request);
    }
    checker->Wait();
    for (const IpCheckResult& iResult : checker->GetCheckResults()) {
        if (iResult.isConnected) {
            connectIps.push_back(iResult.ipPair.dstIp);
            COMMLOG(OS_LOG_DEBUG, "Ip %s can connected.", iResult.ipPair.dstIp.c_str());
        }
    }
    {
        std::unique_lock<std::mutex> lock(m_runningCheckingMtx);
        // set running result
        m_checkingResults[port][runningIps] = connectIps;
        // clear running state
        m_runningCheckings[port].erase(runningIps);
        if (m_runningCheckings[port].empty()) {
            m_runningCheckings.erase(port);
        }
    }
    // wait other thread to get checking result
    DoSleep(SECOND_TO_WAIT_RESULT_BE_GET);
    // clear current checking result from
    {
        std::unique_lock<std::mutex> lock(m_runningCheckingMtx);
        m_checkingResults[port].erase(runningIps);
        if (m_checkingResults[port].empty()) {
            m_checkingResults.erase(port);
        }
    }
    return connectIps;
}

std::vector<std::string> ConnectivityManager::GetConnectedIps(const std::vector<std::string>& dstIps, uint32_t port)
{
    std::vector<std::string> connectIps;
    // runningIps is the key to find the checking result
    std::vector<std::string> runningIps = dstIps;
    if (!IfDuplicatedCheckingIsRunning(runningIps, port)) {
        connectIps = DoGetConnectedIps(runningIps, port);
    } else {
        COMMLOG(OS_LOG_DEBUG, "Found duplicated checking, will wait for the checking finished");
        while (true) {
            {
                std::unique_lock<std::mutex> lock(m_runningCheckingMtx);
                if (m_runningCheckings.count(port) == 0 || m_runningCheckings[port].count(runningIps) == 0) {
                    COMMLOG(OS_LOG_DEBUG, "ip checking is finished");
                    break;
                }
            }
            DoSleep(SECOND_TO_WAIT_RESULT);
        }
        std::unique_lock<std::mutex> lock(m_runningCheckingMtx);
        if (m_checkingResults.count(port) == 0 || m_checkingResults[port].count(runningIps) == 0) {
            COMMLOG(OS_LOG_ERROR, "can't get result");
            return connectIps;
        } else {
            connectIps = m_checkingResults[port][runningIps];
        }
    }
    COMMLOG(OS_LOG_INFO, "There are %d ip can be connected.", connectIps.size());
    return connectIps;
}

void ConnectivityManager::AddCheckRequest(const IpCheckRequest& request)
{
    // producer
    std::unique_lock<std::mutex> lock(m_requestMtx);

    m_producerCond.wait(lock, [this] { return (m_requestQueue.size() < m_queueSize); });
    m_requestQueue.push_back(request);

    m_consumerCond.notify_one();
}

IpCheckRequest ConnectivityManager::GetCheckRequest()
{
    // consumer
    std::unique_lock<std::mutex> lock(m_requestMtx);

    m_consumerCond.wait(lock, [this] { return !m_requestQueue.empty(); });

    IpCheckRequest request = m_requestQueue.front();
    m_requestQueue.pop_front();

    m_producerCond.notify_one();
    return request;
}

void ConnectivityManager::CheckThreadRun()
{
    std::shared_ptr<IHttpClient> httpClient = IHttpClient::CreateNewClient();
    if (httpClient == nullptr) {
        COMMLOG(OS_LOG_ERROR, "HttpClient create failed.");
        return;
    }
    COMMLOG(OS_LOG_INFO, "Worker Run.");

    while (!m_exitFlag) {
        IpCheckRequest request = GetCheckRequest();
        if (request.ipPair.dstIp.empty()) {
            continue;
        }

        bool result = false;
        IpCheckResult ipCheckResult {request.ipPair, false};

        mp_int32 retryTimes = 0;
        while (retryTimes < MAX_RETRY_TIMES) {
            if (httpClient->TestConnectivity(request.ipPair.dstIp, std::to_string(request.ipPair.dstPort))) {
                result = true;
                COMMLOG(OS_LOG_INFO, "Can connect ip(%s).", request.ipPair.dstIp.c_str());
                break;
            }
            retryTimes++;
            DoSleep(ONE_SECOND);
            COMMLOG(OS_LOG_WARN, "Can not connect ip(%s).", request.ipPair.dstIp.c_str());
        }

        ipCheckResult.isConnected = result;
        if (request.checker != nullptr) {
            request.checker->AddCheckResult(ipCheckResult);
        }
    }

    COMMLOG(OS_LOG_DEBUG, "Worker exit.");
}