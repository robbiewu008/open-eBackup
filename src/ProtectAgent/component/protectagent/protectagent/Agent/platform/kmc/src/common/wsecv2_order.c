/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2014-2020. All rights reserved.
 * Description: implementation of the BYTE ORDER common function
 * Author: x00102361
 * Create: 2014-06-16
 * History: 2018-10-08 Zhang Jie (employee ID: 00316590) Rectification by UK
 */

#include "wsecv2_order.h"

#ifdef WSEC_DEBUG
    static WsecBool g_isEndianChecked = WSEC_FALSE;
#endif

#if (WSEC_CPU_ENDIAN_MODE == WSEC_CPU_ENDIAL_AUTO_CHK)
    #define ENDIAN_TEST_SHORT_VALUE 0x1234
    #define ENDIAN_TEST_BYTE_VALUE  0x12
    static WsecBool g_isBigEndian = WSEC_FALSE;
#endif

/* Checks the CPU byte order alignment mode and saves the check result to the global variable g_isBigEndian. */
WsecVoid WsecCheckCpuEndianMode(void)
{
#if (WSEC_CPU_ENDIAN_MODE == WSEC_CPU_ENDIAL_AUTO_CHK)
    WsecUint16 valueShort = ENDIAN_TEST_SHORT_VALUE;
    unsigned char valueByte = *(unsigned char *)&valueShort;

    g_isBigEndian = (valueByte == ENDIAN_TEST_BYTE_VALUE) ? WSEC_TRUE : WSEC_FALSE;
#endif
#ifdef WSEC_DEBUG
    g_isEndianChecked = WSEC_TRUE;
#endif
}

/* Check whether the CPU byte order is big-endian. */
WsecBool WsecIsBigEndianMode(void)
{
#if (WSEC_CPU_ENDIAN_MODE == WSEC_CPU_ENDIAL_BIG) /* Big_endian mode */
    return WSEC_TRUE;
#elif (WSEC_CPU_ENDIAN_MODE == WSEC_CPU_ENDIAL_LITTLE) /* Little-endian mode. */
    return WSEC_FALSE;
#else

#ifdef WSEC_DEBUG
    WSEC_ASSERT(g_isEndianChecked == WSEC_TRUE);
#endif
    return g_isBigEndian;
#endif
}
