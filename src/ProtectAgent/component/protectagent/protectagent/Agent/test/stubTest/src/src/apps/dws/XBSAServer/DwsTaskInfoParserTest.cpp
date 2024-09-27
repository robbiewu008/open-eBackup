#include "common/ConfigXmlParse.h"
#include "common/Path.h"
#include "apps/dws/XBSAServer/DwsTaskInfoParserTest.h"

namespace {
mp_void LogTest(const mp_int32 iLevel, const mp_int32 iFileLine, const mp_string& pszFileName,
    const mp_string& pszFuncction, const mp_string& pszFormat, ...) {}

#define DoGetJsonStringTest() do { \
    stub.set(ADDR(CLogger, Log), LogTest); \
} while (0)

mp_int32 Stub_GetValueInt32_Succ(mp_void* pthis, const mp_string& strSection, const mp_string& strKey,
    mp_int32& iValue)
{
    iValue = 100;
    return MP_SUCCESS;
}

mp_string Stub_GetStmpFilePath(const mp_string& strFileName)
{
    return "/tmp/dws_cacheInfo.txt";
}
}

/*
* 测试用例：dws_cacheInfo.txt不存在
* 前置条件：dws_cacheInfo.txt不存在
* CHECK点：解析函数返回失败
*/
TEST_F(DwsTaskInfoParserTest, ParseCacheInfoTest_FileNotExist)
{
    DoGetJsonStringTest();
    DwsTaskInfoParser parser;
    DwsCacheInfo info;
    mp_int32 ret = parser.ParseCacheInfo(info, std::string("/tmp"));
    EXPECT_NE(ret, MP_SUCCESS);
}

/*
* 测试用例：dws_cacheInfo.txt解析失败
* 前置条件：dws_cacheInfo.txt文件存在但内容Json格式错误
* CHECK点：解析函数返回失败
*/
TEST_F(DwsTaskInfoParserTest, ParseCacheInfoTest_FileFormatError)
{
    DoGetJsonStringTest();
    DwsTaskInfoParser parser;
    DwsCacheInfo info;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
        Stub_GetValueInt32_Succ);
    stub.set((mp_string(CPath::*)(mp_string))ADDR(CPath, GetStmpFilePath), Stub_GetStmpFilePath);

    mp_string cacheFilePath = CPath::GetInstance().GetStmpFilePath("dws_cacheInfo.txt");
    mp_string fileBuf = "{\"cacheRepoPath\":\"\",\"copyId\":\"\"}";
    string cmd = "echo \'" + fileBuf + "\' > " + cacheFilePath;
    system(cmd.c_str());
    mp_string cache_path = "/tmp";
    mp_int32 ret = parser.ParseCacheInfo(info, cache_path);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"cacheRepoPath\":\"1111\",\"copyId\":\"\"}";
    cmd = "echo \'" + fileBuf + "\' > " + cacheFilePath;
    system(cmd.c_str());
    ret = parser.ParseCacheInfo(info, cache_path);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"cacheRepoPath\":\"\",\"copyId\":\"1111\"}";
    cmd = "echo \'" + fileBuf + "\' > " + cacheFilePath;
    system(cmd.c_str());
    ret = parser.ParseCacheInfo(info, cache_path);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"cacheRepoPath\":\"111\",\"copyId\":\"1111\"}";
    cmd = "echo \'" + fileBuf + "\' > " + cacheFilePath;
    system(cmd.c_str());
    ret = parser.ParseCacheInfo(info, cache_path);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"cacheRepoPath\":\"111\",\"metaRepoPath\":\"111\",\"copyId\":\"1111\"}";
    cmd = "echo \'" + fileBuf + "\' > " + cacheFilePath;
    system(cmd.c_str());
    ret = parser.ParseCacheInfo(info, cache_path);
    EXPECT_NE(ret, MP_SUCCESS);

    cmd = "rm -f " + cacheFilePath;
    system(cmd.c_str());
}

/*
* 测试用例：dws_cacheInfo.txt解析成功
* 前置条件：dws_cacheInfo.txt文件存在且格式正确
* CHECK点：解析函数返回成功
*/
TEST_F(DwsTaskInfoParserTest, ParseCacheInfoTest_Normal)
{
    DoGetJsonStringTest();
    DwsTaskInfoParser parser;
    DwsCacheInfo info;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
        Stub_GetValueInt32_Succ);
    stub.set((mp_string(CPath::*)(mp_string))ADDR(CPath, GetStmpFilePath), Stub_GetStmpFilePath);

    mp_string cacheFilePath = CPath::GetInstance().GetStmpFilePath("dws_cacheInfo.txt");
    mp_string fileBuf = "{\"cacheRepoPath\":\"111\",\"metaRepoPath\":\"222\",\"copyId\":\"2222\",\"taskId\":\"3333\","
        "\"hostKey\":\"192.168.1.100\"}";
    string cmd = "echo \'" + fileBuf + "\' > " + cacheFilePath;
    system(cmd.c_str());
    mp_string cache_path = "/tmp";
    mp_int32 ret = parser.ParseCacheInfo(info, cacheFilePath);
    EXPECT_EQ(ret, MP_SUCCESS);
    EXPECT_EQ((info.cacheRepoPath == "111"), true);
    EXPECT_EQ((info.metaRepoPath == "222"), true);
    EXPECT_EQ((info.copyId == "2222"), true);
    EXPECT_EQ((info.taskId == "3333"), true);
    EXPECT_EQ((info.hostKey == "192.168.1.100"), true);

    cmd = "rm -f " + cacheFilePath;
    system(cmd.c_str());
}

/*
* 测试用例：taskInfo.txt不存在
* 前置条件：taskInfo.txt不存在
* CHECK点：解析函数返回失败
*/
TEST_F(DwsTaskInfoParserTest, ParseTaskInfo_FileNotExist)
{
    DoGetJsonStringTest();
    DwsTaskInfoParser parser;
    DwsCacheInfo cacheInfo;
    DwsTaskInfo taskInfo;
    mp_int32 ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);
}

/*
* 测试用例：taskInfo.txt解析失败
* 前置条件：taskInfo.txt文件存在但内容Json格式错误
* CHECK点：解析函数返回失败
*/
TEST_F(DwsTaskInfoParserTest,  ParseTaskInfo_FileFormatError)
{
    DoGetJsonStringTest();
    DwsTaskInfoParser parser;
    DwsCacheInfo cacheInfo;
    DwsTaskInfo taskInfo;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
        Stub_GetValueInt32_Succ);

    cacheInfo.cacheRepoPath = "";
    cacheInfo.copyId = "";
    cacheInfo.hostKey = "host1";
    mp_string taskFilePath = cacheInfo.cacheRepoPath + "/tmp/" + cacheInfo.copyId + "/" + "taskInfo_host1.txt";
    cout << "taskFilePath=" << taskFilePath << endl;

    // 输入参数缺失或为空
    mp_string fileBuf = "{\"taskType\":\"\",\"copyType\":1}";
    string cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    mp_int32 ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"1111\",\"copyType\":1}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"\",\"copyType\":1}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"111\",\"copyType\":1}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":2}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":0,\"deviceSN\":\"\"}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\"}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"\"}]}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"\"}]}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\"}]}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"\"}]}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"fs1\"}]}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"fs1\",\"mountPath\":[]}]}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    // role=0的repo数目为0.
    fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":1,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"fs1\",\"mountPath\":[\"/tmp/dws1/192.168.0.101\"]}]}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    // copyType不合法.
    fileBuf = "{\"taskType\":\"111\",\"copyType\":0,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"fs1\",\"mountPath\":[\"/tmp/dws1/192.168.0.101\"]}]}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    // copyType为归档到云副本但是没有fileserver信息
    fileBuf = "{\"taskType\":\"111\",\"copyType\":8,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"fs1\",\"mountPath\":[\"/tmp/dws1/192.168.0.101\"]}]}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    // copyType为归档到云副本,fileserver不合法
    fileBuf = "{\"taskType\":\"111\",\"copyType\":8,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"fs1\",\"mountPath\":[\"/tmp/dws1/192.168.0.101\"]}]}],"
        "\"archiveFileServers\":[{\"ip\":\"\",\"port\":\"3006\"}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    // copyType为归档到云副本,fileserver不合法
    fileBuf = "{\"taskType\":\"111\",\"copyType\":8,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"fs1\",\"mountPath\":[\"/tmp/dws1/192.168.0.101\"]}]}],"
        "\"archiveFileServers\":[{\"ip\":\"192.168.0.100\",\"port\":65536}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_NE(ret, MP_SUCCESS);

    cmd = "rm -f " + taskFilePath;
    system(cmd.c_str());
}

/*
* 测试用例：taskInfo.txt解析成功
* 前置条件：taskInfo.txt文件存在且内容Json格式正确
* CHECK点：解析函数返回成功
*/
TEST_F(DwsTaskInfoParserTest,  ParseTaskInfo_Normal)
{
    DoGetJsonStringTest();
    DwsTaskInfoParser parser;
    DwsCacheInfo cacheInfo;
    DwsTaskInfo taskInfo;
    stub.set((mp_int32(CConfigXmlParser::*)(const mp_string&,const mp_string&,mp_int32&))ADDR(CConfigXmlParser,GetValueInt32), 
        Stub_GetValueInt32_Succ);

    cacheInfo.cacheRepoPath = "";
    cacheInfo.copyId = "";
    cacheInfo.hostKey = "host1";
    mp_string taskFilePath = cacheInfo.cacheRepoPath + "/tmp/" + cacheInfo.copyId + "/" + "taskInfo_host1.txt";
    cout << "taskFilePath=" << taskFilePath << endl;

    mp_string fileBuf = "{\"taskType\":\"111\",\"copyType\":1,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"fs1\","
        "\"mountPath\":[\"/tmp/dws1/192.168.0.100\",\"/tmp/dws1/192.168.0.101\"]}]}]}";
    mp_string cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    mp_int32 ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_EQ(ret, MP_SUCCESS);

    // 归档副本
    fileBuf = "{\"taskType\":\"111\",\"copyType\":8,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"fs1\","
        "\"mountPath\":[\"/tmp/dws1/192.168.0.101\"]}]}],"
        "\"archiveFileServers\":[{\"ip\":\"192.168.0.100\",\"port\":30066}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_EQ(ret, MP_SUCCESS);

    // 归档到云副本不检查repositories，mountPath可以为空
    fileBuf = "{\"taskType\":\"111\",\"copyType\":8,\"repositories\":[{\"role\":0,\"deviceSN\":\"esn1\","
        "\"filesystems\":[{\"id\":\"11\",\"name\":\"fs1\",\"sharePath\":\"fs1\",\"mountPath\":[]}]}],"
        "\"archiveFileServers\":[{\"ip\":\"192.168.0.100\",\"port\":30066}]}";
    cmd = "echo \'" + fileBuf + "\' > " + taskFilePath;
    system(cmd.c_str());
    ret = parser.ParseTaskInfo(cacheInfo, taskInfo);
    EXPECT_EQ(ret, MP_SUCCESS);

    cmd = "rm -f " + taskFilePath;
    system(cmd.c_str());
}

/*
* 测试用例：ParseFsRelation接口测试，文件不存在
* 前置条件：无
* CHECK点：解析函数返回失败
*/
TEST_F(DwsTaskInfoParserTest, ParseFsRelation_FileNotExist)
{
    DoGetJsonStringTest();
    DwsTaskInfoParser parser;
    DwsFsRelation info;
    mp_string filePath = "/tmp/ParseFsRelation_FileNotExist";
    mp_int32 ret = parser.ParseFsRelation(filePath, info);
    EXPECT_NE(ret, MP_SUCCESS);
}

/*
* 测试用例：ParseFsRelation接口测试，文件格式不正确
* 前置条件：无
* CHECK点：解析函数返回失败
*/
TEST_F(DwsTaskInfoParserTest, ParseFsRelation_FileFormatError)
{
    DoGetJsonStringTest();
    DwsTaskInfoParser parser;
    DwsFsRelation info;
    mp_string filePath = "/tmp/ParseFsRelation_FileFormatError";

    // role不合法
    mp_string fileBuf = "{\"relations\":[{\"role\":2,\"oldEsn\":\"esn1\",\"oldFsName\":\"fs1\",\"oldFsId\":\"123\","
        "\"newEsn\":\"esn1\",\"newFsName\":\"fs1\",\"newFsId\":\"123\"}]}";
    mp_string cmd = "echo \'" + fileBuf + "\' > " + filePath;
    system(cmd.c_str());
    mp_int32 ret = parser.ParseFsRelation(filePath, info);
    EXPECT_NE(ret, MP_SUCCESS);

    // oldEsn为空
    fileBuf = "{\"relations\":[{\"role\":2,\"oldEsn\":\"\",\"oldFsName\":\"fs1\",\"oldFsId\":\"123\","
        "\"newEsn\":\"esn1\",\"newFsName\":\"fs1\",\"newFsId\":\"123\"}]}";
    cmd = "echo \'" + fileBuf + "\' > " + filePath;
    system(cmd.c_str());
    ret = parser.ParseFsRelation(filePath, info);
    EXPECT_NE(ret, MP_SUCCESS);

    // newFsName为空
    fileBuf = "{\"relations\":[{\"role\":2,\"oldEsn\":\"esn1\",\"oldFsName\":\"fs1\",\"oldFsId\":\"123\","
        "\"newEsn\":\"esn1\",\"newFsName\":\"\",\"newFsId\":\"123\"}]}";
    cmd = "echo \'" + fileBuf + "\' > " + filePath;
    system(cmd.c_str());
    ret = parser.ParseFsRelation(filePath, info);
    EXPECT_NE(ret, MP_SUCCESS);

    // relations为空
    fileBuf = "{\"relations\":[]}";
    cmd = "echo \'" + fileBuf + "\' > " + filePath;
    system(cmd.c_str());
    ret = parser.ParseFsRelation(filePath, info);
    EXPECT_NE(ret, MP_SUCCESS);

    cmd = "rm -f " + filePath;
    system(cmd.c_str());
}

/*
* 测试用例：ParseFsRelation接口测试，文件格式正确
* 前置条件：无
* CHECK点：解析函数返回成功
*/
TEST_F(DwsTaskInfoParserTest, ParseFsRelation_Normal)
{
    DoGetJsonStringTest();
    DwsTaskInfoParser parser;
    DwsFsRelation info;
    mp_string filePath = "/tmp/ParseFsRelation_Normal";
    mp_string fileBuf = "{\"relations\":[{\"role\":0,\"oldEsn\":\"esn1\",\"oldFsName\":\"fs1\",\"oldFsId\":\"123\","
        "\"newEsn\":\"esn1\",\"newFsName\":\"fs1\",\"newFsId\":\"123\"}]}";
    mp_string cmd = "echo \'" + fileBuf + "\' > " + filePath;
    system(cmd.c_str());
    mp_int32 ret = parser.ParseFsRelation(filePath, info);
    EXPECT_NE(ret, MP_SUCCESS);

    cmd = "rm -f " + filePath;
    system(cmd.c_str());
}

/*
* 测试用例：ParseBusConfig接口测试，文件格式正确
* 前置条件：无
* CHECK点：解析函数返回成功
*/
TEST_F(DwsTaskInfoParserTest, ParseBusConfig_Normal)
{
    DoGetJsonStringTest();
    DwsTaskInfoParser parser;
    XbsaBusinessConfig info;
    mp_string filePath = "/cachePath/tmp/copyId/business_config.txt";
    mp_string fileBuf = "{\"jobType\":\"0\"}";
    mp_string cmd = "echo \'" + fileBuf + "\' > " + filePath;
    system(cmd.c_str());
    mp_int32 ret = parser.ParseBusConfig("cachePath", "copyId", info);
    EXPECT_NE(ret, MP_SUCCESS);
    cmd = "rm -f " + filePath;
    system(cmd.c_str());
}
