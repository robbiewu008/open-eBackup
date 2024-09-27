/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: Linux KEYRING encapsulation
 * Author: z00316590
 * Create: 2019-03-07
 */
#ifdef WSEC_COMPILE_MIP_LINUX
#include "kmcv3_keyring.h"
#include <sys/types.h>
#include <linux/keyctl.h>
#include <asm/unistd.h>
#include <unistd.h>
#include <sys/syscall.h>

/* Adding a New Key to a Keyring */
long KmcKeyringAddKey(const char *type,
    const char *description,
    const void *payload, size_t plen,
    long keyringId)
{
    return syscall((long)__NR_add_key, type, description, payload, plen, keyringId);
}

/*
 * The key has been added to the request sent to the Keyring. This API is not used at present
 * but is inappropriate to be deleted because the normal key obtaining logic is KmcKeyringRequestKey.
 * This problem is introduced in Linux 3.8. In multi-thread scenarios,
 * thread A adds_key, and thread B requests_key fail.
 * The version later than V3.8 is available. The read and write permissions are granted to the USR.
 * The key is directly read instead of being read by KmcKeyringRequestKey.
 * After the Linux OS is rectified, if the affected kernel version is no longer used,
 * you can use KmcKeyringRequestKey again.
 */
long KmcKeyringRequestKey(const char *type,
    const char *description,
    const char *callout_info,
    long keyringId)
{
    return syscall((long)__NR_request_key, type, description, callout_info, keyringId);
}

/* Read the key from the key ring. */
long KmcKeyringReadKey(long id, unsigned char *buffer, size_t buflen)
{
    return syscall((long)__NR_keyctl, KEYCTL_READ, id, buffer, buflen);
}

/* Set the timeout interval. */
long KmcKeyringSetTimeOut(long id, long timeout)
{
    return syscall((long)__NR_keyctl, KEYCTL_SET_TIMEOUT, id, timeout);
}

/* Revocation of a key from a key ring */
long KmcKeyringRevoke(long id)
{
    return syscall((long)__NR_keyctl, KEYCTL_REVOKE, id);
}

/* Updating the Key Content */
long KmcKeyringUpdate(long id, unsigned char *keyBuff, size_t buffLen)
{
    return syscall((long)__NR_keyctl, KEYCTL_UPDATE, id, keyBuff, buffLen);
}
#endif
