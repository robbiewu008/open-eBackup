#ifndef IMESSAGESERVICE_H_
#define IMESSAGESERVICE_H_

#include "servicecenter/servicefactory/include/IService.h"
#include "servicecenter/messageservice/include/ISubject.h"

namespace messageservice {
class IMessageService : public servicecenter::IService {
public:
    virtual ~IMessageService() = default;
    virtual std::shared_ptr<ISubject> GetSubject() = 0;
};
}
#endif