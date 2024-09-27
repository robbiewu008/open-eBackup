#ifndef __AGENT_IP_TEST_H__
#define __AGENT_IP_TEST_H__

#define private public

#include "common/Ip.h"
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

static mp_bool StubFileExist(const mp_string& pszFilePath)
{
    return MP_TRUE;
}


mp_int32 StubCMpFileReadFile(const mp_string& strFilePath, std::vector<mp_string>& vecOutput)
{
    mp_string v = "10.10.10.10:255 listen";
    mp_string v1 = "fe80::2a6e:d4ff:fe89:4506";
    vecOutput.push_back(v);
    vecOutput.push_back(v1);
    return MP_SUCCESS;
}

#endif
