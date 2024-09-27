/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2021-2021. All rights reserved.
 *
 * @file Task.h
 * @author t00302329
 * @brief 任务执行体对象
 * @version 0.1
 * @date 2021-01-11
 *
 */
#ifndef __TASK_H__
#define __TASK_H__

#include <memory>
#include "common/Types.h"

class Task {
public:
    Task() : m_result(0), m_zero(false){};
    virtual ~Task() = default;

    /**
     *  @brief  任务执行体
     */
    virtual void Exec(){};
    
    /**
     *  @brief  任务执行结果
     */
    inline mp_int32 Result()
    {
        return m_result;
    }

    mp_void SetBuffer(std::shared_ptr<unsigned char[]> buffer)
    {
        m_buffer = buffer;
    }

    mp_string GetErrDesc()
    {
        return mp_string();
    }

    inline bool IsZero()
    {
        return m_zero;
    }

protected:
    mp_int32 m_result;
    bool m_zero;
    std::shared_ptr<unsigned char[]> m_buffer;
};

#endif