/*
* Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
* Description: Prouce meta&xmeta thread for object storage.
* Author: w00444223
* Create: 2023-12-04
*/

#ifndef FS_SCANNER_OBJECT_PRODUCER_THREAD_H
#define FS_SCANNER_OBJECT_PRODUCER_THREAD_H
#include "ProducerThread.h"
#include "ScanCommon.h"

class ObjectProducerThread : public ProducerThread {
public:
    explicit ObjectProducerThread(ProduceParams args, ScanConfig config)
        : ProducerThread(args.scanQueue, args.output, args.statsMgr, args.scanFilter, args.chkPntMgr), m_config(config)
    {};
    ObjectProducerThread() {};
    ~ObjectProducerThread() override {};

    bool Start() override;
    void ProducerMain() override;
    void Exit() override;

private:
    ScanConfig m_config {};
};

#endif