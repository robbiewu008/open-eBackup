/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#ifndef WIN32
#include <sys/time.h>
#include <unistd.h>
#endif

#include <cerrno>
#include "log/Log.h"
#include "Snowflake.h"

#ifdef WIN32
#include <chrono>
#include <random>
using namespace std::chrono;
#endif

namespace Module {
	
const uint64_t MILLISECONDS_IN_SECOND = 1000;
const uint64_t NANOSECONDS_IN_MILLISECOND = 1000000;


Snowflake::Snowflake(void) : m_epoch(0), m_machine(0), m_sequence(0)
{}

Snowflake::~Snowflake(void)
{}

void Snowflake::SetEpoch(uint64_t epoch)
{
    m_epoch = epoch;
}

void Snowflake::SetMachine(size_t machine)
{
    m_machine = machine;
}

uint64_t Snowflake::GenerateId()
{
    std::lock_guard<std::mutex> lock(m_mtx);
    uint64_t value = 0;
    uint64_t time = GetCPUTime() - m_epoch;

    // time 37bit, Use specific values 26 and 16 to generate a globally unique ID;
    value |= time << 26;    // time 37bit, offset 26bit
    uint32_t seed = static_cast<uint32_t>(GetCPUTime());
#ifdef WIN32
    /* Hint: C++11 cross-platform pseudo-random number generation */
    int min = 0;
	int max = 15;
	std::default_random_engine randomEngine(seed);
	std::uniform_int_distribution<> uniformDistribution(min, max);
    uint32_t random = uniformDistribution(randomEngine)  % 16;    // random 4bit
#else
    uint32_t random = static_cast<uint32_t>(rand_r(&seed)) % 16;    // random 4bit
#endif
    value |= static_cast<uint64_t>((random & 0xF) << 22);           // random offset 22bit
    value |=
        static_cast<uint64_t>((static_cast<uint32_t>(m_machine) & 0x3FF) << 12);    // machine id 10bit,offset 12bit
    value |= static_cast<uint64_t>(static_cast<uint32_t>(m_sequence++) & 0xFFF);    // sequency 12bit,offset 0

    if (m_sequence == 0x1000) {
        m_sequence = 0;
    }
    return value;
}

uint64_t Snowflake::GetCPUTime()
{
#ifdef WIN32
    /* Hint: can be refactor using chrono -- a cross-platfrom lib */
    uint64_t time = duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count();
    return time;
#else
    struct timespec tv = { 0, 0 };
    int ret = clock_gettime(CLOCK_MONOTONIC, &tv);
    if (ret != 0) {
        ERRLOG("clock_gettime failed. errno=%d", errno);
    }
    uint64_t time = (uint64_t)tv.tv_sec * MILLISECONDS_IN_SECOND;
    time += (uint64_t)tv.tv_nsec / NANOSECONDS_IN_MILLISECOND;
    return time;
#endif
}

} // namespace Module
