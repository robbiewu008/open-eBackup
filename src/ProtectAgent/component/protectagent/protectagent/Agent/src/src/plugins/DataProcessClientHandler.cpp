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
#include "plugins/DataProcessClientHandler.h"
#include "apps/vmwarenative/VMwareDef.h"

using namespace std;
namespace {
    const uint32_t INIT_DP_RETRY_TIMES = 3;
}

// Init the single instance
DataProcessClientHandler DataProcessClientHandler::singleInst;

DataProcessClientHandler::DataProcessClientHandler()
{
    CMpThread::InitLock(&m_mapLock);
}

DataProcessClientHandler::~DataProcessClientHandler()
{
    {
        CThreadAutoLock tlock(&m_mapLock);
        while (m_mapDpProcessClientInstance.size() > 0) {
            delete m_mapDpProcessClientInstance.begin()->second;
            m_mapDpProcessClientInstance.erase(m_mapDpProcessClientInstance.begin());
        }
    }

    CMpThread::DestroyLock(&m_mapLock);
}

mp_void DataProcessClientHandler::RemoveDpServiceClient(const mp_string& version)
{
    CThreadAutoLock tlock(&m_mapLock);
    m_mapDpProcessClientInstance.erase(version);
}

// must be include in map lock scope
DataPathProcessClient* DataProcessClientHandler::GenerateDpServiceClientMap(
    mp_int32 serviceType, const mp_string& dpParam)
{
    INFOLOG("dataprocess client(%s) does not exist, will create.", dpParam.c_str());
    DataPathProcessClient* pDpClient = new (std::nothrow) DataPathProcessClient(serviceType, dpParam);
    if (pDpClient == NULL) {
        ERRLOG("New instance of class DataPathProcessClient fail!");
        return NULL;
    }

    int retryTimes = 0;
    while (retryTimes <= INIT_DP_RETRY_TIMES) {
        if (MP_SUCCESS == pDpClient->Init()) {
            return pDpClient;
        } else {
            ERRLOG("Init instance of class DataPathProcessClient fail once , retried times: %d",
                retryTimes);
            retryTimes++;
        }
    }
    ERRLOG("Init instance of class DataPathProcessClient fail!");
    delete pDpClient;
    return NULL;
}

DataPathProcessClient* DataProcessClientHandler::FindDpClient(const mp_string& version)
{
    CThreadAutoLock tlock(&m_mapLock);
    INFOLOG("dataprocess client map size is:%u", m_mapDpProcessClientInstance.size());

    std::map<mp_string, DataPathProcessClient*>::iterator iter = m_mapDpProcessClientInstance.find(version);
    if (iter == m_mapDpProcessClientInstance.end()) {
        DataPathProcessClient* pDpClient = GenerateDpServiceClientMap(VMWARENATIVE_TYPEOFSERVICE, version);
        if (pDpClient == NULL) {
            ERRLOG("Unable to init vmware native dataprocess client");
            return NULL;
        }

        m_mapDpProcessClientInstance.insert(std::pair<mp_string, DataPathProcessClient*>(version, pDpClient));
        INFOLOG("Init data process client(%s) for vmware native protection successfully.", version.c_str());
        return pDpClient;
    } else {
        return iter->second;
    }
}
