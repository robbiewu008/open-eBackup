/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file MutexLock.cpp
 * @brief  Contains function declarations MutexLock Operations
 * @version 1.0.0
 * @date 2020-08-01
 * @author wangguitao 00510599
 */
#include "dataprocess/vmwarenative/MutexLock.h"

using namespace AGENT_VMWARENATIVE_MUTEXLOCK;

MutexLock::MutexLock()
{
    // create mutex with default attr
    pthread_mutex_init(&m_mutex, NULL);
}

MutexLock::~MutexLock()
{
    pthread_mutex_destroy(&m_mutex);
}

void MutexLock::lock()
{
    pthread_mutex_lock(&m_mutex);
}

void MutexLock::unlock()
{
    pthread_mutex_unlock(&m_mutex);
}

pthread_mutex_t *MutexLock::getPthreadMutex()
{
    return &m_mutex;
}
