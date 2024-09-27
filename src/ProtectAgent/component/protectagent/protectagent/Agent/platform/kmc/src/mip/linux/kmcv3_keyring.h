/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: KMC internal interface header file, which is not open to external systems.
 * Author: z00316590
 * Create: 2019-03-07
 */

#ifndef KMC_SRC_MIP_LINUX_KMCV3_KEYRING_H
#define KMC_SRC_MIP_LINUX_KMCV3_KEYRING_H

#include "wsecv2_type.h"

/* Adding a New Key to a Keyring */
long KmcKeyringAddKey(const char *type,
    const char *description,
    const void *payload, size_t plen,
    long keyringId);

/*
 * The key has been added to the request sent to the Keyring. This API is not used at present but is
 * inappropriate to be deleted because the normal key obtaining logic is KmcKeyringRequestKey.
 * This problem is introduced in Linux 3.8. In multi-thread scenarios,
 * thread A adds_key, and thread B requests_key fail.
 * The version later than V3.8 is available.The read and write permissions are granted to the USR.
 * The key is directly read instead of being read by KmcKeyringRequestKey.
 * After the bug is fixed in the Linux operating system, if the affected kernel version is no longer used,
 * you can use KmcKeyringRequestKey again.
 */
long KmcKeyringRequestKey(const char *type,
    const char *description,
    const char *callout_info,
    long keyringId);

/* Read the key from the key ring. */
long KmcKeyringReadKey(long id, unsigned char *buffer, size_t buflen);

/* Set the timeout interval. */
long KmcKeyringSetTimeOut(long id, long timeout);

/* Revocation of a key from a key ring */
long KmcKeyringRevoke(long id);

/* Updating the Key Content */
long KmcKeyringUpdate(long id, unsigned char *keyBuff, size_t buffLen);
#endif /* KMC_SRC_MIP_LINUX_KMCV3_KEYRING_H */
