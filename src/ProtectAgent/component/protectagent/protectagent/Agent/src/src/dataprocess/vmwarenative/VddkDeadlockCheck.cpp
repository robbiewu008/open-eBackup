/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file VddkDeadlockCheck.cpp
 * @brief  Contains function declarations about vddk dead lock check
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include <csignal>
#include <thread>
#include "common/Log.h"
#include "dataprocess/vmwarenative/VddkDeadlockCheck.h"

namespace {
const std::string APINAME_VDDK_READ = "Read";
const std::string APINAME_VDDK_WRITE = "Write";
const int CHECK_PERIOD_IN_SECONDS = 600;
const int CHECK_PERIOD_IN_NUMS = 1000;
const int CHECK_DENO = 1000000000;
const int VDDKAPI_MAX_TIMEOUT = 7200;
const int VDDKAPI_MIN_TIMEOUT = 540;
}

std::mutex VddkDeadlockCheck::m_mutex;
mp_uint32 VddkDeadlockCheck::MAX_TRY_TIMES = 10000000;
static thread_local mp_uint64 threadSpecificPtr_requestID(0);
using namespace std;

// Gerate a UUID conresponding to a API name
VddkDeadlockCheck::DeadlockCheck::DeadlockCheck(const std::string& strApiName)
{
    // ensure each thread has its own requestID
    m_ID = VddkDeadlockCheck::GetInstance()->GenerateID(threadSpecificPtr_requestID, strApiName);
}

VddkDeadlockCheck::DeadlockCheck::~DeadlockCheck()
{
    VddkDeadlockCheck::GetInstance()->ReleaseID(m_ID);
}

VddkDeadlockCheck::VddkDeadlockCheck() : m_index(0)
{
    mp_int32 iRet = CConfigXmlParser::GetInstance().GetValueInt32(
        CFG_DATAPROCESS_SECTION, CFG_VDDKAPI_TIMEOUT, m_invokeTimeout);
    if (iRet != MP_SUCCESS) {
        m_invokeTimeout = VDDKAPI_MAX_TIMEOUT;
        COMMLOG(OS_LOG_WARN,
            "Unable to obtain vddk api timeout value from config file, will use default value: '%d'",
            m_invokeTimeout);
    }
}

VddkDeadlockCheck::~VddkDeadlockCheck()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    (void)m_deadlockCheckThread.Stop();
}

// Obtain a single VddkDeadlockCheck instance
VddkDeadlockCheck* VddkDeadlockCheck::GetInstance()
{
    static VddkDeadlockCheck* deadlockCheck = NULL;
    std::unique_lock<std::mutex> lock(m_mutex);
    if (NULL == deadlockCheck) {
        deadlockCheck = new (std::nothrow) VddkDeadlockCheck;
        if (NULL == deadlockCheck) {
            COMMLOG(OS_LOG_ERROR, "New VddkDeadlockCheck instance failed!");
        }
    }
    return deadlockCheck;
}

uint64_t VddkDeadlockCheck::GenerateID(const uint64_t& requestID, const std::string& strApiName)
{
    // Generate ID only when the deadlock thread is running
    {
        std::unique_lock<std::mutex> lock(m_mutex);
        // Make sure the deadlock thread is running
        if (!StartDeadlockThread()) {
            return 0;
        }

        uint64_t tempID;
        if (GenerateIDInner(requestID, strApiName, tempID)) {
            return tempID;
        }
    }

    // Too many tasks are waiting for the resource, will restart the deadlock thread
    KillProcess();

    return 0;
}

// Remove the id:apiinfo map element
void VddkDeadlockCheck::ReleaseID(const uint64_t& ID)
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_invokingApis.erase(ID);
}

void VddkDeadlockCheck::KillProcess()
{
    COMMLOG(OS_LOG_INFO, "Exception occurs, will exit current VddkDeadlockCheck process.");
    int ret = kill(getpid(), SIGKILL);
    if (ret) {
        COMMLOG(OS_LOG_INFO, "Current VddkDeadlockCheck process exits, ret: '%d'.", ret);
    }
}

bool VddkDeadlockCheck::StartDeadlockThread()
{
    if (!m_deadlockCheckThread.IsRunning()) {
        if (!m_deadlockCheckThread.Start()) {
            COMMLOG(OS_LOG_ERROR, "Fail to start the deadlock thread!");
            KillProcess();
            return false;
        }
        m_deadlockCheckThread.GetMessageloop()->PostTask(std::bind(&VddkDeadlockCheck::DoDeadlockCheck, this));
    }
    return true;
}

bool VddkDeadlockCheck::GenerateIDInner(const uint64_t requestID, const std::string& strApiName, uint64_t& tempID)
{
    for (unsigned int i = 0; i < MAX_TRY_TIMES; ++i) {
        uint64_t temp_id = ++m_index;
        if (m_invokingApis.end() == m_invokingApis.find(temp_id)) {
            st_vddkApiInfo apiInfo;
            apiInfo.invokeTime = std::chrono::steady_clock::now();
            apiInfo.requestID = requestID;
            apiInfo.strName = strApiName;
            apiInfo.timeout = VDDKAPI_MIN_TIMEOUT;
            m_invokingApis[temp_id] = std::move(apiInfo);
            tempID = temp_id;
            return true;
        }
    }
    return false;
}

EXTER_ATTACK void VddkDeadlockCheck::DoDeadlockCheck()
{
    COMMLOG(OS_LOG_INFO, "Deadlock check begin ...");
    std::unique_lock<std::mutex> lock(m_mutex);
    std::map<mp_uint64, st_vddkApiInfo>::iterator iter;
    for (iter = m_invokingApis.begin(); iter != m_invokingApis.end(); ++iter) {
        auto now = std::chrono::steady_clock::now();

        COMMLOG(OS_LOG_DEBUG,
            "Check vddk api['%s']'s invoking status: current time '%llu', \
            start time '%llu', id '%llu', request id '%llu'.",
            iter->second.strName.c_str(),
            now.time_since_epoch().count() / CHECK_DENO,
            iter->second.invokeTime.time_since_epoch().count() / CHECK_DENO,
            iter->first,
            iter->second.requestID);

        // for read and write operation, use the max value or value obtained from config file
        int tmpInvokeTimeout = iter->second.timeout;
        if (tmpInvokeTimeout < m_invokeTimeout) {
            tmpInvokeTimeout = m_invokeTimeout;
            COMMLOG(OS_LOG_WARN, "Timeout value will be set to '%d'[s].", tmpInvokeTimeout);
        }

        if ((now - iter->second.invokeTime) > std::chrono::seconds(tmpInvokeTimeout)) {
            COMMLOG(OS_LOG_WARN,
                "Timeout[%d] occurs when invoking vddk api '%s',  will kill current process!",
                tmpInvokeTimeout,
                iter->second.strName.c_str());
            KillProcess();
        }
    }

    if (m_deadlockCheckThread.IsRunning()) {
        m_deadlockCheckThread.GetMessageloop()->PostDelayedTask(
            std::bind(&VddkDeadlockCheck::DoDeadlockCheck, this), CHECK_PERIOD_IN_SECONDS * CHECK_PERIOD_IN_NUMS);
    }
}
