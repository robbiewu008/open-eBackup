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
#ifndef _MULTI_THREAD_BUS_H_
#define _MULTI_THREAD_BUS_H_
#include <iostream>
#include <thread>
#include <vector>

class MultiThreadBus
{
    bool m_operateFile; // �Ƿ�����ʵ�ļ����� 
    std::string m_testFileName; // ����ҵ��Ĳ����ļ� 
public:
    MultiThreadBus();
    ~MultiThreadBus();
    void startBackupBus(int num, const std::string &libPath);
    void startDeleteBus(int num, const std::string &libPath);
    void startRecoverBus(int num, const std::string &libPath);
    void setTestFileName(const std::string &fileName);
};

#endif