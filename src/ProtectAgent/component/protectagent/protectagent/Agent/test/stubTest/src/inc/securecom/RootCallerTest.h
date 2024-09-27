#ifndef _ROOTCALLER_H_
#define _ROOTCALLER_H_

#define private public

#include "securecom/RootCaller.h"
#include "common/Utils.h"
#include "securecom/UniqueId.h"
#include "common/Log.h"
#include "common/File.h"
#include "common/Path.h"
#include "common/ErrorCode.h"
#include "common/CSystemExec.h"
#include "common/ConfigXmlParse.h"
#include "securec.h"
#include "gtest/gtest.h"
#include "stub.h"
#include <vector>

using namespace std;

class CRootCallerTest: public testing::Test{
public:
    Stub stub;
};

//*******************************************************************************
mp_int32 StubWriteInput(mp_string& strUniqueID, mp_string& strInput){
    return -1;
}

mp_int32 StubReadResult(mp_string& strUniqueID, vector<mp_string>& vecRlt){
    return -1;
}

static mp_int32 StubExecSystemWithoutEcho(mp_string strLogCmd, mp_string& strCommand, mp_bool bNeedRedirect){
    return 0;
}

#endif
