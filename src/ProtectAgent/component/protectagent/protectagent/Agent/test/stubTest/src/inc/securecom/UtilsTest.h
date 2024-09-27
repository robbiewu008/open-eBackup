#ifndef _UTILSTEST_H_
#define _UTILSTEST_H_
#define private public

#ifndef WIN32
#include <signal.h>
#include <libgen.h>
#endif
#include <sstream>
#include "securecom/SecureUtils.h"
#include "common/Utils.h"
#include "common/Log.h"
#include "common/Path.h"
#include "securecom/UniqueId.h"
#include "common/ConfigXmlParse.h"
#include "securecom/CryptAlg.h"
#include "securecom/RootCaller.h"
#include "securec.h"
#include "gtest/gtest.h"
#include "stub.h"

typedef mp_void (CLogger::*CLoggerLogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubUtilsVoid(mp_void* pthis);

class UtilsTest: public testing::Test{
public:
    Stub stub;
};

mp_void StubUtilsVoid(mp_void* pthis){
    return;
}

mp_int32 StubCPathInit(const mp_string& pszBinFilePath){
    return 0;
}

mp_int32 StubCConfigXmlParserInit(mp_string strInput){
    return 0;
}

mp_int32 StubReturnResultSetSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}

mp_int32 StubGetValueString_fail(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_FAILED;
}

mp_int32 StubGetValueString_succ(const mp_string& strSection, const mp_string& strKey, mp_string& strValue)
{
    return MP_SUCCESS;
}
#endif
