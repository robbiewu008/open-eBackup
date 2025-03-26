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