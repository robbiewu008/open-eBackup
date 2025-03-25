#ifndef AGENT_CONNECTIVITY_MANAGER_H
#define AGENT_CONNECTIVITY_MANAGER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <functional>
#include <deque>
#include "common/Types.h"
#include "common/Defines.h"

struct IpCheckPair {
    std::string srcIp;
    std::string dstIp;
    uint32_t srcPort { 0 };
    uint32_t dstPort { 0 };
};

struct IpCheckResult {
    IpCheckPair ipPair;
    bool isConnected { false };
};

struct IpConnectChecker {
public:
    void Wait()
    {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait_for(lock, std::chrono::seconds(waitTime), [this] { return !waitFlag; });
        if (waitFlag) {
            timeOutFlag = true;
        }
    }

    void AddCheckResult(IpCheckResult& result)
    {
        std::unique_lock<std::mutex> lock(resultMtx);
        results.push_back(result);
        if (results.size() >= checkIpCount) {
            Notify();
        }
    }

    void Notify()
    {
        std::unique_lock<std::mutex> lock(mtx);
        waitFlag = false;
        cv.notify_one();
    }

    bool IsTimeOut()
    {
        return timeOutFlag;
    }

    void SetCheckIpCount(size_t count)
    {
        checkIpCount = count;
    }

    std::list<IpCheckResult>& GetCheckResults()
    {
        return results;
    }
private:
    std::mutex mtx;
    std::condition_variable cv;
    bool waitFlag {true};
    uint32_t waitTime {300};

    bool timeOutFlag {false};
    uint32_t checkIpCount = {0};

    std::mutex resultMtx;
    std::list<IpCheckResult> results;
};

struct IpCheckRequest {
    IpCheckPair ipPair;
    std::shared_ptr<IpConnectChecker> checker { nullptr };
};

class ConnectivityManager {
public:
    static ConnectivityManager& GetInstance();

    ~ConnectivityManager();
    std::vector<std::string> GetConnectedIps(const std::vector<std::string>& dstIps, uint32_t port);

private:
    ConnectivityManager() {};
    bool IfDuplicatedCheckingIsRunning(std::vector<std::string>& runningIps, uint32_t port);
    std::vector<std::string> DoGetConnectedIps(const std::vector<std::string>& runningIps, uint32_t port);
    void Init();
    void UnInit();
    bool IfIpVectorsSame(const std::vector<std::string>& v1, const std::vector<std::string>& v2);
    void AddCheckRequest(const IpCheckRequest& request);
    IpCheckRequest GetCheckRequest();

    void CheckThreadRun();

    std::atomic<bool> m_initFlag { false };

    std::vector<std::shared_ptr<std::thread>> m_wokers {nullptr};
    std::atomic<bool> m_exitFlag {false};
    unsigned int m_workerNum = 30;

    std::deque<IpCheckRequest> m_requestQueue;
    std::mutex m_requestMtx;
    std::mutex m_initMtx;
    std::condition_variable m_requestCv;
    std::condition_variable m_consumerCond;
    std::condition_variable m_producerCond;
    const unsigned int m_queueSize = 30;
    std::mutex m_runningCheckingMtx;
    std::map<uint64_t, std::map<std::vector<std::string>, std::vector<std::string>>> m_checkingResults;
    std::map<uint64_t, std::set<std::vector<std::string>>> m_runningCheckings;
};
 
#endif // AGENT_CONNECTIVITY_MANAGER_H