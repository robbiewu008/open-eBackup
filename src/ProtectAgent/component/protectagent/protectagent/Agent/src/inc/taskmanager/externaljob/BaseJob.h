#ifndef BASE_JOB_HEADER
#define BASE_JOB_HEADER
#include <memory>
#include <common/Types.h>

namespace  AppProtect {

class BaseJob : public std::enable_shared_from_this<BaseJob> {
public:
    BaseJob() {}
    virtual ~BaseJob() {}
    virtual mp_string GetJobId()
    {
        return m_jobId;
    }
    virtual mp_int32 Exec()=0;

protected:
    mp_string m_jobId;
};
}

#endif