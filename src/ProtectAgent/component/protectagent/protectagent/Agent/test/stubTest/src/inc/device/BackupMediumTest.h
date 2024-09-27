#ifndef __BACKUP_MEDIUM_TEST_H__
#define __BACKUP_MEDIUM_TEST_H__

#define private public

#include "device/BackupMedium.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "securecom/RootCaller.h"
#include "gtest/gtest.h"
#include "stub.h"
#include <vector>
using namespace std;

static Stub *m_stub = NULL;


mp_void StubBackupMediumTestLogVoid(mp_void* pthis);
class BackupMediumTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub;
        m_stub->set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubBackupMediumTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        if(m_stub != NULL) {
            delete m_stub;
        }
    }
    Stub stub;
};

mp_void StubBackupMediumTestLogVoid(mp_void* pthis){
    return;
}


#endif