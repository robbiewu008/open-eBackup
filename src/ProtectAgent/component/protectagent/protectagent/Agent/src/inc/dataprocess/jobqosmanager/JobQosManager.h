/*

 * Copyright (c) Huawei Technologies Co., Ltd. 2012-2019. All rights reserved.

 * Description: xxxxxxxxxxxxx

 * Author: x00351199

 * Create:  Sat Apr 20 06:14:06 2019 +0000

 * Notes:

 * History:

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