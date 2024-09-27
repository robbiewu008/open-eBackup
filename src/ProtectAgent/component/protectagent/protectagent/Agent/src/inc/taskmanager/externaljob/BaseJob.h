/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file BaseJob.h
 * @brief Implement for base job
 * @version 1.1.0
 * @date 2021-10-29
 * @author wangguitao 00510599
 */
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