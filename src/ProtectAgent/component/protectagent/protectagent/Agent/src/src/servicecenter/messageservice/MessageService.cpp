/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file MessageService.cpp
 * @brief  implement for MessageService
 * @version 1.1.0
 * @date 2021-08-31
 * @author caomin 00511255
 */

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