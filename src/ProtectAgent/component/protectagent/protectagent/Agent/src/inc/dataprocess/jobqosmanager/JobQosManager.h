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
#ifndef __JOB_QOS_MANAGER_H__
#define __JOB_QOS_MANAGER_H__
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <vector>
#include <string>
using std::string;

extern const string NODE_QOS_UP;
extern const string NODE_QOS_DOWN;
extern const string PRODUCT_NODE_QOS_UP;
extern const string PRODUCT_NODE_QOS_DOWN;

class JobQosManager {
public:
    JobQosManager();
    ~JobQosManager()
    {}
    void SetJobQos(double qos);
    double GetJobQos();
    static std::shared_ptr<JobQosManager> GetJobQosManager(const string& identity);
    static void UnRegisterQos(const string& identity);
    bool ConsumeQos(double require, int timeout);
    void StopQosLimit();
    void StartQosLimit();
    double GetJobSpeed();
    static void SetProduceFrequency(unsigned int frequency);
    static void Init();

private:
    void ProduceQos();
    static void ThreadFun();
    void TryGetEnoughQos(double require, double& obtain);

private:
    std::mutex m_Mutex;
    static std::thread m_Thread;
    std::condition_variable m_Cond;
    static std::vector<std::shared_ptr<JobQosManager>> JobQosManagerArray;
    double m_ProduceQos;
    double m_Qos;
    double m_ConsumeQos;
    bool m_Stoped;
    static unsigned int Frequency;
    string m_Name;
    uint32_t m_ProduceTimes;
    double m_Speed;
    double m_LastConsumQos;
    static std::mutex m_Gmutex;
};
#endif