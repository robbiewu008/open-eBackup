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
#include "NfsContextContainer.h"

namespace {
    constexpr auto MODULE = "NFS_CONTEXT_CONTAINER";
}

using namespace std;
namespace Module {

NfsContextContainer::NfsContextContainer(std::size_t reqId)
    : m_reqId(reqId)
{}

NfsContextContainer::~NfsContextContainer() {}

std::shared_ptr<NfsContextWrapper> NfsContextContainer::Get(int id)
{
    return m_nfsContexts.at(id);
}

void NfsContextContainer::Insert(int id, std::shared_ptr<NfsContextWrapper> nfsContext)
{
    std::lock_guard<std::mutex> lk(mtx);
    m_nfsContexts.insert(std::make_pair(id, nfsContext));
}

std::shared_ptr<NfsContextWrapper> NfsContextContainer::Traverse()
{
    std::lock_guard<std::mutex> lk(mtx);
    if (m_currentIter == m_nfsContexts.end()) {
        m_currentIter = m_nfsContexts.begin();
    }
    std::shared_ptr<NfsContextWrapper> nfsContext = m_currentIter->second;
    m_currentIter++;
    return nfsContext;
}

std::shared_ptr<NfsContextWrapper> NfsContextContainer::GetCurrContext()
{
    std::lock_guard<std::mutex> lk(mtx);
    if (m_currentIter == m_nfsContexts.end()) {
        m_currentIter = m_nfsContexts.begin();
    }
    return m_currentIter->second;
}

std::map<int, std::shared_ptr<NfsContextWrapper>>::size_type NfsContextContainer::Size()
{
    return m_nfsContexts.size();
}

int64_t NfsContextContainer::GetCurTime()
{
    timeval curTime {};
    gettimeofday(&curTime, nullptr);
    int64_t milli = (curTime.tv_sec * uint64_t(MAX_CNT)) + (curTime.tv_usec / MAX_CNT);
    return milli;
}

int32_t NfsContextContainer::DestroyNfsContext()
{
    for (auto &nfsContext : m_nfsContexts) {
        nfsContext.second->NfsDestroyContext();
    }
    m_nfsContexts.clear();
    m_sendReqCount = 0;
    HCP_Log(INFO, MODULE) << "Damn Nfs context destroyed, currJobCnt: " << m_currJobCount
                        << ", totJobCnt: " << m_totJobCount <<  HCPENDLOG;
    return SUCCESS;
}

void NfsContextContainer::PrintCounters(const std::string containerType)
{
    for (auto &nfsContext : m_nfsContexts) {
        nfsContext.second->PrintRpcCounters(containerType);
    }

    HCP_Log(DEBUG, MODULE) << "TimedNfsServiceCount:" << m_timedNfsServiceCount
        << ", m_currJobCount: " << m_currJobCount << ", m_totJobCount: " << m_totJobCount << HCPENDLOG;
}

}
