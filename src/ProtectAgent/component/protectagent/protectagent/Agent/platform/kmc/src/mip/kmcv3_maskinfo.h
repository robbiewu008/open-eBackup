/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2020. All rights reserved.
 * Description: KMC internal interface header file, which is not open to external systems.
 * Author: z00316590
 * Create: 2019-03-07
 */

#ifndef KMC_SRC_MIP_KMCV3_MASKINFO_H
#define KMC_SRC_MIP_KMCV3_MASKINFO_H

#include "wsecv2_type.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

#define KMC_MASKCODE_LENGTH 128
/*
 * 2: Only the first part is used, and the last part is used for verification.
 */
#define KMC_MASKCODE_KEY_LENGTH (2 * KMC_MASKCODE_LENGTH)
/* Initialize the mask mechanism. */
unsigned long InitMaskCode(void);

/* Protecting Memory Data */
unsigned long ProtectData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen);

/* Disabling Memory Data Protection */
unsigned long UnprotectData(const unsigned char *datain, unsigned int inlen,
    unsigned char *dataout, unsigned int *outlen);

/* Protects memory data to the same buffer. */
unsigned long ProtectDataSameBuf(unsigned char *data, unsigned int len);

/* Unprotecting the Memory Data to the Same Buffer */
unsigned long UnprotectDataSameBuf(unsigned char *data, unsigned int len);

/* Deinitialize the mask mechanism. */
void UninitMaskCode(void);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif

#endif /* KMC_SRC_MIP_KMCV3_MASKINFO_H */
