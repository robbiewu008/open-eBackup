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