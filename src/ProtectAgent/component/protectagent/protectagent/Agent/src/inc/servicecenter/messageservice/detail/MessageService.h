#ifndef MESSAGESERVICE_H_
#define MESSAGESERVICE_H_

#include "servicecenter/messageservice/include/IMessageService.h"

namespace messageservice {
namespace detail {
class MessageService : public IMessageService {
public:
    virtual std::shared_ptr<ISubject> GetSubject();
    virtual bool Initailize();
    virtual bool Uninitailize();
};
}
}
#endif