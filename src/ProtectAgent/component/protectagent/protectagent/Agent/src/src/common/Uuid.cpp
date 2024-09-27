/**
 * Copyright (c) Huawei Technologies Co., Ltd. 2019-2019. All rights reserved.
 *
 * @file Uuid.cpp
 * @brief  Contains function declarations UUID
 * @version 1.0.0
 * @date 2020-08-01
 * @author yangwenjun 00275736
 */
#include "common/Uuid.h"
#include "common/ErrorCode.h"
using namespace std;
namespace {
const mp_uchar UUID_NUM_3 = 3;
const mp_int32 UUID_STR_LEN_32      = 32;
const mp_int32 UUID_STD_STR_LEN  = 36;
}

#ifndef AIX53
/* ------------------------------------------------------------
Description  :获取主机Uuid
Output       :     strUuid---Uuid
Return       :     MP_SUCCESS---成功
                    MP_FAILED---失败
Create By    : tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CUuidNum::GetUuidNumber(mp_string& strUuid)
{
    mp_uuid uuid;
#ifdef WIN32
    HRESULT hRet = CoCreateGuid(&uuid);
    if (S_OK != hRet) {
        COMMLOG(OS_LOG_ERROR, "Create uuid failed.");

        return MP_FAILED;
    }
#elif (defined SOLARIS) || (defined LINUX)
    uuid_generate(uuid);

#else
    // AIX and HP-UX
    mp_uint32 uRet;
    uuid_create(&uuid, &uRet);
    if (uuid_s_ok != uRet) {
        COMMLOG(OS_LOG_ERROR, "Create uuid failed.");

        return MP_FAILED;
    }
#endif

    mp_int32 iRet = ConvertUuidToStandardStr(uuid, strUuid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Formate uuid failed, iRet is %d", iRet);

        return MP_FAILED;
    }

    return MP_SUCCESS;
}

mp_int32 CUuidNum::GetUuidStr(mp_string &strUuid)
{
    mp_uuid uuid;

    mp_int32 iRet = GetUuid(uuid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get uuid failed, iRet is %d", iRet);
        return iRet;
    }

    iRet = FormatUuid(uuid, strUuid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Format uuid failed, iRet is %d", iRet);

        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  :获取主机Uuid标准格式字符串
Output       :     strUuid---Uuid
Return       :     MP_SUCCESS---成功
                    MP_FAILED---失败
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CUuidNum::GetUuidStandardStr(mp_string &strUuid)
{
    mp_uuid uuid;

    mp_int32 iRet = GetUuid(uuid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Get uuid failed, iRet is %d", iRet);
        return iRet;
    }

    iRet = ConvertUuidToStandardStr(uuid, strUuid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert uuid to standard str failed, iRet is %d", iRet);
        return iRet;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  :获取主机Uuid
Output       :     uuid---Uuid
Return       :     MP_SUCCESS---成功
                    MP_FAILED---失败
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CUuidNum::GetUuid(mp_uuid& uuid)
{
#ifdef WIN32
    HRESULT hRet = UuidCreate(&uuid);
    if (S_OK != hRet) {
        COMMLOG(OS_LOG_ERROR, "Create uuid failed.");

        return MP_FAILED;
    }
#elif (defined SOLARIS) || (defined LINUX)
    uuid_generate(uuid);
#else
    mp_uint32 iRet;
    // AIX and HP-UX
    uuid_create(&uuid, &iRet);
    if (uuid_s_ok != iRet) {
        COMMLOG(OS_LOG_ERROR, "Create uuid failed.");

        return MP_FAILED;
    }
#endif

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  :uuid字符串转换为uuid数据结构
Input        :     strUuid -- 待转换的uuid字符串 "663048C44308494893220725458EB305"
Output       :     uuid -- 保存转换后的uuid数据
Return       :     MP_SUCCESS---成功
                    MP_FAILED---失败
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CUuidNum::CovertStrToUuid(mp_string &strUuid, mp_uuid& uuid)
{
    if (strUuid.length() != UUID_STR_LEN_32) {
        COMMLOG(OS_LOG_ERROR, "Uuid string is invalid, strUuid %s.", strUuid.c_str());
        return MP_FAILED;
    }

#if defined(LINUX)
    unsigned char* ucUuid = reinterpret_cast<unsigned char *>(uuid);
#elif defined(AIX)
    unsigned char* ucUuid = (unsigned char *)&uuid;
#else
    unsigned char* ucUuid = reinterpret_cast<unsigned char *>(&uuid);
#endif
    const mp_char len = 3;
    mp_char cTmp[len] = {0};
    const mp_char hexChars = 2;
    const mp_int32 hexAdecimal = 16;
    mp_char* pTmp = NULL;

    const char* pStr = strUuid.c_str();
    for (mp_size i = 0; i < strlen(pStr); i += hexChars, ucUuid++) {
        memcpy_s(cTmp, len, pStr + i, hexChars);
        *ucUuid = (mp_uchar)strtol(cTmp, &pTmp, hexAdecimal);
    }

    COMMLOG(OS_LOG_DEBUG,  "Convert str to uuid succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  :uuid数据结构转换为字符串转换为
            支持Windows/Linux，小型机需要测试验证
Input        :     Uuid -- 待转换的uuid
Output       :     strUuid -- 保存转换后的uuid字符串 "663048C44308494893220725458EB305"
Return       :     MP_SUCCESS---成功
                    MP_FAILED---失败
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CUuidNum::ConvertUuidToStr(mp_uuid& uuid, mp_string& strUuid)
{
    mp_int32 iRet = FormatUuid(uuid, strUuid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Format uuid failed, iRet is %d", iRet);

        return MP_FAILED;
    }

    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  :uuid标准格式字符串转换为uuid数据结构
                支持Windows/Linux
Input        :     strUuid -- 待转换的uuid字符串  "6EE52B0C-6300-4DDA-A6BC-AF156106C582"
Output       :     uuid -- 保存转换后的uuid数据
Return       :     MP_SUCCESS---成功
                    MP_FAILED---失败
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CUuidNum::CovertStandrdStrToUuid(mp_string &strUuid, mp_uuid& uuid)
{
    if (strUuid.length() != UUID_STD_STR_LEN) {
        COMMLOG(OS_LOG_ERROR, "Uuid string is invalid, strUuid %s.", strUuid.c_str());
        return MP_FAILED;
    }

#ifdef LINUX
    mp_int32 iRet = uuid_parse(strUuid.c_str(), uuid);
    if (iRet != 0) {
        COMMLOG(OS_LOG_ERROR, "Convert string to uuid failed, returned iRet = %d", iRet);
        return MP_FAILED;
    }

#elif defined WIN32
    RPC_STATUS ret = UuidFromString((RPC_CSTR)strUuid.c_str(), &uuid);
    if (ret != RPC_S_OK) {
        COMMLOG(OS_LOG_ERROR, "Convert string %s to uuid failed, returned HRESULT = 0x%08lx", strUuid.c_str(), ret);
        return MP_FAILED;
    }

#else
    return ERROR_COMMON_FUNC_UNIMPLEMENT;
#endif

    COMMLOG(OS_LOG_DEBUG,  "Convert str to uuid succ.");
    return MP_SUCCESS;
}

/*------------------------------------------------------------
Description  :uuid数据结构转换为标准格式字符串
                支持Windows/Linux
Input        :     Uuid -- 待转换的uuid
Output       :     strUuid -- 保存转换后的uuid字符串 "6EE52B0C-6300-4DDA-A6BC-AF156106C582"
Return       :     MP_SUCCESS---成功
                    MP_FAILED---失败
Create By    : yangwenjun 00275736
-------------------------------------------------------------*/
mp_int32 CUuidNum::ConvertUuidToStandardStr(mp_uuid& uuid, mp_string& strUuid)
{
    const mp_char buf_size = 40;

#ifdef WIN32
    mp_uchar buf[buf_size] = {0};
    RPC_CSTR pUUIDStr = buf;
    RPC_STATUS ret = UuidToString(&uuid, &pUUIDStr);
    if (ret != RPC_S_OK) {
        COMMLOG(OS_LOG_ERROR, "Convert uuid to string failed, returned HRESULT = 0x%08lx", ret);
        return MP_FAILED;
    }
    strUuid = (mp_char *)pUUIDStr;
#elif defined LINUX
    mp_char buf[buf_size] = {0};
    uuid_unparse_lower(uuid, buf);
    strUuid = buf;
#elif defined AIX
    mp_uchar* buf = NULL;
    mp_uint32 status;
    uuid_to_string(&uuid, &buf, &status);
    strUuid = mp_string((char *)buf);
    COMMLOG(OS_LOG_DEBUG,  "stats is %lu", status);
#elif defined SOLARIS
    mp_char buf[buf_size] = {0};
    uuid_unparse(uuid, buf);
    strUuid = mp_string((char *)buf);
#endif
    COMMLOG(OS_LOG_DEBUG,  "Convert uuid to standard str succ, uuid stand str %s.", strUuid.c_str());
    return MP_SUCCESS;
}

#ifdef WIN32
/*------------------------------------------------------------
Description  : 转换windows下的GUID为网络字节顺序
                字节序大端状态下，GUID的4端直接复制到消息发送，会出现字节颠倒的问题，比如
                42276724-9651-4f45-bdab-acee38b6f610，放到OMA后变成格式
                24672742-5196-454f-abbd-acee38b6f610
Return       : MP_SUCCESS -- 成功
                非MP_SUCCESS -- 失败，返回特定错误码
-------------------------------------------------------------*/
mp_int32 CUuidNum::ConvertGUIDToCharArray(const GUID& pGUID, char pCharBuff[])
{
    if (!pCharBuff) {
        COMMLOG(OS_LOG_ERROR, "ConvertGUIDToCharArray: pCharBuff is NULL.");
        return MP_FAILED;
    }

    mp_uint32 uiValL = htonl(pGUID.Data1);
    memcpy_s(pCharBuff, sizeof(uiValL), &uiValL, sizeof(uiValL));
    pCharBuff += sizeof(uiValL);

    mp_uint16 uiValS = htons(pGUID.Data2);
    memcpy_s(pCharBuff, sizeof(uiValS), &uiValS, sizeof(uiValS));
    pCharBuff += sizeof(uiValS);

    uiValS = htons(pGUID.Data3);
    memcpy_s(pCharBuff, sizeof(uiValS), &uiValS, sizeof(uiValS));
    pCharBuff += sizeof(uiValS);

    memcpy_s(pCharBuff, sizeof(pGUID.Data4), pGUID.Data4, sizeof(pGUID.Data4));

    return MP_SUCCESS;
}

#endif

mp_int32 CUuidNum::ConvertStrUUIToArray(mp_string& strUUID, mp_char pszCharArray[], mp_uint32 uiLen)
{
    mp_uuid uuid;

    COMMLOG(OS_LOG_DEBUG,  "Begin convert uuid string %s to binary array.", strUUID.c_str());
    mp_int32 iRet = CUuidNum::CovertStandrdStrToUuid(strUUID, uuid);
    if (iRet != MP_SUCCESS) {
        COMMLOG(OS_LOG_ERROR, "Convert str uuid failed, iRet %d.", iRet);
        return iRet;
    }

#ifdef WIN32
    CHECK_NOT_OK(CUuidNum::ConvertGUIDToCharArray(uuid, pszCharArray));
#else
    CHECK_NOT_OK(memcpy_s(pszCharArray, uiLen, &uuid, sizeof(uuid)));
#endif
    COMMLOG(OS_LOG_DEBUG,  "Convert uuid string %s to binary array succ.", strUUID.c_str());

    return iRet;
}


/* ------------------------------------------------------------
Description  :格式化Uuid
Input        :     uuid --uuid
Output       :     strUuid---Uuid
Return       :     MP_SUCCESS---成功
                    MP_FAILED---失败
Create By    :    tanyuanjun 00285255
------------------------------------------------------------- */
mp_int32 CUuidNum::FormatUuid(mp_uuid uuid, mp_string& strUuid)
{
#if defined(LINUX)
    unsigned char* ucUuid = reinterpret_cast<unsigned char *>(uuid);
#elif defined(AIX)
    unsigned char* ucUuid = (unsigned char*)&uuid;
#else
    unsigned char* ucUuid = reinterpret_cast<unsigned char *>(&uuid);
#endif
    std::size_t uuidlen = sizeof(mp_uuid);
    mp_char cTmp[UUID_NUM_3] = {0};
    for (mp_size i = 0; i < uuidlen; i++, ucUuid++) {
        CHECK_FAIL(sprintf_s(cTmp, sizeof(cTmp), "%02X", *ucUuid));
        strUuid.append(cTmp);
    }

    COMMLOG(OS_LOG_DEBUG, "FormatUuid succ, uuid is %s", strUuid.c_str());
    return MP_SUCCESS;
}
#endif
