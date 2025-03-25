/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: z30016470
 * Create: 2022/07/05
 */

#ifndef FS_SCANNER_CONSUMER_THREAD_H
#define FS_SCANNER_CONSUMER_THREAD_H

#include "MetaConsumer.h"

class ConsumerThread {
public:
    bool m_isConsumerExecuting = false;

    explicit ConsumerThread(std::shared_ptr<BufferQueue<DirectoryScan>> buffer,
        std::shared_ptr<CommonService> service, std::shared_ptr<FSScannerCheckPoint> chkPntMgr,
        std::shared_ptr<StatisticsMgr> statsMgr)
        : m_buffer(buffer), m_service(service), m_chkPntMgr(chkPntMgr), m_statsMgr(statsMgr)
    {}
    ~ConsumerThread() {}

    bool Start();
    void ConsumerMain();
    void Exit();

private:
    bool m_exit = false;
    std::shared_ptr<std::thread> m_mainThread {};
    MetaConsumer m_metaConsumer;
    std::shared_ptr<BufferQueue<DirectoryScan>> m_buffer {};
    std::shared_ptr<CommonService> m_service {};
    std::shared_ptr<FSScannerCheckPoint> m_chkPntMgr {};
    std::shared_ptr<StatisticsMgr> m_statsMgr;
};

#endif