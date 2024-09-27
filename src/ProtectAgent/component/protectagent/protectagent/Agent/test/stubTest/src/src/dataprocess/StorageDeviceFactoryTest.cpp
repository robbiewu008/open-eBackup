#include "dataprocess/StorageDeviceFactoryTest.h"
#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubStorageDeviceFactoryGetValueInt32Return);                                                                      \
    } while (0)

static mp_void StubCLoggerLog(mp_void){
    return;
}
TEST_F(StorageDeviceFactoryTest, CreateStorageDevice)
{
    mp_int32 storageProtocol = 1;
    StorageDeviceFactory om;

    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    //storageProtocol = 1
    {
        om.CreateStorageDevice(storageProtocol);
    }
    //storageProtocol = 1
    {
        storageProtocol = 2;
        om.CreateStorageDevice(storageProtocol);
    }
    //storageProtocol = 3
    {
        storageProtocol = 3;
        om.CreateStorageDevice(storageProtocol);
    }
}