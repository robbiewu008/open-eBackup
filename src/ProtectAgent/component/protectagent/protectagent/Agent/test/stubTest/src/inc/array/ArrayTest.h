#ifndef __ARRAYTEST_H__
#define __ARRAYTEST_H__

#define private public

#include "array/array.h"
#include "array/disk.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "common/Defines.h"
#include "common/MpString.h"
#include "securecom/UniqueId.h"
#include "common/Path.h"
#include "common/CSystemExec.h"
#include "gtest/gtest.h"
#include "stub.h"

#include <vector>
using namespace std;

mp_void StubCLoggerLogVoid(mp_void* pthis);

static Stub *m_stub = NULL;
static Stub *s_stub = NULL;

class CArrayTest : public testing::Test
{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub;
        m_stub->set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        if(m_stub != NULL) {
            delete m_stub;
        }
    }
    Stub stub;
};

class CDiskTest : public testing::Test
{
public:
    static mp_void SetUpTestCase(){
        s_stub = new Stub;
        s_stub->set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubCLoggerLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        if(s_stub != NULL) {
            delete s_stub;
        }
    }
    Stub stub;
};


mp_bool StubIsSupportXXPage(string page, vector<mp_string>& vecResult)
{
    return MP_TRUE;
}

mp_void StubCLoggerLogVoid(mp_void* pthis)
{
    return;
}
mp_char* StubrealpathEq0(const char *path, char *resolved_path)
{
    return "/home/";
}
mp_char* StubrealpathEq1(const char *path, char *resolved_path)
{
    return NULL;
}
mp_int32 StubopenEq0(const mp_char* pathname, mp_int32 flags)
{
    return 0;
}
mp_int32 StubopenLt0(const mp_char* pathname, mp_int32 flags)
{
    return -1;
}
mp_int32 StubcloseEq0(mp_int32 fd)
{
    return 0;
}
mp_int32 StubioctlLt0(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    return -1;
}
mp_int32 StubioctlEq0(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    return 0;
}
mp_int32 StubioctlEq0GetDisk80Page(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    *((mp_char*)(io_hdr->dxferp) + 3) = 64;
    return 0;
}
mp_int32 StubioctlEq0GetDisk83Page(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    *((mp_char*)(io_hdr->dxferp) + 7) = 32;
    return 0;
}
mp_int32 StubioctlEq0Buf30(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    *((mp_char*)(io_hdr->dxferp) + 7) = 30;
    return 0;
}
mp_int32 StubCRootCallerExecLt0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    return -1;
}
mp_int32 StubCRootCallerExecEq0(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    return 0;
}
mp_int32 StubCRootCallerExecEq0IsDeviceExist(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    mp_string tmp;
    tmp[0] = 0x72;
    pvecResult->push_back(tmp);
    return 0;
}
mp_int32 StubCRootCallerExecEq0IsDeviceExist_1(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    mp_string tmp = "123";
    tmp[0] = 0x69;
    tmp[1] = 0x12;
    tmp[2] = 0x11;
    pvecResult->push_back(tmp);
    return 0;
}
mp_int32 StubCRootCallerExecOk(mp_int32 iCommandID, mp_string strParam, vector<mp_string>* pvecResult)
{
    if (!pvecResult)
    {
        return -1;
    }
    pvecResult->push_back("huawei");
    pvecResult->push_back("rong");
    return 0;
}
mp_int32 StubCSystemExecExecSystemWithEchoLt0(const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    return -1;
}
mp_int32 StubCSystemExecExecSystemWithEchoOk(const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    strEcho.push_back("test");
    return 0;
}
mp_int32 StubCSystemExecExecSystemWithEchoEq0IsCmdDevice(const mp_string& strCommand, vector<mp_string>& strEcho, mp_bool bNeedRedirect)
{
    strEcho.push_back("1");
    strEcho.push_back("1");
    return 0;
}

mp_int32 StubioctlOkGetDiskCapacity(mp_int32 fd, long unsigned int request, sg_io_hdr_t* io_hdr)
{
    if (!io_hdr->dxferp)
    {
        return -1;
    }
    *((mp_char*)(io_hdr->dxferp)) = 0xff;
    *((mp_char*)(io_hdr->dxferp) + 1) = 0xff;
    *((mp_char*)(io_hdr->dxferp) + 2) = 0xff;
    *((mp_char*)(io_hdr->dxferp) + 3) = 0xff;
    return 0;
}
mp_bool StubCDiskIsSdiskEq1(mp_string& strDevice)
{
    return 1;
}
mp_bool StubCDiskIsSdiskEq0(mp_string& strDevice)
{
    return 0;
}
mp_bool StubCDiskIsHdiskEq1(mp_string& strDevice)
{
    return 1;
}
mp_bool StubCDiskIsHdiskEq0(mp_string& strDevice)
{
    return 0;
}
mp_int32 StubCDiskGetAllDiskNameLt0(vector<mp_string>& vecDiskName)
{
    return -1;
}
mp_int32 StubCDiskGetAllDiskNameEq0(vector<mp_string>& vecDiskName)
{
    return 0;
}
mp_int32 StubCDiskGetAllDiskNameOk(vector<mp_string>& vecDiskName)
{
    vecDiskName.push_back("test");
    vecDiskName.push_back("nohuawei");
    vecDiskName.push_back(ARRAY_VENDER_HUAWEI);
    vecDiskName.push_back(ARRAY_VENDER_HUAWEI);
    return 0;
}
mp_int32 StubCArrayGetArrayVendorAndProductOkGetDevNameByWWN(const mp_string& strDev, mp_string& strvendor, mp_string& strproduct)
{
    if (strDev == "/dev/test")
    {
        return -1;
    }
    if (strDev == "/dev/HUAWEI")
    {
        strvendor = ARRAY_VENDER_HUAWEI;
    }
    if (strDev == "/dev/Huawei")
    {
        strproduct = PRODUCT_VBS;
    }
    return 0;
}
mp_int32 StubCArrayGetLunInfoLt0(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    return -1;
}
mp_int32 StubCArrayGetLunInfoOk(mp_string& strDev, mp_string& strLunWWN, mp_string& strLunID)
{
    strLunWWN = "test";
    return 0;
}

mp_int32 StubCGetDiskWWNAndLUNIDLt0(mp_uchar aucBuffer[], mp_string& strLunWWN, mp_string& strLunID)
{
    return -1;
}

mp_int32 StubCGetDiskWWNAndLUNIDEq0(mp_uchar aucBuffer[], mp_string& strLunWWN, mp_string& strLunID)
{
    return 0;
}

mp_int32 StubCGetDiskSNLt0(const mp_uchar aucBuffer[], mp_string& strSN)
{
    return -1;
}

mp_int32 StubCGetDiskSNEq0(const mp_uchar aucBuffer[], mp_string& strSN)
{
    return 0;
}

mp_int32 StubCBinaryToAsciiLt0(mp_char pszHexBuffer[], mp_int32 iBufferLen, const mp_uchar pucBuffer[], mp_int32 iStartBuf, mp_int32 iLength)
{
    return -1;
}

mp_int32 StubCBinaryToAsciiEq0(mp_char pszHexBuffer[], mp_int32 iBufferLen, const mp_uchar pucBuffer[], mp_int32 iStartBuf, mp_int32 iLength)
{
    return 0;
}

mp_int32 StubCConvertLUNIDtoAsciiLt0(mp_char puszAsciiLunID[], mp_int32 iBufferLen, mp_uchar puszLunLunID[], mp_int32 iLen)
{
    return -1;
}

mp_int32 StubCConvertLUNIDtoAsciiEq0(mp_char puszAsciiLunID[], mp_int32 iBufferLen, mp_uchar puszLunLunID[], mp_int32 iLen)
{
    return 0;
}

#endif
