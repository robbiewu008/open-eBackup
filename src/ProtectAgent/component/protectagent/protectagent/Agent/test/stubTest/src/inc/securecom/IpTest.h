#ifndef __AGENT_IP_TEST_H__
#define __AGENT_IP_TEST_H__

#define private public

#include "securecom/Ip.h"
#include "gtest/gtest.h"
 #include "common/Path.h"
#include "common/ConfigXmlParse.h"
#include "common/File.h"
#include "common/Log.h"
#include "stub.h"

class IPTest: public testing::Test {
public:
    Stub stub;
};

mp_int32 StubCheckHostLinkStatus(
    const mp_string& strSrcIp, const mp_string& strHostIp, mp_uint16 uiPort, mp_int32 timeout)
{
    return MP_SUCCESS;
}



#endif
