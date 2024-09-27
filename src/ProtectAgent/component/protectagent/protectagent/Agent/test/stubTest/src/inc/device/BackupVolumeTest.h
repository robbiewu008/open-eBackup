#ifndef __BACKUP_VOLUME_TEST_H__
#define __BACKUP_VOLUME_TEST_H__

#include "device/BackupVolume.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "securecom/RootCaller.h"
#include "gtest/gtest.h"
#include "stub.h"
#include <vector>
using namespace std;

static Stub *m_stub = NULL;


mp_void StubBackupVolumeTestLogVoid(mp_void* pthis);
class BackupVolumeTest: public testing::Test{
public:
    static mp_void SetUpTestCase(){
        m_stub = new Stub;
        m_stub->set((mp_void (CLogger::*)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...))ADDR(CLogger,Log), StubBackupVolumeTestLogVoid);
    }
    static mp_void TearDownTestCase()
    {
        if(m_stub != NULL) {
            delete m_stub;
        }
    }
    Stub stub;
};

mp_void StubBackupVolumeTestLogVoid(mp_void* pthis){
    return;
}


#endif