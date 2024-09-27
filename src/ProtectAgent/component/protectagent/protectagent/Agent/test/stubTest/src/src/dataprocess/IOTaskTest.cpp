#include "dataprocess/IOTaskTest.h"
#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubIOTaskGetValueInt32Return);                                                                      \
    } while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}

TEST_F(IOTaskTest, Exec)
{
    mp_int32 taskType = 1;
    uint64_t startAddr = 1;
    uint64_t bufSize = 1;
    IOTask om(taskType, startAddr, bufSize);

    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    om.Exec();
}
