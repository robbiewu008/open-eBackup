/*
* This file is a part of the open-eBackup project.
* This Source Code Form is subject to the terms of the Mozilla Public License, v. 2.0.
* If a copy of the MPL was not distributed with this file, You can obtain one at
* http://mozilla.org/MPL/2.0/.
*
* Copyright (c) [2024] Huawei Technologies Co.,Ltd.
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*/
#include "common/ConfigXmlParseTest.h"
#include "common/JsonUtils.h"
#include "jsoncpp/include/json/json.h"
#include "common/ConfigXmlParse.h"
mp_int32 StubCConfigXmlParser_Load(){
    return MP_SUCCESS;
}

mp_void SignTestStubCLoggerLog(mp_void){
    return;
}

TEST_F(CConfigXmlParserTest, UpdateModifyTimeTest){
    stub.set(&CLogger::Log, SignTestStubCLoggerLog);
    mp_int32 ret = 0;
    mp_uint32 ivalue;
    mp_uint64 ivalueu64;
    mp_int64 lValue64;
    Json::Value jsValueT;
    mp_string strKey;
    mp_string strValue;
    std::vector<Json::Value> vecValueJson;
    std::vector<mp_int32> vecValue;
    std::vector<mp_string> vecValueStr;


    std::vector<mp_int64> vecValuev64;
    jsValueT.append("111");
    jsValueT.append("111");
    jsValueT.append("111");
    jsValueT.append("111");
    jsValueT.append(111);
    Json::Value jsValue2;
    jsValue2.append(111);
    
    CJsonUtils::GetJsonString(jsValueT, strKey, strValue);
    CJsonUtils::GetJsonUInt32(jsValue2, strKey, ivalue);

    Json::Value jsValue1;
    jsValue1["111"] = "111";
    CJsonUtils::GetJsonUInt32(jsValue1, "111", ivalue);
    CJsonUtils::GetJsonInt64(jsValueT, strKey, lValue64);
    CJsonUtils::GetJsonUInt64(jsValueT, strKey, ivalueu64);

    CJsonUtils::GetJsonArrayInt32(jsValue2, strKey, vecValue);
    CJsonUtils::GetJsonArrayInt64(jsValue2, strKey, vecValuev64);
    CJsonUtils::GetArrayInt32(jsValue2, vecValue);
    CJsonUtils::GetArrayInt64(jsValue2, vecValuev64);
    CJsonUtils::GetArrayString(jsValueT, vecValueStr);
    CJsonUtils::GetArrayJson(jsValueT, vecValueJson);
    ret = CConfigXmlParser::GetInstance().UpdateModifyTime();
}

TEST_F(CConfigXmlParserTest, Init){
    mp_int32 ret = 0;
    mp_string path = "test";
    
    stub.set(ADDR(CConfigXmlParser, Load), StubCConfigXmlParser_Load);
    ret = CConfigXmlParser::GetInstance().Init(path);
    EXPECT_EQ(MP_SUCCESS, ret);
    stub.reset(ADDR(CConfigXmlParser, Load));
}

// TEST_F(CConfigXmlParserTest, GetValueUint64Test){
//     mp_int32 ret = 0;
//     mp_string strSection;
//     mp_string strKey; 
//     mp_uint64 lValue;
//     ret = CConfigXmlParser::GetInstance().GetValueUint64(strSection, strKey, lValue);
// }

TEST_F(CConfigXmlParserTest, LoadTest){
    mp_int32 ret = 0;
    mp_string path = "test";

    ret = CConfigXmlParser::GetInstance().Load();
    CConfigXmlParser::GetInstance().m_strCfgFilePath = "/bin/test";
    ret = CConfigXmlParser::GetInstance().Load();
}

TEST_F(CConfigXmlParserTest, GetChildElement){
    mp_string path = "test";
    XMLElement* ptr = NULL;

    EXPECT_EQ(NULL, CConfigXmlParser::GetInstance().GetChildElement(ptr,path));
}
//打庒失敗，可能涉及到重載函數
TEST_F(CConfigXmlParserTest, ParseNodeValue){
    mp_int32 ret = 0;
    XMLDocument doc;
    XMLElement* ptr = NULL;
    XMLElement* strSection = doc.NewElement( "test" );
    XMLElement* childElement = doc.NewElement( "test1" );
    childElement->SetAttribute( "test2", "test3" ); // 设置元素属性
    strSection->InsertEndChild( childElement );

    NodeValue nodeValue;
    
    //stub.set(ADDR(XMLElement, FirstChildElement), StubFirstChildElement);
    //Stub<FirstChildElementType, StubFirstChildElementType, mp_void> mystub2(&XMLElement::FirstChildElement, &StubFirstChildElement);
    CConfigXmlParser::GetInstance().ParseNodeValue(strSection,nodeValue);
    CConfigXmlParser::GetInstance().ParseNodeValue(ptr,nodeValue);

    //stub.reset(ADDR(XMLElement, FirstChildElement));
}

TEST_F(CConfigXmlParserTest,IsModified){
    mp_int32 ret = 0;
    
    CConfigXmlParser::GetInstance().m_strCfgFilePath = "";
    ret = CConfigXmlParser::GetInstance().IsModified();
    EXPECT_EQ(MP_FALSE, ret);

    CConfigXmlParser::GetInstance().m_strCfgFilePath = "test";
    stub.set(ADDR(CMpFile, GetlLastModifyTime), StubGetlLastModifyTime);
    //CConfigXmlParser::GetInstance().IsModified(); 
    EXPECT_EQ(MP_TRUE, CConfigXmlParser::GetInstance().IsModified());
    // Stub<GetlLastModifyTimeType, StubGetlLastModifyTimeType, mp_void> mystub2(&CMpFile::GetlLastModifyTime, &StubGetlLastModifyTime);
    // CConfigXmlParser::GetInstance().IsModified();
    
    stub.reset(ADDR(CMpFile, GetlLastModifyTime));
}

TEST_F(CConfigXmlParserTest, GetValueString){
     mp_int32 ret = 0;
     time_t timett;
     mp_string strSection = "test";
     mp_string strSection1 = "test";
     mp_string strKey = "test";
     mp_string strValue = "test";
     stub.set(((XMLElement* (tinyxml2::XMLDocument::*)(void)) (&tinyxml2::XMLDocument::RootElement)), StubRootElement);
     ret = CConfigXmlParser::GetInstance().GetValueString(strSection,strKey,strValue);
     ret = CConfigXmlParser::GetInstance().GetValueString(strSection,strSection1,strKey,strValue);  
     stub.set(&CConfigXmlParser::GetChildElement, StubGetChildElement);
     ret = CConfigXmlParser::GetInstance().GetValueString(strSection,strKey,strValue);
     ret = CConfigXmlParser::GetInstance().GetValueString(strSection,strSection1,strKey,strValue);  
     CConfigXmlParser::GetInstance().m_lastTime = timett;
     CConfigXmlParser::GetInstance().GetValueString(strSection,strKey,strValue);
 }

TEST_F(CConfigXmlParserTest, GetValueBool){
    mp_int32 ret = 0;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    mp_bool strValue = 0;
   
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString1);
        ret = CConfigXmlParser::GetInstance().GetValueBool(strSection,strKey,strValue);
        ret = CConfigXmlParser::GetInstance().GetValueBool(strSection,strSection1,strKey,strValue);
    }
}

TEST_F(CConfigXmlParserTest, GetValueInt32){
    mp_int32 ret = 0;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    mp_int32 strValue = 12;  
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString1);
        ret = CConfigXmlParser::GetInstance().GetValueInt32(strSection,strKey,strValue);
        ret = CConfigXmlParser::GetInstance().GetValueInt32(strSection,strSection1,strKey,strValue);
    }
}
TEST_F(CConfigXmlParserTest, GetValueInt64){
    mp_int32 ret = 0;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    mp_int64 strValue = 12; 
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString1);
        ret = CConfigXmlParser::GetInstance().GetValueInt64(strSection,strKey,strValue);
        ret = CConfigXmlParser::GetInstance().GetValueInt64(strSection,strSection1,strKey,strValue);
    }
}
TEST_F(CConfigXmlParserTest, GetValueFloat){
    mp_int32 ret = 0;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    mp_float strValue = 12.1; 
    {
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString);
        stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,const mp_string&,mp_string&))ADDR(CConfigXmlParser,GetValueString), StubGetValueString1);
        ret = CConfigXmlParser::GetInstance().GetValueFloat(strSection,strKey,strValue);
        ret = CConfigXmlParser::GetInstance().GetValueFloat(strSection,strSection1,strKey,strValue);
    }
}

TEST_F(CConfigXmlParserTest, SetValue){
    mp_int32 ret = 0;
    mp_string strSection = "test";
    mp_string strSection1 = "test";
    mp_string strKey = "test";
    // mp_string strValue = "test";
    // {
    //     ret = CConfigXmlParser::GetInstance().SetValue(strSection,strKey,strValue);
    //     ret = CConfigXmlParser::GetInstance().SetValue(strSection,strSection1,strKey,strValue);
    // }
    // {
    //     stub.set(&CMpFile::FileExist, &StubCConfigXmlParserFileExist);
    //     stub.set(((XMLElement* (tinyxml2::XMLDocument::*)(void)) (&tinyxml2::XMLDocument::RootElement)), StubRootElement);
    //     stub.set(&CConfigXmlParser::GetChildElement, &StubGetChildElement);
    //     ret = CConfigXmlParser::GetInstance().SetValue(strSection,strKey,strValue);
    //     ret = CConfigXmlParser::GetInstance().SetValue(strSection,strSection1,strKey,strValue);
    // }
}


