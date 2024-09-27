#include <messageservice/detail/MessageService.h>
#include <messageservice/detail/Subject.h>
#include <servicefactory/include/ServiceFactory.h>

namespace messageservice {
namespace detail {
std::shared_ptr<ISubject>  MessageService::GetSubject()
{
    return std::make_shared<Subject>();
}

bool MessageService::Initailize()
{
    return true;
}

bool MessageService::Uninitailize()
{
    return true;
}
}
}