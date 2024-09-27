#ifndef __DATAMESSAGETEST_H__
#define __DATAMESSAGETEST_H__

#define protected public
#define private public

#include "dataprocess/datamessage/DataMessage.h"
#include "common/ConfigXmlParse.h"
#include "message/tcp/CConnection.h"
#include "message/tcp/DppSocket.h"
#include "common/ErrorCode.h"
#include "common/Path.h"
#include "gtest/gtest.h"
#include "stub.h"

using namespace tinyxml2;


class DataMessageTest: public testing::Test{
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();
public:
    Stub stub;
};

void DataMessageTest::SetUp() {}

void DataMessageTest::TearDown() {}

void DataMessageTest::SetUpTestCase() {}

void DataMessageTest::TearDownTestCase() {}

mp_int32 StubCConfigXmlParserGetValueInt32Return(mp_void* pthis, const mp_string& strSection, const mp_string& strKey, mp_int32& iValue)
{
    return MP_SUCCESS;
}

mp_socket StubGetClientSocketFail() {
    return MP_INVALID_HANDLE;
}

mp_int32 StubCreateClientSocket(mp_socket& clientSock) {
    return MP_SUCCESS;
}

mp_string StubGetLogFilePath(mp_string strFileName) {
    mp_string fileName = "testfile";
    return fileName;
}

mp_int32 StubConnect(mp_socket clientSock, mp_uint32 uiServerAddr, mp_uint16 uiPort) {
    return MP_SUCCESS;
}

mp_int32 BindSucc(mp_socket sock, mp_char* ip, mp_uint16 uiPort) {
    return MP_SUCCESS;
}

mp_int32 BindFail(mp_socket sock, mp_char* ip, mp_uint16 uiPort) {
    return MP_FAILED;
}

mp_int32 StubCreateTcpSocket(mp_socket& sock, mp_bool keepSocketInherit, mp_bool isIPV4) {
    return MP_SUCCESS;
}

mp_int32 StubSetReuseAddr(mp_socket sock) {
    return MP_SUCCESS;
}

mp_uint16 StubGetRandomPort(mp_socket servSock, const mp_string& ip) {
    return 12345;
}

mp_int32 StubWritePort(const mp_string &file, mp_uint16 uiPort) {
    return MP_SUCCESS;
}

mp_int32 StubStartListening(mp_socket sock) {
    return MP_SUCCESS;
}

mp_void StubCloseConnect(CConnection &connection)
{
  connection.linkState = LINK_STATE_NO_LINKED;
}

mp_int32 StubReinitMsgBody() {
    return MP_SUCCESS;
}

mp_int32 StubSendMsg(CDppMessage &message, CConnection &conn) {
    return MP_SUCCESS;
}

mp_char* StubGetBufferNoKey()
{   
    DppMessage dppMessage;
    dppMessage.body = "{\"pages\": \"1\", \"items\": [{\"ip\": \"10.10.10.11\", \"port\": \"8080\", \"status\": \"1\",\"agentip\": \"10.10.10.10\"}]}";
    return dppMessage.body;
}

mp_char* StubGetBuffer()
{   
    DppMessage dppMessage;
    dppMessage.body = "{\"pages\": \"1\", \"body\": [{\"ip\": \"10.10.10.11\", \"port\": \"8080\", \"status\": \"1\",\"agentip\": \"10.10.10.10\"}]}";
    return dppMessage.body;
}

XMLElement* StubFirstChildElement(const mp_char* strSection = 0) {
    XMLDocument doc;
    XMLElement* root = doc.NewElement( "test" );
    doc.InsertEndChild( root );

    XMLElement* childElement = doc.NewElement( "test1" );
    childElement->SetAttribute( "test2", "test3" ); // ÉèÖÃÔªËØÊôÐÔ
    root->InsertEndChild( childElement );
    return childElement;
}

#endif
