#include "dataprocess/DataConfigTest.h"
#include "common/Types.h"
#include "common/Log.h"
#include "common/ErrorCode.h"
#include "sstream"

TEST_F(CDataConfigTest, GetID){
    mp_int32 iRet = 0;
    mp_int32 temp_id = 0;
    DataConfig om;

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32), StubGetValueInt32Success);
    iRet = om.GetID(temp_id);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32), StubGetValueInt32Fail);
    iRet = om.GetID(temp_id);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.reset((mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32));
}

TEST_F(CDataConfigTest, GetState){
    mp_int32 iRet = 0;
    mp_string temp_state = "";
    DataConfig om;
    
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubGetValueStringSuccess);
    iRet = om.GetState(temp_state);
    EXPECT_EQ(MP_SUCCESS, iRet);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString), StubGetValueStringFail);
    iRet = om.GetState(temp_state);
    EXPECT_EQ(MP_FAILED, iRet);

    stub.reset((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser, GetValueString));
}

TEST_F(CDataConfigTest, SetID){
    mp_int32 iRet = 0;
    mp_int32 temp_id = 1234;
    DataConfig om;
    
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue), StubSetValueSuccess);
    iRet = om.SetID(temp_id);
    EXPECT_EQ(MP_TRUE, iRet);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue), StubSetValueFail);
    iRet = om.SetID(temp_id);
    EXPECT_EQ(MP_FALSE, iRet);

    stub.reset((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue));
}

TEST_F(CDataConfigTest, SetState){
    mp_bool iRet = MP_TRUE;
    mp_string temp_state = "1234";
    DataConfig om;
    
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue), StubSetValueSuccess);
    iRet = om.SetState(temp_state);
    EXPECT_EQ(MP_TRUE, iRet);

    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue), StubSetValueFail);
    iRet = om.SetState(temp_state);
    EXPECT_EQ(MP_FALSE, iRet);

    stub.reset((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&))ADDR(CConfigXmlParser,SetValue));
}