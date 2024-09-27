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
#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "stub.h"
#include <boost/asio/connect.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/ssl/stream.hpp>

#include <boost/asio/ssl/detail/io.hpp>
#include <boost/asio/ssl/detail/stream_core.hpp>
#include <boost/system/error_code.hpp>
#include <protect_engines/kubernetes/rest/KubernetesApi.h>
#include <protect_engines/kubernetes/common/KubeCommonInfo.h>
#include <protect_engines/kubernetes/common/KubeErrorCodes.h>
#include <protect_engines/kubernetes/rest/config/KubeConfig.h>
#include "KubernetesApiTestData.h"
#include "curl_http/HttpClientInterface.h"

namespace HDT_TEST {

using Module::IHttpResponse;
using Module::HttpRequest;
using Module::IHttpClient;
using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using namespace KubernetesPlugin;

class KubernetesApiTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

};
int32_t SendRequestStubSuccess(void * obj, HttpRequest& httpParam, HttpResponseInfo& httpResponse)
{
    httpResponse.m_statusCode = 200;
    if (httpParam.url.find("statefulsets/uvcdb-1-1-m") != std::string::npos){
        httpResponse.m_body = KubernetesTestData::t_singleStatefulSetBody;
    }
    else if (httpParam.url.find("statefulsets") != std::string::npos) {
        httpResponse.m_body = KubernetesTestData::t_statefulSetBody;
    } else if (httpParam.url.find("pods/invmdb-1-1-m-0") != std::string::npos) {
        httpResponse.m_body = KubernetesTestData::t_perPodBody;
    } else if (httpParam.url.find("pods") != std::string::npos) {
        httpResponse.m_body = KubernetesTestData::t_podBody;
    } else if (httpParam.url.find("persistentvolumes") != std::string::npos) {
        httpResponse.m_body = KubernetesTestData::t_pvBody;
    } else if (httpParam.url.find("/api/v1/namespaces") != std::string::npos) {
        httpResponse.m_body = KubernetesTestData::t_namespacesBody;
    }
    return Module::SUCCESS;
}

int32_t Stub_SendRequestForReplicasSuccess(void * obj, HttpRequest& httpParam, HttpResponseInfo& httpResponse)
{
    httpResponse.m_statusCode = 200;

    if (httpParam.method == "PATCH") {
        return Module::SUCCESS;
    } else if (httpParam.method == "Get") {
        httpResponse.m_body = KubernetesTestData::t_stsDeadStatus;
        return Module::SUCCESS;
    }
}

int32_t Stub_SendRequestForReplicasFailed(void * obj, HttpRequest& httpParam, HttpResponseInfo& httpResponse)
{
    httpResponse.m_statusCode = 0;

    if (httpParam.method == "PATCH") {
        return Module::FAILED;
    }
}

int32_t Stub_SendRequestForReplicasTimeOut(void * obj, HttpRequest& httpParam, HttpResponseInfo& httpResponse)
{
    httpResponse.m_statusCode = 200;

    if (httpParam.method == "PATCH") {
        return Module::SUCCESS;
    }  else if (httpParam.method == "Get") {
        httpResponse.m_body = KubernetesTestData::t_stsAlive;
        return Module::SUCCESS;
    }
}

void Stub_sleep()
{
    return;
}

void KubernetesApiTest::SetUp()
{

}

void KubernetesApiTest::TearDown()
{
}

void KubernetesApiTest::SetUpTestCase() {

}

void KubernetesApiTest::TearDownTestCase() {}



/**
 * 测试用例：测试ListNameSpace接口成功
 * 前置条件：底层flexvolume返回正常
 * Check点：ListNameSpace不返回错误码。
 */
TEST_F(KubernetesApiTest, ListNameSpacesShouldSuccess) {
    Stub stub;
    stub.set(ADDR(KubeClient, SendRequest), SendRequestStubSuccess);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);
    std::vector<ApplicationResource> nameSpaceList;
    std::tie(ret, nameSpaceList) = kubernetesApi.ListNameSpaces();

    EXPECT_EQ(nameSpaceList[0].name, "ns000000000000000000001");
    EXPECT_EQ(nameSpaceList[0].id, "70775d73-c7cf-47b5-b140-14959679b96c");
}

/**
 * 测试用例：测试ListStorages接口成功
 * 前置条件：底层flexvolume返回正常
 * Check点：ListStorages不返回错误码，且返回对应存储的rest信息。
 */
TEST_F(KubernetesApiTest, ListStoragesShouldSuccess) {
    Stub stub;
    stub.set(ADDR(KubeClient, SendRequest), SendRequestStubSuccess);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);
    std::set<std::string> storageUrlSet;
    std::tie(ret, storageUrlSet) = kubernetesApi.ListStorages();

    EXPECT_EQ(*storageUrlSet.begin(), "https://8.40.111.70:8088/deviceManager/rest");
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：测试ListStateFulSet接口成功
 * 前置条件：底层返回正常。
 * Check点：ListStateFulSet返回值为0, 判断stateFulSet模板中VolumeName数组大小和Pod中的PV数组大小相同
 */
TEST_F(KubernetesApiTest, ListStateFulSetShouldSuccess) {
    Stub stub;
    stub.set(ADDR(KubeClient, SendRequest), SendRequestStubSuccess);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    Application nameSpaceResource;
    nameSpaceResource.id = "";
    nameSpaceResource.name = "ns000000000000000000001";
    std::vector<ApplicationResource> stateFulSetResourceList;
    std::tie(ret, stateFulSetResourceList) = kubernetesApi.ListStatefulSet(nameSpaceResource);
    EXPECT_EQ(ret, 0);
    EXPECT_GE(stateFulSetResourceList.size(), 0);
    for (auto item : stateFulSetResourceList) {
        StateFulSetExtend stsExtendInfo;
        Module::JsonHelper::JsonStringToStruct(item.extendInfo, stsExtendInfo);
        StateFulSet stateFulSet;
        Module::JsonHelper::JsonStringToStruct(stsExtendInfo.sts, stateFulSet);
        EXPECT_EQ(stateFulSet.volumeNames.size(), stateFulSet.pods[0].pvs.size());
    }
}

/**
 * 测试用例：测试ListStateFulSet接口获取单个sts成功
 * 前置条件：底层返回正常。
 * Check点：ListStateFulSet返回值为0, 判断stateFulSet模板中VolumeName数组大小和Pod中的PV数组大小相同
 */
TEST_F(KubernetesApiTest, ListStateFulSetFetchSingleStsSuccess) {
Stub stub;
stub.set(ADDR(KubeClient, SendRequest), SendRequestStubSuccess);

KubeClient client;
auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
KubernetesApi kubernetesApi(client, config);

Application nameSpaceResource;
nameSpaceResource.id = "";
nameSpaceResource.name = "ns000000000000000000001";
nameSpaceResource.extendInfo = "uvcdb-1-1-m";
std::vector<ApplicationResource> stateFulSetResourceList;
std::tie(ret, stateFulSetResourceList) = kubernetesApi.ListStatefulSet(nameSpaceResource);
EXPECT_EQ(ret, 0);
EXPECT_EQ(stateFulSetResourceList.size(), 1);
for (auto item : stateFulSetResourceList) {
StateFulSetExtend stsExtendInfo;
Module::JsonHelper::JsonStringToStruct(item.extendInfo, stsExtendInfo);
StateFulSet stateFulSet;
Module::JsonHelper::JsonStringToStruct(stsExtendInfo.sts, stateFulSet);
EXPECT_EQ(stateFulSet.volumeNames.size(), stateFulSet.pods[0].pvs.size());
}
}

/**
 * 测试用例：测试ListPods接口成功
 * 前置条件：底层返回正常。
 * Check点：ListPods返回值为0，且查找出的pods的数量为1
 */
TEST_F(KubernetesApiTest, ListPodsShouldSuccess) {
    Stub stub;
    stub.set(ADDR(KubeClient, SendRequest), SendRequestStubSuccess);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    Application statefulSet;
    statefulSet.name = "invmdb-1-1-m";
    statefulSet.parentName = "ns000000000000000000001";
    std::vector<ApplicationResource> pods;
    std::tie(ret, pods) = kubernetesApi.ListPods(statefulSet);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(pods.size(), 1);
}

/**
 * 测试用例：测试ListStateFulSet接口成功
 * 前置条件：底层返回正常。
 * Check点：ListStateFulSet返回值为0
 */
TEST_F(KubernetesApiTest, ListPvsShouldSuccess) {
    Stub stub;
    stub.set(ADDR(KubeClient, SendRequest), SendRequestStubSuccess);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    ApplicationResource pod;
    pod.name = "invmdb-1-1-m-0";
    pod.parentName = "ns000000000000000000001";
    std::vector<std::string> volumeNames = {"gmdbredo", "gmdbdata", "gmdblredo"};

    std::vector<Pv> pvs;
    std::tie(ret, pvs) = kubernetesApi.ListPvs(pod, volumeNames);
    EXPECT_EQ(ret, 0);
    EXPECT_EQ(pvs.size(), 3);
}

/**
 * 测试用例：测试StopStateFulSet接口成功
 * 前置条件：底层返回正常。
 * Check点：StopStateFulSet返回值为0。
 */
TEST_F(KubernetesApiTest, StopStateFulSetShouldSuccess) {
    Stub stub;
    stub.set(ADDR(KubeClient, SendRequest), Stub_SendRequestForReplicasSuccess);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    StateFulSet sts;
    sts.nameSpace = "ns000000000000000000001";
    sts.name = "adaptermdb-1-1-m";
    sts.replicasNum = 1;

    int result = kubernetesApi.StopStateFulSet(sts);
    EXPECT_EQ(result, Module::SUCCESS);
}

/**
 * 测试用例：测试StopStateFulSet接口失败
 * 前置条件：FlexVolume设置接口返回失败。
 * Check点：StopStateFulSet返回值为-1。
 */
TEST_F(KubernetesApiTest, StopStateFulSetShouldFailedWhenFlexVolumeFailed) {
    Stub stub;
    stub.set(ADDR(KubeClient, SendRequest), Stub_SendRequestForReplicasFailed);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    StateFulSet sts;
    sts.nameSpace = "ns000000000000000000001";
    sts.name = "adaptermdb-1-1-m";
    sts.replicasNum = 1;

    int result = kubernetesApi.StopStateFulSet(sts);
    EXPECT_EQ(result, Module::FAILED);
}

/**
 * 测试用例：测试StopStateFulSet接口失败
 * 前置条件：FlexVolume设置接口成功，但StateFulSet状态一直是活跃的。
 * Check点：StopStateFulSet返回值为-1。
 */
TEST_F(KubernetesApiTest, StopStateFulSetShouldFailedWhenStsAlive) {
    Stub stub;
    stub.set(ADDR(KubeClient, SendRequest), Stub_SendRequestForReplicasTimeOut);
    stub.set(sleep, Stub_sleep);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    StateFulSet sts;
    sts.nameSpace = "ns000000000000000000001";
    sts.name = "adaptermdb-1-1-m";
    sts.replicasNum = 1;

    int result = kubernetesApi.StopStateFulSet(sts);
}

/**
 * 测试用例：测试RestoreStateFulSet接口成功
 * 前置条件：底层返回正常。
 * Check点：RestoreStateFulSet返回值为0。
 */
TEST_F(KubernetesApiTest, RestoreStateFulSetShouldSuccess) {
    Stub stub;
    stub.set(ADDR(KubeClient, SendRequest), Stub_SendRequestForReplicasSuccess);
    stub.set(sleep, Stub_sleep);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    StateFulSet sts;
    sts.nameSpace = "ns000000000000000000001";
    sts.name = "adaptermdb-1-1-m";
    sts.replicasNum = 1;

    int result = kubernetesApi.RestoreStateFulSet(sts);
    EXPECT_EQ(result, 0);
}

/**
 * 测试用例：测试RestoreStateFulSet接口成功
 * 前置条件：FlexVolume Patch接口返回失败。
 * Check点：RestoreStateFulSet返回值为FAIELD。
 */
TEST_F(KubernetesApiTest, RestoreStateFulSetShouldFailedWhenFlexVolumeFailed) {
    Stub stub;
    stub.set(ADDR(KubeClient, SendRequest), Stub_SendRequestForReplicasFailed);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    StateFulSet sts;
    sts.nameSpace = "ns000000000000000000001";
    sts.name = "adaptermdb-1-1-m";
    sts.replicasNum = 1;

    int result = kubernetesApi.RestoreStateFulSet(sts);
    EXPECT_EQ(result, Module::FAILED);
}

/**
 * 测试用例：检查构造的WebSocket请求URL是否满足要求
 * 前置条件：输入参数满足合法
 * Check点： 返回正确的URL
 */
TEST_F(KubernetesApiTest, CreateWebsocketRequestSuccess) {
    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    std::string url = kubernetesApi.CreateWebsocketRequest("ns1", "pod1", "container1", "sh test.sh");
    EXPECT_EQ(url, "/api/v1/namespaces/ns1/pods/pod1/exec?command=sh&command=test.sh&container=container1&stderr=true&stdout=true");
}

/**
 * 测试用例：检查构造的WebSocket请求URL是否满足要求
 * 前置条件：输入参数不满足合法
 * Check点： 返回的URL为空
 */
TEST_F(KubernetesApiTest, CreateWebsocketRequestFailed) {
    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    std::string url = kubernetesApi.CreateWebsocketRequest("ns1", "pod1", "container1", "");
    EXPECT_EQ(url, "");
}

std::pair<int, std::string> Stub_GetHttpBodyFromFlexVolume_WhenUrlisSts_Failed(void *obj, const std::string &url)
{
    if (url.find("statefulset") != std::string::npos) {
        return std::make_pair(Module::FAILED, "");
    }
    return std::make_pair(Module::SUCCESS, "");
}

std::pair<int, std::string> Stub_GetHttpBodyFromFlexVolume_WhenUrlHasPods_Failed(void *obj, const std::string &url)
{
    if (url.find("pods") != std::string::npos) {
        return std::make_pair(Module::FAILED, "");
    }
    return std::make_pair(Module::SUCCESS, "");
}

std::pair<int, std::string> Stub_GetHttpBodyFromFlexVolume_WhenUrlIsPV_Failed(void *obj, const std::string &url)
{
    if (url.find("persistentvolumes") != std::string::npos) {
        return std::make_pair(Module::FAILED, "");
    } else if (url.find("pods") != std::string::npos) {
        return std::make_pair(Module::SUCCESS, KubernetesTestData::t_podBody);
    }
    return std::make_pair(Module::SUCCESS, "");
}

std::pair<int, std::string> Stub_GetHttpBodyFromFlexVolume_ThrowInvalidNode(void *obj, const std::string &url)
{
    throw YAML::InvalidNode("Invalid Node from GetHttpBodyFromFlexVolume");
}

std::pair<int, std::string> Stub_GetHttpBodyFromFlexVolume_WhenUrlisNameSpaces_Failed(void *obj, const std::string &url)
{
    if (url.find("namespaces") != std::string::npos) {
        return std::make_pair(Module::FAILED, "");
    }
    return std::make_pair(Module::SUCCESS, "");
}

std::pair<int, std::string> Stub_GetHttpBodyFromFlexVolume_Success(void *obj, const std::string &url)
{
    int ret = Module::SUCCESS;
    std::string retBody = "";
    if (url.find("illegalNameSpaceForPod") != std::string::npos) {
        retBody = KubernetesTestData::t_illPodsBody;
    }
    return std::make_pair(ret, retBody);
}


std::vector<ApplicationResource> Stub_GetNameSpacesFromHttpBody_ThrowInvalidNode(void *obj,
                                                                                 const std::string &stsBody,
                                                                                 const std::string &namespaceBody)
{
    throw YAML::InvalidNode("invalid Node");
}

std::vector<ApplicationResource> Stub_GetNameSpacesFromHttpBody_ThrowParseError(void *obj,
                                                                                const std::string &stsBody,
                                                                                const std::string &namespaceBody)
{
    throw YAML::ParserException(YAML::Mark::null_mark(), "parseException");
}

/**
 * 测试用例：测试ListNameSpace接口失败
 * 前置条件：当url含statefulsets时，GetHttpBodyFromFlexVolume接口返回失败
 * Check点：ListNameSpace返回失败。
 */
TEST_F(KubernetesApiTest, ListNameSpaces_Failed_WhenGetStateFulSet_Failed)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_WhenUrlisSts_Failed);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);
    std::vector<ApplicationResource> nameSpaceList;
    int result = 0;
    std::tie(result, nameSpaceList) = kubernetesApi.ListNameSpaces();
    EXPECT_EQ(result, Module::FAILED);
}

/**
 * 测试用例：测试ListNameSpace接口失败
 * 前置条件：当url含namespace时，GetHttpBodyFromFlexVolume接口返回失败
 * Check点：ListNameSpace返回失败。
 */
TEST_F(KubernetesApiTest, ListNameSpaces_Failed_WhenGetNameSpace_Failed)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_WhenUrlisNameSpaces_Failed);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);
    std::vector<ApplicationResource> nameSpaceList;
    int result = 0;
    std::tie(result, nameSpaceList) = kubernetesApi.ListNameSpaces();
    EXPECT_EQ(result, Module::FAILED);
}

/**
 * 测试用例：测试ListNameSpace接口失败
 * 前置条件：当url含namespace时，GetHttpBodyFromFlexVolume接口成功，GetNameSpacesFromHttpBody抛出InvalidNode异常。
 * Check点：ListNameSpace返回失败。
 */
TEST_F(KubernetesApiTest, ListNameSpaces_Failed_WhenGetNameSpacesFromHttpBody_ThrowInvalidNode)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_Success);
    stub.set(ADDR(KubernetesApi, GetNameSpacesFromHttpBody), Stub_GetNameSpacesFromHttpBody_ThrowInvalidNode);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);
    std::vector<ApplicationResource> nameSpaceList;
    int result = 0;
    std::tie(result, nameSpaceList) = kubernetesApi.ListNameSpaces();
    EXPECT_EQ(result, CONNECT_FAILED);
}

/**
 * 测试用例：测试ListNameSpace接口失败
 * 前置条件：当url含namespace时，GetHttpBodyFromFlexVolume接口成功，GetNameSpacesFromHttpBody抛出ParseException异常。
 * Check点：ListNameSpace返回失败。
 */
TEST_F(KubernetesApiTest, ListNameSpaces_Failed_WhenGetNameSpacesFromHttpBody_ThrowParseError)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_Success);
    stub.set(ADDR(KubernetesApi, GetNameSpacesFromHttpBody), Stub_GetNameSpacesFromHttpBody_ThrowParseError);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);
    std::vector<ApplicationResource> nameSpaceList;
    int result = 0;
    std::tie(result, nameSpaceList) = kubernetesApi.ListNameSpaces();
    EXPECT_EQ(result, CONNECT_FAILED);
}

/**
 * 测试用例：测试ListPods接口成功
 * 前置条件：当url含pods时，有些pod中不含containers字段
 * Check点：ListPods接口成功
 */
TEST_F(KubernetesApiTest, ListPods_Success_When_GetPodHasSomeIllInput)
{
    Stub stub;
    // 测试数据中bmpdb中containers为空，adaptermdb中无containers字段
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_Success);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);
    Application sts;
    sts.parentName = "illegalNameSpaceForPod";
    sts.name = "adaptermdb-1-1-m";

    std::vector<ApplicationResource> podList;
    int result = 0;
    std::tie(result, podList) = kubernetesApi.ListPods(sts);
    EXPECT_EQ(result, Module::SUCCESS);
    EXPECT_TRUE(podList.empty());

    sts.name = "bmpdb-1-1-m";
    std::tie(result, podList) = kubernetesApi.ListPods(sts);
    EXPECT_EQ(result, Module::SUCCESS);
    EXPECT_TRUE(podList.empty());
}

/**
 * 测试用例：测试ListPvs接口失败
 * 前置条件：调用/api/v1/persistentvolumes接口返回失败。
 * Check点：ListPvs返回值为Failed
 */
TEST_F(KubernetesApiTest, ListPvs_Failed_WhenGetPV_Failed)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_WhenUrlIsPV_Failed);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    ApplicationResource pod;
    pod.name = "invmdb-1-1-m-0";
    pod.parentName = "ns000000000000000000001";
    std::vector<std::string> volumeNames = {"gmdbredo", "gmdbdata", "gmdblredo"};

    std::vector<Pv> pvs;
    int result = 0;
    std::tie(result, pvs) = kubernetesApi.ListPvs(pod, volumeNames);
    EXPECT_EQ(result, Module::FAILED);
}

/**
 * 测试用例：测试ListPvs接口失败
 * 前置条件：底层k8s返回异常数据，GetHttpBodyFromFlexVolume，抛出异常。
 * Check点：ListPvs返回值为Failed
 */
TEST_F(KubernetesApiTest, ListPvs_Failed_WhenGetPV_Exception)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_ThrowInvalidNode);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    ApplicationResource pod;
    pod.name = "invmdb-1-1-m-0";
    pod.parentName = "ns000000000000000000001";
    std::vector<std::string> volumeNames = {"gmdbredo", "gmdbdata", "gmdblredo"};

    std::vector<Pv> pvs;
    int result = 0;
    std::tie(result, pvs) = kubernetesApi.ListPvs(pod, volumeNames);
    EXPECT_EQ(result, Module::FAILED);
}

/**
 * 测试用例：测试ListStatefulSet接口失败
 * 前置条件：底层k8s调用list pods接口异常。
 * Check点：ListStatefulSet返回值为Failed
 */
TEST_F(KubernetesApiTest, ListStatefulSet_Failed_WhenGetPods_Failed)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_WhenUrlHasPods_Failed);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    Application nameSpaceResource;
    nameSpaceResource.id = "";
    nameSpaceResource.name = "ns000000000000000000001";
    std::vector<ApplicationResource> stateFulSetResourceList;
    int result = 0;
    std::tie(result, stateFulSetResourceList) = kubernetesApi.ListStatefulSet(nameSpaceResource);
    EXPECT_EQ(result, Module::FAILED);
}

/**
 * 测试用例：测试ListStatefulSet接口失败
 * 前置条件：底层k8s调用list pv接口异常。
 * Check点：ListStatefulSet返回值为Failed
 */
TEST_F(KubernetesApiTest, ListStatefulSet_Failed_WhenGetPV_Failed)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_WhenUrlIsPV_Failed);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    Application nameSpaceResource;
    nameSpaceResource.id = "";
    nameSpaceResource.name = "ns000000000000000000001";
    std::vector<ApplicationResource> stateFulSetResourceList;
    int result = 0;
    std::tie(result, stateFulSetResourceList) = kubernetesApi.ListStatefulSet(nameSpaceResource);
    EXPECT_EQ(result, Module::FAILED);
}

/**
 * 测试用例：测试ListStatefulSet接口失败
 * 前置条件：底层k8s调用list statefulsets接口异常。
 * Check点：ListStatefulSet返回值为Failed
 */
TEST_F(KubernetesApiTest, ListStatefulSet_Failed_WhenGetSts_Failed)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_WhenUrlisSts_Failed);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    Application nameSpaceResource;
    nameSpaceResource.id = "";
    nameSpaceResource.name = "ns000000000000000000001";
    std::vector<ApplicationResource> stateFulSetResourceList;
    int result = 0;
    std::tie(result, stateFulSetResourceList) = kubernetesApi.ListStatefulSet(nameSpaceResource);
    EXPECT_EQ(result, Module::FAILED);
}

std::pair<int, ApplicationResource> Stub_ConvertStateFulSetToAppResource_Failed(void *obj, StateFulSet &stateFulSet,
                                                                                const std::string &nameSpaceId) {
    ApplicationResource resource;
    return std::make_pair(CONNECT_FAILED, resource);
}

/**
 * 测试用例：测试ListStatefulSet接口失败
 * 前置条件：ConvertStateFulSetToAppResource转换失败。
 * Check点：ListStatefulSet返回值为CONNECT_FAILED
 */
TEST_F(KubernetesApiTest, ListStatefulSet_Failed_ConvertStateFulSetToAppResource_Failed)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, ConvertStateFulSetToAppResource), Stub_ConvertStateFulSetToAppResource_Failed);
    stub.set(ADDR(KubeClient, SendRequest), SendRequestStubSuccess);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    Application nameSpaceResource;
    nameSpaceResource.id = "";
    nameSpaceResource.name = "ns000000000000000000001";
    std::vector<ApplicationResource> stateFulSetResourceList;
    int result = 0;
    std::tie(result, stateFulSetResourceList) = kubernetesApi.ListStatefulSet(nameSpaceResource);
    EXPECT_EQ(result, CONNECT_FAILED);
}


std::vector<StateFulSet> Stub_GetStatefulSetsFromHttpBody_ThrowInvalidNode(void *obj, const std::string &stsBody) {
    throw YAML::InvalidNode("invalid Node");
}

/**
 * 测试用例：测试ListStatefulSet接口失败
 * 前置条件：GetStatefulSetsFromHttpBody接口抛出异常。
 * Check点：ListStatefulSet返回值为CONNECT_FAILED
 */
TEST_F(KubernetesApiTest, ListStatefulSet_Failed_GetStatefulSetsFromHttpBody_ThrowExcept)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetStatefulSetsFromHttpBody), Stub_GetStatefulSetsFromHttpBody_ThrowInvalidNode);
    stub.set(ADDR(KubeClient, SendRequest), SendRequestStubSuccess);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    Application nameSpaceResource;
    nameSpaceResource.id = "";
    nameSpaceResource.name = "ns000000000000000000001";
    std::vector<ApplicationResource> stateFulSetResourceList;
    int result = 0;
    std::tie(result, stateFulSetResourceList) = kubernetesApi.ListStatefulSet(nameSpaceResource);
    EXPECT_EQ(result, CONNECT_FAILED);
}

/**
 * 测试用例：测试ListStorages接口失败
 * 前置条件：GetStatefulSetsFromHttpBody接口抛出异常。
 * Check点：ListStorages返回值为CONNECT_FAILED
 */
TEST_F(KubernetesApiTest, ListStorages_Failed_GetPV_Failed)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, GetHttpBodyFromFlexVolume), Stub_GetHttpBodyFromFlexVolume_WhenUrlIsPV_Failed);
    stub.set(ADDR(KubeClient, SendRequest), SendRequestStubSuccess);

    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);


    std::set<std::string> storageResourceSet;
    int result = 0;
    std::tie(result, storageResourceSet) = kubernetesApi.ListStorages();
    EXPECT_EQ(result, Module::FAILED);
}

using NextLayerStream = boost::asio::basic_stream_socket<boost::asio::ip::tcp, boost::asio::execution::any_executor<boost::asio::execution::context_as_t<boost::asio::execution_context &>, boost::asio::execution::detail::blocking::never_t<0>, boost::asio::execution::prefer_only<boost::asio::execution::detail::blocking::possibly_t<0>>, boost::asio::execution::prefer_only<boost::asio::execution::detail::outstanding_work::tracked_t<0>>, boost::asio::execution::prefer_only<boost::asio::execution::detail::outstanding_work::untracked_t<0>>, boost::asio::execution::prefer_only<boost::asio::execution::detail::relationship::fork_t<0>>, boost::asio::execution::prefer_only<boost::asio::execution::detail::relationship::continuation_t<0>>>>;
std::size_t Stub_io_handshake(NextLayerStream& next_layer, boost::asio::ssl::detail::stream_core& core, const boost::asio::ssl::detail::handshake_op& op, boost::system::error_code& ec) {
    return 0;
}

bool Stub_SetSSLContext_Failed(void *obj, boost::asio::ssl::context &sslCtx) {
    return false;
}

/**
 * 测试用例：测试KubeExec接口失败
 * 前置条件：config.SetSSLContext返回失败。
 * Check点：返回空字符串。
 */

TEST_F(KubernetesApiTest, KubeExec_Failed_When_SetSSLContext_Failed)
{
    Stub stub;
    stub.set(ADDR(KubeConfig, SetSSLContext), Stub_SetSSLContext_Failed);
    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);


    std::string cmdResult = kubernetesApi.KubeExec("ns0001", "pod01", "container01", "ls -al");
    EXPECT_EQ("", cmdResult);
}

bool Stub_GetClusterServer_Failed(void *obj, std::string &ip, std::string &port) {
    return false;
}

/**
 * 测试用例：测试KubeExec接口成功
 * 前置条件：config.GetClusterServer返回失败。
 * Check点：返回空字符串。
 */
TEST_F(KubernetesApiTest, KubeExec_Failed_When_GetClusterServer_Failed)
{
    Stub stub;
    stub.set((bool (KubeConfig::*)(std::string&, std::string&) const)ADDR(KubeConfig, GetClusterServer), Stub_GetClusterServer_Failed);
    KubeClient client;
    auto [ret, config] = KubeConfig::Create(KubernetesTestData::t_codedContents);
    KubernetesApi kubernetesApi(client, config);

    std::string cmdResult = kubernetesApi.KubeExec("ns0001", "pod01", "container01", "ls -al");
    EXPECT_EQ("", cmdResult);
}

}