#include "servicecenter/thriftservice/detail/TServerEventHandlerImpl.h"

namespace thriftservice {
namespace detail {
void TServerEventHandlerImpl::preServe()
{
    m_promise->set_value(true);
}

void TServerEventHandlerImpl::SetPromise(std::shared_ptr<std::promise<bool>> p)
{
    m_promise = p;
}
}
}