#ifndef _MULTI_THREAD_BUS_H_
#define _MULTI_THREAD_BUS_H_
#include <iostream>
#include <thread>
#include <vector>

class MultiThreadBus
{
    bool m_operateFile; // 是否有真实文件操作 
    std::string m_testFileName; // 用于业务的测试文件 
public:
    MultiThreadBus();
    ~MultiThreadBus();
    void startBackupBus(int num, const std::string &libPath);
    void startDeleteBus(int num, const std::string &libPath);
    void startRecoverBus(int num, const std::string &libPath);
    void setTestFileName(const std::string &fileName);
};

#endif