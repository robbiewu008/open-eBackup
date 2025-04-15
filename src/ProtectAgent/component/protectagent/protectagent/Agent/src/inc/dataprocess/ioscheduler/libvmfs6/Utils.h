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
#ifndef __VMFS6_UTILS_H__
#define __VMFS6_UTILS_H__

#include <cstdio>
#include <cstring>
#include <uuid/uuid.h>
#include <inttypes.h>
#include <securec.h>


namespace Vmfs6IO {

const int NUM_TWO = 2;
const int NUM_THREE = 3;
const int NUM_FOUR = 4;
const int NUM_FIVE = 5;
const int NUM_SIX = 6;
const int NUM_SEVEN = 7;
const int NUM_EIGHT = 8;
const int NUM_NINE = 9;
const int NUM_16 = 16;
const int NUM_24 = 24;
const int NUM_32 = 32;
const int NUM_446 = 446;
const int NUM_510 = 510;
const int NUM_512 = 512;
const int NUM_1024 = 1024;
const int VMFS6_M_SECTOR_SIZE = 512;
const int VMFS6_M_BLK_SIZE = 4096;
const int NUM_TIME_OUT = 5000;
const int NUM_8192 = 8192;

/* Max and min macro */
#define M_MAX(a, b) (((a) > (b)) ? (a) : (b))
#define M_MIN(a, b) (((a) < (b)) ? (a) : (b))


#if defined(__amd64__) || defined(__i386__)
#define LE_AND_NO_ALIGN 1
#endif

/* Read a 16-bit word in little endian format */
inline uint16_t ReadLE16(const u_char *p, int offset)
{
#ifdef LE_AND_NO_ALIGN
    return (*((uint16_t *)&p[offset]));
#else
    return ((uint16_t)p[offset] | ((uint16_t)p[offset + 1] << NUM_EIGHT));
#endif
}

/* Read a 32-bit word in little endian format */
inline uint32_t ReadLE32(const u_char *p, int offset)
{
#ifdef LE_AND_NO_ALIGN
    return (*((uint32_t *)&p[offset]));
#else
    return ((uint32_t)p[offset] | ((uint32_t)p[offset + 1] << NUM_EIGHT) | ((uint32_t)p[offset + NUM_TWO] << NUM_16) |
        ((uint32_t)p[offset + NUM_THREE] << NUM_24));
#endif
}

/* Write a 32-bit word in little endian format */
inline void WriteLE32(u_char *p, int offset, uint32_t val)
{
#ifdef LE_AND_NO_ALIGN
    *(uint32_t *)&p[offset] = val;
#else
    p[offset] = val & 0xFF;
    p[offset + 1] = val >> NUM_EIGHT;
    p[offset + NUM_TWO] = val >> NUM_16;
    p[offset + NUM_THREE] = val >> NUM_24;
#endif
}

/* Read a 64-bit word in little endian format */
inline uint64_t ReadLE64(const u_char *p, int offset)
{
#ifdef LE_AND_NO_ALIGN
    return (*((uint64_t *)&p[offset]));
#else
    return ((uint64_t)ReadLE32(p, offset) + ((uint64_t)ReadLE32(p, offset + NUM_FOUR) << NUM_32));
#endif
}

/* Write a 64-bit word in little endian format */
inline void WriteLE64(u_char *p, int offset, uint64_t val)
{
#ifdef LE_AND_NO_ALIGN
    *(uint64_t *)&p[offset] = val;
#else
    WriteLE32(p, offset, val);
    WriteLE32(p, offset + NUM_FOUR, val);
#endif
}

/* Read an UUID at a given offset in a buffer */
inline bool ReadUuid(const u_char *buf, int offset, uuid_t *uuid)
{
    if (memcpy_s(uuid, sizeof(uuid_t), buf + offset, sizeof(uuid_t)) == 0) {
        return true;
    }
    return false;
}

/* Write an UUID to a given offset in a buffer */
inline bool WriteUuid(u_char *buf, int offset, const uuid_t *uuid)
{
    if (memcpy_s(buf + offset, sizeof(uuid_t), uuid, sizeof(uuid_t)) == 0) {
        return true;
    }
    return false;
}


#define V6_M_DIO_BLK_SIZE 4096

#define V6_ALIGN_CHECK(val, mult) (((val) & ((mult)-1)) == 0)
#define V6_ALIGN_NUMBER(val, mult) (((val) + ((mult)-1)) & ~(((mult)-1)))
#define V6_ALIGN_PTR(ptr, mult) (void *)V6_ALIGN_NUMBER((uintptr_t)(ptr), mult)

#define V6_DECL_ALIGNED_BUFFER(name, size)                                  \
    u_char __##name[(size) + VMFS6_M_SECTOR_SIZE];                          \
    u_char *(name) = (u_char *)V6_ALIGN_PTR(__##name, VMFS6_M_SECTOR_SIZE); \
    size_t name##_len = (size)

#define V6_DECL_ALIGNED_BUFF_WOL(name, size)     \
    u_char __##name[(size) + VMFS6_M_SECTOR_SIZE]; \
    u_char *(name) = (u_char *)V6_ALIGN_PTR(__##name, VMFS6_M_SECTOR_SIZE)

/* Allocate a buffer with alignment compatible for direct I/O */
u_char *IOBufferAlloc(size_t len);

/* Free a buffer previously allocated by IOBufferAlloc() */
void IOBufferFree(u_char *buf);

/* Read from file descriptor at a given offset */
ssize_t PReadFromFD(int fd, void *buf, size_t count, off_t offset);
}

#endif
