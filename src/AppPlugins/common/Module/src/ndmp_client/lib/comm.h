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
#ifndef __MODULE_COMM_H__
#define __MODULE_COMM_H__

#define MODULE_FREE(p) do \
    {\
        if (NULL != p) \
            free(p);\
        p = NULL;        \
    } while (0)

#define CHECK_RESULT_RETURN(r, n, fmt) \
    do { \
        if ((r) != (n)) { \
            ERRLOG("%s, ret:%d" fmt, r); \
            return (r); \
        } \
    } while (0)
 
#define CHECK_RESULT_GOTO(r, n, l)\
    do {  \
        if ((r) != (n)) { \
            ERRLOG("Check result(%d) failed.", (int)(r)); \
            goto l;  \
        } \
    } while (0)
 
#define CHECK_NULL_POINTER_GOTO(p, l) \
    do { \
        if ((p) == NULL) { \
            ERRLOG("NULL POINTER ERR."); \
            goto l; \
        } \
    } while (0)
 
#define CHECK_NULL_POINTER_RETURN(p, r) \
    do { \
        if ((p) == NULL) { \
            ERRLOG("NULL POINTER ERR, ret:%d.", (int)(r)); \
            return (r); \
        } \
    } while (0)

#define CHECK_MEMCPY(r ,n) \
    do { \
        if ((r) != (n)) { \
            ERRLOG("memcpy_failed, ret:%d.", (int)(r)); \
        } \
    } while (0)
 
void close_thread(pthread_t *pt);
#define IPV4_ADDRSTRLEN 48;
 
#endif