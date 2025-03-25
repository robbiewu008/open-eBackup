/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Prouce meta&xmeta thread for object storage.
* Author: w00444223
* Create: 2023-12-04
*/

#include "ObjectProducerThread.h"
#include <functional>
#include "log/Log.h"
#include "ScanCommon.h"
#include "ObjectMetaProducer.h"

namespace {
    constexpr int NUMBER_ONE_HUNDRED = 100;
}

bool ObjectProducerThread::Start()
{
    ProduceParams args = {m_scanQueue, m_output, m_statsMgr, m_scanFilter, m_chkPntMgr};
    m_metaProducer = std::make_shared<ObjectMetaProducer>(args, m_config);
    if (m_metaProducer == nullptr) {
        ERRLOG("Meta producer of object make_shared failed.");
        return false;
    }
    if (m_metaProducer->InitContext() != SCANNER_STATUS::SUCCESS) {
        return false;
    }
    try {
        m_mainThread = std::make_shared<std::thread>(std::bind(&ObjectProducerThread::ProducerMain, this));
    } catch (std::exception &e) {
        ERRLOG("Exception when creating ThreadMain is: %s", e.what());
        return false;
    }
    return true;
}

void ObjectProducerThread::ProducerMain()
{
    HCPTSP::getInstance().reset(m_config.reqID);
    int flag = 0;
    while (true) {
        m_isMetaProducerExecuting = true;
        m_metaProducer->Produce(NUMBER_ONE_HUNDRED);
        m_isMetaProducerExecuting = false;
        if (m_exit) {
            break;
        }
    }
}

void ObjectProducerThread::Exit()
{
    m_exit = true;
    m_metaProducer->m_state = SCANNER_STATUS::ABORTED;
    if (m_mainThread->joinable()) {
        m_mainThread->join();
    }
    FS_SCANNER::MemoryTrim();
    INFOLOG("Exit.");
}
