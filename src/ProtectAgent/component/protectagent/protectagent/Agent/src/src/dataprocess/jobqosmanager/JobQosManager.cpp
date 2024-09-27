#include <cmath>
#include <chrono>
#include <sys/time.h>
#include "common/Log.h"
#include "dataprocess/jobqosmanager/JobQosManager.h"

std::vector<std::shared_ptr<JobQosManager>> JobQosManager::JobQosManagerArray;
unsigned int JobQosManager::Frequency = 1;
std::mutex JobQosManager::m_Gmutex;
namespace {
const string NODE_QOS_UP = "UP";
const string NODE_QOS_DOWN = "DOWN";
const string PRODUCT_NODE_QOS_UP = "PRODUCT_UP";
const string PRODUCT_NODE_QOS_DOWN = "PRODUCT_DOWN";
constexpr int DISTANCE = 0.0001;
constexpr int MAX_COUNT = 2000000;
constexpr int MAX_BANDWIDTH = 3;
constexpr int TIME_OUT = 600;
constexpr double DEFAULT_QOS_SPEED = 300;
}  // namespace

JobQosManager::JobQosManager()
{
    m_ProduceQos = 0;
    m_Qos = DEFAULT_QOS_SPEED;
    m_ConsumeQos = 0;
    m_Stoped = true;
    m_ProduceTimes = 0;
    m_Speed = 0;
    m_LastConsumQos = 0;
}

void JobQosManager::SetJobQos(double qos)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    COMMLOG(OS_LOG_INFO, "JobQosManager '%s' set node qos = '%f'.", m_Name.c_str(), qos);
    m_Qos = qos;
}

double JobQosManager::GetJobQos()
{
    return m_Qos;
}

void JobQosManager::Init()
{
    {
        static bool isInited = false;
        std::lock_guard<std::mutex> lock(m_Gmutex);
        if (isInited == false) {
            std::thread thread(&JobQosManager::ThreadFun);
            thread.detach();
            isInited = true;
        }
    }
}

std::shared_ptr<JobQosManager> JobQosManager::GetJobQosManager(const string& identity)
{
    std::lock_guard<std::mutex> lock(m_Gmutex);
    std::vector<std::shared_ptr<JobQosManager>>::iterator iter = JobQosManagerArray.begin();
    while (iter != JobQosManagerArray.end()) {
        if (strcmp(((*iter)->m_Name).c_str(), identity.c_str()) == 0) {
            return (*iter);
        }
        ++iter;
    }
    INFOLOG("JobQosManager create a new JobQosManager '%s'.", identity.c_str());
    std::shared_ptr<JobQosManager> pJobQosManager = std::make_shared<JobQosManager>();
    if (pJobQosManager == NULL) {
        return NULL;
    }
    pJobQosManager->m_Name = identity;
    JobQosManagerArray.push_back(pJobQosManager);
    return pJobQosManager;
}

void JobQosManager::UnRegisterQos(const string& identity)
{
    INFOLOG("Begin to unregisterQos(%s)", identity.c_str());
    std::lock_guard<std::mutex> lock(m_Gmutex);
    std::vector<std::shared_ptr<JobQosManager>>::iterator iter = JobQosManagerArray.begin();
    while (iter != JobQosManagerArray.end()) {
        if (strcmp(((*iter)->m_Name).c_str(), identity.c_str()) == 0) {
            std::shared_ptr<JobQosManager> pJOb = (*iter);
            JobQosManagerArray.erase(iter);
            INFOLOG("JobQosManager Delete JobQosManager(%s)", identity.c_str());
            break;
        }
        ++iter;
    }
}

void JobQosManager::ThreadFun()
{
    std::vector<std::shared_ptr<JobQosManager>>::iterator iter;
    while (true) {
        {
            std::lock_guard<std::mutex> lock(m_Gmutex);
            iter = JobQosManagerArray.begin();
            while (iter != JobQosManagerArray.end()) {
                (*iter)->ProduceQos();
                ++iter;
            }
        }
        constexpr int US_PER_SECOND = 1000000;
        const int qosProducerCheckInterval = US_PER_SECOND / Frequency;
        usleep(qosProducerCheckInterval);
    }
}

void JobQosManager::ProduceQos()
{
    bool bCheck = (m_Stoped || Frequency == 0);
    if (bCheck) {
        return;
    }

    m_ProduceTimes++;
    // begin new count interval
    if (m_ProduceTimes == 1) {
        m_LastConsumQos = m_ConsumeQos;
        if (m_LastConsumQos > MAX_COUNT) {
            m_LastConsumQos = m_LastConsumQos - MAX_COUNT;
        }
    }

    // arrive at a full count interval
    if (m_ProduceTimes == Frequency + 1) {
        m_Speed = m_ConsumeQos - m_LastConsumQos;
        if (m_ConsumeQos < m_LastConsumQos) {
            m_Speed = m_ConsumeQos + MAX_COUNT - m_LastConsumQos;
        }
        m_LastConsumQos = m_ConsumeQos;
        if (m_LastConsumQos > MAX_COUNT) {
            m_LastConsumQos = m_LastConsumQos - MAX_COUNT;
        }
        m_ProduceTimes = 1;
        if (m_ProduceQos > m_ConsumeQos) {
            m_ProduceQos = m_ConsumeQos;
        }
    }

    // produce new value for consuming
    m_ProduceQos += m_Qos / Frequency;
    if (m_ProduceQos - m_ConsumeQos > MAX_BANDWIDTH * m_Qos) {
        m_ProduceQos -= m_Qos / Frequency;
    }
    if (m_ProduceQos > MAX_COUNT) {
        m_ProduceQos = m_ProduceQos - MAX_COUNT;
    }

    m_Cond.notify_all();
}

void JobQosManager::TryGetEnoughQos(double require, double& obtain)
{
    // double value revert
    if (m_ConsumeQos - m_ProduceQos > MAX_BANDWIDTH * m_Qos) {
        if (m_ProduceQos + MAX_COUNT - m_ConsumeQos >= (require - obtain)) {
            m_ConsumeQos += (require - obtain);
            if (m_ConsumeQos > MAX_COUNT) {
                m_ConsumeQos = m_ConsumeQos - MAX_COUNT;
            }
            obtain = require;
        } else {
            obtain += (m_ProduceQos + MAX_COUNT - m_ConsumeQos);
            m_ConsumeQos = m_ProduceQos;
        }
    } else {
        // product Qos have meet the consume requirment
        if (m_ProduceQos - m_ConsumeQos >= (require - obtain)) {
            m_ConsumeQos += (require - obtain);
            if (m_ConsumeQos > MAX_COUNT) {
                m_ConsumeQos = m_ConsumeQos - MAX_COUNT;
            }
            obtain = require;
            DBGLOG("Get qos num %f.", obtain);
        } else {
            // wait for next produce
            obtain += (m_ProduceQos - m_ConsumeQos);
            // make sure the condition: (abs(m_ConsumeQos - m_ProduceQos) < DISTANCE)
            m_ConsumeQos = m_ProduceQos;
            DBGLOG("Wait for next produce qos.");
        }
    }
}

bool JobQosManager::ConsumeQos(double require, int timeout)
{
    if (m_Stoped) {
        return true;
    }
    double obtain = 0;
    struct timeval start_time;
    struct timeval end_time;
    gettimeofday(&start_time, NULL);
    while (obtain < require) {
        std::unique_lock<std::mutex> lock(m_Mutex);
        while (abs(m_ConsumeQos - m_ProduceQos) < DISTANCE) {
            if (m_Stoped) {
                INFOLOG("stop qosmanager, id=%s.", m_Name.c_str());
                return true;
            }
            gettimeofday(&end_time, NULL);
            if (timeout != -1 && (end_time.tv_sec - start_time.tv_sec) > timeout) {
                return false;
            }
            if (m_Cond.wait_for(lock, std::chrono::seconds(TIME_OUT)) == std::cv_status::timeout) {
                return false;
            }
        }

        TryGetEnoughQos(require, obtain);

        if (timeout != -1) {
            gettimeofday(&end_time, NULL);
            if ((end_time.tv_sec - start_time.tv_sec) > timeout) {
                return false;
            }
        }
        
        if (m_Stoped) {
            INFOLOG("stop qosmanager, id=%s.", m_Name.c_str());
            return true;
        }
    }
    return true;
}

void JobQosManager::SetProduceFrequency(unsigned int frequency)
{
    if (frequency <= 0) {
        return;
    }
    Frequency = frequency;
}

double JobQosManager::GetJobSpeed()
{
    return m_Speed;
}

void JobQosManager::StopQosLimit()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    INFOLOG("Stop Qos, id:%s.", m_Name.c_str());
    m_Stoped = true;
}

void JobQosManager::StartQosLimit()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    INFOLOG("Start Qos, id:%s.", m_Name.c_str());
    m_Stoped = false;
}
