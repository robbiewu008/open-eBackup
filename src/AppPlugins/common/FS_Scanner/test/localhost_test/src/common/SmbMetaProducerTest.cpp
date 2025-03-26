// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/acl.h>
// #include <cerrno>
// #include <codecvt>
// #include "log/Log.h"
// #include "Time.h"
// #include "ScannerUtils.h"
// #include "SmbMetaProducer.h"

// #include "gtest/gtest.h"
// #include "mockcpp/mockcpp.hpp"

// using namespace std;
// using namespace Module;

// class SmbMetaProducerTest : public testing::Test {
// public:
//     void SetUp();
//     void TearDown();
//     static void SetUpTestCase();
//     static void TearDownTestCase();
//     std::unique_ptr<SmbMetaProducer> m_ins {nullptr};
// };

// void SmbMetaProducerTest::SetUp() 
// {
//     std::shared_ptr<ScanQueue> scanQueue;
//     std::shared_ptr<BufferQueue<DirectoryScan>> output;
//     std::shared_ptr<StatisticsMgr> statsMgr;
//     std::shared_ptr<ScanFilter> scanFilter;
//     std::shared_ptr<FSScannerCheckPoint> chkPntMgr;
//     Module::SmbContextArgs args;
//     ScanConfig config;
//     m_ins = std::make_unique<SmbMetaProducer>(scanQueue, output, statsMgr, scanFilter, chkPntMgr, args, config);
// }
// void SmbMetaProducerTest::TearDown() 
// {
//     GlobalMockObject::verify(); // 校验mock规范并清除mock规范
// }
// void SmbMetaProducerTest::SetUpTestCase() {}
// void SmbMetaProducerTest::TearDownTestCase() {}

// static bool Stub_Func_True()
// {
//     return true;
// }

// static bool Stub_Func_False()
// {
//     return false;
// }

// static void Stub_Func_Void()
// {}

// /*
//  * 用例名称：ResetSmbClient
//  * 前置条件：无
//  * check点：重置Smb Client
//  **/
// // TEST_F(SmbMetaProducerTest, ResetSmbClient)
// // {
// //     MOCKER_CPP(&Module::SmbContextWrapper::Init)
// //             .stubs()
// //             .will(returnValue(false))
// //             .then(returnValue(true)); 
// //     MOCKER_CPP(&Module::SmbContextWrapper::SmbConnect)
// //             .stubs()
// //             .will(returnValue(true));

// // //     EXPECT_EQ(m_ins->ResetSmbClient(), false);       
// // //     EXPECT_EQ(m_ins2->ResetSmbClient(), true); // 待解决智能指针，reset() segmatation fault问题      
// // }

// /*
//  * 用例名称：InitContext
//  * 前置条件：无
//  * check点：初始化
//  **/
// TEST_F(SmbMetaProducerTest, RequestOpendir)
// {
// //     struct smb2_context *smb = nullptr;
// //     int status = 0;
// //     m_ins->OpendirCallback(smb, status, nullptr, nullptr);
// //     m_ins->StatDirCallback(smb, status, nullptr, nullptr);
// //     m_ins->StatFileCallback(smb, status, nullptr, nullptr);

// //     MOCKER_CPP(&Module::SmbContextWrapper::SmbProcess)
// //             .stubs()
// //             .will(ignoreReturnValue()); 
// //     MOCKER_CPP(&SmbMetaProducer::ResetSmbClient)
// //             .stubs()
// //             .will(returnValue(false));
// //     m_ins->m_resetSmb = true;
// //     m_ins->Produce(0);

// //     string path = "/a/file.txt";
// //     DirectoryCacheMap *cacheMap = nullptr;
// //     std::string str = "nanjing";
// //     MOCKER_CPP(&Module::SmbContextWrapper::SmbOpendirAsync)
// //             .stubs()
// //             .will(returnValue(1));
// //     MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
// //             .stubs()
// //             .will(returnValue(str));        
// //     EXPECT_EQ(m_ins->RequestOpendir(path, cacheMap), false);
// }

// /*
//  * 用例名称：HandleFailedDirectory
//  * 前置条件：无
//  * check点：除了连接断开的场景，其他异常场景需要重试该接口
//  **/
// TEST_F(SmbMetaProducerTest, HandleFailedDirectory)
// {
// //     MOCKER_CPP(&Module::SmbContextWrapper::SmbCloseDir)
// //             .stubs()
// //             .will(ignoreReturnValue());
// //     MOCKER_CPP(&SmbMetaProducer::ReturnCacheMapToScanQueue)
// //             .stubs()
// //             .will(ignoreReturnValue()); 
// //     MOCKER_CPP(&DirectoryCacheMap::GetDirPath)
// //             .stubs()
// //             .will(ignoreReturnValue());
// //     MOCKER_CPP(&Module::SmbContextWrapper::SmbOpendirAsync)
// //             .stubs()
// //             .will(returnValue(1))
// //             .then(returnValue(0));
// //     std::string str = "nanjing";
// //     MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
// //             .stubs()
// //             .will(returnValue(str));
// //     MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
// //             .stubs()
// //             .will(ignoreReturnValue());
// //     m_ins->m_resetSmb = false;
// //     m_ins->HandleFailedDirectory(nullptr, nullptr);

// //     m_ins->m_resetSmb = true;
// //     // struct smb2dir smbDirTemp {}; // 初始化问题
// //     struct smb2dir *smbDir = nullptr;
// //     std::string path = "/a/file.txt";
// //     DirectoryCacheMap *cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// //     m_ins->HandleFailedDirectory(smbDir, nullptr);
// //     m_ins->HandleFailedDirectory(smbDir, cacheMap);   
// //     m_ins->m_resetSmb = false;
// //     m_ins->HandleFailedDirectory(smbDir, nullptr);
// //     m_ins->HandleFailedDirectory(smbDir, cacheMap);
// //     m_ins->HandleFailedDirectory(smbDir, cacheMap);
// //     EXPECT_EQ(m_ins->m_resetSmb, false);
// }

// /*
//  * 用例名称：ReStatFile
//  * 前置条件：无
//  * check点：处理失败情况
//  **/
// TEST_F(SmbMetaProducerTest, ReStatFile)
// {
//     MOCKER_CPP(&SmbMetaProducer::ClearCacheMap)
//             .stubs()
//             .will(ignoreReturnValue());
//     MOCKER_CPP(&Module::SmbContextWrapper::SmbGetInfoAsync)
//             .stubs()
//             .will(returnValue(1))
//             .then(returnValue(0));
//     std::string str = "nanjing";
//     MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
//             .stubs()
//             .will(returnValue(str));
//     MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());

//     m_ins->m_resetSmb = true;
//     string fullPath = "/a/file.txt";
//     struct SMB2_ALL_INFO *allInfo = nullptr;
//     std::string path = "/a/file.txt";
//     DirectoryCacheMap *cacheMap = new DirectoryCacheMap(nullptr, path, 0);
//     m_ins->ReStatFile(fullPath, allInfo, nullptr);
//     m_ins->ReStatFile(fullPath, allInfo, cacheMap);   
//     m_ins->m_resetSmb = false;
//     m_ins->ReStatFile(fullPath, allInfo, nullptr);
//     m_ins->ReStatFile(fullPath, allInfo, cacheMap);
//     EXPECT_EQ(m_ins->m_state, SCANNER_STATUS::FAILED);
// }

// TEST_F(SmbMetaProducerTest, ReStatFile_2)
// {
//     MOCKER_CPP(&SmbMetaProducer::ClearCacheMap)
//             .stubs()
//             .will(ignoreReturnValue()); 
// 	MOCKER_CPP(&Module::SmbContextWrapper::SmbGetInfoAsync)
//             .stubs()
//             .will(returnValue(1))
//             .then(returnValue(0));
//     std::string str = "nanjing";
//     MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
//             .stubs()
//             .will(returnValue(str));
//     MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());
//     m_ins->m_resetSmb = false;
//     string fullPath = "/a/file.txt";
//     struct SMB2_ALL_INFO *allInfo = new SMB2_ALL_INFO();
// 	std::string path = "/a/file.txt";
// 	DirectoryCacheMap *cacheMap = new DirectoryCacheMap(nullptr, path, 0);
//     allInfo->SecurityDescriptor = nullptr;
// 	m_ins->ReStatFile(fullPath, allInfo, cacheMap);
// 	allInfo->Name = new uint8_t(1);
//     allInfo->SecurityDescriptor = nullptr;
//     m_ins->ReStatFile(fullPath, allInfo, cacheMap);
//     m_ins->ReStatFile(fullPath, allInfo, cacheMap);
//     m_ins->ReStatFile(fullPath, nullptr, cacheMap);
//     EXPECT_EQ(m_ins->m_state, SCANNER_STATUS::FAILED);
// }

// /*
//  * 用例名称：SkipDirEntry
//  * 前置条件：无
//  * check点：跳过目录
//  **/
// TEST_F(SmbMetaProducerTest, SkipDirEntry)
// {
// 	m_ins->FreeSmbAllInfo(nullptr);

// 	string temp = "";
// 	for (int i = 0; i < 5000; i++) {
// 		temp += 'a';
// 	}
// 	bool ret = m_ins->SkipDirEntry("file.txt", temp);
// 	EXPECT_EQ(ret, true);
// }

// /*
//  * 用例名称：ClearCacheMap
//  * 前置条件：无
//  * check点：清空cachemap
//  **/
// // TEST_F(SmbMetaProducerTest, ClearCacheMap)
// // {
// // 	MOCKER_CPP(&DirectoryCacheMap::GetDirPath)
// //             .stubs()
// //             .will(ignoreReturnValue());
// // 	MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
// //             .stubs()
// //             .will(ignoreReturnValue());
// // 	MOCKER_CPP(&SmbMetaProducer::PushCacheMapToWriteQueue)
// //             .stubs()
// //             .will(ignoreReturnValue());			
// // 	MOCKER_CPP(&DirectoryCacheMap::CacheCompleted)
// //             .stubs()
// //             .will(returnValue(true))
// //             .then(returnValue(false)); 
// // 	string fullPath;
// // 	string path = "/a";
// // 	DirectoryCacheMap *cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// // 	m_ins->ClearCacheMap(fullPath, cacheMap);

// // 	fullPath = "/a/b/file.txt";
// // 	m_ins->ClearCacheMap(fullPath, cacheMap);
// // 	m_ins->ClearCacheMap(fullPath, cacheMap);

// // 	MOCKER_CPP(&Module::SmbContextWrapper::SmbStat64)
// //             .stubs()
// //             .will(returnValue(1))
// //             .then(returnValue(0));
// // 	DirStat dirStat;
// // 	dirStat.m_path = ".";
// // 	m_ins->FillDirStat(dirStat);
// // 	m_ins->FillDirStat(dirStat);
// // 	EXPECT_EQ(dirStat.m_path, "/");
// // }

// /*
//  * 用例名称：RequestStatFile
//  * 前置条件：无
//  * check点：封装stat报文以获得文件的元数据信息
//  **/
// TEST_F(SmbMetaProducerTest, RequestStatFile)
// {
// 	string path = "/a/file.txt";
// 	bool ret = m_ins->RequestStatFile(path, nullptr);
// 	EXPECT_EQ(ret, false);

// 	MOCKER_CPP(&Module::SmbContextWrapper::SmbGetInfoAsync)
//             .stubs()
//             .will(returnValue(1))
//             .then(returnValue(0));
// 	MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());
// 	std::string str = "nanjing";
//     MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
//             .stubs()
//             .will(returnValue(str));
// 	DirectoryCacheMap *cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	ret = m_ins->RequestStatFile(path, cacheMap);
// 	EXPECT_EQ(ret, false);

// 	ret = m_ins->RequestStatFile(path, cacheMap);
// 	EXPECT_EQ(ret, true);
// }

// /*
//  * 用例名称：RequestStatDir
//  * 前置条件：无
//  * check点：封装stat报文以获得目录的元数据信息
//  **/
// TEST_F(SmbMetaProducerTest, RequestStatDir)
// {
// 	MOCKER_CPP(&Module::SmbContextWrapper::SmbGetInfoAsync)
//             .stubs()
//             .will(returnValue(1))
//             .then(returnValue(0));
// 	MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());
// 	std::string str = "nanjing";
//     MOCKER_CPP(&Module::SmbContextWrapper::SmbGetError)
//             .stubs()
//             .will(returnValue(str));
// 	string path = "/a/file.txt";
// 	DirectoryCacheMap *cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	bool ret = m_ins->RequestStatDir(path, cacheMap);
// 	EXPECT_EQ(ret, false);

// 	ret = m_ins->RequestStatDir(path, cacheMap);
// 	EXPECT_EQ(ret, true);
// }

// /*
//  * 用例名称：StatFileResponse
//  * 前置条件：无
//  * check点：获取文件响应的元数据
//  **/
// TEST_F(SmbMetaProducerTest, StatFileResponse)
// {
// 	MOCKER_CPP(&SmbMetaProducer::ClearCacheMap)
//             .stubs()
//             .will(ignoreReturnValue()); 
// 	MOCKER_CPP(&StatisticsMgr::DecrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&SmbMetaProducer::CacheFile)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&DirectoryCacheMap::CacheCompleted)
//             .stubs()
//             .will(returnValue(true))
//             .then(returnValue(false)); 
// 	MOCKER_CPP(&SmbMetaProducer::ReStatFile)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&SmbMetaProducer::PushCacheMapToWriteQueue)
//             .stubs()
//             .will(ignoreReturnValue());
//     int *qd = new int(1);
//     MOCKER_CPP(&DirectoryCacheMap::GetData)
//             .stubs()
//             .will(returnValue((void *)qd)); 
//     MOCKER_CPP(&SmbMetaProducer::RequestQueryDir)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&SmbMetaProducer::FreeSmbAllInfo)
//             .stubs()
//             .will(ignoreReturnValue());
// 	string path = "/a/file.txt";
// 	DirectoryCacheMap *cacheMap = new DirectoryCacheMap(nullptr, path, 0);

// 	int status = -ENOENT;
// 	struct SMB2_ALL_INFO *allInfo = new SMB2_ALL_INFO();
//     allInfo->Name = new uint8_t(1);
// 	m_ins->StatFileResponse(status, allInfo, nullptr);
// 	m_ins->StatFileResponse(status, allInfo, cacheMap);

// 	status = -EBADF;
// 	m_ins->StatFileResponse(status, allInfo, cacheMap);

// 	status = -ETIMEDOUT;
// 	m_ins->StatFileResponse(status, allInfo, cacheMap);

// 	status = 1;
// 	m_ins->StatFileResponse(status, allInfo, cacheMap);
// 	m_ins->StatFileResponse(status, allInfo, cacheMap);
// 	EXPECT_EQ(m_ins->m_resetSmb, true);
// }

// /*
//  * 用例名称：StatDirResponse
//  * 前置条件：无
//  * check点：获取目录响应元数据
//  **/
// TEST_F(SmbMetaProducerTest, StatDirResponse)
// {
// 	MOCKER_CPP(&StatisticsMgr::DecrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&DirectoryCacheMap::CacheCompleted)
//             .stubs()
//             .will(returnValue(true))
//             .then(returnValue(false)); 
// 	MOCKER_CPP(&SmbMetaProducer::ReStatDir)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&SmbMetaProducer::ReturnDirToScanQueue)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&SmbMetaProducer::CacheDirectory)
//             .stubs()
//             .will(ignoreReturnValue());
//     MOCKER_CPP(&SmbMetaProducer::RequestQueryDir)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&SmbMetaProducer::FreeSmbAllInfo)
//             .stubs()
//             .will(ignoreReturnValue());
//     MOCKER_CPP(&SmbMetaProducer::RequestAccessDir)
//             .stubs()
//             .will(returnValue(false))
// 			.then(returnValue(true));
// 	string path = "/a/file.txt";
// 	DirectoryCacheMap *cacheMap = new DirectoryCacheMap(nullptr, path, 0);

// 	int status = -ENOENT;
// 	struct SMB2_ALL_INFO *allInfo = new SMB2_ALL_INFO();
//     allInfo->Name = new uint8_t(1);
// 	m_ins->StatDirResponse(status, allInfo, nullptr);
//     cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	m_ins->StatDirResponse(status, allInfo, cacheMap);

// 	status = -EBADF;
//     cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	m_ins->StatDirResponse(status, allInfo, cacheMap);

// 	status = -ETIMEDOUT;
//     cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	m_ins->StatDirResponse(status, allInfo, cacheMap);

// 	status = 1;
//     cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	m_ins->StatDirResponse(status, allInfo, cacheMap);
//     cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	m_ins->StatDirResponse(status, allInfo, cacheMap);
// 	EXPECT_EQ(m_ins->m_resetSmb, true);
// }

// /*
//  * 用例名称：AccessDirResponse
//  * 前置条件：无
//  * check点：目录响应
//  **/
// TEST_F(SmbMetaProducerTest, AccessDirResponse)
// {
// 	MOCKER_CPP(&StatisticsMgr::DecrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&DirectoryCacheMap::GetDirPath)
//             .stubs()
//             .will(ignoreReturnValue());
// 	MOCKER_CPP(&SmbMetaProducer::ReAccessDir)
//             .stubs()
//             .will(ignoreReturnValue()); 
// 	MOCKER_CPP(&SmbMetaProducer::RequestQueryDir)
//             .stubs()
//             .will(ignoreReturnValue());
//     MOCKER_CPP(&SmbMetaProducer::RequestCloseDir)
//             .stubs()
//             .will(ignoreReturnValue());
//     MOCKER_CPP(&ScanQueue::Push)
//             .stubs()
//             .will(returnValue(true));
// 	string path = "/a/file.txt";
// 	DirectoryCacheMap *cacheMap = new DirectoryCacheMap(nullptr, path, 0);

// 	int status = -ENOENT;
// 	struct smb2_query_directory_data *queryData = nullptr;
// 	m_ins->AccessDirResponse(status, queryData, nullptr);
// 	m_ins->AccessDirResponse(status, queryData, cacheMap);

// 	status = -EBADF;
//     cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	m_ins->AccessDirResponse(status, queryData, cacheMap);

// 	status = -ETIMEDOUT;
//     cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	m_ins->AccessDirResponse(status, queryData, cacheMap);

// 	status = 1;
//     cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	m_ins->AccessDirResponse(status, queryData, cacheMap);
//     cacheMap = new DirectoryCacheMap(nullptr, path, 0);
// 	m_ins->AccessDirResponse(status, queryData, cacheMap);
// 	EXPECT_EQ(m_ins->m_resetSmb, true);
// }

// /*
//  * 用例名称：Produce
//  * 前置条件：无
//  * check点：生产
//  **/
//  static bool Stub_PopBatch(ScanQueue* This, std::vector<DirStat> &dirStatList, int readCount)
//  {
// 	DirStat dirStat;
// 	dirStat.m_filterFlag = 0;
// 	dirStatList.push_back(dirStat);
// 	dirStatList.push_back(dirStat);
// 	dirStatList.push_back(dirStat);
// 	return true;
//  }
// TEST_F(SmbMetaProducerTest, Produce)
// {
// 	MOCKER_CPP(&StatisticsMgr::IncrCommStatsByType)
//             .stubs()
//             .will(ignoreReturnValue());
//         MOCKER_CPP(&Module::SmbContextWrapper::SmbProcess)
//             .stubs()
//             .will(returnValue(true));
// 	m_ins->m_resetSmb = false;
// 	uint64_t res = 0;
// 	MOCKER_CPP(&StatisticsMgr::GetCommStatsByType)
//             .stubs()
//             .will(returnValue(res));
// 	MOCKER_CPP(&ScanQueue::PopBatch)
//             .stubs()
//             .will(invoke(Stub_PopBatch)); // mockcpp打桩
// 	MOCKER_CPP(&SmbMetaProducer::RequestAccessDir)
//             .stubs()
//             .will(returnValue(false))
// 			.then(returnValue(true));
// 	MOCKER_CPP(&SmbMetaProducer::RequestStatDir)
//             .stubs()
//             .will(returnValue(false))
// 			.then(returnValue(true));	
// 	m_ins->Produce(0);	

// 	m_ins->m_resetSmb = true;
// 	MOCKER_CPP(&SmbMetaProducer::ResetSmbClient)
//             .stubs()
//             .will(returnValue(false));
// 	m_ins->Produce(0);
// 	EXPECT_EQ(m_ins->m_state, SCANNER_STATUS::PROTECTED_SERVER_NOT_REACHABLE);
// }