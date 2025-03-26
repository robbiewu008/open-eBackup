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