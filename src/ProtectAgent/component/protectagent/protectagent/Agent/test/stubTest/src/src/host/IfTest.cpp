#include "host/IfTest.h"

static mp_int32 StubCConfigXmlParserGetValueInt32ReturnSuccess(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}
#define StubClogToVoidLogNullPointReference() do { \
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), StubCConfigXmlParserGetValueInt32ReturnSuccess); \
} while (0)

TEST_F(IfTest, GetAllIfInfo)
{
    vector<if_info_t> ifs;
    mp_int32 iRet;
    CIf work;

    iRet = work.GetAllIfInfo(ifs);
    EXPECT_EQ(iRet, MP_SUCCESS);
}