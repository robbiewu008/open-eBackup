#ifndef __PLUGINCFG_PARSE_H__
#define __PLUGINCFG_PARSE_H__

#define private public
#define protected public

#include "pluginfx/PluginCfgParse.h"
#include "common/Log.h"
#include "securec.h"
#include "common/Defines.h"
#include "common/ErrorCode.h"
#include "gtest/gtest.h"
#include "stub.h"

static Stub *m_stub = NULL;

class CMpPluginCfgParseTest: public testing::Test{
protected:
};

mp_void StubCMpPluginCfgTestLogVoid(mp_void* pthis);

class CMpPluginCfgTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub;
        m_stub->set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubCMpPluginCfgTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        if(m_stub != NULL) {
            delete m_stub;
        }
    }
    Stub stub;
};

mp_void StubCMpPluginCfgTestLogVoid(mp_void* pthis){
    return;
}

#endif



