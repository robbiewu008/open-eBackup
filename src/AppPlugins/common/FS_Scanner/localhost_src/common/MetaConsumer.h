/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2022. All rights reserved.
 * Author: z30016470
 * Create: 2022/07/05
 */
#ifndef FS_SCANNER_META_CONSUMER_H
#define FS_SCANNER_META_CONSUMER_H

#include "BufferQueue.h"
#include "ScanStructs.h"
#include "CommonService.h"
#include "FSScannerCheckPoint.h"

class MetaConsumer {
public:

    explicit MetaConsumer(std::shared_ptr<BufferQueue<DirectoryScan>> buffer,
        std::shared_ptr<CommonService> service) : m_buffer(buffer), m_service(service)
    {};
    MetaConsumer() {};
    ~MetaConsumer() {};

    void Consume(int count);

private:
    std::shared_ptr<BufferQueue<DirectoryScan>> m_buffer {};
    std::shared_ptr<CommonService> m_service {};
};

#endif