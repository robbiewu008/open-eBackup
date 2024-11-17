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
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
#include <cerrno>
#include <cstring>
#include <ctime>
#include <sys/types.h>
#include <sys/stat.h>
#include <uuid/uuid.h>
#include <libgen.h>
#ifdef NO_POSIX_MEMALIGN
#include <malloc.h>
#endif

#include "dataprocess/ioscheduler/libvmfs6/Vmfs.h"

namespace Vmfs6IO {
/* Allocate a buffer with alignment compatible for direct I/O */
u_char *IOBufferAlloc(size_t len)
{
    size_t bufLen;
    void *buffer;

    bufLen = V6_ALIGN_NUMBER(len, V6_M_DIO_BLK_SIZE);
#ifdef NO_POSIX_MEMALIGN
    if (!(buffer = memalign(V6_M_DIO_BLK_SIZE, bufLen))) {
#else
    if (posix_memalign((void **)&buffer, V6_M_DIO_BLK_SIZE, bufLen)) {
#endif
        return nullptr;
    }

    return (u_char *)buffer;
}

/* Free a buffer previously allocated by IOBufferAlloc() */
void IOBufferFree(u_char *buffer)
{
    if (buffer != NULL) {
        free(buffer);
    }
}

/* Read from file descriptor at a given offset */
ssize_t PReadFromFD(int fd, void *buffer, size_t count, off_t offset)
{
    int maxRetries = 10;
    u_char *ptr = (u_char *)buffer;
    size_t hlen = 0;
    ssize_t len;

    while (hlen < count) {
        len = pread(fd, ptr, count - hlen, offset + hlen);
        if (len < 0) {
            if (EIO == errno) {
                if (maxRetries-- == 0) {
                    return -1;
                }
                continue;
            }

            if (EINTR != errno) {
                return -1;
            }
        } else {
            if (len == 0) {
                break;
            }

            hlen += len;
            ptr += len;
        }
    }

    return hlen;
}
}