#include "message/tcp/MessageHandlerTest.h"

#define StubClogToVoidLogNullPointReference()                                                                          \
    do {                                                                                                               \
        stub.set(                                                                                                      \
            (mp_int32(CConfigXmlParser::*)(const mp_string&, const mp_string&, mp_int32&))ADDR(CConfigXmlParser, GetValueInt32),     \
            StubMessageHandlerGetValueInt32Return);                                                                    \
    } while (0)

static mp_void StubCLoggerLog(mp_void)
{
    return;
}

TEST_F(MessageHandlerTest, PushReqMsg)
{
    message_pair_t msgPair;
    msgPair.pReqMsg = new CBasicReqMsg;
    MessageHandler* om = new MessageHandler;
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om->PushReqMsg(msgPair);
    EXPECT_EQ(MP_SUCCESS, iRet);

    for (int i = 0; i < MAX_QUEUE_MESSAGE_NUM + 1; i++) {
        om->msgReqList.push_back(msgPair);
    }
    iRet = om->PushReqMsg(msgPair);
    EXPECT_EQ(MP_FAILED, iRet);
    om->msgReqList.clear();
    delete om;
}

TEST_F(MessageHandlerTest, PopReqMsg)
{
    message_pair_t msgPair;
    msgPair.pReqMsg = new CBasicReqMsg;
    MessageHandler om;
    om.msgReqList.push_back(msgPair);
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.PopReqMsg(msgPair);
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = om.PopReqMsg(msgPair);
    EXPECT_EQ(MP_FAILED, iRet);
}

TEST_F(MessageHandlerTest, PushRspMsg)
{
    message_pair_t msgPair;
    msgPair.pRspMsg = new CBasicRspMsg;
    MessageHandler* om = new MessageHandler;
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om->PushRspMsg(msgPair);
    EXPECT_EQ(MP_SUCCESS, iRet);

    for (int i = 0; i < MAX_QUEUE_MESSAGE_NUM + 1; i++) {
        om->msgRspList.push_back(msgPair);
    }
    iRet = om->PushRspMsg(msgPair);
    EXPECT_EQ(MP_FAILED, iRet);
    om->msgRspList.clear();
    delete om;
}

TEST_F(MessageHandlerTest, PopRspMsg)
{
    message_pair_t msgPair;
    msgPair.pRspMsg = new CBasicRspMsg;
    MessageHandler om;
    om.msgRspList.push_back(msgPair);
    StubClogToVoidLogNullPointReference();
    stub.set(&CLogger::Log, StubCLoggerLog);
    mp_int32 iRet = om.PopRspMsg(msgPair);
    EXPECT_EQ(MP_SUCCESS, iRet);

    iRet = om.PopRspMsg(msgPair);
    EXPECT_EQ(MP_FAILED, iRet);
}