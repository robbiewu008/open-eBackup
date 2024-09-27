#ifndef I_JOB_SERVER_H_
#define I_JOB_SERVER_H_
#include <servicecenter/servicefactory/include/IService.h>

namespace jobservice {
class IJobServer : public servicecenter::IService {
public:
    virtual ~IJobServer(){};
};
}
#endif