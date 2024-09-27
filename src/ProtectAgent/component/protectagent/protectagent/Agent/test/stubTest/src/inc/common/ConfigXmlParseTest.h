#ifndef _CONFIGXMLPARSETEST_H_
#define _CONFIGXMLPARSETEST_H_

#define private public

#include "common/ConfigXmlParse.h"
#include "common/Types.h"
#include "common/File.h"
#include "common/ErrorCode.h"
#include "common/Utils.h"
#include "gtest/gtest.h"
#include "stub.h"

using namespace tinyxml2;

typedef mp_void (CLogger::*CLoggerLogType)(const mp_int32, const mp_int32, const mp_string&, const mp_string&, ...);
typedef mp_void (*StubCLoggerLogType)(mp_void* pthis);
mp_void StubCLoggerLogVoid(mp_void* pthis);

class CConfigXmlParserTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
// protected:
//     static mp_void SetUpTestCase(){
//         stub = new Stub<CLoggerLogType, StubCLoggerLogType, mp_void>(&CLogger::Log, &StubCLoggerLogVoid);
//     }
//     static mp_void TearDownTestCase()
//     {
//         delete stub;
//     }
private:
    Stub stub;
};

class CConfigXmlParserMock: public CConfigXmlParser{
public:


};

void CConfigXmlParserTest::SetUp() {}

void CConfigXmlParserTest::TearDown() {}

void CConfigXmlParserTest::SetUpTestCase() {}

void CConfigXmlParserTest::TearDownTestCase() {}


//Stub<CLoggerLogType, StubCLoggerLogType, mp_void>* CConfigXmlParserTest::m_stub;

//******************************************************************************
typedef  XMLElement* (CConfigXmlParser::*GetChildElementType)(XMLElement* pParentElement, mp_string strSection);
typedef  XMLElement* (*StubGetChildElementType)(XMLElement* pParentElement, mp_string strSection);
//
typedef  XMLElement* (XMLElement::*FirstChildElementType)(const mp_char* c );
typedef XMLElement* (*StubFirstChildElementType)(const mp_char* c );
//
typedef const XMLAttribute* (XMLElement::*FirstAttributeType)();
typedef const XMLAttribute* (*StubFirstAttributeType)();

typedef mp_int32 (CConfigXmlParser::*GetValueStringType)(const mp_string& strSection, const mp_string& strKey, mp_string& strValue);
typedef mp_int32 (*StubGetValueStringType)(const mp_string& strSection, const mp_string& strKey, mp_string& strValue);

typedef mp_int32 (CConfigXmlParser::*GetValueString1Type)(const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey, mp_string& strValue);
typedef mp_int32 (*StubGetValueString1Type)(const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey, mp_string& strValue);

typedef mp_int32 (*GetlLastModifyTimeType)(const mp_char* pszFilePath, mp_time& tLastModifyTime);
typedef mp_int32 (*StubGetlLastModifyTimeType)(const mp_char* pszFilePath, mp_time& tLastModifyTime);

typedef XMLElement* (XMLDocument::* RootElementType)();
typedef XMLElement* (*StubRootElementType)(mp_void* pthis);

typedef mp_bool (*CConfigXmlParserFileExistType)(const mp_string& pszFilePath);
typedef mp_bool (*StubCConfigXmlParserFileExistType)(const mp_string& pszFilePath);
//*******************************************************************************
mp_bool StubCConfigXmlParserFileExist(const mp_string& pszFilePath){
    return -1;
}

XMLElement* StubRootElement(mp_void* pthis){
    XMLDocument doc;
    XMLElement* root = doc.NewElement( "test" );
    doc.InsertEndChild( root );

    XMLElement* cityElement = doc.NewElement( "test1" );
    cityElement->SetAttribute( "test2", "test3" ); // 设置元素属性
    root->InsertEndChild( cityElement );
    return root;
}

XMLElement* StubGetChildElement(XMLElement* pParentElement, mp_string strSection){
    XMLDocument doc;
    XMLElement* root = doc.NewElement( "test" );
    doc.InsertEndChild( root );

    XMLElement* childElement = doc.NewElement( "test1" );
    childElement->SetAttribute( "test2", "test3" ); // 设置元素属性
    root->InsertEndChild( childElement );
    return childElement;
}

 XMLElement* StubFirstChildElement(const mp_char* strSection = 0){
    XMLDocument doc;
    XMLElement* root = doc.NewElement( "test" );
    doc.InsertEndChild( root );

    XMLElement* childElement = doc.NewElement( "test1" );
    childElement->SetAttribute( "test2", "test3" ); // 设置元素属性
    root->InsertEndChild( childElement );
    return childElement;
}

const XMLAttribute* StubFirstAttribute(){
    const XMLAttribute* test;
    return test;
}

mp_int32 StubGetlLastModifyTime(const mp_char* pszFilePath, mp_time& tLastModifyTime){

    tLastModifyTime = 1237779431;

    return MP_SUCCESS;
}

mp_int32 StubGetValueString(const mp_string& strSection, const mp_string& strKey, mp_string& strValue){
    return 0;
}

mp_int32 StubGetValueString1(const mp_string& strParentSection, const mp_string& strChildSection, const mp_string& strKey, mp_string& strValue){
    return 0;
}

mp_void StubCLoggerLogVoid(mp_void* pthis){
    return;
}

#endif
