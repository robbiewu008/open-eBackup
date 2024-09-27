#ifndef __AGENT_FILE_TEST_H__
#define __AGENT_FILE_TEST_H__
#define private public
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "common/File.h"
#include "gtest/gtest.h"
 #include "common/JsonUtils.h"
#include "common/ConfigXmlParse.h"
#include "stub.h"


typedef mp_void (CLogger::*CLoggerLogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CMpFileTest: public testing::Test{
public:
    Stub stub;
};

class CIPCFileTest: public testing::Test{
public:
    Stub stub;
};


#endif

