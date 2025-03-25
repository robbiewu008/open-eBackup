#ifndef JOB_SERVICE_H_
#define JOB_SERVICE_H_

#include <servicecenter/servicefactory/include/IService.h>
#include <services/jobservice/include/IJobServer.h>
#include <memory>

namespace jobservice {
class JobServer : public IJobServer {
public:
    virtual ~JobServer(){};
    virtual bool Initailize();
    virtual bool Uninitailize();
private:
    void RegisterRpcObserver();
};
}
#endif