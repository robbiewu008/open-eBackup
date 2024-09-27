/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file DataProcessClientHandler.cpp
 * @brief  Contains function declarations for DataProcessClientHandler
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
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
