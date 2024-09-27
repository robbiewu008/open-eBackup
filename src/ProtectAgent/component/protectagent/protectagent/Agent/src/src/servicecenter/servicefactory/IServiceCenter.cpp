#include "servicefactory/include/IServiceCenter.h"

namespace servicecenter {
std::shared_ptr<IServiceCenter> IServiceCenter::g_instance = nullptr;

std::shared_ptr<IServiceCenter> IServiceCenter::GetInstance()
{
    return g_instance;
}
}