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
#include <common/PluginTypes.h>
#include <protect_engines/engine_factory/EngineFactory.h>
#include <protect_engines/kubernetes/rest/KubernetesApi.h>
#include <protect_engines/kubernetes/common/KubeCommonInfo.h>
#include <protect_engines/kubernetes/common/KubeErrorCodes.h>
#include <protect_engines/kubernetes/rest/config/KubeConfig.h>
#include <protect_engines/kubernetes/rest/client/StorageClient.h>
#include <protect_engines/kubernetes/rest/client/KubeClient.h>
#include "protect_engines/kubernetes/rest/KubernetesApiTestData.h"
#include "job_controller/jobs/VirtualizationBasicJob.h"
#include <protect_engines/kubernetes/util/Transformer.h>
#include <volume_handlers/oceanstor/OceanStorVolumeHandler.h>

#include <protect_engines/kubernetes/KubernetesProtectEngine.h>

namespace HDT_TEST {

using ::testing::_;
using ::testing::Return;
using ::testing::ReturnRef;
using VirtPlugin::JobHandle;
using Module::HttpRequest;
using namespace KubernetesPlugin;

int32_t g_logSize = 0;

class KubernetesProtectEngineTest : public testing::Test {
public:
    void SetUp();
    void TearDown();
    static void SetUpTestCase();
    static void TearDownTestCase();

    // 备份Handler
    std::shared_ptr<KubernetesProtectEngine> m_k8sProtectEngineHandler;
    // 恢复Handler
    std::shared_ptr<KubernetesProtectEngine> m_k8sRestoreProtectEngineHandler;
};

void KubernetesProtectEngineTest::SetUp()
{
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::make_shared<AppProtect::BackupJob>();
    std::shared_ptr<JobCommonInfo> m_jobInfo = std::make_shared<JobCommonInfo>(backupJob);
    std::shared_ptr<VirtPlugin::JobHandle> m_jobHandle =
        std::make_shared<VirtPlugin::JobHandle>(JobType::BACKUP, m_jobInfo);
    m_k8sProtectEngineHandler = std::make_shared<KubernetesProtectEngine>(m_jobHandle, "", "");


    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::make_shared<AppProtect::RestoreJob>();
    std::shared_ptr<JobCommonInfo> m_restoreJobInfo = std::make_shared<JobCommonInfo>(restoreJob);
    std::shared_ptr<VirtPlugin::JobHandle> m_restoreJobHandle = std::make_shared<VirtPlugin::JobHandle>(JobType::RESTORE, m_restoreJobInfo);
    m_k8sRestoreProtectEngineHandler = std::make_shared<KubernetesProtectEngine>(m_restoreJobHandle, "", "");
}

void KubernetesProtectEngineTest::TearDown()
{
}

void KubernetesProtectEngineTest::SetUpTestCase() {
}

void KubernetesProtectEngineTest::TearDownTestCase() {}


std::pair<bool, KubeConfig> Stub_CreatConfigSuccess(const std::string &codedConfig)
{
    KubeConfig dummy;
    return std::make_pair(true, dummy);
}

std::pair<bool, KubeConfig> Stub_CreatConfigFailed(const std::string &codedConfig)
{
    KubeConfig dummy;
    return std::make_pair(false, dummy);
}

std::pair<int, std::vector<ApplicationResource>> Stub_ListNameSpaceSuccess(void * obj)
{
    std::vector<ApplicationResource> ret;
    ApplicationResource nameSpace;
    nameSpace.type = "Namespace";
    nameSpace.subType = "KubernetesNamespace";
    ret.push_back(nameSpace);
    return std::make_pair(0, ret);
}

int32_t StubTestDeviceConnectionSuccess(const std::string &authExtendInfo, int32_t &erro)
{
    return Module::SUCCESS;
}

std::pair<int, std::vector<ApplicationResource>> Stub_ListNameSpaceFail(void *obj)
{
    return std::make_pair(-1, std::vector<ApplicationResource>{});
}

std::pair<int, std::set<std::string>> Stub_ListStorageSuccess(void * obj)
{
    std::set<std::string> result;
    result.insert("https://8.40.111.70:8088/deviceManager/rest");
    return std::make_pair(0, result);
}

std::pair<int, std::vector<ApplicationResource>> Stub_ListStatefulSetSuccess(void * obj, const Application &nameSpace)
{
    std::vector<ApplicationResource> ret;
    ApplicationResource sts;
    sts.type = "StatefulSet";
    sts.subType = "KubernetesStatefulSet";
    ret.push_back(sts);
    return std::make_pair(0, ret);
}

std::pair<int, std::vector<ApplicationResource>> Stub_ListStatefulSetFailed(void * obj, const Application &nameSpace)
{
    std::vector<ApplicationResource> ret;
    return std::make_pair(-1, ret);
}

std::pair<int32_t, StorageDeviceInfo> Stub_StorageClient_GetDeviceBaseInfo()
{
    int32_t ret = Module::SUCCESS;
    StorageDeviceInfo storageDeviceInfo;
    return std::make_pair(Module::SUCCESS, storageDeviceInfo);
}
std::pair<int32_t, LunInfoData> Stub_StorageClient_GetLunInfoData(const std::string &lunName)
{
    LunInfoData lunInfoData;
    if (lunName == "1-adaptermdb-1-1-m-0-lredo") {
        lunInfoData.m_wwn = "64cf55b1009769b4150f7d5e00000025";
    } else if (lunName == "1-adaptermdb-1-1-m-0-redo") {
        lunInfoData.m_wwn = "64cf55b1009769b4150f7d5e00000023";
    } else if (lunName == "1-adaptermdb-1-1-m-0-data") {
        lunInfoData.m_wwn = "64cf55b1009769b4150f7d5e00000024";
    }
    lunInfoData.m_sectorSize = "512";
    lunInfoData.m_capacity = "128";
    return std::make_pair(Module::SUCCESS, lunInfoData);
}

int Stub_StopStateFulSet_Success(const StateFulSet &sts) {
    return Module::SUCCESS;
}

int Stub_StopStateFulSet_Failed(const StateFulSet &sts) {
    return Module::FAILED;
}

int Stub_RestoreStateFulSet_Success(const StateFulSet &sts) {
    return Module::SUCCESS;
}

int Stub_RestoreStateFulSet_Failed(const StateFulSet &sts) {
    return Module::FAILED;
}

int Stub_CheckProtectEnvConn_Success(const AppProtect::ApplicationEnvironment &env) {
    return Module::SUCCESS;
}

int Stub_CheckAllStorageConnection_Success(const std::string &authExtendInfo){
    return Module::SUCCESS;
}

std::optional<KubernetesApi> Stub_GetKubernetesApiFromAppEnv_success(const std::string &appEnvAuthExtend){
    std::string authExtendInfo = R"({"config":"a2luZDogQ29uZmlnCmFwaVZlcnNpb246IHYxCmNsdXN0ZXJzOgotIGNsdXN0ZXI6CiAgICBjZXJ0aWZpY2F0ZS1hdXRob3JpdHktZGF0YTogTFMwdExTMUNSVWRKVGlCRFJWSlVTVVpKUTBGVVJTMHRMUzB0Q2sxSlNVaE9la05EUWxJclowRjNTVUpCWjBsS1FVMXZhblpFUVZWYVJrWnJUVUV3UjBOVGNVZFRTV0l6UkZGRlFrTjNWVUZOU1Vjd1RWRnpkME5SV1VRS1ZsRlJSMFYzU2tSVWFrVlRUVUpCUjBFeFZVVkRRWGRLVWpOV2FHSnRaRVZpTWpWdVRWSkZkMFIzV1VSV1VWRklSRUZvVkdGSFZuVlhiV2hzWW1wRmJBcE5RMDFIUVRGVlJVTm5kMk5UU0Zab1pESldjRWxHVW14Wk1taDFZako0ZGxveWJHeGplVUpFWW5rMGMwbEZlREJhUkVWb1RVSTRSMEV4VlVWRGQzZFpDbFF4VGxSSlExbG5WVEpXZVdSdGJHcGFVMEpWWWpJNWMyTjVRa1ZhV0VJd1RWSkpkMFZCV1VSV1VWRkVSRUZzVUZVeFRYcE1ha0ZuVVRCRmVFbEVRV1VLUW1kcmNXaHJhVWM1ZHpCQ1ExRkZWMFZYT1hwamVrNXFXVlZDYjJSWFJqTmFWMnQxV1RJNWRFMUNORmhFVkVsNVRVUlpkMDFxUVhwT1JHY3hUbXh2V0FwRVZFMTVUVVJWZWsxRVFYcE9SR2N4VG14dmQyZGlVWGhEZWtGS1FtZE9Wa0pCV1ZSQmEwNVBUVkpKZDBWQldVUldVVkZKUkVGc1NHUlhSblZhTUZKMkNtSnRZM2hGVkVGUVFtZE9Wa0pCWTAxRFJrNXZXbGMxWVdGSFZuVk5VMVYzU1hkWlJGWlJVVXRFUW5oSlpGZEdNMXBYYTJkV1IxWnFZVWMxZG1KSE9XNEtZVmRXZWtsRlRuWk1hWGRuVkVoU2EwMVRSWGRJZDFsRVZsRlJURVJDYUZCVk1VMW5TbWxDVkZwWVNqSmhWMDVzU1VaU2RtSXllSHBKUlZKc1kwaFJlQXBGYWtGUlFtZE9Wa0pCVFUxRFZUbFVWWHBOZFUxRFFrUlJWRVZuVFVJMFIwTlRjVWRUU1dJelJGRkZTa0ZTV1ZKaU0wNTZUVEpPYUZGSGFERlpXR1JzQ21GVE5XcGlNakIzWjJkSmFVMUJNRWREVTNGSFUwbGlNMFJSUlVKQlVWVkJRVFJKUTBSM1FYZG5aMGxMUVc5SlEwRlJSRk5LUkhsd2QzbFlURWhDTHpVS2JYSkZhR2gwV1RWalQzbGxUbVF2VVVWNGJuWlJZV1JhUW5KSGQwRjVNVzQ1YjNvM2JGTTVWREpHUm01NmJVbFlVMDVLWlZkVVFYVk5PV1F5V0c1RGFBbzRXVE5oYVdoemIxSTBORmhWTDNWd015dE5ZWFJPVG1kMlpuTXpORGhYYm05M2RHdFRkREpHVW1FcmNuaEZWbWQxZDIwNFNubHVkV2xDWmpaMVZ6WjBDbFpuYUVSSVMzSXJSbVVyYVV0ek5uaGlOWGR4YXpGWlRuaE1VRVkxVEdVeWNWVkdlVEJVY1ZKSU9YTmFUazlwWkZOdk9XUk1aR0pNVEROa2RreHNjRzhLUTBoVFF6QmpPUzkzTmxaVU5YVlRkMVY2V1VkT2MwUnlaeXRxVWtGVFdtaE5SVnBWVGtoNVJXeGxkWGQwUzBwNVlsTjJSbmhwV0hKa2RHdDRWMVphYndwcFRrOW5WbkZOZGpCWU5HOVJha1J3YlRWdlNYQnZhR2hYV0VWUE5VVndhblpZZEdkYU5rSnRiVlZVTURFclpHMWFkMjFpVXpadmRGSTFWemhPTjBoTUNrcHhTVFpRYVRWRlIwNUlSakppTkROR1VGUnZNblo0UjBSclNFRjFORXhPU0RkS05rRm5lRVZ1TW1rMlJtOTNXRWd6ZWtwd1FqQmFaVzVaYW5WMlZUWUtOMFE1UVdjcmFFUnVjRzkxUW5CSlVXaENSR28zVVdKSk5YUXhjVzFNVDFNMU1FMTNTWEFyWmxwWGFsVmtNMmxsYml0TVNqaGlRWFpZWTNKa1NYbENWd28yTmxweVV6STVPR0paYlVkWFdUTnJhVWhuUVZsalptWnBMMkkwWXpaNk1uWnNWWG8yYmxWMmJ5OWpXbHBaYTFOR1dHOVNWWE5uUzNJNWMzcFZNblppQ2s1VmRIbFVkekZqVFRKTk1qaDRNVWQwTDNkWVpsTlZlVXNyVjJKMVYzZHpkMFJRYTBOcE9HVk1RVTloU0dZeVFUaEdNa1prYUVkVk1FaGlVV1p6TVVrS1ZrTlJLMkpWWkdWVGMxWm1PVUY1VURGU2RHSnNWMjF1YzNkTU1FYzRiemRtY3pjeWRrNXRTVVIyY2pGc1pXZHRLMHh0VVVKRVpEVkVUVTFYWlVOcE9RcFVVbGRVUXpseE1tWmpVbHAxTldWeU0wOVJSVmdyZFVKT05IZEVkRkZKUkVGUlFVSnZORWxDVTBSRFEwRlZVWGRJVVZsRVZsSXdUMEpDV1VWR1RucFZDbkp2Wm5kR1RHOTJaR0pIZFhnd1F6ZHViRTAyVjBZNFpFMUpTSEJDWjA1V1NGTk5SV2RsUlhkblpEWkJSazU2VlhKdlpuZEdURzkyWkdKSGRYZ3dRemNLYm14Tk5sZEdPR1J2V1VjMmNFbEhNMDFKUnpCTlVYTjNRMUZaUkZaUlVVZEZkMHBFVkdwRlUwMUNRVWRCTVZWRlEwRjNTbEl6Vm1oaWJXUkZZakkxYmdwTlVrVjNSSGRaUkZaUlVVaEVRV2hVWVVkV2RWZHRhR3hpYWtWc1RVTk5SMEV4VlVWRFozZGpVMGhXYUdReVZuQkpSbEpzV1RKb2RXSXllSFphTW14c0NtTjVRa1JpZVRSelNVVjRNRnBFUldoTlFqaEhRVEZWUlVOM2QxbFVNVTVVU1VOWloxVXlWbmxrYld4cVdsTkNWV0l5T1hOamVVSkZXbGhDTUUxU1NYY0tSVUZaUkZaUlVVUkVRV3hRVlRGTmVreHFRV2RSTUVWNFNVUkJaVUpuYTNGb2EybEhPWGN3UWtOUlJWZEZWemw2WTNwT2FsbFZRbTlrVjBZeldsZHJkUXBaTWpsMFoyZHJRWGxwVHpoTlFsSnJWVmRSZDBSQldVUldVakJVUWtGVmQwRjNSVUl2ZWtGTVFtZE9Wa2hST0VWQ1FVMURRVkZaZDBoQldVUldVakJTQ2tKQ1ZYZEZORVZTWWpOT2VrMHlUbWhSUjJneFdWaGtiR0ZUTldwaU1qQjNSRkZaU2t0dldrbG9kbU5PUVZGRlRFSlJRVVJuWjBsQ1FVTlVTWGxFVkdzS05YQlBRVVJrVldzemJVdGpUR1Z5V2pGR1NVMTVhMlkxUzNScFFVNTBZMHhyUjJZNVdGTjVURmw2WW5OQ2FUWklhbTVUTjJ0MFoxZFFhRWQ2WlN0b2RRcENhMGwzYkhkNlJEVmlkbU0wSzJWV1Uyd3ZUa0Y1YzBsSmFuQlNialI1Y0cxRmVYQkNRM3B6TVdwV1FuVndiak5SVERWYVoyTkZNSFJWZEZvemJVbHNDblJxTVdwSVJqQmtVSFoxY3padk0yc3pNaXN5TjIwMlRVeHFOeXRFYW1GTGRUSkJlalY1YkROdWVTdFBVVGs1VkZKemRWQk9XblYxUjNWSGJuUndORUVLUml0MGREWjBlbTB5VkRnd1ptZG1WVmxQTDNGMVNHdEJTVGRoWlRSQk5FWkxOSGhZYldKeldWRmxaR0Z2VmxoMU1VVldiRFExTkZCWVIxaG5aMjB2ZVFwT01qUlpRbXNyYzFSWWRFNUpUMjVyU21Rd1NFZE5VRzlYT1U1SlRFUk5aM2tyYTB0VmRFWkJVakV5T1Rad1lsaHJUM3A1ZVRWSk5IQjJSV3BzTlRSV0NuVkNWRzF2WW1aRlJFUlFNbk5YV2sxa1RrbFNlRlZUVVdwV2JHRm9Na3B5TVhGTFdFOWtOVlpJTTBaVU1ucDRlbXRJZGpkMGJrZHVWblpSZEc5cU1Xb0tOVzEzT1cxS0sydFRhMDFEY1VoRFZEZElXRGRuVWxkeWRERmlTekZSTkRoMlVYbHZObGx5VG01MmVrZExaMEZPZUhZNWNrZE9Na3RPWm5wclowSjNTQXA2T1RGU2QwOVZUbnBDVUhsc00yMUZRUzh5Wm5OUlVrWXlPRVZ6YURrMFprOUdTVmRYUnpOeVdHTjBRa1UyWm5oMWNUVmlhVUYwZG01SFoxVkthbWhYQ2tkcWREVTNVREpFZWtkNU5rMTBWM2RSUzJJMlRsZ3haMVZpZFZsQldYZHZOelpRU1RsaWVuWlFlVU53UTJFeWFXaEplR1YzYTNOWlNHSndVMWhDUkdVS0syZHZhR05KY21oNlYybE9hamMwUXpCNVRXMXZURzl4WkhWNFpHVk9OM2RrYWpnNVJWTlpTbXRaZG5kU2F6RkZRWHAwWVZOeFlXMUNOV2RoYzA5UFRRb3dUVVp5TTA1a2FISkpVVFpVUmxOQ1VFOU5SV2xYTUU5bE9VdFRTbEZ3TW5oWlJuRUtMUzB0TFMxRlRrUWdRMFZTVkVsR1NVTkJWRVV0TFMwdExRbz0KICAgIHNlcnZlcjogaHR0cHM6Ly84LjQwLjEzNy43OjU0NDMKICBuYW1lOiBjbHVzdGVyCnVzZXJzOgotIG5hbWU6IGNmZS1tYXN0ZXIKICB1c2VyOgogICAgY2xpZW50LWNlcnRpZmljYXRlLWRhdGE6IExTMHRMUzFDUlVkSlRpQkRSVkpVU1VaSlEwRlVSUzB0TFMwdENrMUpTVWN3ZWtORFFreDFaMEYzU1VKQlowbEpWRmRYUTBsUlpqZ3ZWa2wzUkZGWlNrdHZXa2xvZG1OT1FWRkZURUpSUVhkbllsRjRRM3BCU2tKblRsWUtRa0ZaVkVGclRrOU5Va2wzUlVGWlJGWlJVVWxFUVd4SVpGZEdkVm93VW5aaWJXTjRSVlJCVUVKblRsWkNRV05OUTBaT2IxcFhOV0ZoUjFaMVRWTlZkd3BKZDFsRVZsRlJTMFJDZUVsa1YwWXpXbGRyWjFaSFZtcGhSelYyWWtjNWJtRlhWbnBKUlU1MlRHbDNaMVJJVW10TlUwVjNTSGRaUkZaUlVVeEVRbWhRQ2xVeFRXZEthVUpVV2xoS01tRlhUbXhKUmxKMllqSjRla2xGVW14alNGRjRSV3BCVVVKblRsWkNRVTFOUTFVNVZGVjZUWFZOUTBKRVVWUkZaMDFDTkVjS1ExTnhSMU5KWWpORVVVVktRVkpaVW1JelRucE5NazVvVVVkb01WbFlaR3hoVXpWcVlqSXdkMGxDWTA1TmFrbDNUbXBCZVUxRVZYZE5hbEV5VjJoblVBcE5ha0V6VFZSQk1rMUVTWGRPVkVGNVRrUmFZVTFKUjNCTlVYTjNRMUZaUkZaUlVVZEZkMHBFVkdwRlUwMUNRVWRCTVZWRlEwSk5TbEl6Vm1oaWJXUkZDbUl5Tlc1TlVrVjNSSGRaUkZaUlVVaEZkMmhVWVVkV2RWZHRhR3hpYWtWc1RVTk5SMEV4VlVWRGFFMWpVMGhXYUdReVZuQkpSbEpzV1RKb2RXSXllSFlLV2pKc2JHTjVRa1JpZVRSelNVVjRNRnBFUldoTlFqaEhRVEZWUlVOM2QxbFVNVTVVU1VOWloxVXlWbmxrYld4cVdsTkNWV0l5T1hOamVVSkZXbGhDTUFwTlVrbDNSVUZaUkZaUlVVUkZkMnhRVlRGTmVreHFRV2RSTUVWNFJsUkJWRUpuVFhCQlVVVlVSRWhDYUZsWVRYUmlNakIwV1RJNWVWcFVRME5CYVVsM0NrUlJXVXBMYjFwSmFIWmpUa0ZSUlVKQ1VVRkVaMmRKVUVGRVEwTkJaMjlEWjJkSlFrRk9WV05hU1VacWQwWTNablU0YWpkMlJtdG9WbUZIVlhORVdsSUtkMGx2WnpOb1JISmhWemxEVFRaaVFqWmhaRXAxVEVOeWEzSnBSM1p3VGt0cFZWUm1hVkZZVFhaeVZWWk1iMmROUVdsUmJrazRSR1JoZGtGcmVYZDViZ294T0hCM2NGRjZZME5hYm01SVR6Rllaamw0VEN0SFdHTkhUV013Tm5OaFNVUk9lbmRzUTNacE9URlhNemgwVlVod2EwRjVibFpuVm0weFpuUm9OVFUxQ21OaFRXSXhWVkZEV1RGNGNpOVRTRk13WlVOaFNsUkVOQ3RqZVZwTlNuWjNSRzg0VlhoblRuRTFhMU5LVlZwTVEybEhlbUZ1V1dKdVNGVmtLeXR1ZWxJS2VHOVNPR05LVlhseGVtWXhlVmRqTjAxNGRUbDFaR0YwUm1WMll5czVURk5KTlRaM1MwYzFjbFE1WkVkUU5VNWlkelF6TVhKdVJFaFJURTkxYUZCTE5BcGFRamd5UXk5R2NHWnRZelJZVG05c1dEQTVXVUp1WTBSV1lrTnVTMlJFUVc1SWVtcGlWbnAwVVVGUWExUm9ORVYxV1hsMVNGQlhURFJDU1RkWE5sVjBDbWt3Y21weFZFTTBaVEZuYlRKNlpuTjJZa3MxVG1ocFVsaHdUblpETlVwbk1GSnRVR05CTjFsT05YRnBiMHhNTDFjMGJqbHZRa3htYzBkVGJuRkJia0lLZDBGTFMwSnlSRzUzV2xjelVVcEdWalp5UldsWk9FZFBUVk5aWjB4cFVuSnplVXhNZURGV05pc3dVbkpyUm0xRFpGUlRlVmxvTVZGQlJUSllRbGhVVVFwVVQwdFFSMlZYYjJndlFVbE9Vbk5NVDNSMWNVMUllVmQzUTFSaFIyazBTa2hXUVcxc0wxTTVaRzEzVjFrMU4xQnpTek5qUjIxS1NtUllWRWxFUmpKU0NsbG5iRGN3VVRWWWNFRmxRems1Tkc5WGRFWmtVMGRuTW5VdmNXdFJhbmR2UjNoSFpqUlNTemR2YzB4ME5rdEJUVUpTTjFJclJuaHlkMEUyYzJoRWFVUUtORXBsUkdZM2FUaFpZWFZFYW01T2NGRXpMMDV2WlVSS1lsRkZabFphYzNkSmQzcHFVVk5KVDFwelpubEJORUZNYUVOeWNqZEpialJtTDBKcFpteG9UQXBLYkUxUGJHTjBhSEUzWXpjdlRtNHpRV2ROUWtGQlIycG5aVGgzWjJWM2QwUm5XVVJXVWpCUVFWRklMMEpCVVVSQlowdHJUVUl3UjBFeFZXUktVVkZYQ2sxQ1VVZERRM05IUVZGVlJrSjNUVU5DWjJkeVFtZEZSa0pSWTBSQlZFRk5RbWRPVmtoU1RVSkJaamhGUVdwQlFVMUNPRWRCTVZWa1NYZFJXVTFDWVVFS1JrNTZWWEp2Wm5kR1RHOTJaR0pIZFhnd1F6ZHViRTAyVjBZNFpFMUpSMHhDWjA1V1NGSkZSV2RaVFhkbldVTkRRak5DYUZsWVRYUmlNakpEUVVsSllncExhVFZyV2xkYWFHUlhlREJNYms0eVdYazFhbUpJVm5wa1IxWjVURzE0ZGxreVJuTm5hRmx4VEcwNWRFeHVUakpaZVRWcVlraFdlbVJIVm5sTWJYaDJDbGt5Um5ObmFHOXhURzB4YUdKdFJtNWFVelY2WkcxTmRWa3llREZqTTFKc1kyazFjMkl5VG1oaVNXTkZjMEozUVVGWlkwVnpRbmRCUVc5alJXWjNRVUVLUVZsalJVTkRhVXBDTkdORlEwTnBTa0kwWTBWRFEybEtRbnBCVGtKbmEzRm9hMmxIT1hjd1FrRlJjMFpCUVU5RFFXZEZRVWRNVkdSUVVXbFBha1l6THdwWUt5OUZibm94T0RKVUt6bG1TMGsyYlhvclMwVjJOME42WTNKaFJ5OTVhM0I0YmxsT2JWaHhlbEYyS3preVYwOXhTemczY1VoR2VHNU1SME4yTDAxMENqSlJkRGtyVGtVeWRsbDZWRkpZWVZkdFVuaDVUVmNyVkVWTmN6bEdWVU5sYldobFJqbFpSVTF2TW1OTVJHeHZVbkZzZURseVEyVnJiRmhHTkZSNVJUQUtZVEIzVUZKTk9VVlZlVmxFVlRod01uaFZWSGRrVWt4NGJHVjBWVU13YVhWSmFYQldhVmRYT0dwM1NqRlNWQzh2TmxoRVFWVnpkVklyV1dOT1dFTnBkQW93VVZWNVRVaGxkRVk1TVRsRFlXazVNamhDUzJ4NFpHRjJZMHR5UTBwU2FXRkZUV3REYms4M1dFMXVRa1Z6WTBORE9HVk1LMFo0Um1WcmFuTnZjMDQ1Q2xJM09IbHhURXBuU1U5NFpEUTNTRWhtUVc4MlpVWk1TMGs0U1hCMVJYVXdTSFI1T0cxcVMxRjJNMHRwYXpab05YbFBVM2hrYzNwaVpERTJjamhEWkRJS1ptaE1LMDAyYUdVd2FTOHlWMFExZUZOWGQweElNMGRxY3pjeVVIcG1aMnR3U0dvdlRHaHlhbmxEV0ZKR2FHbDRaMUJMZDJ4TVp6TlRMekZzUTA1alNBcEthekZWWTBGSmRESTVhVWdyWlZSQlRXZHRUR1E0U0hwc1drWm5kemxQZUhoT2NFWm9XVzV6UTBwQ1EyOXVaVGhaVkc5SGFDdDFiMU5MVXpCWFFtZ3JDazFDUms5bmRXUXpPRGNyTmpnMVVIZ3dWa2RDVldWd2NuUmlUbWtyUkVaT09FbENibUUxV2s1b1NUSXJaMnBKVDJack9XdFdUbkEyZEdOdFNYSlVkVE1LVWtkNlJTOHZiazA0VUc1alRHNWFaVWRWVFRaMmF6YzVZM3BKVUROa2IzUlFSMWhNUzNCYVNXRm9RVE5VYVVaVE5WTlBNVWRrVjAwdk9Ib3djMjlST0FvckwwWXlibVJzZEhSUVVrc3pSM1JsZWxOWk1uQnRhVUpsVjFvM01IVkVOMlpLTnpKUE5qUkhVSE5sVDBzMU5VOUdSSGhpZEZWNVVHOHZiblk1VjFkSUNqSjBhVlpVZGxnMlpGcEZkR2hrYWs5Qk4xYzRaRXRSYmxoT2JrWkhVMk05Q2kwdExTMHRSVTVFSUVORlVsUkpSa2xEUVZSRkxTMHRMUzBLCiAgICBjbGllbnQta2V5LWRhdGE6IExTMHRMUzFDUlVkSlRpQlNVMEVnVUZKSlZrRlVSU0JMUlZrdExTMHRMUXBOU1VsS1NuZEpRa0ZCUzBOQlowVkJNVko0YTJkWFVFRllkQ3MzZVZCMU9GZFRSbFp2V2xOM1RteElRV2xwUkdWRlQzUndZakJKZW5CelNIQndNRzAwQ25OTGRWTjFTV0VyYXpCeFNsSk9LMHBDWTNrcmRGSlZkV2xCZDBOS1EyTnFkMDR4Y1RoRFZFeEVTMlpZZVc1RGJFUk9kMHB0WldOak4xWmtMek5GZGpRS1dtUjNXWGg2VkhGNGIyZE5NMUJEVlVzclRETldZbVo1TVZGbGJWRkVTMlJYUWxkaVZpc3lTRzV1YkhodmVIWldVa0ZLYWxoSGRqbEpaRXhTTkVwdmJBcE5VR28xZWtwcmQyMHZRVTlxZUZSSFFUSnliVkpKYkZKcmMwdEpZazV4WkdoMVkyUlNNemMyWms1SVIyaEllSGRzVkV0eVRpOVlTbHA2YzNwSE56STFDakZ4TUZZMk9YbzNNSFJKYW01eVFXOWliWFJRTVRCWkwyc3hka1JxWmxkMVkwMWtRWE0yTmtVNGNtaHJTSHBaVERoWGJDdGFlbWhqTW1sV1psUXhaMGNLWkhkT1ZuTkxZM0F3VFVOalprOU9kRmhQTVVGQksxSlBTR2RUTldwTE5HTTVXWFpuUldwMFluQlRNa3hUZFU5d1RVeG9OMWREWW1KT0szazVjM0pyTWdwSFNrWmxhekk0VEd0dFJGSkhXVGwzUkhSbk0yMXhTMmR6ZGpsaWFXWXlaMFYwSzNkYVMyVnZRMk5JUVVGdmIwZHpUMlpDYkdKa1FXdFdXSEZ6VTBwcUNuZFpOSGhLYVVGMVNrZDFla2x6ZGtoV1dISTNVa2QxVVZkWlNqRk9URXBwU0ZaQlFWUmFZMFprVGtKTk5HODRXalZoYVVnNFFXY3hSM2R6TmpJMmIzY0taa3BpUVVwT2IyRk1aMnRrVlVOaFdEbE1NVEppUWxwcWJuTXJkM0prZDJGWmEyd3haRTFuVFZoYVJtbERXSFpTUkd4bGEwSTBURE16YVdoaE1GWXhTUXBoUkdFM0szRlNRMUJEWjJKRldpOW9SWEoxYVhkMU0yOXZRWGRHU0hSSU5GaEhka0ZFY1hsRlQwbFFaMncwVGk5MVRIaG9jVFJQVDJNeWJFUm1PREpvQ2pSTmJIUkJVamxXYlhwQmFrUlBUa0pKWnpWdGVDOUpSR2RCZFVWTGRYWnphV1pvTHpoSFNpdFhSWE50VlhjMlZua3lSM0owZW5ZNE1tWmpRMEYzUlVFS1FWRkxRMEZuUW1nd2NWZHFObmxyVEc4NFIzTk5WRzB3TlVOT1RtZ3hXVmh4VWtWeWJHOXBOREZaWWxKU2JEZ3hVbUlySzFKWk0yTnpkbFZ1WkU1eFdBb3JPV1pyTkVsTmFWRlhhWE00UzNOeFZXWkVUbkphVjNjeWVrcFRVakpHV2xSMFdUQkhPVGN6WkhreVJrVjBlSEJoWjJaTlJrdGtVMEZ0VkhoV1F6Uk5Dbk5pYWs0eGFsUTVja1JMUTJreWVWSk5ka041WlZWc1YyWktlVkpGVUhneU1YbEpPV292U1cxU1REVmtWakJwWW5SUk1GZ3ZNRzVvU0M5b2VWaFZUVGtLTkZWVFpHdDRhVTU1ZHpCdFIzRTFlVWx2TDBNdk9YSmtZVTF6T0ROWFIxZ3lRMk0yS3pOQmJWaEdjbEl4YVZnNFJpdFhUemN4YTFZelRpczJTUzlqU1FwUUswOVFNMU5pUTBGTmNHMWpPVEpPUVRoMFVrbzFlRXROYm5oV2RtdzVVMng1WlVkWFExaEdVeTl3TURSU09XMUdNQ3RZZFZReFIyUnBXbTlpTWswekNtRnhTM0ZvVXpkWVVVTlNjRnBQZDJsNFYwcDVNMEl4TkRkU2MwOUVUemhvWXk5clFVaDNOMDFuYTBwUlkyOXhNM2ROYWs1dll6Wk5MMm9yVkZCUGNXc0tkR0pRTUZaRVltSnRjbkZaUmpOaE5DdFRTMnRrVGtzNFptOVFhMnRLYldzMmNDdE1RM0ExUW5oa05sTXpjRkUxZW1Gc1MxTm9RMlpXYUUwMVZGVmhWZ3BrUTNWalJ6ZFdPRzFqTW5CelRFZEVhM0JvUVRGUmVXZ3piMVprY2tOeVlWaDBUamhzVWtJelVsQjFZeTlLWm5vemJtbG9SMjlDY2tONVNIRXJXbGxrQ2xJMlprTjFNVmhSWmpCWVYwMUtZVlJPTTJ0eGVUWlpPRlJuYURsRVVrNXROVEZ4VDFCMFZVNU1kWEpOUlhnNGEzb3dReXRsS3pWVGNVMTRhMWg0ZWtrS2VtSTVkekoxUjI1dmFtdzBOMFUzY0V4aU5IZDZiMmR5ZW1aNmJTOU9VbUV5VVZONWNIcHhRM2hJWXpCcWRuZDVaMGxMU0hJclNUWkhkM2Q1ZGpSc1RncEtOa2wyT0cxRUszRllUVWxZVWs5b2VVWm1Ta1JOV0RVcmVrSmpkRzlHVjBkNWMweExTSEJLTlRrMFdXMDNWVUYzVVV0RFFWRkZRVGRrU3k5UmJXUlRDbGw2U0ZscVZtRlRVbVJ6VldoTU1uUjNTMFZWVUM5TGN6Tlhjbk15VmxGelJtaDViMWx2TDNkQ2JuTkpNbFZhWTFwWGFYWTRjVWt4ZUc4MlVuQjZOVllLZFRWWFZtMWxiMUZGYmtNd1pYWlZiVk5xU2xoRlNXNU1kR2RCVWtoUWEwaEllRGd2UnpoSk1raHZOalZuT1UweVFWcElkRkIwZDBKM1pGVTFUemQxVkFvck5WWTNOME5uYW1VNU5Ia3JjRzFLUkhod1RYY3Ziak5TY2tGa1RYQlFiRTFEU0ROVllucDJjelZ1ZFVKT2JWY3ZLMkp2VFcxaFlXdHNkMVptYVd0UkNsUnZha3hpTld3cmQwWndUR2hyUkROT1ZFRllRbEZrU2l0NlpqTm9VM3BvUjI5NmVYTmxaWGx2ZGxVNGVrcFJibkozVEZOaGJGZFRTamc0ZWxWUE5YTUtiM0prV0V0b1kySjZPR1l2WlhZeWJtWmlhRzQ1VldoNlRtRkViRGt4Tm1aeFFrVTBha3RxWjNWcE9HWk5OV2xNTW5kMlZtMU5VRVpsWTBGM09FTlhlZ3BvYkhoMVlWRkVOMmhTYW5CV2QwdERRVkZGUVRWWFdXWnpkbXBZYjAxVlozRktPRGRQVEhkMFRuSnNRbmxSYkU5RGEwZ3JhVFZaY1dKSVlUVlNaMEp6Q2xCalRFOVJZMmxFVWsxM0syZG9TaXRoVWxWU1NHTnZaV3hxWmt4MmVVOUJjRFZOWkUwMmVIRnBNVlZQWjFWUmNrTXhOelZhYURoMFZsTkthV1V3TDFFS1JGcG9RVXhJVW1zM1YwdExNRm94VWtsdWRWVlhWVWRCZUhKTmVYSlRlVUpEYkUxeWRHZ3JRU3N6V2tzd05sZFZjRWxRYmtsMFNVOWxWMjUyTURCVGRBcFdPSEY1Wm5aMVdDdHFXQzlLTUdobFlrTlpkbVpZUzJwQ1dFMXRWVmR0VGsxRmIxbERhM3BXU0dWbE4xWkNUa0kwUTBaeVJVSTBjMWRHUTBReVNUYzNDbkJKV0dJNE1pOHJSRmRoVEdOd1IwUklTSEpIVHpSTWVGcE1MMGRsZFRFM00yVXdkVFYwY0djM2F6VTVVSFJGUzFoR2N6bGpRMkpTWlRkd1dXNWFlV3NLVFU1bFEzZERXR2QwUVdWbWNrbE9XV2hLVWxvMVNrSnRXQ3QwTlU4NEsxWlVaMjFCY3pKRlVWbFJTME5CVVVGbWRtUkJVRWxrUlhGTmFETjNRakI2ZHdwcEsxQTJURFJNUVZGbU9UZzFXSFpUT1ZwR01VWkVibkZJV25OUWVFcDRTbXRUVWtkTk5VUnFZa0Y1U0hWelUyOXZjVU52UkV4aVQyMWFSbXhCU1hSNUNtMXdOVTFDZFc4NFdIVnRiRFpITlVzNVZXeFVZalV4VjNSM2JuRlFOSEZRYlRSalFrcEVRME5HZVV4MVJEbHJaMDFOUVRWNU4yaGtha3c0UldKMmMwWUtZbXQxY0dwT1NuaE9TbHB2Vm1KV0swNXpWU3N5WlZKMEwzbHhUMDh3VWxjeVZWZGxWMmhvUVZGWFdtVlROMnd6Y2xacGJFSlJRVXhMTVVSVWNsVXpkd3BwUm5seFNqUlBXRVZ5YzFCNlkwRXZRMjFKTnpSU2FVOVFXaXR4ZUZCNlpuRlNSMkpWUnpkWVpXVXplR2xWWnk5cVpsVlJZVlJKVVhGYU1qRnVWek14Q2pGdlEzWjJjV3RpU3pSRFkybFVXSFpSTTBaUFEyUkJVRlV6VlZSMVVIQTVabmR4VVVSV01HeFBSR2RwT0c5WmNEQk5Mek5VVjBSbWJuWnBVbWhVVFUwS2RWb3hla0Z2U1VKQlNGQkhiWFpGUlhwcE9FSjZiak5NTHpKUFpUbFhURTVHVUhGV2FUWkdOWFJVU2xZd05IUjVWVUZsV1ZwM2FUQnlUWGhsYlc5SE5ncFpNRWhoWlRWdmJGYzRTV2hLUm1KRlUycDRjR1Z4UlZscGIwSjBORVZtTlRSYWQzb3ZNVlkxTVZneFNUbDFVV3QxYm1Sd2VrdE9VREV2VDFoMWRXaHFDamsxVUdKU1puVldVekJZVVROWFJuSlNORkpJUm1Oc1ZVOUtaSFZTZG1GcGRWSnJaMVpIYWtSbFdVRkdZUzlEZWtkeGMxSXlhbkZaZG05WFFVUlRMMjBLTWt0UVIzcHZSR1YwVGtWdVltdGtkREpyVkRCa1MwZFBaelYxVUUwelFXOWhSVGhEWlZKcVdsSlBUMXBzZEM5eWIzUTVSa3R3VUdGVGVYZE9kR0poYkFweWJrNW5TbFF2VjFwMk0yRkdSMHMwZEhvclRGcENlVnBtUkZwbGRVNVZUa2xCUkRSR04yRk9RbVpFTkZwdWJDOXFkMGhqT0V4RlJqQTJUMDlsYkROcENrTjVRbE5IWm1GMmRVdzJNbU5xVTNVM2VucFZaelZKYWtWaFZqSnpORVZEWjJkRlFVWndXWFJSYVZWbWMwVkxOV0ZUVms0cldHWlFUMmhWYlV0VlFrUUtVVkprY2tObmFISXJja2hDUnpjdmJESkhSMDlTY1RCMkwydEJjMHR5TldSUllXOXBMMHBaUnpOeVMwTTJZMmcyVm1sck5GQllOUzlhVEVWRlZsWkROQXB3VFZwVmNWbDNWMDh4Y0VGeFUza3JNMjUwYjNoSmRFUk5NWEZ2ZEc4eFVuVnVPV1kwTVROWWFVNDFXbkpEUVRWTmFYbzBiR2hwV2xoaWREQkRkalUyQ2toWE4xbG5WRWRXZDBsWVltSnFTM05MTmtadVJIQkJlbkZwYVZNd1NHZFNWMFp6UWtVd1JWSmlTekpWYTNOMmFVdEhVV3AzUlVaU1ZrMXNjR3BzUTFRS2MxUmxaalJIYkd0V2NraDFiVkkzZDNobWFUVmpORmhMVWxoR1RuZEZObkoxZUdoa1RrZHFUV3BsTW5wTE5uSnZSVlpMVTNCdWFXSjBTMHBKWVZsVmN3cEVla1ZGZUZRM2VIRTRVR3RMY2tSMmJWWlZVbGQxWjFoV2NuQTNObGRUTW5vemNHUkhibFZRU1dkVFdraHJjM3BhTVdGS01HZEpOWEJSUFQwS0xTMHRMUzFGVGtRZ1VsTkJJRkJTU1ZaQlZFVWdTMFZaTFMwdExTMEsKY29udGV4dHM6Ci0gY29udGV4dDoKICAgIGNsdXN0ZXI6IGNsdXN0ZXIKICAgIHVzZXI6IGNmZS1tYXN0ZXIKICBuYW1lOiBkZWZhdWx0Q29udGV4dApjdXJyZW50LWNvbnRleHQ6IGRlZmF1bHRDb250ZXh0Cg==","storages":"[{\"username\": \"test\",\"password\": \"test\",\"ip\": \"8.40.111.70\",\"port\": 8088},{\"username\": \"test\",\"password\": \"test\",\"ip\": \"127.0.0.1\",\"port\": 8088}]"})";
    auto k8sApi = KubeHelper::GetKubernetesApiFromAppEnv(authExtendInfo);
    return k8sApi;
}

/**
 * 打桩k8s容器内执行命令成功
 */
std::string KubeExecSuccess(const std::string &nameSpace, const std::string &pod, const std::string &container, const std::string &command)
{
    return "[feedback]status=normal";
}

/**
 * 打桩k8s容器内执行命令失败
 */
std::string KubeExecFail(const std::string &nameSpace, const std::string &pod, const std::string &container, const std::string &command)
{
    return "OCI runtime exec failed: exec failed: handshake fail.";
}

std::pair<int, std::vector<ApplicationResource>> ListPodsSuccess(const Application &statefulSet) {
    ApplicationResource applicationResource;
    applicationResource.name = "pod-01";
    applicationResource.parentName = "ns000000000000000000001";
    std::vector<ApplicationResource> pods = {applicationResource};
    return std::make_pair(Module::SUCCESS, pods);
}

/**
 * 模拟SetReplicas zero后，K8S集群的行为。第1次返回有正确的pod，后面返回空pod。
 */
std::pair<int, std::vector<ApplicationResource>> Stub_ListPods_AfterSetReplicas_Success(void *obj, const Application &statefulSet) {
    static int cnt = 0;
    std::vector<ApplicationResource> pods = {};
    if (cnt == 0) {
        ApplicationResource applicationResource;
        applicationResource.name = "pod-01";
        applicationResource.parentName = "ns000000000000000000001";
        pods.push_back(applicationResource);
        cnt++;
    } else if (cnt > 0) {
        cnt =  0;
    }
    return std::make_pair(Module::SUCCESS, pods);
}

/**
 * 模拟SetReplicas zero后，K8S集群的行为。一直返回1个pod，pod无法停止
 */
std::pair<int, std::vector<ApplicationResource>> Stub_ListPods_AlwaysOne(void *obj, const Application &statefulSet) {
    std::vector<ApplicationResource> pods = {};
    ApplicationResource applicationResource;
    applicationResource.name = "pod-01";
    applicationResource.parentName = "ns000000000000000000001";
    pods.push_back(applicationResource);
    return std::make_pair(Module::SUCCESS, pods);
}

/**
 * 模拟RestoreReplicas zero后，K8S集群的行为。第1次返回非空，后面返回正确的pod。
 */
std::pair<int, std::vector<ApplicationResource>> Stub_ListPods_AfterRestoreReplicas_Success(void *obj, const Application &statefulSet) {
    static int cnt = 0;
    std::vector<ApplicationResource> pods = {};
    if (cnt == 1) {
        ApplicationResource applicationResource;
        applicationResource.name = "pod-01";
        applicationResource.parentName = "ns000000000000000000001";
        pods.push_back(applicationResource);
        cnt = 0;
    } else if (cnt == 0) {
        cnt++;
    }
    return std::make_pair(Module::SUCCESS, pods);
}

/**
 * 模拟RestoreReplicas zero后，K8S集群的行为。一直返回0个pod，无法上电。
 */
std::pair<int, std::vector<ApplicationResource>> Stub_ListPods_AlwaysZero(void *obj, const Application &statefulSet) {
    std::vector<ApplicationResource> pods = {};
    return std::make_pair(Module::SUCCESS, pods);
}

/**
 * 打桩存储客户端所有接口执行成功
 */
int32_t SendStorageClientApiSuccess(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/system/") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"CACHEWRITEQUOTA\":\"333\",\"CONFIGMODEL\":\"1\",\"DESCRIPTION\":\"\",\"DOMAINNAME\":\"\",\"FREEDISKSCAPACITY\":\"0\",\"HEALTHSTATUS\":\"1\",\"HOTSPAREDISKSCAPACITY\":\"0\",\"ID\":\"2102353GTD10L9000007\",\"LOCATION\":\"\",\"MEMBERDISKSCAPACITY\":\"60000000000\",\"NAME\":\"Huawei.Storage\",\"PRODUCTMODE\":\"821\",\"PRODUCTVERSION\":\"V600R005C21\",\"RUNNINGSTATUS\":\"1\",\"SECTORSIZE\":\"512\",\"STORAGEPOOLCAPACITY\":\"33030913444\",\"STORAGEPOOLFREECAPACITY\":\"28565410974\",\"STORAGEPOOLHOSTSPARECAPACITY\":\"6606182688\",\"STORAGEPOOLRAWCAPACITY\":\"52849461510\",\"STORAGEPOOLUSEDCAPACITY\":\"4465502470\",\"THICKLUNSALLOCATECAPACITY\":\"0\",\"THICKLUNSUSEDCAPACITY\":\"-1\",\"THINLUNSALLOCATECAPACITY\":\"108406352\",\"THINLUNSMAXCAPACITY\":\"1887436800\",\"THINLUNSUSEDCAPACITY\":\"-1\",\"TOTALCAPACITY\":\"60000000000\",\"TYPE\":201,\"UNAVAILABLEDISKSCAPACITY\":\"0\",\"USEDCAPACITY\":\"4465502470\",\"VASA_ALTERNATE_NAME\":\"Huawei.Storage\",\"VASA_SUPPORT_BLOCK\":\"FC,FCOE,ISCSI,Others\",\"VASA_SUPPORT_FILESYSTEM\":\"\",\"VASA_SUPPORT_PROFILE\":\"BlockDeviceProfile,CapabilityProfile,VirtualVolumeProfile\",\"WRITETHROUGHSW\":\"1\",\"WRITETHROUGHTIME\":\"192\",\"mappedLunsCountCapacity\":\"0\",\"patchVersion\":\"\",\"pointRelease\":\"1.2.1RC1\",\"productModeString\":\"OceanProtect X8000\",\"unMappedLunsCountCapacity\":\"1887436800\",\"userFreeCapacity\":\"68233794713\",\"wwn\":\"21004cf55b9769b4\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/lun?range=%5B0-100%5D&filter=SUBTYPE%3A%3A0%20and%20NAME%3Aflex_volume_lun0000") != std::string::npos) {
        httpResponse.m_body = "{\"data\":[{\"ALLOCCAPACITY\":\"0\",\"ALLOCTYPE\":\"1\",\"CAPABILITY\":\"3\",\"CAPACITY\":\"2097152\",\"CAPACITYALARMLEVEL\":\"2\",\"COMPRESSION\":\"0\",\"COMPRESSIONSAVEDCAPACITY\":\"0\",\"COMPRESSIONSAVEDRATIO\":\"0\",\"DEDUPSAVEDCAPACITY\":\"0\",\"DEDUPSAVEDRATIO\":\"0\",\"DESCRIPTION\":\"\",\"DISGUISEREMOTEARRAYID\":\"--\",\"DISGUISESTATUS\":\"0\",\"DRS_ENABLE\":\"false\",\"ENABLECOMPRESSION\":\"true\",\"ENABLEISCSITHINLUNTHRESHOLD\":\"false\",\"ENABLESMARTDEDUP\":\"false\",\"EXPOSEDTOINITIATOR\":\"false\",\"EXTENDIFSWITCH\":\"false\",\"HASRSSOBJECT\":\"{\\\"SnapShot\\\":\\\"FALSE\\\",\\\"LunCopy\\\":\\\"FALSE\\\",\\\"RemoteReplication\\\":\\\"FALSE\\\",\\\"SplitMirror\\\":\\\"FALSE\\\",\\\"LunMigration\\\":\\\"FALSE\\\",\\\"LUNMirror\\\":\\\"FALSE\\\",\\\"HyperMetro\\\":\\\"FALSE\\\",\\\"LunClone\\\":\\\"FALSE\\\",\\\"HyperCopy\\\":\\\"FALSE\\\",\\\"HyperCDP\\\":\\\"FALSE\\\",\\\"CloudBackup\\\":\\\"FALSE\\\",\\\"drStar\\\":\\\"FALSE\\\"}\",\"HEALTHSTATUS\":\"1\",\"HYPERCDPSCHEDULEDISABLE\":\"0\",\"ID\":\"35\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISADD2LUNGROUP\":\"false\",\"ISCHECKZEROPAGE\":\"true\",\"ISCLONE\":\"false\",\"ISCSITHINLUNTHRESHOLD\":\"90\",\"MIRRORPOLICY\":\"1\",\"MIRRORTYPE\":\"0\",\"NAME\":\"flex_volume_lun0000\",\"NGUID\":\"71009769b4150f7d4cf55b5e00000023\",\"OWNINGCONTROLLER\":\"--\",\"PARENTID\":\"0\",\"PARENTNAME\":\"s0\",\"PREFETCHPOLICY\":\"0\",\"PREFETCHVALUE\":\"0\",\"REMOTELUNID\":\"--\",\"REPLICATION_CAPACITY\":\"0\",\"RUNNINGSTATUS\":\"27\",\"RUNNINGWRITEPOLICY\":\"1\",\"SC_HITRAGE\":\"0\",\"SECTORSIZE\":\"512\",\"SMARTCACHEPARTITIONID\":\"\",\"SNAPSHOTSCHEDULEID\":\"--\",\"SUBTYPE\":\"0\",\"THINCAPACITYUSAGE\":\"0\",\"TOTALSAVEDCAPACITY\":\"0\",\"TOTALSAVEDRATIO\":\"0\",\"TYPE\":11,\"USAGETYPE\":\"0\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WRITEPOLICY\":\"1\",\"WWN\":\"64cf55b1009769b4150f7d5e00000023\",\"blockDeviceName\":\"--\",\"devController\":\"--\",\"functionType\":\"1\",\"grainSize\":\"64\",\"hyperCdpScheduleId\":\"0\",\"isShowDedupAndCompression\":\"true\",\"lunCgId\":\"0\",\"mapped\":\"false\",\"remoteLunWwn\":\"--\",\"serviceEnabled\":\"false\",\"takeOverLunWwn\":\"--\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/lun?range=%5B0-100%5D&filter=SUBTYPE%3A%3A0%20and%20NAME%3Aflex_volume_lun0001") != std::string::npos) {
        httpResponse.m_body = "{\"data\":[{\"ALLOCCAPACITY\":\"0\",\"ALLOCTYPE\":\"1\",\"CAPABILITY\":\"3\",\"CAPACITY\":\"2097152\",\"CAPACITYALARMLEVEL\":\"2\",\"COMPRESSION\":\"0\",\"COMPRESSIONSAVEDCAPACITY\":\"0\",\"COMPRESSIONSAVEDRATIO\":\"0\",\"DEDUPSAVEDCAPACITY\":\"0\",\"DEDUPSAVEDRATIO\":\"0\",\"DESCRIPTION\":\"\",\"DISGUISEREMOTEARRAYID\":\"--\",\"DISGUISESTATUS\":\"0\",\"DRS_ENABLE\":\"false\",\"ENABLECOMPRESSION\":\"true\",\"ENABLEISCSITHINLUNTHRESHOLD\":\"false\",\"ENABLESMARTDEDUP\":\"false\",\"EXPOSEDTOINITIATOR\":\"false\",\"EXTENDIFSWITCH\":\"false\",\"HASRSSOBJECT\":\"{\\\"SnapShot\\\":\\\"FALSE\\\",\\\"LunCopy\\\":\\\"FALSE\\\",\\\"RemoteReplication\\\":\\\"FALSE\\\",\\\"SplitMirror\\\":\\\"FALSE\\\",\\\"LunMigration\\\":\\\"FALSE\\\",\\\"LUNMirror\\\":\\\"FALSE\\\",\\\"HyperMetro\\\":\\\"FALSE\\\",\\\"LunClone\\\":\\\"FALSE\\\",\\\"HyperCopy\\\":\\\"FALSE\\\",\\\"HyperCDP\\\":\\\"FALSE\\\",\\\"CloudBackup\\\":\\\"FALSE\\\",\\\"drStar\\\":\\\"FALSE\\\"}\",\"HEALTHSTATUS\":\"1\",\"HYPERCDPSCHEDULEDISABLE\":\"0\",\"ID\":\"36\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISADD2LUNGROUP\":\"false\",\"ISCHECKZEROPAGE\":\"true\",\"ISCLONE\":\"false\",\"ISCSITHINLUNTHRESHOLD\":\"90\",\"MIRRORPOLICY\":\"1\",\"MIRRORTYPE\":\"0\",\"NAME\":\"flex_volume_lun0001\",\"NGUID\":\"71009769b4150f7d4cf55b5e00000024\",\"OWNINGCONTROLLER\":\"--\",\"PARENTID\":\"0\",\"PARENTNAME\":\"s0\",\"PREFETCHPOLICY\":\"0\",\"PREFETCHVALUE\":\"0\",\"REMOTELUNID\":\"--\",\"REPLICATION_CAPACITY\":\"0\",\"RUNNINGSTATUS\":\"27\",\"RUNNINGWRITEPOLICY\":\"1\",\"SC_HITRAGE\":\"0\",\"SECTORSIZE\":\"512\",\"SMARTCACHEPARTITIONID\":\"\",\"SNAPSHOTSCHEDULEID\":\"--\",\"SUBTYPE\":\"0\",\"THINCAPACITYUSAGE\":\"0\",\"TOTALSAVEDCAPACITY\":\"0\",\"TOTALSAVEDRATIO\":\"0\",\"TYPE\":11,\"USAGETYPE\":\"0\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WRITEPOLICY\":\"1\",\"WWN\":\"64cf55b1009769b4150f7d5e00000024\",\"blockDeviceName\":\"--\",\"devController\":\"--\",\"functionType\":\"1\",\"grainSize\":\"64\",\"hyperCdpScheduleId\":\"0\",\"isShowDedupAndCompression\":\"true\",\"lunCgId\":\"0\",\"mapped\":\"false\",\"remoteLunWwn\":\"--\",\"serviceEnabled\":\"false\",\"takeOverLunWwn\":\"--\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/lun?range=%5B0-100%5D&filter=SUBTYPE%3A%3A0%20and%20NAME%3Aflex_volume_lun0002") != std::string::npos) {
        httpResponse.m_body = "{\"data\":[{\"ALLOCCAPACITY\":\"0\",\"ALLOCTYPE\":\"1\",\"CAPABILITY\":\"3\",\"CAPACITY\":\"2097152\",\"CAPACITYALARMLEVEL\":\"2\",\"COMPRESSION\":\"0\",\"COMPRESSIONSAVEDCAPACITY\":\"0\",\"COMPRESSIONSAVEDRATIO\":\"0\",\"DEDUPSAVEDCAPACITY\":\"0\",\"DEDUPSAVEDRATIO\":\"0\",\"DESCRIPTION\":\"\",\"DISGUISEREMOTEARRAYID\":\"--\",\"DISGUISESTATUS\":\"0\",\"DRS_ENABLE\":\"false\",\"ENABLECOMPRESSION\":\"true\",\"ENABLEISCSITHINLUNTHRESHOLD\":\"false\",\"ENABLESMARTDEDUP\":\"false\",\"EXPOSEDTOINITIATOR\":\"false\",\"EXTENDIFSWITCH\":\"false\",\"HASRSSOBJECT\":\"{\\\"SnapShot\\\":\\\"FALSE\\\",\\\"LunCopy\\\":\\\"FALSE\\\",\\\"RemoteReplication\\\":\\\"FALSE\\\",\\\"SplitMirror\\\":\\\"FALSE\\\",\\\"LunMigration\\\":\\\"FALSE\\\",\\\"LUNMirror\\\":\\\"FALSE\\\",\\\"HyperMetro\\\":\\\"FALSE\\\",\\\"LunClone\\\":\\\"FALSE\\\",\\\"HyperCopy\\\":\\\"FALSE\\\",\\\"HyperCDP\\\":\\\"FALSE\\\",\\\"CloudBackup\\\":\\\"FALSE\\\",\\\"drStar\\\":\\\"FALSE\\\"}\",\"HEALTHSTATUS\":\"1\",\"HYPERCDPSCHEDULEDISABLE\":\"0\",\"ID\":\"37\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISADD2LUNGROUP\":\"false\",\"ISCHECKZEROPAGE\":\"true\",\"ISCLONE\":\"false\",\"ISCSITHINLUNTHRESHOLD\":\"90\",\"MIRRORPOLICY\":\"1\",\"MIRRORTYPE\":\"0\",\"NAME\":\"flex_volume_lun0002\",\"NGUID\":\"71009769b4150f7d4cf55b5e00000025\",\"OWNINGCONTROLLER\":\"--\",\"PARENTID\":\"0\",\"PARENTNAME\":\"s0\",\"PREFETCHPOLICY\":\"0\",\"PREFETCHVALUE\":\"0\",\"REMOTELUNID\":\"--\",\"REPLICATION_CAPACITY\":\"0\",\"RUNNINGSTATUS\":\"27\",\"RUNNINGWRITEPOLICY\":\"1\",\"SC_HITRAGE\":\"0\",\"SECTORSIZE\":\"512\",\"SMARTCACHEPARTITIONID\":\"\",\"SNAPSHOTSCHEDULEID\":\"--\",\"SUBTYPE\":\"0\",\"THINCAPACITYUSAGE\":\"0\",\"TOTALSAVEDCAPACITY\":\"0\",\"TOTALSAVEDRATIO\":\"0\",\"TYPE\":11,\"USAGETYPE\":\"0\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WRITEPOLICY\":\"1\",\"WWN\":\"64cf55b1009769b4150f7d5e00000025\",\"blockDeviceName\":\"--\",\"devController\":\"--\",\"functionType\":\"1\",\"grainSize\":\"64\",\"hyperCdpScheduleId\":\"0\",\"isShowDedupAndCompression\":\"true\",\"lunCgId\":\"0\",\"mapped\":\"false\",\"remoteLunWwn\":\"--\",\"serviceEnabled\":\"false\",\"takeOverLunWwn\":\"--\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/stop") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/activate") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/22") != std::string::npos && httpParam.method == "DELETE") {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot?range") != std::string::npos && httpParam.method == "GET") {
        httpResponse.m_body = "{\"error\":{\"code\":0,\"description\":\"\"},\"data\":[{\"ID\":\"snap1\",\"NAME\":\"Protect_12_SNAP_1661497876\",\"RUNNINGSTATUS\":\"27\",\"WWN\":\"1\"},{\"ID\":\"snap2\",\"NAME\":\"lun0001_abc_1661497873\",\"RUNNINGSTATUS\":\"27\",\"WWN\":\"1\"}]}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"CASCADEDLEVEL\":\"0\",\"CASCADEDNUM\":\"0\",\"CONSUMEDCAPACITY\":\"0\",\"DESCRIPTION\":\"\",\"EXPOSEDTOINITIATOR\":\"false\",\"HEALTHSTATUS\":\"1\",\"HYPERCOPYIDS\":\"\",\"ID\":\"62\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISSCHEDULEDSNAP\":\"0\",\"NAME\":\"guohao_test3\",\"NGUID\":\"71009769b4153fa84cf55b4d0000003e\",\"PARENTID\":\"35\",\"PARENTNAME\":\"flex_volume_lun0000\",\"PARENTTYPE\":11,\"ROLLBACKENDTIME\":\"-1\",\"ROLLBACKRATE\":\"-1\",\"ROLLBACKSPEED\":\"2\",\"ROLLBACKSTARTTIME\":\"-1\",\"ROLLBACKTARGETOBJID\":\"4294967295\",\"ROLLBACKTARGETOBJNAME\":\"--\",\"RUNNINGSTATUS\":\"43\",\"SOURCELUNCAPACITY\":\"2097152\",\"SOURCELUNID\":\"35\",\"SOURCELUNNAME\":\"flex_volume_lun0000\",\"SUBTYPE\":\"0\",\"TIMESTAMP\":\"1657144036\",\"TYPE\":27,\"USERCAPACITY\":\"2097152\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WWN\":\"64cf55b1009769b4153fa84d0000003e\",\"coupleUuid\":\"3e0060424c50b501b469975bf54c0021\",\"isReadOnly\":\"0\",\"replicationCapacity\":\"0\",\"snapCgId\":\"4294967295\",\"snapCgName\":\"--\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/18") != std::string::npos && httpParam.method == "DELETE") {
        httpResponse.m_body = "{\"data\":{\"CASCADEDLEVEL\":\"0\",\"CASCADEDNUM\":\"0\",\"CONSUMEDCAPACITY\":\"0\",\"DESCRIPTION\":\"\",\"EXPOSEDTOINITIATOR\":\"false\",\"HEALTHSTATUS\":\"1\",\"HYPERCOPYIDS\":\"\",\"ID\":\"18\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISSCHEDULEDSNAP\":\"0\",\"NAME\":\"guohao_test3\",\"NGUID\":\"71009769b4153fa84cf55b4d0000003e\",\"PARENTID\":\"35\",\"PARENTNAME\":\"flex_volume_lun0000\",\"PARENTTYPE\":11,\"ROLLBACKENDTIME\":\"-1\",\"ROLLBACKRATE\":\"-1\",\"ROLLBACKSPEED\":\"2\",\"ROLLBACKSTARTTIME\":\"-1\",\"ROLLBACKTARGETOBJID\":\"4294967295\",\"ROLLBACKTARGETOBJNAME\":\"--\",\"RUNNINGSTATUS\":\"43\",\"SOURCELUNCAPACITY\":\"2097152\",\"SOURCELUNID\":\"35\",\"SOURCELUNNAME\":\"flex_volume_lun0000\",\"SUBTYPE\":\"0\",\"TIMESTAMP\":\"1657144036\",\"TYPE\":27,\"USERCAPACITY\":\"2097152\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WWN\":\"64cf55b1009769b4153fa84d0000003e\",\"coupleUuid\":\"3e0060424c50b501b469975bf54c0021\",\"isReadOnly\":\"0\",\"replicationCapacity\":\"0\",\"snapCgId\":\"4294967295\",\"snapCgName\":\"--\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/storagepool/") != std::string::npos && httpParam.method == "GET") {
        httpResponse.m_body = "{\"data\":{\"USERTOTALCAPACITY\":\"100\",\"USERFREECAPACITY\":\"21\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/lun?range=%5B0-100%5D&filter=SUBTYPE%3A%3A0%20and%20NAME%3Aflex_volume_lun0000") != std::string::npos) {
        httpResponse.m_body = "{\"data\":[{\"ALLOCCAPACITY\":\"0\",\"ALLOCTYPE\":\"1\",\"CAPABILITY\":\"3\",\"CAPACITY\":\"2097152\",\"CAPACITYALARMLEVEL\":\"2\",\"COMPRESSION\":\"0\",\"COMPRESSIONSAVEDCAPACITY\":\"0\",\"COMPRESSIONSAVEDRATIO\":\"0\",\"DEDUPSAVEDCAPACITY\":\"0\",\"DEDUPSAVEDRATIO\":\"0\",\"DESCRIPTION\":\"\",\"DISGUISEREMOTEARRAYID\":\"--\",\"DISGUISESTATUS\":\"0\",\"DRS_ENABLE\":\"false\",\"ENABLECOMPRESSION\":\"true\",\"ENABLEISCSITHINLUNTHRESHOLD\":\"false\",\"ENABLESMARTDEDUP\":\"false\",\"EXPOSEDTOINITIATOR\":\"false\",\"EXTENDIFSWITCH\":\"false\",\"HASRSSOBJECT\":\"{\\\"SnapShot\\\":\\\"FALSE\\\",\\\"LunCopy\\\":\\\"FALSE\\\",\\\"RemoteReplication\\\":\\\"FALSE\\\",\\\"SplitMirror\\\":\\\"FALSE\\\",\\\"LunMigration\\\":\\\"FALSE\\\",\\\"LUNMirror\\\":\\\"FALSE\\\",\\\"HyperMetro\\\":\\\"FALSE\\\",\\\"LunClone\\\":\\\"FALSE\\\",\\\"HyperCopy\\\":\\\"FALSE\\\",\\\"HyperCDP\\\":\\\"FALSE\\\",\\\"CloudBackup\\\":\\\"FALSE\\\",\\\"drStar\\\":\\\"FALSE\\\"}\",\"HEALTHSTATUS\":\"1\",\"HYPERCDPSCHEDULEDISABLE\":\"0\",\"ID\":\"35\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISADD2LUNGROUP\":\"false\",\"ISCHECKZEROPAGE\":\"true\",\"ISCLONE\":\"false\",\"ISCSITHINLUNTHRESHOLD\":\"90\",\"MIRRORPOLICY\":\"1\",\"MIRRORTYPE\":\"0\",\"NAME\":\"flex_volume_lun0000\",\"NGUID\":\"71009769b4150f7d4cf55b5e00000023\",\"OWNINGCONTROLLER\":\"--\",\"PARENTID\":\"0\",\"PARENTNAME\":\"s0\",\"PREFETCHPOLICY\":\"0\",\"PREFETCHVALUE\":\"0\",\"REMOTELUNID\":\"--\",\"REPLICATION_CAPACITY\":\"0\",\"RUNNINGSTATUS\":\"27\",\"RUNNINGWRITEPOLICY\":\"1\",\"SC_HITRAGE\":\"0\",\"SECTORSIZE\":\"512\",\"SMARTCACHEPARTITIONID\":\"\",\"SNAPSHOTSCHEDULEID\":\"--\",\"SUBTYPE\":\"0\",\"THINCAPACITYUSAGE\":\"0\",\"TOTALSAVEDCAPACITY\":\"0\",\"TOTALSAVEDRATIO\":\"0\",\"TYPE\":11,\"USAGETYPE\":\"0\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WRITEPOLICY\":\"1\",\"WWN\":\"64cf55b1009769b4150f7d5e00000023\",\"blockDeviceName\":\"--\",\"devController\":\"--\",\"functionType\":\"1\",\"grainSize\":\"64\",\"hyperCdpScheduleId\":\"0\",\"isShowDedupAndCompression\":\"true\",\"lunCgId\":\"0\",\"mapped\":\"false\",\"remoteLunWwn\":\"--\",\"serviceEnabled\":\"false\",\"takeOverLunWwn\":\"--\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    }
    return Module::SUCCESS;
}

int32_t SendStorageClientAuthFail(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077987870,\"description\":\"0\"}}";
    }
    return Module::SUCCESS;
}


int32_t SendStorageClientApiFail(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/system/") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077937880,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/18") != std::string::npos && httpParam.method == "DELETE") {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":12354534564,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/22") != std::string::npos && httpParam.method == "DELETE") {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077937880,\"description\":\"0\"}}";
    }
    return Module::SUCCESS;
}

/**
 * 打桩存储客户端只有获取卷详情接口异常
 */
int32_t StorageClientGetLunInfoFailOnly(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/system/") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/lun?range=%5B0-100%5D&filter=SUBTYPE%3A%3A0%20and%20NAME%3Aflex_volume_lun0000") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077937880,\"description\":\"0\"}}";
    }
    return Module::SUCCESS;
}

/**
 * 打桩存储客户端只有获取“创建快照”接口异常
 */
int32_t StorageClientCreateSnapFailOnly(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/system/") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/lun?range=%5B0-100%5D&filter=SUBTYPE%3A%3A0%20and%20NAME%3Aflex_volume_lun0000") != std::string::npos) {
        httpResponse.m_body = "{\"data\":[{\"ALLOCCAPACITY\":\"0\",\"ALLOCTYPE\":\"1\",\"CAPABILITY\":\"3\",\"CAPACITY\":\"2097152\",\"CAPACITYALARMLEVEL\":\"2\",\"COMPRESSION\":\"0\",\"COMPRESSIONSAVEDCAPACITY\":\"0\",\"COMPRESSIONSAVEDRATIO\":\"0\",\"DEDUPSAVEDCAPACITY\":\"0\",\"DEDUPSAVEDRATIO\":\"0\",\"DESCRIPTION\":\"\",\"DISGUISEREMOTEARRAYID\":\"--\",\"DISGUISESTATUS\":\"0\",\"DRS_ENABLE\":\"false\",\"ENABLECOMPRESSION\":\"true\",\"ENABLEISCSITHINLUNTHRESHOLD\":\"false\",\"ENABLESMARTDEDUP\":\"false\",\"EXPOSEDTOINITIATOR\":\"false\",\"EXTENDIFSWITCH\":\"false\",\"HASRSSOBJECT\":\"{\\\"SnapShot\\\":\\\"FALSE\\\",\\\"LunCopy\\\":\\\"FALSE\\\",\\\"RemoteReplication\\\":\\\"FALSE\\\",\\\"SplitMirror\\\":\\\"FALSE\\\",\\\"LunMigration\\\":\\\"FALSE\\\",\\\"LUNMirror\\\":\\\"FALSE\\\",\\\"HyperMetro\\\":\\\"FALSE\\\",\\\"LunClone\\\":\\\"FALSE\\\",\\\"HyperCopy\\\":\\\"FALSE\\\",\\\"HyperCDP\\\":\\\"FALSE\\\",\\\"CloudBackup\\\":\\\"FALSE\\\",\\\"drStar\\\":\\\"FALSE\\\"}\",\"HEALTHSTATUS\":\"1\",\"HYPERCDPSCHEDULEDISABLE\":\"0\",\"ID\":\"35\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISADD2LUNGROUP\":\"false\",\"ISCHECKZEROPAGE\":\"true\",\"ISCLONE\":\"false\",\"ISCSITHINLUNTHRESHOLD\":\"90\",\"MIRRORPOLICY\":\"1\",\"MIRRORTYPE\":\"0\",\"NAME\":\"flex_volume_lun0000\",\"NGUID\":\"71009769b4150f7d4cf55b5e00000023\",\"OWNINGCONTROLLER\":\"--\",\"PARENTID\":\"0\",\"PARENTNAME\":\"s0\",\"PREFETCHPOLICY\":\"0\",\"PREFETCHVALUE\":\"0\",\"REMOTELUNID\":\"--\",\"REPLICATION_CAPACITY\":\"0\",\"RUNNINGSTATUS\":\"27\",\"RUNNINGWRITEPOLICY\":\"1\",\"SC_HITRAGE\":\"0\",\"SECTORSIZE\":\"512\",\"SMARTCACHEPARTITIONID\":\"\",\"SNAPSHOTSCHEDULEID\":\"--\",\"SUBTYPE\":\"0\",\"THINCAPACITYUSAGE\":\"0\",\"TOTALSAVEDCAPACITY\":\"0\",\"TOTALSAVEDRATIO\":\"0\",\"TYPE\":11,\"USAGETYPE\":\"0\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WRITEPOLICY\":\"1\",\"WWN\":\"64cf55b1009769b4150f7d5e00000023\",\"blockDeviceName\":\"--\",\"devController\":\"--\",\"functionType\":\"1\",\"grainSize\":\"64\",\"hyperCdpScheduleId\":\"0\",\"isShowDedupAndCompression\":\"true\",\"lunCgId\":\"0\",\"mapped\":\"false\",\"remoteLunWwn\":\"--\",\"serviceEnabled\":\"false\",\"takeOverLunWwn\":\"--\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077937880,\"description\":\"0\"}}";
    }
    return Module::SUCCESS;
}

/**
 * 打桩存储客户端只有获取“取消激活快照”接口异常
 */
int32_t StorageClientStopSnapFailOnly(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/system/") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/lun?range=%5B0-100%5D&filter=SUBTYPE%3A%3A0%20and%20NAME%3Aflex_volume_lun0000") != std::string::npos) {
        httpResponse.m_body = "{\"data\":[{\"ALLOCCAPACITY\":\"0\",\"ALLOCTYPE\":\"1\",\"CAPABILITY\":\"3\",\"CAPACITY\":\"2097152\",\"CAPACITYALARMLEVEL\":\"2\",\"COMPRESSION\":\"0\",\"COMPRESSIONSAVEDCAPACITY\":\"0\",\"COMPRESSIONSAVEDRATIO\":\"0\",\"DEDUPSAVEDCAPACITY\":\"0\",\"DEDUPSAVEDRATIO\":\"0\",\"DESCRIPTION\":\"\",\"DISGUISEREMOTEARRAYID\":\"--\",\"DISGUISESTATUS\":\"0\",\"DRS_ENABLE\":\"false\",\"ENABLECOMPRESSION\":\"true\",\"ENABLEISCSITHINLUNTHRESHOLD\":\"false\",\"ENABLESMARTDEDUP\":\"false\",\"EXPOSEDTOINITIATOR\":\"false\",\"EXTENDIFSWITCH\":\"false\",\"HASRSSOBJECT\":\"{\\\"SnapShot\\\":\\\"FALSE\\\",\\\"LunCopy\\\":\\\"FALSE\\\",\\\"RemoteReplication\\\":\\\"FALSE\\\",\\\"SplitMirror\\\":\\\"FALSE\\\",\\\"LunMigration\\\":\\\"FALSE\\\",\\\"LUNMirror\\\":\\\"FALSE\\\",\\\"HyperMetro\\\":\\\"FALSE\\\",\\\"LunClone\\\":\\\"FALSE\\\",\\\"HyperCopy\\\":\\\"FALSE\\\",\\\"HyperCDP\\\":\\\"FALSE\\\",\\\"CloudBackup\\\":\\\"FALSE\\\",\\\"drStar\\\":\\\"FALSE\\\"}\",\"HEALTHSTATUS\":\"1\",\"HYPERCDPSCHEDULEDISABLE\":\"0\",\"ID\":\"35\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISADD2LUNGROUP\":\"false\",\"ISCHECKZEROPAGE\":\"true\",\"ISCLONE\":\"false\",\"ISCSITHINLUNTHRESHOLD\":\"90\",\"MIRRORPOLICY\":\"1\",\"MIRRORTYPE\":\"0\",\"NAME\":\"flex_volume_lun0000\",\"NGUID\":\"71009769b4150f7d4cf55b5e00000023\",\"OWNINGCONTROLLER\":\"--\",\"PARENTID\":\"0\",\"PARENTNAME\":\"s0\",\"PREFETCHPOLICY\":\"0\",\"PREFETCHVALUE\":\"0\",\"REMOTELUNID\":\"--\",\"REPLICATION_CAPACITY\":\"0\",\"RUNNINGSTATUS\":\"27\",\"RUNNINGWRITEPOLICY\":\"1\",\"SC_HITRAGE\":\"0\",\"SECTORSIZE\":\"512\",\"SMARTCACHEPARTITIONID\":\"\",\"SNAPSHOTSCHEDULEID\":\"--\",\"SUBTYPE\":\"0\",\"THINCAPACITYUSAGE\":\"0\",\"TOTALSAVEDCAPACITY\":\"0\",\"TOTALSAVEDRATIO\":\"0\",\"TYPE\":11,\"USAGETYPE\":\"0\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WRITEPOLICY\":\"1\",\"WWN\":\"64cf55b1009769b4150f7d5e00000023\",\"blockDeviceName\":\"--\",\"devController\":\"--\",\"functionType\":\"1\",\"grainSize\":\"64\",\"hyperCdpScheduleId\":\"0\",\"isShowDedupAndCompression\":\"true\",\"lunCgId\":\"0\",\"mapped\":\"false\",\"remoteLunWwn\":\"--\",\"serviceEnabled\":\"false\",\"takeOverLunWwn\":\"--\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/stop") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077937880,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"CASCADEDLEVEL\":\"0\",\"CASCADEDNUM\":\"0\",\"CONSUMEDCAPACITY\":\"0\",\"DESCRIPTION\":\"\",\"EXPOSEDTOINITIATOR\":\"false\",\"HEALTHSTATUS\":\"1\",\"HYPERCOPYIDS\":\"\",\"ID\":\"62\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISSCHEDULEDSNAP\":\"0\",\"NAME\":\"guohao_test3\",\"NGUID\":\"71009769b4153fa84cf55b4d0000003e\",\"PARENTID\":\"35\",\"PARENTNAME\":\"flex_volume_lun0000\",\"PARENTTYPE\":11,\"ROLLBACKENDTIME\":\"-1\",\"ROLLBACKRATE\":\"-1\",\"ROLLBACKSPEED\":\"2\",\"ROLLBACKSTARTTIME\":\"-1\",\"ROLLBACKTARGETOBJID\":\"4294967295\",\"ROLLBACKTARGETOBJNAME\":\"--\",\"RUNNINGSTATUS\":\"43\",\"SOURCELUNCAPACITY\":\"2097152\",\"SOURCELUNID\":\"35\",\"SOURCELUNNAME\":\"flex_volume_lun0000\",\"SUBTYPE\":\"0\",\"TIMESTAMP\":\"1657144036\",\"TYPE\":27,\"USERCAPACITY\":\"2097152\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WWN\":\"64cf55b1009769b4153fa84d0000003e\",\"coupleUuid\":\"3e0060424c50b501b469975bf54c0021\",\"isReadOnly\":\"0\",\"replicationCapacity\":\"0\",\"snapCgId\":\"4294967295\",\"snapCgName\":\"--\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    }
    return Module::SUCCESS;
}

/**
 * 打桩存储客户端只有获取“一致性激活快照”接口异常
 */
int32_t StorageClientActivateSnapFailOnly(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if (httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/system/") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/lun?range=%5B0-100%5D&filter=SUBTYPE%3A%3A0%20and%20NAME%3Aflex_volume_lun000") != std::string::npos) {
        httpResponse.m_body = "{\"data\":[{\"ALLOCCAPACITY\":\"0\",\"ALLOCTYPE\":\"1\",\"CAPABILITY\":\"3\",\"CAPACITY\":\"2097152\",\"CAPACITYALARMLEVEL\":\"2\",\"COMPRESSION\":\"0\",\"COMPRESSIONSAVEDCAPACITY\":\"0\",\"COMPRESSIONSAVEDRATIO\":\"0\",\"DEDUPSAVEDCAPACITY\":\"0\",\"DEDUPSAVEDRATIO\":\"0\",\"DESCRIPTION\":\"\",\"DISGUISEREMOTEARRAYID\":\"--\",\"DISGUISESTATUS\":\"0\",\"DRS_ENABLE\":\"false\",\"ENABLECOMPRESSION\":\"true\",\"ENABLEISCSITHINLUNTHRESHOLD\":\"false\",\"ENABLESMARTDEDUP\":\"false\",\"EXPOSEDTOINITIATOR\":\"false\",\"EXTENDIFSWITCH\":\"false\",\"HASRSSOBJECT\":\"{\\\"SnapShot\\\":\\\"FALSE\\\",\\\"LunCopy\\\":\\\"FALSE\\\",\\\"RemoteReplication\\\":\\\"FALSE\\\",\\\"SplitMirror\\\":\\\"FALSE\\\",\\\"LunMigration\\\":\\\"FALSE\\\",\\\"LUNMirror\\\":\\\"FALSE\\\",\\\"HyperMetro\\\":\\\"FALSE\\\",\\\"LunClone\\\":\\\"FALSE\\\",\\\"HyperCopy\\\":\\\"FALSE\\\",\\\"HyperCDP\\\":\\\"FALSE\\\",\\\"CloudBackup\\\":\\\"FALSE\\\",\\\"drStar\\\":\\\"FALSE\\\"}\",\"HEALTHSTATUS\":\"1\",\"HYPERCDPSCHEDULEDISABLE\":\"0\",\"ID\":\"35\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISADD2LUNGROUP\":\"false\",\"ISCHECKZEROPAGE\":\"true\",\"ISCLONE\":\"false\",\"ISCSITHINLUNTHRESHOLD\":\"90\",\"MIRRORPOLICY\":\"1\",\"MIRRORTYPE\":\"0\",\"NAME\":\"flex_volume_lun0000\",\"NGUID\":\"71009769b4150f7d4cf55b5e00000023\",\"OWNINGCONTROLLER\":\"--\",\"PARENTID\":\"0\",\"PARENTNAME\":\"s0\",\"PREFETCHPOLICY\":\"0\",\"PREFETCHVALUE\":\"0\",\"REMOTELUNID\":\"--\",\"REPLICATION_CAPACITY\":\"0\",\"RUNNINGSTATUS\":\"27\",\"RUNNINGWRITEPOLICY\":\"1\",\"SC_HITRAGE\":\"0\",\"SECTORSIZE\":\"512\",\"SMARTCACHEPARTITIONID\":\"\",\"SNAPSHOTSCHEDULEID\":\"--\",\"SUBTYPE\":\"0\",\"THINCAPACITYUSAGE\":\"0\",\"TOTALSAVEDCAPACITY\":\"0\",\"TOTALSAVEDRATIO\":\"0\",\"TYPE\":11,\"USAGETYPE\":\"0\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WRITEPOLICY\":\"1\",\"WWN\":\"64cf55b1009769b4150f7d5e00000023\",\"blockDeviceName\":\"--\",\"devController\":\"--\",\"functionType\":\"1\",\"grainSize\":\"64\",\"hyperCdpScheduleId\":\"0\",\"isShowDedupAndCompression\":\"true\",\"lunCgId\":\"0\",\"mapped\":\"false\",\"remoteLunWwn\":\"--\",\"serviceEnabled\":\"false\",\"takeOverLunWwn\":\"--\"}],\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/stop") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/activate") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077937880,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"CASCADEDLEVEL\":\"0\",\"CASCADEDNUM\":\"0\",\"CONSUMEDCAPACITY\":\"0\",\"DESCRIPTION\":\"\",\"EXPOSEDTOINITIATOR\":\"false\",\"HEALTHSTATUS\":\"1\",\"HYPERCOPYIDS\":\"\",\"ID\":\"62\",\"IOCLASSID\":\"\",\"IOPRIORITY\":\"1\",\"ISSCHEDULEDSNAP\":\"0\",\"NAME\":\"guohao_test3\",\"NGUID\":\"71009769b4153fa84cf55b4d0000003e\",\"PARENTID\":\"35\",\"PARENTNAME\":\"flex_volume_lun0000\",\"PARENTTYPE\":11,\"ROLLBACKENDTIME\":\"-1\",\"ROLLBACKRATE\":\"-1\",\"ROLLBACKSPEED\":\"2\",\"ROLLBACKSTARTTIME\":\"-1\",\"ROLLBACKTARGETOBJID\":\"4294967295\",\"ROLLBACKTARGETOBJNAME\":\"--\",\"RUNNINGSTATUS\":\"43\",\"SOURCELUNCAPACITY\":\"2097152\",\"SOURCELUNID\":\"35\",\"SOURCELUNNAME\":\"flex_volume_lun0000\",\"SUBTYPE\":\"0\",\"TIMESTAMP\":\"1657144036\",\"TYPE\":27,\"USERCAPACITY\":\"2097152\",\"WORKINGCONTROLLER\":\"--\",\"WORKLOADTYPEID\":\"1026\",\"WORKLOADTYPENAME\":\"San_Backup_Default\",\"WWN\":\"64cf55b1009769b4153fa84d0000003e\",\"coupleUuid\":\"3e0060424c50b501b469975bf54c0021\",\"isReadOnly\":\"0\",\"replicationCapacity\":\"0\",\"snapCgId\":\"4294967295\",\"snapCgName\":\"--\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    }
    return Module::SUCCESS;
}

/**
 * 打桩存储客户端删除快照响应不同结果
 */
int32_t StorageClientDeleteSnapMutiRet(HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    httpResponse.cookies = {
            "session=ismsession=EA27698A079EBAC777077D174395232FFE67D86ECB094E274D61E2015214467D;SameSite=Lax;path=/;httponly;secure;"};
    if (httpParam.url.find("/deviceManager/rest/xxxxx/sessions") != std::string::npos) {
        httpResponse.m_body = "{\"data\":{\"accountstate\":1,\"deviceid\":\"2102353GTD10L9000007\",\"iBaseToken\":\"57B90CC0894098717CD5516EE5DDBCA79F072BABF60A4A6D936FE5FFDD01090C\",\"lastloginip\":\"8.40.99.131\",\"lastlogintime\":1656339028,\"pwdchangetime\":1654707090,\"roleId\":\"1\",\"usergroup\":\"\",\"userid\":\"admin\",\"username\":\"admin\",\"userscope\":\"0\"},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/22") != std::string::npos && httpParam.method == "DELETE") {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":0,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/23") != std::string::npos && httpParam.method == "DELETE") {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077937880,\"description\":\"0\"}}";
    } else if(httpParam.url.find("/deviceManager/rest/2102353GTD10L9000007/snapshot/24") != std::string::npos && httpParam.method == "DELETE") {
        httpResponse.m_body = "{\"data\":{},\"error\":{\"code\":1077937881,\"description\":\"0\"}}";
    }
    return Module::SUCCESS;
}

int32_t SendKubeClientApiSuccess(void *obj, const Module::HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods/invmdb-1-1-m-0") !=
        std::string::npos) {
        httpResponse.m_body = "{\"spec\":{\"volumes\":[{\"name\":\"gmdbredo\",\"persistentVolumeClaim\":{\"claimName\":\"gmdbredo-invmdb-1-1-m-0\"}},{\"name\":\"gmdbdata\",\"persistentVolumeClaim\":{\"claimName\":\"gmdbdata-invmdb-1-1-m-0\"}},{\"name\":\"gmdblredo\",\"persistentVolumeClaim\":{\"claimName\":\"gmdblredo-invmdb-1-1-m-0\"}}]}}";
    } else if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"invmdb-1-1-m-0\"},\"spec\":{\"containers\":[{\"name\":\"invmdb\"}]}}]}";
    } else if (httpParam.url.find("/api/v1/persistentvolumes") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"pv name 01\"},\"spec\":{\"claimRef\":{\"name\":\"gmdbredo-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0000\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}},{\"metadata\":{\"name\":\"pv name 02\"},\"spec\":{\"claimRef\":{\"name\":\"gmdbdata-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0001\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}},{\"metadata\":{\"name\":\"pv name 03\"},\"spec\":{\"claimRef\":{\"name\":\"gmdblredo-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0002\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}}]}";
    }
    return Module::SUCCESS;
}

int32_t SendKubeClientApiSuccess2(void *obj, const HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"invmdb-1-1-m-0\"},\"spec\":{\"containers\":[]}}]}";
    }
    return Module::SUCCESS;
}

/**
 * 打桩同一POD下的PV不在同一个存储上
 */
int32_t KubeClientSendPvLunUrlNotSameStorage(void *obj, const HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods/invmdb-1-1-m-0") != std::string::npos) {
        httpResponse.m_body = "{\"spec\":{\"volumes\":[{\"name\":\"gmdbredo\",\"persistentVolumeClaim\":{\"claimName\":\"gmdbredo-invmdb-1-1-m-0\"}},{\"name\":\"gmdbdata\",\"persistentVolumeClaim\":{\"claimName\":\"gmdbdata-invmdb-1-1-m-0\"}},{\"name\":\"gmdblredo\",\"persistentVolumeClaim\":{\"claimName\":\"gmdblredo-invmdb-1-1-m-0\"}}]}}";
    } else if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"invmdb-1-1-m-0\"},\"spec\":{\"containers\":[{\"name\":\"invmdb\"}]}}]}";
    } else if (httpParam.url.find("/api/v1/persistentvolumes") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"pv name 01\"},\"spec\":{\"claimRef\":{\"name\":\"gmdbredo-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0000\",\"url\":\"https://8.40.102.116:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}},{\"metadata\":{\"name\":\"pv name 02\"},\"spec\":{\"claimRef\":{\"name\":\"gmdbdata-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0001\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}},{\"metadata\":{\"name\":\"pv name 03\"},\"spec\":{\"claimRef\":{\"name\":\"gmdblredo-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0002\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}}]}";
    }
    return Module::SUCCESS;
}

int32_t SendKubeClientFail(void *obj, const HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 404;
    httpResponse.m_body = "API NOT FOUND.";
    return Module::FAILED;
}

int32_t SendKubeClientApiFail2(void *obj, const HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods/invmdb-1-1-m-0") !=
        std::string::npos) {
        httpResponse.m_body = "{\"spec\":{\"volumes\":[{\"name\":\"gmdbredo\",\"persistentVolumeClaim\":{\"claimName\":\"gmdbredo-invmdb-1-1-m-0\"}},{\"name\":\"gmdbdata\",\"persistentVolumeClaim\":{\"claimName\":\"gmdbdata-invmdb-1-1-m-0\"}},{\"name\":\"gmdblredo\",\"persistentVolumeClaim\":{\"claimName\":\"gmdblredo-invmdb-1-1-m-0\"}}]}}";
        return Module::FAILED;
    } else if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"invmdb-1-1-m-0\"},\"spec\":{\"containers\":[{\"name\":\"invmdb\"}]}}]}";
        return Module::SUCCESS;
    } else if (httpParam.url.find("/api/v1/persistentvolumes") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"pv name 01\"},\"spec\":{\"claimRef\":{\"name\":\"gmdbredo-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0000\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}},{\"metadata\":{\"name\":\"pv name 02\"},\"spec\":{\"claimRef\":{\"name\":\"gmdbdata-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0001\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}},{\"metadata\":{\"name\":\"pv name 03\"},\"spec\":{\"claimRef\":{\"name\":\"gmdblredo-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0002\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}}]}";
        return Module::FAILED;
    }
}
/**
 * 打桩跟据pvc查找不到pv信息
 */
int32_t KubeClientSendNotFindPvsByVolumes(void *obj, const HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods/invmdb-1-1-m-0") != std::string::npos) {
        httpResponse.m_body = "{\"spec\":{\"volumes\":[{\"name\":\"hostdata\",\"persistentVolumeClaim\":{\"claimName\":\"hostdata-chargingcacheb-1-0\"}},{\"name\":\"cbs-zkinfo-configmap\",\"configMap\":{\"name\":\"cbs-zkinfo-configmap1\",\"defaultMode\":420}},{\"name\":\"df-tool-encrypt\",\"hostPath\":{\"path\":\"/home/uniagent/agent_plugins/DeployPlugin/ext/fusionstage/encrypt/mixeddc_tool\",\"type\":\"\"}},{\"name\":\"df-tool-strings\",\"hostPath\":{\"path\":\"/home/uniagent/agent_plugins/DeployPlugin/ext/fusionstage/encrypt/strings_tool\",\"type\":\"\"}},{\"name\":\"multus\",\"hostPath\":{\"path\":\"/onip/config/multus\",\"type\":\"\"}}]}}";
    } else if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"invmdb-1-1-m-0\"},\"spec\":{\"containers\":[{\"name\":\"invmdb\"}]}}]}";
    } else if (httpParam.url.find("/api/v1/persistentvolumes") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"pv name 01\"},\"spec\":{\"claimRef\":{\"name\":\"gmdbredo-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0000\",\"url\":\"https://8.40.102.116:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}},{\"metadata\":{\"name\":\"pv name 02\"},\"spec\":{\"claimRef\":{\"name\":\"gmdbdata-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0001\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}},{\"metadata\":{\"name\":\"pv name 03\"},\"spec\":{\"claimRef\":{\"name\":\"gmdblredo-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0002\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}}]}";
    }
    return Module::SUCCESS;
}
/**
 * 打桩获取到两个pods需要备份
 */
int32_t KubeClientGetTwoPods(void *obj, const Module::HttpRequest &httpParam, HttpResponseInfo &httpResponse)
{
    httpResponse.m_statusCode = 200;
    if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods/invmdb-1-1-m") != std::string::npos) {
        httpResponse.m_body = "{\"spec\":{\"volumes\":[{\"name\":\"gmdbredo\",\"persistentVolumeClaim\":{\"claimName\":\"gmdbredo-invmdb-1-1-m-0\"}},{\"name\":\"gmdbdata\",\"persistentVolumeClaim\":{\"claimName\":\"gmdbdata-invmdb-1-1-m-0\"}},{\"name\":\"gmdblredo\",\"persistentVolumeClaim\":{\"claimName\":\"gmdblredo-invmdb-1-1-m-0\"}}]}}";
    } else if (httpParam.url.find("/api/v1/namespaces/ns000000000000000000001/pods") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"invmdb-1-1-m-0\"},\"spec\":{\"containers\":[{\"name\":\"invmdb\"}]}},{\"metadata\":{\"name\":\"invmdb-1-1-m-1\"},\"spec\":{\"containers\":[{\"name\":\"invmdb\"}]}}]}";
    } else if (httpParam.url.find("/api/v1/persistentvolumes") != std::string::npos) {
        httpResponse.m_body = "{\"items\":[{\"metadata\":{\"name\":\"pv name 01\"},\"spec\":{\"claimRef\":{\"name\":\"gmdbredo-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0000\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}},{\"metadata\":{\"name\":\"pv name 02\"},\"spec\":{\"claimRef\":{\"name\":\"gmdbdata-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0001\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}},{\"metadata\":{\"name\":\"pv name 03\"},\"spec\":{\"claimRef\":{\"name\":\"gmdblredo-invmdb-1-1-m-0\"},\"flexVolume\":{\"options\":{\"lunname\":\"flex_volume_lun0002\",\"url\":\"https://8.40.102.115:8088/deviceManager/rest\"}},\"capacity\":{\"storage\":\"12Gi\"}}}]}";
    }
    return Module::SUCCESS;
}

void ReportLog2AgentSuccess(VirtPlugin::ReportLog2AgentParam &param)
{
    g_logSize++;
}

std::pair<int32_t, std::shared_ptr<StorageClient>> Stub_StorageClientCreate2(const std::string &ip, int port,
    const AccessAuthParam &accessAuthParam, const std::vector<std::string> &ipList)
{
    StorageDeviceAuthData authData = {"", "", {""}};
    std::shared_ptr <StorageClient> dummy = std::make_shared<StorageClient>(ip, port, accessAuthParam, authData, ipList);
    if ((ip == "8.40.111.70" || ip == "127.0.0.1" ) && port == 8088 &&
        accessAuthParam.m_userName == "abc" && accessAuthParam.m_userkey == "123") {
        return std::make_pair(0, dummy);
    } else {
        return std::make_pair(Module::FAILED, dummy);
    }
}

/**
 * 测试用例：测试CheckApplicatoin接口成功
 * 前置条件：底层返回正常。
 * Check点：CheckApplicatoin返回值为0
 */
TEST_F(KubernetesProtectEngineTest, CheckApplicatoin_Success) {
    typedef std::pair<int, std::vector<ApplicationResource>> (*lPtr)();
    lPtr ListNameSpacesPtr = (lPtr)(&KubernetesApi::ListNameSpaces);
    typedef std::pair<int, std::set<std::string>> (*lsPtr)();
    lsPtr ListStoragesPtr = (lsPtr)(&KubernetesApi::ListStorages);

    Stub stub;
    stub.set(ListNameSpacesPtr, Stub_ListNameSpaceSuccess);
    stub.set(ListStoragesPtr, Stub_ListStorageSuccess);
    stub.set(ADDR(KubeConfig,Create), Stub_CreatConfigSuccess);
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);


    ActionResult result;
    ApplicationEnvironment env;
    env.auth.extendInfo = R"({"config":"123","storages":"[{\"username\": \"test\",\"password\": \"test\",\"ip\": \"8.40.111.70\",\"ipList\": \"8.40.111.70\",\"port\": 8088}]"})";
    Application app;
    m_k8sProtectEngineHandler->CheckApplication(result, env, app);
    EXPECT_EQ(result.code, Module::SUCCESS);
}

/**
 * 测试用例：测试CheckApplicatoin接口失败
 * 前置条件：输入env.auth.extendInfo为空。
 * Check点：返回指定CONNECT_FAILED错误码
 */
TEST_F(KubernetesProtectEngineTest, CheckApplication_Failed_WhenInputAuthExtendEmpty) {
    ActionResult result;
    ApplicationEnvironment env;
    env.auth.extendInfo = "";
    Application app;
    m_k8sProtectEngineHandler->CheckApplication(result, env, app);
    EXPECT_EQ(result.bodyErr, CONNECT_FAILED);
}


/**
 * 测试用例：测试CheckApplicatoin接口失败
 * 前置条件：输入env.auth.extendInfo.configs配置文件非法。
 * Check点：返回指定CONNECT_FAILED错误码
 */
TEST_F(KubernetesProtectEngineTest, CheckApplication_Failed_WhenInputConfigsWrong) {
    Stub stub;
    stub.set(ADDR(KubeConfig,Create), Stub_CreatConfigFailed);

    ActionResult result;
    ApplicationEnvironment env;
    env.auth.extendInfo = R"({"config":"illegal","storages":"[{\"username\": \"test\",\"password\": \"test\",\"ip\": \"127.0.0.1\",\"port\": 6666}]"})";
    Application app;
    m_k8sProtectEngineHandler->CheckApplication(result, env, app);
    EXPECT_EQ(result.bodyErr, CONNECT_FAILED);
}

/**
 * 测试用例：测试CheckApplication接口失败
 * 前置条件：输入存储信息与ListStorages信息不匹配。
 * Check点：返回错误码为期望错误码STORAGE_NOT_MATCH。
 */
TEST_F(KubernetesProtectEngineTest, CheckApplication_Failed_WhenStorageNotMatch) {
    typedef std::pair<int, std::vector<ApplicationResource>> (*lPtr)();
    lPtr ListNameSpacesPtr = (lPtr)(&KubernetesApi::ListNameSpaces);
    typedef std::pair<int, std::set<std::string>> (*lsPtr)();
    lsPtr ListStoragesPtr = (lsPtr)(&KubernetesApi::ListStorages);

    Stub stub;
    stub.set(ListNameSpacesPtr, Stub_ListNameSpaceSuccess);
    stub.set(ListStoragesPtr, Stub_ListStorageSuccess);
    stub.set(ADDR(KubeConfig,Create), Stub_CreatConfigSuccess);
    stub.set(ADDR(StorageClient, Send), SendStorageClientAuthFail);


    ActionResult result;
    ApplicationEnvironment env;
    //输入存储ip为 8.40.111.71， 与8.40.111.70不匹配
    env.auth.extendInfo = R"({"config":"123","storages":"[{\"username\": \"test\",\"password\": \"test\",\"ip\": \"8.40.111.71\",\"port\": 8088}]"})";
    Application app;
    m_k8sProtectEngineHandler->CheckApplication(result, env, app);
    EXPECT_EQ(result.bodyErr, STORAGE_NOT_MATCH);
}

/**
 * 测试用例：测试ListApplicationResourceV2接口成功。
 * 前置条件：输入ok。查顶层namespace资源。
 * Check点：不抛出异常。
 */
TEST_F(KubernetesProtectEngineTest, ListApplicationResourceV2_ForNamespaceSuccess) {
    typedef std::pair<int, std::vector<ApplicationResource>> (*lPtr)();
    lPtr ListNameSpacesPtr = (lPtr)(&KubernetesApi::ListNameSpaces);

    Stub stub;
    stub.set(ListNameSpacesPtr, Stub_ListNameSpaceSuccess);
    stub.set(ADDR(KubeConfig,Create), Stub_CreatConfigSuccess);

    ListResourceRequest request;
    ResourceResultByPage result;
    request.appEnv.auth.extendInfo = R"({"config":"illegal","storages":"[{\"username\": \"test\",\"password\": \"test\",\"port\": 6666}]"})";
    request.applications = {};
    m_k8sProtectEngineHandler->ListApplicationResourceV2(result, request);
    EXPECT_EQ(result.items.size(), 1);
}

/**
 * 测试用例：测试ListApplicationResourceV2接口成功。
 * 前置条件：输入ok。查底层statefulSet资源。
 * Check点：不抛出异常。
 */
TEST_F(KubernetesProtectEngineTest, ListApplicationResourceV2_ForStsSuccess) {
    typedef std::pair<int, std::vector<ApplicationResource>> (*lPtr)();
    lPtr ListStatefulSetPtr = (lPtr)(&KubernetesApi::ListStatefulSet);

    Stub stub;
    stub.set(ListStatefulSetPtr, Stub_ListStatefulSetSuccess);
    stub.set(ADDR(KubeConfig,Create), Stub_CreatConfigSuccess);

    ListResourceRequest request;
    ResourceResultByPage result;
    request.appEnv.auth.extendInfo = R"({"config":"illegal","storages":"[{\"username\": \"test\",\"password\": \"test\",\"port\": 6666}]"})";
    Application parentResource;
    parentResource.subType = "KubernetesNamespace";
    request.applications.push_back(parentResource);
    m_k8sProtectEngineHandler->ListApplicationResourceV2(result, request);
    EXPECT_EQ(result.items.size(), 1);
}

/**
 * 测试用例：测试ListApplicationResourceV2接口失败。
 * 前置条件：输入错误参数。查底层statefulSet资源。
 * Check点：抛出异常
 */
TEST_F(KubernetesProtectEngineTest, ListApplicationResourceV2_fail_when_appEnv_auth_extendInfo_worry)
{
    typedef std::pair<int, std::vector<ApplicationResource>> (*lPtr)();
    lPtr ListStatefulSetPtr = (lPtr) (&KubernetesApi::ListStatefulSet);

    Stub stub;
    stub.set(ListStatefulSetPtr, Stub_ListStatefulSetSuccess);
    stub.set(ADDR(KubeConfig, Create), Stub_CreatConfigSuccess);

    ListResourceRequest request;
    ResourceResultByPage result;
    request.appEnv.auth.extendInfo = R"({)";
    Application parentResource;
    parentResource.subType = "KubernetesNamespace";
    request.applications.push_back(parentResource);
    try {
        m_k8sProtectEngineHandler->ListApplicationResourceV2(result, request);
        AppProtect::AppProtectPluginException testFailException;
        throw testFailException;
    } catch (AppProtect::AppProtectPluginException exception) {
        EXPECT_EQ(exception.code, Module::FAILED);
        EXPECT_EQ(exception.message, "Fail to extract authExtendInfo.");
    }
}

/**
 * 测试用例：测试ListApplicationResourceV2接口失败。
 * 前置条件：输入正确参数。查底层statefulSet资源失败。
 * Check点：抛出异常
 */
TEST_F(KubernetesProtectEngineTest, ListApplicationResourceV2_fail_when_k8s_list_ns_fail)
{
    typedef std::pair<int, std::vector<ApplicationResource>> (*lPtr)();
    lPtr ListNameSpacesPtr = (lPtr)(&KubernetesApi::ListNameSpaces);

    Stub stub;
    stub.set(ListNameSpacesPtr, Stub_ListNameSpaceFail);
    stub.set(ADDR(KubeConfig,Create), Stub_CreatConfigSuccess);

    ListResourceRequest request;
    ResourceResultByPage result;
    request.appEnv.auth.extendInfo = R"({"config":"illegal","storages":"[{\"username\": \"test\",\"password\": \"test\",\"port\": 6666}]"})";
    request.applications = {};
    try {
        m_k8sProtectEngineHandler->ListApplicationResourceV2(result, request);
        AppProtect::AppProtectPluginException testFailException;
        throw testFailException;
    } catch (AppProtect::AppProtectPluginException exception) {
        EXPECT_EQ(exception.code, Module::FAILED);
        EXPECT_EQ(exception.message, "Fail to get namespaces from k8s cluster.");
    }
}

/**
 * 测试用例：测试ListApplicationResourceV2接口失败。
 * 前置条件：输入正确参数。查底层statefulSet资源失败。
 * Check点：抛出异常
 */
TEST_F(KubernetesProtectEngineTest, ListApplicationResourceV2_fail_when_k8s_list_sts_fail)
{
    typedef std::pair<int, std::vector<ApplicationResource>> (*lPtr)();
    lPtr ListNameSpacesPtr = (lPtr)(&KubernetesApi::ListNameSpaces);
    typedef std::pair<int, std::vector<ApplicationResource>> (*lStsPtr)();
    lStsPtr ListStatefulSetPtr = (lStsPtr)(&KubernetesApi::ListStatefulSet);

    Stub stub;
    stub.set(ListNameSpacesPtr, Stub_ListNameSpaceSuccess);
    stub.set(ListStatefulSetPtr, Stub_ListStatefulSetFailed);
    stub.set(ADDR(KubeConfig,Create), Stub_CreatConfigSuccess);

    ListResourceRequest request;
    ResourceResultByPage result;
    request.appEnv.auth.extendInfo = R"({"config":"illegal","storages":"[{\"username\": \"test\",\"password\": \"test\",\"port\": 6666}]"})";
    Application parentResource;
    parentResource.subType = "KubernetesNamespace";
    request.applications.push_back(parentResource);
    try {
        m_k8sProtectEngineHandler->ListApplicationResourceV2(result, request);
        AppProtect::AppProtectPluginException testFailException;
        throw testFailException;
    } catch (AppProtect::AppProtectPluginException exception) {
        EXPECT_EQ(exception.code, Module::FAILED);
        EXPECT_EQ(exception.message, "Fail to get stateful sets from k8s cluster.");
    }
}

/**
 * 测试用例：获取集群信息成功
 * 前置条件：1、存储认证，且获取基本信息正常
 * Check点：成功获取该存储sn信息。
 */
TEST_F(KubernetesProtectEngineTest, DiscoverAppCluster_Success) {
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);

    ApplicationEnvironment returnEnv;
    ApplicationEnvironment appEnv;
    Application application;
    application.auth.extendInfo = R"({"config":"illegal","storages":"[{\"ip\": \"8.40.102.115\",\"username\": \"test\",\"password\": \"test\",\"port\": 6666}]"})";
    m_k8sProtectEngineHandler->DiscoverAppCluster(returnEnv, appEnv, application);

    Json::Value extendValue;
    Module::JsonHelper::JsonStringToJsonValue(returnEnv.extendInfo, extendValue);
    EXPECT_EQ(extendValue.get("8.40.102.115", "").asString(), "21004cf55b9769b4");
}

/**
 * 测试用例：获取集群信息成功
 * 前置条件：1、下发参数正确；2、存储认证失败
 * Check点：没有获取到期望的值
 */
TEST_F(KubernetesProtectEngineTest, DiscoverAppCluster_success_when_storage_auth_fail)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageClientAuthFail);
    ApplicationEnvironment returnEnv;
    ApplicationEnvironment appEnv;
    Application application;
    application.auth.extendInfo = R"({"config":"illegal","storages":"[{\"ip\": \"8.40.102.115\",\"username\": \"test\",\"password\": \"test\",\"port\": 6666}]"})";
    m_k8sProtectEngineHandler->DiscoverAppCluster(returnEnv, appEnv, application);

    Json::Value extendValue;
    Module::JsonHelper::JsonStringToJsonValue(returnEnv.extendInfo, extendValue);
    EXPECT_EQ(extendValue.get("8.40.102.115", "").asString(), "");
}

/**
 * 测试用例：获取集群信息成功
 * 前置条件：1、下发参数不正确；
 * Check点：抛出异常
 */
TEST_F(KubernetesProtectEngineTest, DiscoverAppCluster_fail_when_application_auth_extendInfo_worry)
{
    ApplicationEnvironment returnEnv;
    ApplicationEnvironment appEnv;
    Application application;
    application.auth.extendInfo = R"({)";
    try {
        m_k8sProtectEngineHandler->DiscoverAppCluster(returnEnv, appEnv, application);
        AppProtect::AppProtectPluginException testFailException;
        throw testFailException;
    } catch (AppProtect::AppProtectPluginException exception) {
        EXPECT_EQ(exception.code, Module::FAILED);
        EXPECT_EQ(exception.message, "Fail to extract authExtendInfo.");
    }
}

/**
 * 测试用例：获取集群信息成功
 * 前置条件：1、下发参数正确；2、存储认证成功；3、获取存储基本信息失败
 * Check点：没有获取到期望的值
 */
TEST_F(KubernetesProtectEngineTest, DiscoverAppCluster_success_when_get_storage_auth_base_info_fail)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiFail);
    ApplicationEnvironment returnEnv;
    ApplicationEnvironment appEnv;
    Application application;
    application.auth.extendInfo = R"({"config":"illegal","storages":"[{\"ip\": \"8.40.102.115\",\"username\": \"test\",\"password\": \"test\",\"port\": 6666}]"})";
    m_k8sProtectEngineHandler->DiscoverAppCluster(returnEnv, appEnv, application);

    Json::Value extendValue;
    Module::JsonHelper::JsonStringToJsonValue(returnEnv.extendInfo, extendValue);
    EXPECT_EQ(extendValue.get("8.40.102.115", "").asString(), "");
}

/**
 * 测试用例：创建快照成功
 * 前置条件：1、k8s集群API响应正常；2、storage API响应正常；
 * Check点：创建一致性快照成功
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_Success)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);
    stub.set(ADDR(VirtPlugin::VirtualizationBasicJob, ReportLog2Agent), ReportLog2AgentSuccess);

    VirtPlugin::VirtualizationBasicJob virtualizationBasicJob;
    std::function<void(const VirtPlugin::ApplicationLabelType &)> handler = std::bind(&VirtPlugin::VirtualizationBasicJob::ReportApplicationLabels, &virtualizationBasicJob, std::placeholders::_1);
    m_k8sProtectEngineHandler->SetReportJobDetailHandler(handler);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088},{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.116\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(g_logSize, 2);
}

/**
 * 测试用例：创建快照成功，当存在两个两个PODs的时候
 * 前置条件：1、k8s集群API响应正常；2、storage API响应正常；
 * Check点：创建一致性快照成功
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_Success_when_there_are_two_pods_backup)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), KubeClientGetTwoPods);
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088},{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.116\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：创建快照成功
 * 前置条件：1、k8s集群API响应正常；2、storage API响应正常；3、前置脚本不为空；4、脚本回显标志位
 * Check点：创建一致性快照成功
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_success_when_pre_script_not_empty)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\",\"pre_script\":\"/test.py\"}";

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088},{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.116\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：创建快照成功
 * 前置条件：1、k8s集群API响应正常；2、storage API响应正常；3、前置脚本不为空；4、脚本执行失败
 * Check点：创建一致性快照成功
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_pre_script_exec_fail_and_no_pods_backup)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecFail);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\",\"pre_script\":\"/test.py\"}";

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088},{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.116\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、k8s集群API响应正常；2、storage API响应正常；3、statefulset管理容器数量为0
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_pods_in_charge_by_statfulset_number_is_zore)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess2);
    stub.set(ADDR(VirtPlugin::VirtualizationBasicJob, ReportLog2Agent), ReportLog2AgentSuccess);

    VirtPlugin::VirtualizationBasicJob virtualizationBasicJob;
    std::function<void(const VirtPlugin::ApplicationLabelType &)> handler = std::bind(&VirtPlugin::VirtualizationBasicJob::ReportApplicationLabels, &virtualizationBasicJob, std::placeholders::_1);
    m_k8sProtectEngineHandler->SetReportJobDetailHandler(handler);


    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
    EXPECT_EQ(g_logSize, 0);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参不正确
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_protect_object_extendInfo_worry)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参不正确
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_protectEnv_auth_extendInfo_worry)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs异常
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_k8s_list_pods_fail)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientFail);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs异常
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_k8s_list_pods_empty)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "bmpdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD失败；
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_k8s_list_pvcs_fail)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiFail2);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD失败；
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_parse_storages_fail)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"{\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD失败；
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_pods_pvcs_not_in_same_storage)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), KubeClientSendPvLunUrlNotSameStorage);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD成功；4、入参存储不匹配PVC存储
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_storages_has_not_match_storage)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.116\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD成功；4、入参存储不匹配PVC存储
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_product_storage_auth_fail)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), SendStorageClientAuthFail);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD成功；4、根据传入的卷未查询到PV信息
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_not_find_pvs_by_volumes)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), KubeClientSendNotFindPvsByVolumes);
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}


/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD成功；4、获取存储基本信息接口异常
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_product_storage_get_base_info_fail)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiFail);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD成功；4、获取存储基本信息接口异常
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_product_storage_get_lun_info_by_name_fail)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), StorageClientGetLunInfoFailOnly);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD成功；4、获取创建快照接口异常
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_product_storage_create_snap_fail)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), StorageClientCreateSnapFailOnly);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD成功；4、获取取消激活快照接口异常
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_product_storage_stop_snap_fail)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), StorageClientStopSnapFailOnly);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：创建快照失败
 * 前置条件：1、入参正确；2、k8s集群结构查询PODs正常；3、k8s集群查询详细POD成功；4、获取一致性激活快照接口异常
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, CreateSnapshot_fail_when_product_storage_activate_snap_fail)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), StorageClientActivateSnapFailOnly);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：获取存储基本信息成功
 * 前置条件：1、k8s集群API响应正常；2、storage API响应正常；
 * Check点：创建一致性快照成功
 */
TEST_F(KubernetesProtectEngineTest, GetMachineMetadata_Success)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088}]\"}";

    SnapshotInfo snapshotInfo;
    std::string errCode;
    int ret = m_k8sProtectEngineHandler->CreateSnapshot(snapshotInfo, errCode);
    EXPECT_EQ(ret, Module::SUCCESS);

    VMInfo vmInfo;
    ret = m_k8sProtectEngineHandler->GetMachineMetadata(vmInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(vmInfo.m_moRef, "statefulset ID");
    EXPECT_EQ(vmInfo.m_uuid, "statefulset ID");
    EXPECT_EQ(vmInfo.m_name, "invmdb-1-1-m");
    EXPECT_EQ(vmInfo.m_location, "kubernetes cluster ID");
    EXPECT_EQ(vmInfo.m_volList.size(), 3);
    EXPECT_EQ(vmInfo.m_volList[0].m_volSizeInBytes, 2097152 * 512);
}

/**
 * 测试用例：删除快照成功
 * 前置条件：1、storage API响应正常；
 * Check点：响应成功码
 */
TEST_F(KubernetesProtectEngineTest, DeleteSnapshot_Success)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolSnapInfo volSnapInfo;
    volSnapInfo.m_snapshotId = "22";
    volSnapInfo.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "123456";
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    int ret = m_k8sProtectEngineHandler->DeleteSnapshot(snapshotInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：删除快照失败
 * 前置条件：1、备份参数中的备份设备参数不是标准json；
 * Check点：响应失败错误码
 */
TEST_F(KubernetesProtectEngineTest, DeleteSnapshot_Fail)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectEnv.auth.extendInfo = "l";

    SnapshotInfo snapshotInfo;
    int ret = m_k8sProtectEngineHandler->DeleteSnapshot(snapshotInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：删除快照失败
 * 前置条件：1、快照所在存储设备认证信息不在备份参数中；
 * Check点：响应失败错误码
 */
TEST_F(KubernetesProtectEngineTest, DeleteSnapshot_Fail2)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolSnapInfo volSnapInfo;

    DatastoreInfo datastoreInfo;
    // 和上面存储SN不一致
    datastoreInfo.m_moRef = "1234567";
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;
    int ret = m_k8sProtectEngineHandler->DeleteSnapshot(snapshotInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

std::pair<int32_t, SnapshotInfoData> QuerySnapshotStubRet(const std::string &snapshotId)
{
    SnapshotCreateResponse snapshotCreateResponse;
    snapshotCreateResponse.m_snapshotInfoData.m_runningStatus == "55";
    return std::make_pair(0, snapshotCreateResponse.m_snapshotInfoData);
}

/**
 * 测试用例：删除快照成功
 * 前置条件：1、storage API响应正常；
 * Check点：响应成功码
 */
TEST_F(KubernetesProtectEngineTest, DeleteSnapshot_Success_when_muti_snaps_delete)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), StorageClientDeleteSnapMutiRet);
    stub.set(ADDR(StorageClient, QuerySnapshot), QuerySnapshotStubRet);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolSnapInfo volSnapInfo1;
    volSnapInfo1.m_snapshotId = "22";
    volSnapInfo1.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "123456";
    volSnapInfo1.m_datastore = datastoreInfo;

    VirtPlugin::VolSnapInfo volSnapInfo2;
    volSnapInfo2.m_snapshotId = "23";
    volSnapInfo2.m_snapshotName = "test_name2";
    volSnapInfo2.m_datastore = datastoreInfo;

    VirtPlugin::VolSnapInfo volSnapInfo3;
    volSnapInfo3.m_snapshotId = "24";
    volSnapInfo3.m_snapshotName = "test_name3";
    volSnapInfo3.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo1, volSnapInfo2, volSnapInfo3};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    int ret = m_k8sProtectEngineHandler->DeleteSnapshot(snapshotInfo);
    EXPECT_EQ(ret, Module::FAILED);
}


/**
 * 测试用例：删除快照失败
 * 前置条件：1、storage API响应正常；2、要删除的快照已经不存在
 * Check点：响应成功错误码
 */
TEST_F(KubernetesProtectEngineTest, DeleteSnapshot_success_when_snap_not_exists)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiFail);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolSnapInfo volSnapInfo;
    volSnapInfo.m_snapshotId = "22";
    volSnapInfo.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "123456";
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    int ret = m_k8sProtectEngineHandler->DeleteSnapshot(snapshotInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
}


/**
 * 测试用例：查询快照信息成功
 * 前置条件：1、初始化传入备份正确参数；2、所有快照都存在
 * Check点：调用成功
 */
TEST_F(KubernetesProtectEngineTest, QuerySnapshotExists_Success)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolSnapInfo volSnapInfo;
    volSnapInfo.m_snapshotId = "18";
    volSnapInfo.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "123456";
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    int ret = m_k8sProtectEngineHandler->QuerySnapshotExists(snapshotInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(snapshotInfo.m_deleted, false);
}

/**
 * 测试用例：查询快照信息成功
 * 前置条件：1、初始化传入备份正确参数；2、所有快照都存在
 * Check点：1、调用成功；2、删除标记位为true
 */
TEST_F(KubernetesProtectEngineTest, QuerySnapshotExists_Success2)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiFail);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolSnapInfo volSnapInfo;
    volSnapInfo.m_snapshotId = "18";
    volSnapInfo.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "123456";
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    int ret = m_k8sProtectEngineHandler->QuerySnapshotExists(snapshotInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(snapshotInfo.m_deleted, true);
}

/**
 * 测试用例：查询快照信息失败，由于备份参数中没有存储信息
 * 前置条件：1、初始化传入备份错误参数；2、所有快照都存在
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, QuerySnapshotExists_Fail_there_is_no_storages_param)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "";

    VirtPlugin::VolSnapInfo volSnapInfo;
    volSnapInfo.m_snapshotId = "18";
    volSnapInfo.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "123456";
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    int ret = m_k8sProtectEngineHandler->QuerySnapshotExists(snapshotInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：查询快照信息失败，由于备份参数中存储和快照所在存储信息不匹配
 * 前置条件：1、初始化传入备份正确参数；2、所有快照都存在；3、存储信息不匹配
 * Check点：返回失败码
 */
TEST_F(KubernetesProtectEngineTest, QuerySnapshotExists_Fail_there_is_no_storages_match)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolSnapInfo volSnapInfo;
    volSnapInfo.m_snapshotId = "18";
    volSnapInfo.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "1234567";
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    int ret = m_k8sProtectEngineHandler->QuerySnapshotExists(snapshotInfo);
    EXPECT_EQ(ret, Module::FAILED);
}


/**
 * 测试用例：调用GetSnapshotsOfVolume函数成功
 * 前置条件：1. 参数填写正确。2. 底层接口调用成功。
 * Check点：返回值成功
 */
TEST_F(KubernetesProtectEngineTest, GetSnapshotsOfVolume_Success)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolInfo volInfo;
    volInfo.m_uuid = "0001";
    volInfo.m_name = "lun0001";
    VirtPlugin::DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "123456"; 
    volInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> snapList;
    int ret = m_k8sProtectEngineHandler->GetSnapshotsOfVolume(volInfo, snapList);
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(snapList.size(), 1);
}

/**
 * 测试用例：调用GetSnapshotsOfVolume函数失败
 * 前置条件：1. 卷参数不完整，缺少存储的sn号
 * Check点：返回值失败
 */
TEST_F(KubernetesProtectEngineTest, GetSnapshotsOfVolume_Failed)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolInfo volInfo;
    volInfo.m_uuid = "0001";
    volInfo.m_name = "lun0001";
    std::vector<VirtPlugin::VolSnapInfo> snapList;
    int ret = m_k8sProtectEngineHandler->GetSnapshotsOfVolume(volInfo, snapList);
    EXPECT_EQ(ret, Module::FAILED);
    EXPECT_EQ(snapList.size(), 0);
}

/**
 * 测试用例：检查增量备份是否能执行成功
 * 前置条件：1、初始化传入备份正确参数；2、所有快照都存在；
 * Check点：1、返回成功码；2、检查结果为true
 */
TEST_F(KubernetesProtectEngineTest, CheckBackupJobType_success)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);

    AppProtect::BackupJob backupJob;
    backupJob.protectEnv.id = "kubernetes cluster ID";
    backupJob.protectEnv.type = "VirtualPlatform";
    backupJob.protectEnv.subType = "Kubernetes";
    backupJob.protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolSnapInfo volSnapInfo;
    volSnapInfo.m_snapshotId = "18";
    volSnapInfo.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "123456";
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    VirtPlugin::JobTypeParam jobTypeParam;
    jobTypeParam.m_job = backupJob;
    jobTypeParam.m_snapshotInfo = snapshotInfo;

    bool checkRet = false;
    int ret = m_k8sProtectEngineHandler->CheckBackupJobType(jobTypeParam, checkRet);
    EXPECT_EQ(ret, Module::SUCCESS);
    EXPECT_EQ(checkRet, true);
}


/**
 * 测试用例：检查增量备份是否能执行失败
 * 前置条件：1、初始化传入备份正确参数；2、所有快照都存在；3、存储信息不匹配
 * Check点：1、返回失败码；2、检查结果为false
 */
TEST_F(KubernetesProtectEngineTest, CheckBackupJobType_fail_when_storage_param_not_match)
{
    AppProtect::BackupJob backupJob;
    backupJob.protectEnv.id = "kubernetes cluster ID";
    backupJob.protectEnv.type = "VirtualPlatform";
    backupJob.protectEnv.subType = "Kubernetes";
    backupJob.protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    VirtPlugin::VolSnapInfo volSnapInfo;
    volSnapInfo.m_snapshotId = "18";
    volSnapInfo.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "1234567";
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    VirtPlugin::JobTypeParam jobTypeParam;
    jobTypeParam.m_job = backupJob;
    jobTypeParam.m_snapshotInfo = snapshotInfo;

    bool checkRet = false;
    int ret = m_k8sProtectEngineHandler->CheckBackupJobType(jobTypeParam, checkRet);
    EXPECT_EQ(ret, Module::FAILED);
    EXPECT_EQ(checkRet, false);
}

/**
 * 测试用例：检查增量备份是否能执行失败
 * 前置条件：1、初始化传入备份不正确参数；2、所有快照都存在；
 * Check点：1、返回失败码；2、检查结果为false
 */
TEST_F(KubernetesProtectEngineTest, CheckBackupJobType_fail_when_storage_worry)
{
    AppProtect::BackupJob backupJob;
    backupJob.protectEnv.id = "kubernetes cluster ID";
    backupJob.protectEnv.type = "VirtualPlatform";
    backupJob.protectEnv.subType = "Kubernetes";
    backupJob.protectEnv.auth.extendInfo = "";
    VirtPlugin::VolSnapInfo volSnapInfo;
    volSnapInfo.m_snapshotId = "18";
    volSnapInfo.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    datastoreInfo.m_moRef = "1234567";
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    VirtPlugin::JobTypeParam jobTypeParam;
    jobTypeParam.m_job = backupJob;
    jobTypeParam.m_snapshotInfo = snapshotInfo;

    bool checkRet = false;
    int ret = m_k8sProtectEngineHandler->CheckBackupJobType(jobTypeParam, checkRet);
    EXPECT_EQ(ret, Module::FAILED);
    EXPECT_EQ(checkRet, false);
}

/**
 * 测试用例：检查增量备份是否能执行失败
 * 前置条件：1、初始化传入备份不正确参数；2、所有快照都存在；3、上次快照信息错误
 * Check点：1、返回失败码；2、检查结果为false
 */
TEST_F(KubernetesProtectEngineTest, CheckBackupJobType_fail_when_snapshot_datastore_sn_empty)
{
    AppProtect::BackupJob backupJob;
    backupJob.protectEnv.id = "kubernetes cluster ID";
    backupJob.protectEnv.type = "VirtualPlatform";
    backupJob.protectEnv.subType = "Kubernetes";
    backupJob.protectEnv.auth.extendInfo = "{\"config\":\"\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";
    VirtPlugin::VolSnapInfo volSnapInfo;
    volSnapInfo.m_snapshotId = "18";
    volSnapInfo.m_snapshotName = "test_name";

    DatastoreInfo datastoreInfo;
    volSnapInfo.m_datastore = datastoreInfo;
    std::vector<VirtPlugin::VolSnapInfo> volsnapInfos = {volSnapInfo};

    SnapshotInfo snapshotInfo;
    snapshotInfo.m_volSnapList = volsnapInfos;

    VirtPlugin::JobTypeParam jobTypeParam;
    jobTypeParam.m_job = backupJob;
    jobTypeParam.m_snapshotInfo = snapshotInfo;

    bool checkRet = false;
    int ret = m_k8sProtectEngineHandler->CheckBackupJobType(jobTypeParam, checkRet);
    EXPECT_EQ(ret, Module::FAILED);
    EXPECT_EQ(checkRet, false);
}

/**
 * 测试用例：备份后置任务执行脚本
 * 前置条件：1、初始化传入备份正确参数；2、任务类型为备份；
 * Check点：备份后置hook执行成功
 */
TEST_F(KubernetesProtectEngineTest, PostHook_success)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);

    JobType jobType = m_k8sProtectEngineHandler->GetJobHandle()->GetJobType();
    jobType = JobType::BACKUP;

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents +
                                            "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    backupJob->protectObject.extendInfo = R"({"vmRef":"statfulset ID","volumeNames":"[]","pre_script":"/test.sh","post_script":"/clear.sh","failed_script":"/clear.sh"})";
    backupJob->protectObject.parentName = "ns000000000000000000001";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    int ret = m_k8sProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：备份后置任务执行脚本
 * 前置条件：1、初始化传入备份不正确参数；2、任务类型为备份；
 * Check点：备份后置hook执行成功
 */
TEST_F(KubernetesProtectEngineTest, PostHook_exec_post_script_success_when_protectEnv_auth_extendInfo_worry)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);

    JobType jobType = m_k8sProtectEngineHandler->GetJobHandle()->GetJobType();
    jobType = JobType::BACKUP;

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{";

    backupJob->protectObject.extendInfo = R"({"vmRef":"statfulset ID","volumeNames":"[]","pre_script":"/test.sh","post_script":"/clear.sh","failed_script":"/clear.sh"})";
    backupJob->protectObject.parentName = "ns000000000000000000001";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    int ret = m_k8sProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：备份后置任务执行脚本
 * 前置条件：1、初始化传入备份不正确参数；2、任务类型为备份；
 * Check点：备份后置hook执行成功
 */
TEST_F(KubernetesProtectEngineTest, PostHook_exec_post_script_fail_when_protectObject_extendInfo_worry)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);

    JobType jobType = m_k8sProtectEngineHandler->GetJobHandle()->GetJobType();
    jobType = JobType::BACKUP;

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectObject.extendInfo = R"({)";
    backupJob->protectObject.parentName = "ns000000000000000000001";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    int ret = m_k8sProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：备份后置任务执行脚本
 * 前置条件：1、初始化传入备份正确参数；2、任务类型为备份；3、k8s集群查询POD失败
 * Check点：备份后置hook执行成功
 */
TEST_F(KubernetesProtectEngineTest, PostHook_exec_post_script_success_when_k8s_list_pods_fail)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientFail);

    JobType jobType = m_k8sProtectEngineHandler->GetJobHandle()->GetJobType();
    jobType = JobType::BACKUP;

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents +
                                            "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    backupJob->protectObject.extendInfo = R"({"vmRef":"statfulset ID","volumeNames":"[]","pre_script":"/test.sh","post_script":"/clear.sh","failed_script":"/clear.sh"})";
    backupJob->protectObject.parentName = "ns000000000000000000001";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    int ret = m_k8sProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：备份后置任务执行脚本
 * 前置条件：1、初始化传入备份正确参数；2、任务类型为备份；3、k8s集群查询POD失败
 * Check点：备份后置hook执行成功
 */
TEST_F(KubernetesProtectEngineTest, PostHook_exec_post_script_fail_when_k8s_list_pods_empty)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess2);

    JobType jobType = m_k8sProtectEngineHandler->GetJobHandle()->GetJobType();
    jobType = JobType::BACKUP;

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents +
                                            "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    backupJob->protectObject.extendInfo = R"({"vmRef":"statfulset ID","volumeNames":"[]","pre_script":"/test.sh","post_script":"/clear.sh","failed_script":"/clear.sh"})";
    backupJob->protectObject.parentName = "ns000000000000000000001";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    int ret = m_k8sProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：备份后置任务失败
 * 前置条件：1、初始化传入备份不正确参数；2、任务类型为备份；
 * Check点：备份后置hook执行失败
 */
TEST_F(KubernetesProtectEngineTest, PostHook_fail_when_protect_object_extendInfo_worry)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);

    JobType jobType = m_k8sProtectEngineHandler->GetJobHandle()->GetJobType();
    jobType = JobType::BACKUP;

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectObject.extendInfo = R"({)";
    backupJob->protectObject.parentName = "ns000000000000000000001";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    int ret = m_k8sProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：备份后置任务执行脚本
 * 前置条件：1、初始化传入备份正确参数；2、任务类型为备份；
 * Check点：备份后置hook执行成功
 */
TEST_F(KubernetesProtectEngineTest, PostHook_exec_fail_script_success)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);

    JobType jobType = m_k8sProtectEngineHandler->GetJobHandle()->GetJobType();
    jobType = JobType::BACKUP;

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());
    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents +
                                            "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088, \\\"sn\\\": \\\"123456\\\"}]\"}";

    backupJob->protectObject.extendInfo = R"({"vmRef":"statfulset ID","volumeNames":"[]","pre_script":"/test.sh","post_script":"/clear.sh","failed_script":"/clear.sh"})";
    backupJob->protectObject.parentName = "ns000000000000000000001";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    execHookParam.jobExecRet = Module::FAILED;
    int ret = m_k8sProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：备份hook任务默认成功
 * 前置条件：1、初始化传入备份正确参数；2、任务类型为备份；
 * Check点：备份后置hook执行成功
 */
TEST_F(KubernetesProtectEngineTest, PostHook_default_success)
{
    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::PRE_PREREQUISITE;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    execHookParam.jobExecRet = Module::FAILED;
    int ret = m_k8sProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::SUCCESS);
}

// *******************【恢复任务】恢复任务分割线*************************

void SetMockRestore(std::shared_ptr<AppProtect::RestoreJob> restoreJob) {
    restoreJob->targetEnv.name = "FlexVolume01";
    restoreJob->targetEnv.id = "123";
    restoreJob->targetEnv.auth.extendInfo = R"({"config":"a2luZDogQ29uZmlnCmFwaVZlcnNpb246IHYxCmNsdXN0ZXJzOgotIGNsdXN0ZXI6CiAgICBjZXJ0aWZpY2F0ZS1hdXRob3JpdHktZGF0YTogTFMwdExTMUNSVWRKVGlCRFJWSlVTVVpKUTBGVVJTMHRMUzB0Q2sxSlNVaE9la05EUWxJclowRjNTVUpCWjBsS1FVMXZhblpFUVZWYVJrWnJUVUV3UjBOVGNVZFRTV0l6UkZGRlFrTjNWVUZOU1Vjd1RWRnpkME5SV1VRS1ZsRlJSMFYzU2tSVWFrVlRUVUpCUjBFeFZVVkRRWGRLVWpOV2FHSnRaRVZpTWpWdVRWSkZkMFIzV1VSV1VWRklSRUZvVkdGSFZuVlhiV2hzWW1wRmJBcE5RMDFIUVRGVlJVTm5kMk5UU0Zab1pESldjRWxHVW14Wk1taDFZako0ZGxveWJHeGplVUpFWW5rMGMwbEZlREJhUkVWb1RVSTRSMEV4VlVWRGQzZFpDbFF4VGxSSlExbG5WVEpXZVdSdGJHcGFVMEpWWWpJNWMyTjVRa1ZhV0VJd1RWSkpkMFZCV1VSV1VWRkVSRUZzVUZVeFRYcE1ha0ZuVVRCRmVFbEVRV1VLUW1kcmNXaHJhVWM1ZHpCQ1ExRkZWMFZYT1hwamVrNXFXVlZDYjJSWFJqTmFWMnQxV1RJNWRFMUNORmhFVkVsNVRVUlpkMDFxUVhwT1JHY3hUbXh2V0FwRVZFMTVUVVJWZWsxRVFYcE9SR2N4VG14dmQyZGlVWGhEZWtGS1FtZE9Wa0pCV1ZSQmEwNVBUVkpKZDBWQldVUldVVkZKUkVGc1NHUlhSblZhTUZKMkNtSnRZM2hGVkVGUVFtZE9Wa0pCWTAxRFJrNXZXbGMxWVdGSFZuVk5VMVYzU1hkWlJGWlJVVXRFUW5oSlpGZEdNMXBYYTJkV1IxWnFZVWMxZG1KSE9XNEtZVmRXZWtsRlRuWk1hWGRuVkVoU2EwMVRSWGRJZDFsRVZsRlJURVJDYUZCVk1VMW5TbWxDVkZwWVNqSmhWMDVzU1VaU2RtSXllSHBKUlZKc1kwaFJlQXBGYWtGUlFtZE9Wa0pCVFUxRFZUbFVWWHBOZFUxRFFrUlJWRVZuVFVJMFIwTlRjVWRUU1dJelJGRkZTa0ZTV1ZKaU0wNTZUVEpPYUZGSGFERlpXR1JzQ21GVE5XcGlNakIzWjJkSmFVMUJNRWREVTNGSFUwbGlNMFJSUlVKQlVWVkJRVFJKUTBSM1FYZG5aMGxMUVc5SlEwRlJSRk5LUkhsd2QzbFlURWhDTHpVS2JYSkZhR2gwV1RWalQzbGxUbVF2VVVWNGJuWlJZV1JhUW5KSGQwRjVNVzQ1YjNvM2JGTTVWREpHUm01NmJVbFlVMDVLWlZkVVFYVk5PV1F5V0c1RGFBbzRXVE5oYVdoemIxSTBORmhWTDNWd015dE5ZWFJPVG1kMlpuTXpORGhYYm05M2RHdFRkREpHVW1FcmNuaEZWbWQxZDIwNFNubHVkV2xDWmpaMVZ6WjBDbFpuYUVSSVMzSXJSbVVyYVV0ek5uaGlOWGR4YXpGWlRuaE1VRVkxVEdVeWNWVkdlVEJVY1ZKSU9YTmFUazlwWkZOdk9XUk1aR0pNVEROa2RreHNjRzhLUTBoVFF6QmpPUzkzTmxaVU5YVlRkMVY2V1VkT2MwUnlaeXRxVWtGVFdtaE5SVnBWVGtoNVJXeGxkWGQwUzBwNVlsTjJSbmhwV0hKa2RHdDRWMVphYndwcFRrOW5WbkZOZGpCWU5HOVJha1J3YlRWdlNYQnZhR2hYV0VWUE5VVndhblpZZEdkYU5rSnRiVlZVTURFclpHMWFkMjFpVXpadmRGSTFWemhPTjBoTUNrcHhTVFpRYVRWRlIwNUlSakppTkROR1VGUnZNblo0UjBSclNFRjFORXhPU0RkS05rRm5lRVZ1TW1rMlJtOTNXRWd6ZWtwd1FqQmFaVzVaYW5WMlZUWUtOMFE1UVdjcmFFUnVjRzkxUW5CSlVXaENSR28zVVdKSk5YUXhjVzFNVDFNMU1FMTNTWEFyWmxwWGFsVmtNMmxsYml0TVNqaGlRWFpZWTNKa1NYbENWd28yTmxweVV6STVPR0paYlVkWFdUTnJhVWhuUVZsalptWnBMMkkwWXpaNk1uWnNWWG8yYmxWMmJ5OWpXbHBaYTFOR1dHOVNWWE5uUzNJNWMzcFZNblppQ2s1VmRIbFVkekZqVFRKTk1qaDRNVWQwTDNkWVpsTlZlVXNyVjJKMVYzZHpkMFJRYTBOcE9HVk1RVTloU0dZeVFUaEdNa1prYUVkVk1FaGlVV1p6TVVrS1ZrTlJLMkpWWkdWVGMxWm1PVUY1VURGU2RHSnNWMjF1YzNkTU1FYzRiemRtY3pjeWRrNXRTVVIyY2pGc1pXZHRLMHh0VVVKRVpEVkVUVTFYWlVOcE9RcFVVbGRVUXpseE1tWmpVbHAxTldWeU0wOVJSVmdyZFVKT05IZEVkRkZKUkVGUlFVSnZORWxDVTBSRFEwRlZVWGRJVVZsRVZsSXdUMEpDV1VWR1RucFZDbkp2Wm5kR1RHOTJaR0pIZFhnd1F6ZHViRTAyVjBZNFpFMUpTSEJDWjA1V1NGTk5SV2RsUlhkblpEWkJSazU2VlhKdlpuZEdURzkyWkdKSGRYZ3dRemNLYm14Tk5sZEdPR1J2V1VjMmNFbEhNMDFKUnpCTlVYTjNRMUZaUkZaUlVVZEZkMHBFVkdwRlUwMUNRVWRCTVZWRlEwRjNTbEl6Vm1oaWJXUkZZakkxYmdwTlVrVjNSSGRaUkZaUlVVaEVRV2hVWVVkV2RWZHRhR3hpYWtWc1RVTk5SMEV4VlVWRFozZGpVMGhXYUdReVZuQkpSbEpzV1RKb2RXSXllSFphTW14c0NtTjVRa1JpZVRSelNVVjRNRnBFUldoTlFqaEhRVEZWUlVOM2QxbFVNVTVVU1VOWloxVXlWbmxrYld4cVdsTkNWV0l5T1hOamVVSkZXbGhDTUUxU1NYY0tSVUZaUkZaUlVVUkVRV3hRVlRGTmVreHFRV2RSTUVWNFNVUkJaVUpuYTNGb2EybEhPWGN3UWtOUlJWZEZWemw2WTNwT2FsbFZRbTlrVjBZeldsZHJkUXBaTWpsMFoyZHJRWGxwVHpoTlFsSnJWVmRSZDBSQldVUldVakJVUWtGVmQwRjNSVUl2ZWtGTVFtZE9Wa2hST0VWQ1FVMURRVkZaZDBoQldVUldVakJTQ2tKQ1ZYZEZORVZTWWpOT2VrMHlUbWhSUjJneFdWaGtiR0ZUTldwaU1qQjNSRkZaU2t0dldrbG9kbU5PUVZGRlRFSlJRVVJuWjBsQ1FVTlVTWGxFVkdzS05YQlBRVVJrVldzemJVdGpUR1Z5V2pGR1NVMTVhMlkxUzNScFFVNTBZMHhyUjJZNVdGTjVURmw2WW5OQ2FUWklhbTVUTjJ0MFoxZFFhRWQ2WlN0b2RRcENhMGwzYkhkNlJEVmlkbU0wSzJWV1Uyd3ZUa0Y1YzBsSmFuQlNialI1Y0cxRmVYQkNRM3B6TVdwV1FuVndiak5SVERWYVoyTkZNSFJWZEZvemJVbHNDblJxTVdwSVJqQmtVSFoxY3padk0yc3pNaXN5TjIwMlRVeHFOeXRFYW1GTGRUSkJlalY1YkROdWVTdFBVVGs1VkZKemRWQk9XblYxUjNWSGJuUndORUVLUml0MGREWjBlbTB5VkRnd1ptZG1WVmxQTDNGMVNHdEJTVGRoWlRSQk5FWkxOSGhZYldKeldWRmxaR0Z2VmxoMU1VVldiRFExTkZCWVIxaG5aMjB2ZVFwT01qUlpRbXNyYzFSWWRFNUpUMjVyU21Rd1NFZE5VRzlYT1U1SlRFUk5aM2tyYTB0VmRFWkJVakV5T1Rad1lsaHJUM3A1ZVRWSk5IQjJSV3BzTlRSV0NuVkNWRzF2WW1aRlJFUlFNbk5YV2sxa1RrbFNlRlZUVVdwV2JHRm9Na3B5TVhGTFdFOWtOVlpJTTBaVU1ucDRlbXRJZGpkMGJrZHVWblpSZEc5cU1Xb0tOVzEzT1cxS0sydFRhMDFEY1VoRFZEZElXRGRuVWxkeWRERmlTekZSTkRoMlVYbHZObGx5VG01MmVrZExaMEZPZUhZNWNrZE9Na3RPWm5wclowSjNTQXA2T1RGU2QwOVZUbnBDVUhsc00yMUZRUzh5Wm5OUlVrWXlPRVZ6YURrMFprOUdTVmRYUnpOeVdHTjBRa1UyWm5oMWNUVmlhVUYwZG01SFoxVkthbWhYQ2tkcWREVTNVREpFZWtkNU5rMTBWM2RSUzJJMlRsZ3haMVZpZFZsQldYZHZOelpRU1RsaWVuWlFlVU53UTJFeWFXaEplR1YzYTNOWlNHSndVMWhDUkdVS0syZHZhR05KY21oNlYybE9hamMwUXpCNVRXMXZURzl4WkhWNFpHVk9OM2RrYWpnNVJWTlpTbXRaZG5kU2F6RkZRWHAwWVZOeFlXMUNOV2RoYzA5UFRRb3dUVVp5TTA1a2FISkpVVFpVUmxOQ1VFOU5SV2xYTUU5bE9VdFRTbEZ3TW5oWlJuRUtMUzB0TFMxRlRrUWdRMFZTVkVsR1NVTkJWRVV0TFMwdExRbz0KICAgIHNlcnZlcjogaHR0cHM6Ly84LjQwLjEzNy43OjU0NDMKICBuYW1lOiBjbHVzdGVyCnVzZXJzOgotIG5hbWU6IGNmZS1tYXN0ZXIKICB1c2VyOgogICAgY2xpZW50LWNlcnRpZmljYXRlLWRhdGE6IExTMHRMUzFDUlVkSlRpQkRSVkpVU1VaSlEwRlVSUzB0TFMwdENrMUpTVWN3ZWtORFFreDFaMEYzU1VKQlowbEpWRmRYUTBsUlpqZ3ZWa2wzUkZGWlNrdHZXa2xvZG1OT1FWRkZURUpSUVhkbllsRjRRM3BCU2tKblRsWUtRa0ZaVkVGclRrOU5Va2wzUlVGWlJGWlJVVWxFUVd4SVpGZEdkVm93VW5aaWJXTjRSVlJCVUVKblRsWkNRV05OUTBaT2IxcFhOV0ZoUjFaMVRWTlZkd3BKZDFsRVZsRlJTMFJDZUVsa1YwWXpXbGRyWjFaSFZtcGhSelYyWWtjNWJtRlhWbnBKUlU1MlRHbDNaMVJJVW10TlUwVjNTSGRaUkZaUlVVeEVRbWhRQ2xVeFRXZEthVUpVV2xoS01tRlhUbXhKUmxKMllqSjRla2xGVW14alNGRjRSV3BCVVVKblRsWkNRVTFOUTFVNVZGVjZUWFZOUTBKRVVWUkZaMDFDTkVjS1ExTnhSMU5KWWpORVVVVktRVkpaVW1JelRucE5NazVvVVVkb01WbFlaR3hoVXpWcVlqSXdkMGxDWTA1TmFrbDNUbXBCZVUxRVZYZE5hbEV5VjJoblVBcE5ha0V6VFZSQk1rMUVTWGRPVkVGNVRrUmFZVTFKUjNCTlVYTjNRMUZaUkZaUlVVZEZkMHBFVkdwRlUwMUNRVWRCTVZWRlEwSk5TbEl6Vm1oaWJXUkZDbUl5Tlc1TlVrVjNSSGRaUkZaUlVVaEZkMmhVWVVkV2RWZHRhR3hpYWtWc1RVTk5SMEV4VlVWRGFFMWpVMGhXYUdReVZuQkpSbEpzV1RKb2RXSXllSFlLV2pKc2JHTjVRa1JpZVRSelNVVjRNRnBFUldoTlFqaEhRVEZWUlVOM2QxbFVNVTVVU1VOWloxVXlWbmxrYld4cVdsTkNWV0l5T1hOamVVSkZXbGhDTUFwTlVrbDNSVUZaUkZaUlVVUkZkMnhRVlRGTmVreHFRV2RSTUVWNFJsUkJWRUpuVFhCQlVVVlVSRWhDYUZsWVRYUmlNakIwV1RJNWVWcFVRME5CYVVsM0NrUlJXVXBMYjFwSmFIWmpUa0ZSUlVKQ1VVRkVaMmRKVUVGRVEwTkJaMjlEWjJkSlFrRk9WV05hU1VacWQwWTNablU0YWpkMlJtdG9WbUZIVlhORVdsSUtkMGx2WnpOb1JISmhWemxEVFRaaVFqWmhaRXAxVEVOeWEzSnBSM1p3VGt0cFZWUm1hVkZZVFhaeVZWWk1iMmROUVdsUmJrazRSR1JoZGtGcmVYZDViZ294T0hCM2NGRjZZME5hYm01SVR6Rllaamw0VEN0SFdHTkhUV013Tm5OaFNVUk9lbmRzUTNacE9URlhNemgwVlVod2EwRjVibFpuVm0weFpuUm9OVFUxQ21OaFRXSXhWVkZEV1RGNGNpOVRTRk13WlVOaFNsUkVOQ3RqZVZwTlNuWjNSRzg0VlhoblRuRTFhMU5LVlZwTVEybEhlbUZ1V1dKdVNGVmtLeXR1ZWxJS2VHOVNPR05LVlhseGVtWXhlVmRqTjAxNGRUbDFaR0YwUm1WMll5czVURk5KTlRaM1MwYzFjbFE1WkVkUU5VNWlkelF6TVhKdVJFaFJURTkxYUZCTE5BcGFRamd5UXk5R2NHWnRZelJZVG05c1dEQTVXVUp1WTBSV1lrTnVTMlJFUVc1SWVtcGlWbnAwVVVGUWExUm9ORVYxV1hsMVNGQlhURFJDU1RkWE5sVjBDbWt3Y21weFZFTTBaVEZuYlRKNlpuTjJZa3MxVG1ocFVsaHdUblpETlVwbk1GSnRVR05CTjFsT05YRnBiMHhNTDFjMGJqbHZRa3htYzBkVGJuRkJia0lLZDBGTFMwSnlSRzUzV2xjelVVcEdWalp5UldsWk9FZFBUVk5aWjB4cFVuSnplVXhNZURGV05pc3dVbkpyUm0xRFpGUlRlVmxvTVZGQlJUSllRbGhVVVFwVVQwdFFSMlZYYjJndlFVbE9Vbk5NVDNSMWNVMUllVmQzUTFSaFIyazBTa2hXUVcxc0wxTTVaRzEzVjFrMU4xQnpTek5qUjIxS1NtUllWRWxFUmpKU0NsbG5iRGN3VVRWWWNFRmxRems1Tkc5WGRFWmtVMGRuTW5VdmNXdFJhbmR2UjNoSFpqUlNTemR2YzB4ME5rdEJUVUpTTjFJclJuaHlkMEUyYzJoRWFVUUtORXBsUkdZM2FUaFpZWFZFYW01T2NGRXpMMDV2WlVSS1lsRkZabFphYzNkSmQzcHFVVk5KVDFwelpubEJORUZNYUVOeWNqZEpialJtTDBKcFpteG9UQXBLYkUxUGJHTjBhSEUzWXpjdlRtNHpRV2ROUWtGQlIycG5aVGgzWjJWM2QwUm5XVVJXVWpCUVFWRklMMEpCVVVSQlowdHJUVUl3UjBFeFZXUktVVkZYQ2sxQ1VVZERRM05IUVZGVlJrSjNUVU5DWjJkeVFtZEZSa0pSWTBSQlZFRk5RbWRPVmtoU1RVSkJaamhGUVdwQlFVMUNPRWRCTVZWa1NYZFJXVTFDWVVFS1JrNTZWWEp2Wm5kR1RHOTJaR0pIZFhnd1F6ZHViRTAyVjBZNFpFMUpSMHhDWjA1V1NGSkZSV2RaVFhkbldVTkRRak5DYUZsWVRYUmlNakpEUVVsSllncExhVFZyV2xkYWFHUlhlREJNYms0eVdYazFhbUpJVm5wa1IxWjVURzE0ZGxreVJuTm5hRmx4VEcwNWRFeHVUakpaZVRWcVlraFdlbVJIVm5sTWJYaDJDbGt5Um5ObmFHOXhURzB4YUdKdFJtNWFVelY2WkcxTmRWa3llREZqTTFKc1kyazFjMkl5VG1oaVNXTkZjMEozUVVGWlkwVnpRbmRCUVc5alJXWjNRVUVLUVZsalJVTkRhVXBDTkdORlEwTnBTa0kwWTBWRFEybEtRbnBCVGtKbmEzRm9hMmxIT1hjd1FrRlJjMFpCUVU5RFFXZEZRVWRNVkdSUVVXbFBha1l6THdwWUt5OUZibm94T0RKVUt6bG1TMGsyYlhvclMwVjJOME42WTNKaFJ5OTVhM0I0YmxsT2JWaHhlbEYyS3preVYwOXhTemczY1VoR2VHNU1SME4yTDAxMENqSlJkRGtyVGtVeWRsbDZWRkpZWVZkdFVuaDVUVmNyVkVWTmN6bEdWVU5sYldobFJqbFpSVTF2TW1OTVJHeHZVbkZzZURseVEyVnJiRmhHTkZSNVJUQUtZVEIzVUZKTk9VVlZlVmxFVlRod01uaFZWSGRrVWt4NGJHVjBWVU13YVhWSmFYQldhVmRYT0dwM1NqRlNWQzh2TmxoRVFWVnpkVklyV1dOT1dFTnBkQW93VVZWNVRVaGxkRVk1TVRsRFlXazVNamhDUzJ4NFpHRjJZMHR5UTBwU2FXRkZUV3REYms4M1dFMXVRa1Z6WTBORE9HVk1LMFo0Um1WcmFuTnZjMDQ1Q2xJM09IbHhURXBuU1U5NFpEUTNTRWhtUVc4MlpVWk1TMGs0U1hCMVJYVXdTSFI1T0cxcVMxRjJNMHRwYXpab05YbFBVM2hrYzNwaVpERTJjamhEWkRJS1ptaE1LMDAyYUdVd2FTOHlWMFExZUZOWGQweElNMGRxY3pjeVVIcG1aMnR3U0dvdlRHaHlhbmxEV0ZKR2FHbDRaMUJMZDJ4TVp6TlRMekZzUTA1alNBcEthekZWWTBGSmRESTVhVWdyWlZSQlRXZHRUR1E0U0hwc1drWm5kemxQZUhoT2NFWm9XVzV6UTBwQ1EyOXVaVGhaVkc5SGFDdDFiMU5MVXpCWFFtZ3JDazFDUms5bmRXUXpPRGNyTmpnMVVIZ3dWa2RDVldWd2NuUmlUbWtyUkVaT09FbENibUUxV2s1b1NUSXJaMnBKVDJack9XdFdUbkEyZEdOdFNYSlVkVE1LVWtkNlJTOHZiazA0VUc1alRHNWFaVWRWVFRaMmF6YzVZM3BKVUROa2IzUlFSMWhNUzNCYVNXRm9RVE5VYVVaVE5WTlBNVWRrVjAwdk9Ib3djMjlST0FvckwwWXlibVJzZEhSUVVrc3pSM1JsZWxOWk1uQnRhVUpsVjFvM01IVkVOMlpLTnpKUE5qUkhVSE5sVDBzMU5VOUdSSGhpZEZWNVVHOHZiblk1VjFkSUNqSjBhVlpVZGxnMlpGcEZkR2hrYWs5Qk4xYzRaRXRSYmxoT2JrWkhVMk05Q2kwdExTMHRSVTVFSUVORlVsUkpSa2xEUVZSRkxTMHRMUzBLCiAgICBjbGllbnQta2V5LWRhdGE6IExTMHRMUzFDUlVkSlRpQlNVMEVnVUZKSlZrRlVSU0JMUlZrdExTMHRMUXBOU1VsS1NuZEpRa0ZCUzBOQlowVkJNVko0YTJkWFVFRllkQ3MzZVZCMU9GZFRSbFp2V2xOM1RteElRV2xwUkdWRlQzUndZakJKZW5CelNIQndNRzAwQ25OTGRWTjFTV0VyYXpCeFNsSk9LMHBDWTNrcmRGSlZkV2xCZDBOS1EyTnFkMDR4Y1RoRFZFeEVTMlpZZVc1RGJFUk9kMHB0WldOak4xWmtMek5GZGpRS1dtUjNXWGg2VkhGNGIyZE5NMUJEVlVzclRETldZbVo1TVZGbGJWRkVTMlJYUWxkaVZpc3lTRzV1YkhodmVIWldVa0ZLYWxoSGRqbEpaRXhTTkVwdmJBcE5VR28xZWtwcmQyMHZRVTlxZUZSSFFUSnliVkpKYkZKcmMwdEpZazV4WkdoMVkyUlNNemMyWms1SVIyaEllSGRzVkV0eVRpOVlTbHA2YzNwSE56STFDakZ4TUZZMk9YbzNNSFJKYW01eVFXOWliWFJRTVRCWkwyc3hka1JxWmxkMVkwMWtRWE0yTmtVNGNtaHJTSHBaVERoWGJDdGFlbWhqTW1sV1psUXhaMGNLWkhkT1ZuTkxZM0F3VFVOalprOU9kRmhQTVVGQksxSlBTR2RUTldwTE5HTTVXWFpuUldwMFluQlRNa3hUZFU5d1RVeG9OMWREWW1KT0szazVjM0pyTWdwSFNrWmxhekk0VEd0dFJGSkhXVGwzUkhSbk0yMXhTMmR6ZGpsaWFXWXlaMFYwSzNkYVMyVnZRMk5JUVVGdmIwZHpUMlpDYkdKa1FXdFdXSEZ6VTBwcUNuZFpOSGhLYVVGMVNrZDFla2x6ZGtoV1dISTNVa2QxVVZkWlNqRk9URXBwU0ZaQlFWUmFZMFprVGtKTk5HODRXalZoYVVnNFFXY3hSM2R6TmpJMmIzY0taa3BpUVVwT2IyRk1aMnRrVlVOaFdEbE1NVEppUWxwcWJuTXJkM0prZDJGWmEyd3haRTFuVFZoYVJtbERXSFpTUkd4bGEwSTBURE16YVdoaE1GWXhTUXBoUkdFM0szRlNRMUJEWjJKRldpOW9SWEoxYVhkMU0yOXZRWGRHU0hSSU5GaEhka0ZFY1hsRlQwbFFaMncwVGk5MVRIaG9jVFJQVDJNeWJFUm1PREpvQ2pSTmJIUkJVamxXYlhwQmFrUlBUa0pKWnpWdGVDOUpSR2RCZFVWTGRYWnphV1pvTHpoSFNpdFhSWE50VlhjMlZua3lSM0owZW5ZNE1tWmpRMEYzUlVFS1FWRkxRMEZuUW1nd2NWZHFObmxyVEc4NFIzTk5WRzB3TlVOT1RtZ3hXVmh4VWtWeWJHOXBOREZaWWxKU2JEZ3hVbUlySzFKWk0yTnpkbFZ1WkU1eFdBb3JPV1pyTkVsTmFWRlhhWE00UzNOeFZXWkVUbkphVjNjeWVrcFRVakpHV2xSMFdUQkhPVGN6WkhreVJrVjBlSEJoWjJaTlJrdGtVMEZ0VkhoV1F6Uk5Dbk5pYWs0eGFsUTVja1JMUTJreWVWSk5ka041WlZWc1YyWktlVkpGVUhneU1YbEpPV292U1cxU1REVmtWakJwWW5SUk1GZ3ZNRzVvU0M5b2VWaFZUVGtLTkZWVFpHdDRhVTU1ZHpCdFIzRTFlVWx2TDBNdk9YSmtZVTF6T0ROWFIxZ3lRMk0yS3pOQmJWaEdjbEl4YVZnNFJpdFhUemN4YTFZelRpczJTUzlqU1FwUUswOVFNMU5pUTBGTmNHMWpPVEpPUVRoMFVrbzFlRXROYm5oV2RtdzVVMng1WlVkWFExaEdVeTl3TURSU09XMUdNQ3RZZFZReFIyUnBXbTlpTWswekNtRnhTM0ZvVXpkWVVVTlNjRnBQZDJsNFYwcDVNMEl4TkRkU2MwOUVUemhvWXk5clFVaDNOMDFuYTBwUlkyOXhNM2ROYWs1dll6Wk5MMm9yVkZCUGNXc0tkR0pRTUZaRVltSnRjbkZaUmpOaE5DdFRTMnRrVGtzNFptOVFhMnRLYldzMmNDdE1RM0ExUW5oa05sTXpjRkUxZW1Gc1MxTm9RMlpXYUUwMVZGVmhWZ3BrUTNWalJ6ZFdPRzFqTW5CelRFZEVhM0JvUVRGUmVXZ3piMVprY2tOeVlWaDBUamhzVWtJelVsQjFZeTlLWm5vemJtbG9SMjlDY2tONVNIRXJXbGxrQ2xJMlprTjFNVmhSWmpCWVYwMUtZVlJPTTJ0eGVUWlpPRlJuYURsRVVrNXROVEZ4VDFCMFZVNU1kWEpOUlhnNGEzb3dReXRsS3pWVGNVMTRhMWg0ZWtrS2VtSTVkekoxUjI1dmFtdzBOMFUzY0V4aU5IZDZiMmR5ZW1aNmJTOU9VbUV5VVZONWNIcHhRM2hJWXpCcWRuZDVaMGxMU0hJclNUWkhkM2Q1ZGpSc1RncEtOa2wyT0cxRUszRllUVWxZVWs5b2VVWm1Ta1JOV0RVcmVrSmpkRzlHVjBkNWMweExTSEJLTlRrMFdXMDNWVUYzVVV0RFFWRkZRVGRrU3k5UmJXUlRDbGw2U0ZscVZtRlRVbVJ6VldoTU1uUjNTMFZWVUM5TGN6Tlhjbk15VmxGelJtaDViMWx2TDNkQ2JuTkpNbFZhWTFwWGFYWTRjVWt4ZUc4MlVuQjZOVllLZFRWWFZtMWxiMUZGYmtNd1pYWlZiVk5xU2xoRlNXNU1kR2RCVWtoUWEwaEllRGd2UnpoSk1raHZOalZuT1UweVFWcElkRkIwZDBKM1pGVTFUemQxVkFvck5WWTNOME5uYW1VNU5Ia3JjRzFLUkhod1RYY3Ziak5TY2tGa1RYQlFiRTFEU0ROVllucDJjelZ1ZFVKT2JWY3ZLMkp2VFcxaFlXdHNkMVptYVd0UkNsUnZha3hpTld3cmQwWndUR2hyUkROT1ZFRllRbEZrU2l0NlpqTm9VM3BvUjI5NmVYTmxaWGx2ZGxVNGVrcFJibkozVEZOaGJGZFRTamc0ZWxWUE5YTUtiM0prV0V0b1kySjZPR1l2WlhZeWJtWmlhRzQ1VldoNlRtRkViRGt4Tm1aeFFrVTBha3RxWjNWcE9HWk5OV2xNTW5kMlZtMU5VRVpsWTBGM09FTlhlZ3BvYkhoMVlWRkVOMmhTYW5CV2QwdERRVkZGUVRWWFdXWnpkbXBZYjAxVlozRktPRGRQVEhkMFRuSnNRbmxSYkU5RGEwZ3JhVFZaY1dKSVlUVlNaMEp6Q2xCalRFOVJZMmxFVWsxM0syZG9TaXRoVWxWU1NHTnZaV3hxWmt4MmVVOUJjRFZOWkUwMmVIRnBNVlZQWjFWUmNrTXhOelZhYURoMFZsTkthV1V3TDFFS1JGcG9RVXhJVW1zM1YwdExNRm94VWtsdWRWVlhWVWRCZUhKTmVYSlRlVUpEYkUxeWRHZ3JRU3N6V2tzd05sZFZjRWxRYmtsMFNVOWxWMjUyTURCVGRBcFdPSEY1Wm5aMVdDdHFXQzlLTUdobFlrTlpkbVpZUzJwQ1dFMXRWVmR0VGsxRmIxbERhM3BXU0dWbE4xWkNUa0kwUTBaeVJVSTBjMWRHUTBReVNUYzNDbkJKV0dJNE1pOHJSRmRoVEdOd1IwUklTSEpIVHpSTWVGcE1MMGRsZFRFM00yVXdkVFYwY0djM2F6VTVVSFJGUzFoR2N6bGpRMkpTWlRkd1dXNWFlV3NLVFU1bFEzZERXR2QwUVdWbWNrbE9XV2hLVWxvMVNrSnRXQ3QwTlU4NEsxWlVaMjFCY3pKRlVWbFJTME5CVVVGbWRtUkJVRWxrUlhGTmFETjNRakI2ZHdwcEsxQTJURFJNUVZGbU9UZzFXSFpUT1ZwR01VWkVibkZJV25OUWVFcDRTbXRUVWtkTk5VUnFZa0Y1U0hWelUyOXZjVU52UkV4aVQyMWFSbXhCU1hSNUNtMXdOVTFDZFc4NFdIVnRiRFpITlVzNVZXeFVZalV4VjNSM2JuRlFOSEZRYlRSalFrcEVRME5HZVV4MVJEbHJaMDFOUVRWNU4yaGtha3c0UldKMmMwWUtZbXQxY0dwT1NuaE9TbHB2Vm1KV0swNXpWU3N5WlZKMEwzbHhUMDh3VWxjeVZWZGxWMmhvUVZGWFdtVlROMnd6Y2xacGJFSlJRVXhMTVVSVWNsVXpkd3BwUm5seFNqUlBXRVZ5YzFCNlkwRXZRMjFKTnpSU2FVOVFXaXR4ZUZCNlpuRlNSMkpWUnpkWVpXVXplR2xWWnk5cVpsVlJZVlJKVVhGYU1qRnVWek14Q2pGdlEzWjJjV3RpU3pSRFkybFVXSFpSTTBaUFEyUkJVRlV6VlZSMVVIQTVabmR4VVVSV01HeFBSR2RwT0c5WmNEQk5Mek5VVjBSbWJuWnBVbWhVVFUwS2RWb3hla0Z2U1VKQlNGQkhiWFpGUlhwcE9FSjZiak5NTHpKUFpUbFhURTVHVUhGV2FUWkdOWFJVU2xZd05IUjVWVUZsV1ZwM2FUQnlUWGhsYlc5SE5ncFpNRWhoWlRWdmJGYzRTV2hLUm1KRlUycDRjR1Z4UlZscGIwSjBORVZtTlRSYWQzb3ZNVlkxTVZneFNUbDFVV3QxYm1Sd2VrdE9VREV2VDFoMWRXaHFDamsxVUdKU1puVldVekJZVVROWFJuSlNORkpJUm1Oc1ZVOUtaSFZTZG1GcGRWSnJaMVpIYWtSbFdVRkdZUzlEZWtkeGMxSXlhbkZaZG05WFFVUlRMMjBLTWt0UVIzcHZSR1YwVGtWdVltdGtkREpyVkRCa1MwZFBaelYxVUUwelFXOWhSVGhEWlZKcVdsSlBUMXBzZEM5eWIzUTVSa3R3VUdGVGVYZE9kR0poYkFweWJrNW5TbFF2VjFwMk0yRkdSMHMwZEhvclRGcENlVnBtUkZwbGRVNVZUa2xCUkRSR04yRk9RbVpFTkZwdWJDOXFkMGhqT0V4RlJqQTJUMDlsYkROcENrTjVRbE5IWm1GMmRVdzJNbU5xVTNVM2VucFZaelZKYWtWaFZqSnpORVZEWjJkRlFVWndXWFJSYVZWbWMwVkxOV0ZUVms0cldHWlFUMmhWYlV0VlFrUUtVVkprY2tObmFISXJja2hDUnpjdmJESkhSMDlTY1RCMkwydEJjMHR5TldSUllXOXBMMHBaUnpOeVMwTTJZMmcyVm1sck5GQllOUzlhVEVWRlZsWkROQXB3VFZwVmNWbDNWMDh4Y0VGeFUza3JNMjUwYjNoSmRFUk5NWEZ2ZEc4eFVuVnVPV1kwTVROWWFVNDFXbkpEUVRWTmFYbzBiR2hwV2xoaWREQkRkalUyQ2toWE4xbG5WRWRXZDBsWVltSnFTM05MTmtadVJIQkJlbkZwYVZNd1NHZFNWMFp6UWtVd1JWSmlTekpWYTNOMmFVdEhVV3AzUlVaU1ZrMXNjR3BzUTFRS2MxUmxaalJIYkd0V2NraDFiVkkzZDNobWFUVmpORmhMVWxoR1RuZEZObkoxZUdoa1RrZHFUV3BsTW5wTE5uSnZSVlpMVTNCdWFXSjBTMHBKWVZsVmN3cEVla1ZGZUZRM2VIRTRVR3RMY2tSMmJWWlZVbGQxWjFoV2NuQTNObGRUTW5vemNHUkhibFZRU1dkVFdraHJjM3BhTVdGS01HZEpOWEJSUFQwS0xTMHRMUzFGVGtRZ1VsTkJJRkJTU1ZaQlZFVWdTMFZaTFMwdExTMEsKY29udGV4dHM6Ci0gY29udGV4dDoKICAgIGNsdXN0ZXI6IGNsdXN0ZXIKICAgIHVzZXI6IGNmZS1tYXN0ZXIKICBuYW1lOiBkZWZhdWx0Q29udGV4dApjdXJyZW50LWNvbnRleHQ6IGRlZmF1bHRDb250ZXh0Cg==","storages":"[{\"username\": \"abc\",\"password\": \"123\",\"ip\": \"8.40.111.70\",\"ipList\": \"8.40.111.70\",\"port\": 8088}]"})";

    restoreJob->targetObject.id = "statefulset ID";
    restoreJob->targetObject.name = "invmdb-1-1-m";
    restoreJob->targetObject.type = "StatefulSet";
    restoreJob->targetObject.subType = "KubernetesStatefulSet";
    restoreJob->targetObject.parentId = "namespace ID";
    restoreJob->targetObject.parentName = "ns000000000000000000001";

    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\",\"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"/home/test.sh","post_script":"","failed_script":""})";
    ApplicationResource volume01;
    volume01.id = "64cf55b1009769b4150f7d5e00000023";
    volume01.name = "gmdbredo-adaptermdb-1-1-m-0";
    volume01.extendInfo = R"({"pv":"{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-2-1-s-1\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"}"})";

    ApplicationResource volume02;
    volume02.id = "64cf55b1009769b4150f7d5e00000024";
    volume02.name = "gmdbdata-adaptermdb-1-1-m-0";
    volume02.extendInfo = R"({"pv":"{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-2-1-s-1\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"}"})";

    ApplicationResource volume03;
    volume03.id = "64cf55b1009769b4150f7d5e00000025";
    volume03.name = "gmdblredo-adaptermdb-1-1-m-0";
    volume03.extendInfo = R"({"pv":"{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-2-1-s-1\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}"})";

    restoreJob->restoreSubObjects.push_back(volume01);
    restoreJob->restoreSubObjects.push_back(volume02);
    restoreJob->restoreSubObjects.push_back(volume03);
}

/**
 * 测试用例：恢复前检查成功
 * 前置条件：恢复参数输入正确；
 * Check点：CheckBeforeRecovery调用成功。GenVolPair成功
 */
TEST_F(KubernetesProtectEngineTest, CheckBeforeRecovery_Success) {
    Stub stub;
    stub.set(ADDR(StorageClient, Create), Stub_StorageClientCreate2);
    stub.set(ADDR(StorageClient, GetLunInfoData), Stub_StorageClient_GetLunInfoData);
    stub.set(ADDR(StorageClient, GetDeviceBaseInfo), Stub_StorageClient_GetDeviceBaseInfo);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);
    VMInfo vmInfo;
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    ApplicationResource volume01 = restoreJob->restoreSubObjects.at(0);
    int ret;
    // ret = m_k8sRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    // EXPECT_EQ(ret, Module::SUCCESS);
    VolPair volumePair;
    VolInfo copyVol;
    copyVol.m_uuid = "64cf55b1009769b4150f7d5e00000023";
    VolMatchPairInfo volPairs;
    // ret = m_k8sRestoreProtectEngineHandler->GenVolPair(vmInfo, copyVol, volume01, volPairs);
    // EXPECT_EQ(ret, Module::SUCCESS);
    copyVol.m_uuid = "123";
    // ret = m_k8sRestoreProtectEngineHandler->GenVolPair(vmInfo, copyVol, volume01, volPairs);
    // EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：恢复前检查成功
 * 前置条件：恢复参数输入正确，扫描有多个存储系统
 * Check点：CheckBeforeRecovery调用成功。GenVolPair成功
 */
TEST_F(KubernetesProtectEngineTest, CheckBeforeRecovery_WithMultipleStorages_Success) {
    Stub stub;
    stub.set(ADDR(StorageClient, Create), Stub_StorageClientCreate2);
    stub.set(ADDR(StorageClient, GetLunInfoData), Stub_StorageClient_GetLunInfoData);
    stub.set(ADDR(StorageClient, GetDeviceBaseInfo), Stub_StorageClient_GetDeviceBaseInfo);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);
    VMInfo vmInfo;
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetEnv.auth.extendInfo = R"({"config":"a2luZDogQ29uZmlnCmFwaVZlcnNpb246IHYxCmNsdXN0ZXJzOgotIGNsdXN0ZXI6CiAgICBjZXJ0aWZpY2F0ZS1hdXRob3JpdHktZGF0YTogTFMwdExTMUNSVWRKVGlCRFJWSlVTVVpKUTBGVVJTMHRMUzB0Q2sxSlNVaE9la05EUWxJclowRjNTVUpCWjBsS1FVMXZhblpFUVZWYVJrWnJUVUV3UjBOVGNVZFRTV0l6UkZGRlFrTjNWVUZOU1Vjd1RWRnpkME5SV1VRS1ZsRlJSMFYzU2tSVWFrVlRUVUpCUjBFeFZVVkRRWGRLVWpOV2FHSnRaRVZpTWpWdVRWSkZkMFIzV1VSV1VWRklSRUZvVkdGSFZuVlhiV2hzWW1wRmJBcE5RMDFIUVRGVlJVTm5kMk5UU0Zab1pESldjRWxHVW14Wk1taDFZako0ZGxveWJHeGplVUpFWW5rMGMwbEZlREJhUkVWb1RVSTRSMEV4VlVWRGQzZFpDbFF4VGxSSlExbG5WVEpXZVdSdGJHcGFVMEpWWWpJNWMyTjVRa1ZhV0VJd1RWSkpkMFZCV1VSV1VWRkVSRUZzVUZVeFRYcE1ha0ZuVVRCRmVFbEVRV1VLUW1kcmNXaHJhVWM1ZHpCQ1ExRkZWMFZYT1hwamVrNXFXVlZDYjJSWFJqTmFWMnQxV1RJNWRFMUNORmhFVkVsNVRVUlpkMDFxUVhwT1JHY3hUbXh2V0FwRVZFMTVUVVJWZWsxRVFYcE9SR2N4VG14dmQyZGlVWGhEZWtGS1FtZE9Wa0pCV1ZSQmEwNVBUVkpKZDBWQldVUldVVkZKUkVGc1NHUlhSblZhTUZKMkNtSnRZM2hGVkVGUVFtZE9Wa0pCWTAxRFJrNXZXbGMxWVdGSFZuVk5VMVYzU1hkWlJGWlJVVXRFUW5oSlpGZEdNMXBYYTJkV1IxWnFZVWMxZG1KSE9XNEtZVmRXZWtsRlRuWk1hWGRuVkVoU2EwMVRSWGRJZDFsRVZsRlJURVJDYUZCVk1VMW5TbWxDVkZwWVNqSmhWMDVzU1VaU2RtSXllSHBKUlZKc1kwaFJlQXBGYWtGUlFtZE9Wa0pCVFUxRFZUbFVWWHBOZFUxRFFrUlJWRVZuVFVJMFIwTlRjVWRUU1dJelJGRkZTa0ZTV1ZKaU0wNTZUVEpPYUZGSGFERlpXR1JzQ21GVE5XcGlNakIzWjJkSmFVMUJNRWREVTNGSFUwbGlNMFJSUlVKQlVWVkJRVFJKUTBSM1FYZG5aMGxMUVc5SlEwRlJSRk5LUkhsd2QzbFlURWhDTHpVS2JYSkZhR2gwV1RWalQzbGxUbVF2VVVWNGJuWlJZV1JhUW5KSGQwRjVNVzQ1YjNvM2JGTTVWREpHUm01NmJVbFlVMDVLWlZkVVFYVk5PV1F5V0c1RGFBbzRXVE5oYVdoemIxSTBORmhWTDNWd015dE5ZWFJPVG1kMlpuTXpORGhYYm05M2RHdFRkREpHVW1FcmNuaEZWbWQxZDIwNFNubHVkV2xDWmpaMVZ6WjBDbFpuYUVSSVMzSXJSbVVyYVV0ek5uaGlOWGR4YXpGWlRuaE1VRVkxVEdVeWNWVkdlVEJVY1ZKSU9YTmFUazlwWkZOdk9XUk1aR0pNVEROa2RreHNjRzhLUTBoVFF6QmpPUzkzTmxaVU5YVlRkMVY2V1VkT2MwUnlaeXRxVWtGVFdtaE5SVnBWVGtoNVJXeGxkWGQwUzBwNVlsTjJSbmhwV0hKa2RHdDRWMVphYndwcFRrOW5WbkZOZGpCWU5HOVJha1J3YlRWdlNYQnZhR2hYV0VWUE5VVndhblpZZEdkYU5rSnRiVlZVTURFclpHMWFkMjFpVXpadmRGSTFWemhPTjBoTUNrcHhTVFpRYVRWRlIwNUlSakppTkROR1VGUnZNblo0UjBSclNFRjFORXhPU0RkS05rRm5lRVZ1TW1rMlJtOTNXRWd6ZWtwd1FqQmFaVzVaYW5WMlZUWUtOMFE1UVdjcmFFUnVjRzkxUW5CSlVXaENSR28zVVdKSk5YUXhjVzFNVDFNMU1FMTNTWEFyWmxwWGFsVmtNMmxsYml0TVNqaGlRWFpZWTNKa1NYbENWd28yTmxweVV6STVPR0paYlVkWFdUTnJhVWhuUVZsalptWnBMMkkwWXpaNk1uWnNWWG8yYmxWMmJ5OWpXbHBaYTFOR1dHOVNWWE5uUzNJNWMzcFZNblppQ2s1VmRIbFVkekZqVFRKTk1qaDRNVWQwTDNkWVpsTlZlVXNyVjJKMVYzZHpkMFJRYTBOcE9HVk1RVTloU0dZeVFUaEdNa1prYUVkVk1FaGlVV1p6TVVrS1ZrTlJLMkpWWkdWVGMxWm1PVUY1VURGU2RHSnNWMjF1YzNkTU1FYzRiemRtY3pjeWRrNXRTVVIyY2pGc1pXZHRLMHh0VVVKRVpEVkVUVTFYWlVOcE9RcFVVbGRVUXpseE1tWmpVbHAxTldWeU0wOVJSVmdyZFVKT05IZEVkRkZKUkVGUlFVSnZORWxDVTBSRFEwRlZVWGRJVVZsRVZsSXdUMEpDV1VWR1RucFZDbkp2Wm5kR1RHOTJaR0pIZFhnd1F6ZHViRTAyVjBZNFpFMUpTSEJDWjA1V1NGTk5SV2RsUlhkblpEWkJSazU2VlhKdlpuZEdURzkyWkdKSGRYZ3dRemNLYm14Tk5sZEdPR1J2V1VjMmNFbEhNMDFKUnpCTlVYTjNRMUZaUkZaUlVVZEZkMHBFVkdwRlUwMUNRVWRCTVZWRlEwRjNTbEl6Vm1oaWJXUkZZakkxYmdwTlVrVjNSSGRaUkZaUlVVaEVRV2hVWVVkV2RWZHRhR3hpYWtWc1RVTk5SMEV4VlVWRFozZGpVMGhXYUdReVZuQkpSbEpzV1RKb2RXSXllSFphTW14c0NtTjVRa1JpZVRSelNVVjRNRnBFUldoTlFqaEhRVEZWUlVOM2QxbFVNVTVVU1VOWloxVXlWbmxrYld4cVdsTkNWV0l5T1hOamVVSkZXbGhDTUUxU1NYY0tSVUZaUkZaUlVVUkVRV3hRVlRGTmVreHFRV2RSTUVWNFNVUkJaVUpuYTNGb2EybEhPWGN3UWtOUlJWZEZWemw2WTNwT2FsbFZRbTlrVjBZeldsZHJkUXBaTWpsMFoyZHJRWGxwVHpoTlFsSnJWVmRSZDBSQldVUldVakJVUWtGVmQwRjNSVUl2ZWtGTVFtZE9Wa2hST0VWQ1FVMURRVkZaZDBoQldVUldVakJTQ2tKQ1ZYZEZORVZTWWpOT2VrMHlUbWhSUjJneFdWaGtiR0ZUTldwaU1qQjNSRkZaU2t0dldrbG9kbU5PUVZGRlRFSlJRVVJuWjBsQ1FVTlVTWGxFVkdzS05YQlBRVVJrVldzemJVdGpUR1Z5V2pGR1NVMTVhMlkxUzNScFFVNTBZMHhyUjJZNVdGTjVURmw2WW5OQ2FUWklhbTVUTjJ0MFoxZFFhRWQ2WlN0b2RRcENhMGwzYkhkNlJEVmlkbU0wSzJWV1Uyd3ZUa0Y1YzBsSmFuQlNialI1Y0cxRmVYQkNRM3B6TVdwV1FuVndiak5SVERWYVoyTkZNSFJWZEZvemJVbHNDblJxTVdwSVJqQmtVSFoxY3padk0yc3pNaXN5TjIwMlRVeHFOeXRFYW1GTGRUSkJlalY1YkROdWVTdFBVVGs1VkZKemRWQk9XblYxUjNWSGJuUndORUVLUml0MGREWjBlbTB5VkRnd1ptZG1WVmxQTDNGMVNHdEJTVGRoWlRSQk5FWkxOSGhZYldKeldWRmxaR0Z2VmxoMU1VVldiRFExTkZCWVIxaG5aMjB2ZVFwT01qUlpRbXNyYzFSWWRFNUpUMjVyU21Rd1NFZE5VRzlYT1U1SlRFUk5aM2tyYTB0VmRFWkJVakV5T1Rad1lsaHJUM3A1ZVRWSk5IQjJSV3BzTlRSV0NuVkNWRzF2WW1aRlJFUlFNbk5YV2sxa1RrbFNlRlZUVVdwV2JHRm9Na3B5TVhGTFdFOWtOVlpJTTBaVU1ucDRlbXRJZGpkMGJrZHVWblpSZEc5cU1Xb0tOVzEzT1cxS0sydFRhMDFEY1VoRFZEZElXRGRuVWxkeWRERmlTekZSTkRoMlVYbHZObGx5VG01MmVrZExaMEZPZUhZNWNrZE9Na3RPWm5wclowSjNTQXA2T1RGU2QwOVZUbnBDVUhsc00yMUZRUzh5Wm5OUlVrWXlPRVZ6YURrMFprOUdTVmRYUnpOeVdHTjBRa1UyWm5oMWNUVmlhVUYwZG01SFoxVkthbWhYQ2tkcWREVTNVREpFZWtkNU5rMTBWM2RSUzJJMlRsZ3haMVZpZFZsQldYZHZOelpRU1RsaWVuWlFlVU53UTJFeWFXaEplR1YzYTNOWlNHSndVMWhDUkdVS0syZHZhR05KY21oNlYybE9hamMwUXpCNVRXMXZURzl4WkhWNFpHVk9OM2RrYWpnNVJWTlpTbXRaZG5kU2F6RkZRWHAwWVZOeFlXMUNOV2RoYzA5UFRRb3dUVVp5TTA1a2FISkpVVFpVUmxOQ1VFOU5SV2xYTUU5bE9VdFRTbEZ3TW5oWlJuRUtMUzB0TFMxRlRrUWdRMFZTVkVsR1NVTkJWRVV0TFMwdExRbz0KICAgIHNlcnZlcjogaHR0cHM6Ly84LjQwLjEzNy43OjU0NDMKICBuYW1lOiBjbHVzdGVyCnVzZXJzOgotIG5hbWU6IGNmZS1tYXN0ZXIKICB1c2VyOgogICAgY2xpZW50LWNlcnRpZmljYXRlLWRhdGE6IExTMHRMUzFDUlVkSlRpQkRSVkpVU1VaSlEwRlVSUzB0TFMwdENrMUpTVWN3ZWtORFFreDFaMEYzU1VKQlowbEpWRmRYUTBsUlpqZ3ZWa2wzUkZGWlNrdHZXa2xvZG1OT1FWRkZURUpSUVhkbllsRjRRM3BCU2tKblRsWUtRa0ZaVkVGclRrOU5Va2wzUlVGWlJGWlJVVWxFUVd4SVpGZEdkVm93VW5aaWJXTjRSVlJCVUVKblRsWkNRV05OUTBaT2IxcFhOV0ZoUjFaMVRWTlZkd3BKZDFsRVZsRlJTMFJDZUVsa1YwWXpXbGRyWjFaSFZtcGhSelYyWWtjNWJtRlhWbnBKUlU1MlRHbDNaMVJJVW10TlUwVjNTSGRaUkZaUlVVeEVRbWhRQ2xVeFRXZEthVUpVV2xoS01tRlhUbXhKUmxKMllqSjRla2xGVW14alNGRjRSV3BCVVVKblRsWkNRVTFOUTFVNVZGVjZUWFZOUTBKRVVWUkZaMDFDTkVjS1ExTnhSMU5KWWpORVVVVktRVkpaVW1JelRucE5NazVvVVVkb01WbFlaR3hoVXpWcVlqSXdkMGxDWTA1TmFrbDNUbXBCZVUxRVZYZE5hbEV5VjJoblVBcE5ha0V6VFZSQk1rMUVTWGRPVkVGNVRrUmFZVTFKUjNCTlVYTjNRMUZaUkZaUlVVZEZkMHBFVkdwRlUwMUNRVWRCTVZWRlEwSk5TbEl6Vm1oaWJXUkZDbUl5Tlc1TlVrVjNSSGRaUkZaUlVVaEZkMmhVWVVkV2RWZHRhR3hpYWtWc1RVTk5SMEV4VlVWRGFFMWpVMGhXYUdReVZuQkpSbEpzV1RKb2RXSXllSFlLV2pKc2JHTjVRa1JpZVRSelNVVjRNRnBFUldoTlFqaEhRVEZWUlVOM2QxbFVNVTVVU1VOWloxVXlWbmxrYld4cVdsTkNWV0l5T1hOamVVSkZXbGhDTUFwTlVrbDNSVUZaUkZaUlVVUkZkMnhRVlRGTmVreHFRV2RSTUVWNFJsUkJWRUpuVFhCQlVVVlVSRWhDYUZsWVRYUmlNakIwV1RJNWVWcFVRME5CYVVsM0NrUlJXVXBMYjFwSmFIWmpUa0ZSUlVKQ1VVRkVaMmRKVUVGRVEwTkJaMjlEWjJkSlFrRk9WV05hU1VacWQwWTNablU0YWpkMlJtdG9WbUZIVlhORVdsSUtkMGx2WnpOb1JISmhWemxEVFRaaVFqWmhaRXAxVEVOeWEzSnBSM1p3VGt0cFZWUm1hVkZZVFhaeVZWWk1iMmROUVdsUmJrazRSR1JoZGtGcmVYZDViZ294T0hCM2NGRjZZME5hYm01SVR6Rllaamw0VEN0SFdHTkhUV013Tm5OaFNVUk9lbmRzUTNacE9URlhNemgwVlVod2EwRjVibFpuVm0weFpuUm9OVFUxQ21OaFRXSXhWVkZEV1RGNGNpOVRTRk13WlVOaFNsUkVOQ3RqZVZwTlNuWjNSRzg0VlhoblRuRTFhMU5LVlZwTVEybEhlbUZ1V1dKdVNGVmtLeXR1ZWxJS2VHOVNPR05LVlhseGVtWXhlVmRqTjAxNGRUbDFaR0YwUm1WMll5czVURk5KTlRaM1MwYzFjbFE1WkVkUU5VNWlkelF6TVhKdVJFaFJURTkxYUZCTE5BcGFRamd5UXk5R2NHWnRZelJZVG05c1dEQTVXVUp1WTBSV1lrTnVTMlJFUVc1SWVtcGlWbnAwVVVGUWExUm9ORVYxV1hsMVNGQlhURFJDU1RkWE5sVjBDbWt3Y21weFZFTTBaVEZuYlRKNlpuTjJZa3MxVG1ocFVsaHdUblpETlVwbk1GSnRVR05CTjFsT05YRnBiMHhNTDFjMGJqbHZRa3htYzBkVGJuRkJia0lLZDBGTFMwSnlSRzUzV2xjelVVcEdWalp5UldsWk9FZFBUVk5aWjB4cFVuSnplVXhNZURGV05pc3dVbkpyUm0xRFpGUlRlVmxvTVZGQlJUSllRbGhVVVFwVVQwdFFSMlZYYjJndlFVbE9Vbk5NVDNSMWNVMUllVmQzUTFSaFIyazBTa2hXUVcxc0wxTTVaRzEzVjFrMU4xQnpTek5qUjIxS1NtUllWRWxFUmpKU0NsbG5iRGN3VVRWWWNFRmxRems1Tkc5WGRFWmtVMGRuTW5VdmNXdFJhbmR2UjNoSFpqUlNTemR2YzB4ME5rdEJUVUpTTjFJclJuaHlkMEUyYzJoRWFVUUtORXBsUkdZM2FUaFpZWFZFYW01T2NGRXpMMDV2WlVSS1lsRkZabFphYzNkSmQzcHFVVk5KVDFwelpubEJORUZNYUVOeWNqZEpialJtTDBKcFpteG9UQXBLYkUxUGJHTjBhSEUzWXpjdlRtNHpRV2ROUWtGQlIycG5aVGgzWjJWM2QwUm5XVVJXVWpCUVFWRklMMEpCVVVSQlowdHJUVUl3UjBFeFZXUktVVkZYQ2sxQ1VVZERRM05IUVZGVlJrSjNUVU5DWjJkeVFtZEZSa0pSWTBSQlZFRk5RbWRPVmtoU1RVSkJaamhGUVdwQlFVMUNPRWRCTVZWa1NYZFJXVTFDWVVFS1JrNTZWWEp2Wm5kR1RHOTJaR0pIZFhnd1F6ZHViRTAyVjBZNFpFMUpSMHhDWjA1V1NGSkZSV2RaVFhkbldVTkRRak5DYUZsWVRYUmlNakpEUVVsSllncExhVFZyV2xkYWFHUlhlREJNYms0eVdYazFhbUpJVm5wa1IxWjVURzE0ZGxreVJuTm5hRmx4VEcwNWRFeHVUakpaZVRWcVlraFdlbVJIVm5sTWJYaDJDbGt5Um5ObmFHOXhURzB4YUdKdFJtNWFVelY2WkcxTmRWa3llREZqTTFKc1kyazFjMkl5VG1oaVNXTkZjMEozUVVGWlkwVnpRbmRCUVc5alJXWjNRVUVLUVZsalJVTkRhVXBDTkdORlEwTnBTa0kwWTBWRFEybEtRbnBCVGtKbmEzRm9hMmxIT1hjd1FrRlJjMFpCUVU5RFFXZEZRVWRNVkdSUVVXbFBha1l6THdwWUt5OUZibm94T0RKVUt6bG1TMGsyYlhvclMwVjJOME42WTNKaFJ5OTVhM0I0YmxsT2JWaHhlbEYyS3preVYwOXhTemczY1VoR2VHNU1SME4yTDAxMENqSlJkRGtyVGtVeWRsbDZWRkpZWVZkdFVuaDVUVmNyVkVWTmN6bEdWVU5sYldobFJqbFpSVTF2TW1OTVJHeHZVbkZzZURseVEyVnJiRmhHTkZSNVJUQUtZVEIzVUZKTk9VVlZlVmxFVlRod01uaFZWSGRrVWt4NGJHVjBWVU13YVhWSmFYQldhVmRYT0dwM1NqRlNWQzh2TmxoRVFWVnpkVklyV1dOT1dFTnBkQW93VVZWNVRVaGxkRVk1TVRsRFlXazVNamhDUzJ4NFpHRjJZMHR5UTBwU2FXRkZUV3REYms4M1dFMXVRa1Z6WTBORE9HVk1LMFo0Um1WcmFuTnZjMDQ1Q2xJM09IbHhURXBuU1U5NFpEUTNTRWhtUVc4MlpVWk1TMGs0U1hCMVJYVXdTSFI1T0cxcVMxRjJNMHRwYXpab05YbFBVM2hrYzNwaVpERTJjamhEWkRJS1ptaE1LMDAyYUdVd2FTOHlWMFExZUZOWGQweElNMGRxY3pjeVVIcG1aMnR3U0dvdlRHaHlhbmxEV0ZKR2FHbDRaMUJMZDJ4TVp6TlRMekZzUTA1alNBcEthekZWWTBGSmRESTVhVWdyWlZSQlRXZHRUR1E0U0hwc1drWm5kemxQZUhoT2NFWm9XVzV6UTBwQ1EyOXVaVGhaVkc5SGFDdDFiMU5MVXpCWFFtZ3JDazFDUms5bmRXUXpPRGNyTmpnMVVIZ3dWa2RDVldWd2NuUmlUbWtyUkVaT09FbENibUUxV2s1b1NUSXJaMnBKVDJack9XdFdUbkEyZEdOdFNYSlVkVE1LVWtkNlJTOHZiazA0VUc1alRHNWFaVWRWVFRaMmF6YzVZM3BKVUROa2IzUlFSMWhNUzNCYVNXRm9RVE5VYVVaVE5WTlBNVWRrVjAwdk9Ib3djMjlST0FvckwwWXlibVJzZEhSUVVrc3pSM1JsZWxOWk1uQnRhVUpsVjFvM01IVkVOMlpLTnpKUE5qUkhVSE5sVDBzMU5VOUdSSGhpZEZWNVVHOHZiblk1VjFkSUNqSjBhVlpVZGxnMlpGcEZkR2hrYWs5Qk4xYzRaRXRSYmxoT2JrWkhVMk05Q2kwdExTMHRSVTVFSUVORlVsUkpSa2xEUVZSRkxTMHRMUzBLCiAgICBjbGllbnQta2V5LWRhdGE6IExTMHRMUzFDUlVkSlRpQlNVMEVnVUZKSlZrRlVSU0JMUlZrdExTMHRMUXBOU1VsS1NuZEpRa0ZCUzBOQlowVkJNVko0YTJkWFVFRllkQ3MzZVZCMU9GZFRSbFp2V2xOM1RteElRV2xwUkdWRlQzUndZakJKZW5CelNIQndNRzAwQ25OTGRWTjFTV0VyYXpCeFNsSk9LMHBDWTNrcmRGSlZkV2xCZDBOS1EyTnFkMDR4Y1RoRFZFeEVTMlpZZVc1RGJFUk9kMHB0WldOak4xWmtMek5GZGpRS1dtUjNXWGg2VkhGNGIyZE5NMUJEVlVzclRETldZbVo1TVZGbGJWRkVTMlJYUWxkaVZpc3lTRzV1YkhodmVIWldVa0ZLYWxoSGRqbEpaRXhTTkVwdmJBcE5VR28xZWtwcmQyMHZRVTlxZUZSSFFUSnliVkpKYkZKcmMwdEpZazV4WkdoMVkyUlNNemMyWms1SVIyaEllSGRzVkV0eVRpOVlTbHA2YzNwSE56STFDakZ4TUZZMk9YbzNNSFJKYW01eVFXOWliWFJRTVRCWkwyc3hka1JxWmxkMVkwMWtRWE0yTmtVNGNtaHJTSHBaVERoWGJDdGFlbWhqTW1sV1psUXhaMGNLWkhkT1ZuTkxZM0F3VFVOalprOU9kRmhQTVVGQksxSlBTR2RUTldwTE5HTTVXWFpuUldwMFluQlRNa3hUZFU5d1RVeG9OMWREWW1KT0szazVjM0pyTWdwSFNrWmxhekk0VEd0dFJGSkhXVGwzUkhSbk0yMXhTMmR6ZGpsaWFXWXlaMFYwSzNkYVMyVnZRMk5JUVVGdmIwZHpUMlpDYkdKa1FXdFdXSEZ6VTBwcUNuZFpOSGhLYVVGMVNrZDFla2x6ZGtoV1dISTNVa2QxVVZkWlNqRk9URXBwU0ZaQlFWUmFZMFprVGtKTk5HODRXalZoYVVnNFFXY3hSM2R6TmpJMmIzY0taa3BpUVVwT2IyRk1aMnRrVlVOaFdEbE1NVEppUWxwcWJuTXJkM0prZDJGWmEyd3haRTFuVFZoYVJtbERXSFpTUkd4bGEwSTBURE16YVdoaE1GWXhTUXBoUkdFM0szRlNRMUJEWjJKRldpOW9SWEoxYVhkMU0yOXZRWGRHU0hSSU5GaEhka0ZFY1hsRlQwbFFaMncwVGk5MVRIaG9jVFJQVDJNeWJFUm1PREpvQ2pSTmJIUkJVamxXYlhwQmFrUlBUa0pKWnpWdGVDOUpSR2RCZFVWTGRYWnphV1pvTHpoSFNpdFhSWE50VlhjMlZua3lSM0owZW5ZNE1tWmpRMEYzUlVFS1FWRkxRMEZuUW1nd2NWZHFObmxyVEc4NFIzTk5WRzB3TlVOT1RtZ3hXVmh4VWtWeWJHOXBOREZaWWxKU2JEZ3hVbUlySzFKWk0yTnpkbFZ1WkU1eFdBb3JPV1pyTkVsTmFWRlhhWE00UzNOeFZXWkVUbkphVjNjeWVrcFRVakpHV2xSMFdUQkhPVGN6WkhreVJrVjBlSEJoWjJaTlJrdGtVMEZ0VkhoV1F6Uk5Dbk5pYWs0eGFsUTVja1JMUTJreWVWSk5ka041WlZWc1YyWktlVkpGVUhneU1YbEpPV292U1cxU1REVmtWakJwWW5SUk1GZ3ZNRzVvU0M5b2VWaFZUVGtLTkZWVFpHdDRhVTU1ZHpCdFIzRTFlVWx2TDBNdk9YSmtZVTF6T0ROWFIxZ3lRMk0yS3pOQmJWaEdjbEl4YVZnNFJpdFhUemN4YTFZelRpczJTUzlqU1FwUUswOVFNMU5pUTBGTmNHMWpPVEpPUVRoMFVrbzFlRXROYm5oV2RtdzVVMng1WlVkWFExaEdVeTl3TURSU09XMUdNQ3RZZFZReFIyUnBXbTlpTWswekNtRnhTM0ZvVXpkWVVVTlNjRnBQZDJsNFYwcDVNMEl4TkRkU2MwOUVUemhvWXk5clFVaDNOMDFuYTBwUlkyOXhNM2ROYWs1dll6Wk5MMm9yVkZCUGNXc0tkR0pRTUZaRVltSnRjbkZaUmpOaE5DdFRTMnRrVGtzNFptOVFhMnRLYldzMmNDdE1RM0ExUW5oa05sTXpjRkUxZW1Gc1MxTm9RMlpXYUUwMVZGVmhWZ3BrUTNWalJ6ZFdPRzFqTW5CelRFZEVhM0JvUVRGUmVXZ3piMVprY2tOeVlWaDBUamhzVWtJelVsQjFZeTlLWm5vemJtbG9SMjlDY2tONVNIRXJXbGxrQ2xJMlprTjFNVmhSWmpCWVYwMUtZVlJPTTJ0eGVUWlpPRlJuYURsRVVrNXROVEZ4VDFCMFZVNU1kWEpOUlhnNGEzb3dReXRsS3pWVGNVMTRhMWg0ZWtrS2VtSTVkekoxUjI1dmFtdzBOMFUzY0V4aU5IZDZiMmR5ZW1aNmJTOU9VbUV5VVZONWNIcHhRM2hJWXpCcWRuZDVaMGxMU0hJclNUWkhkM2Q1ZGpSc1RncEtOa2wyT0cxRUszRllUVWxZVWs5b2VVWm1Ta1JOV0RVcmVrSmpkRzlHVjBkNWMweExTSEJLTlRrMFdXMDNWVUYzVVV0RFFWRkZRVGRrU3k5UmJXUlRDbGw2U0ZscVZtRlRVbVJ6VldoTU1uUjNTMFZWVUM5TGN6Tlhjbk15VmxGelJtaDViMWx2TDNkQ2JuTkpNbFZhWTFwWGFYWTRjVWt4ZUc4MlVuQjZOVllLZFRWWFZtMWxiMUZGYmtNd1pYWlZiVk5xU2xoRlNXNU1kR2RCVWtoUWEwaEllRGd2UnpoSk1raHZOalZuT1UweVFWcElkRkIwZDBKM1pGVTFUemQxVkFvck5WWTNOME5uYW1VNU5Ia3JjRzFLUkhod1RYY3Ziak5TY2tGa1RYQlFiRTFEU0ROVllucDJjelZ1ZFVKT2JWY3ZLMkp2VFcxaFlXdHNkMVptYVd0UkNsUnZha3hpTld3cmQwWndUR2hyUkROT1ZFRllRbEZrU2l0NlpqTm9VM3BvUjI5NmVYTmxaWGx2ZGxVNGVrcFJibkozVEZOaGJGZFRTamc0ZWxWUE5YTUtiM0prV0V0b1kySjZPR1l2WlhZeWJtWmlhRzQ1VldoNlRtRkViRGt4Tm1aeFFrVTBha3RxWjNWcE9HWk5OV2xNTW5kMlZtMU5VRVpsWTBGM09FTlhlZ3BvYkhoMVlWRkVOMmhTYW5CV2QwdERRVkZGUVRWWFdXWnpkbXBZYjAxVlozRktPRGRQVEhkMFRuSnNRbmxSYkU5RGEwZ3JhVFZaY1dKSVlUVlNaMEp6Q2xCalRFOVJZMmxFVWsxM0syZG9TaXRoVWxWU1NHTnZaV3hxWmt4MmVVOUJjRFZOWkUwMmVIRnBNVlZQWjFWUmNrTXhOelZhYURoMFZsTkthV1V3TDFFS1JGcG9RVXhJVW1zM1YwdExNRm94VWtsdWRWVlhWVWRCZUhKTmVYSlRlVUpEYkUxeWRHZ3JRU3N6V2tzd05sZFZjRWxRYmtsMFNVOWxWMjUyTURCVGRBcFdPSEY1Wm5aMVdDdHFXQzlLTUdobFlrTlpkbVpZUzJwQ1dFMXRWVmR0VGsxRmIxbERhM3BXU0dWbE4xWkNUa0kwUTBaeVJVSTBjMWRHUTBReVNUYzNDbkJKV0dJNE1pOHJSRmRoVEdOd1IwUklTSEpIVHpSTWVGcE1MMGRsZFRFM00yVXdkVFYwY0djM2F6VTVVSFJGUzFoR2N6bGpRMkpTWlRkd1dXNWFlV3NLVFU1bFEzZERXR2QwUVdWbWNrbE9XV2hLVWxvMVNrSnRXQ3QwTlU4NEsxWlVaMjFCY3pKRlVWbFJTME5CVVVGbWRtUkJVRWxrUlhGTmFETjNRakI2ZHdwcEsxQTJURFJNUVZGbU9UZzFXSFpUT1ZwR01VWkVibkZJV25OUWVFcDRTbXRUVWtkTk5VUnFZa0Y1U0hWelUyOXZjVU52UkV4aVQyMWFSbXhCU1hSNUNtMXdOVTFDZFc4NFdIVnRiRFpITlVzNVZXeFVZalV4VjNSM2JuRlFOSEZRYlRSalFrcEVRME5HZVV4MVJEbHJaMDFOUVRWNU4yaGtha3c0UldKMmMwWUtZbXQxY0dwT1NuaE9TbHB2Vm1KV0swNXpWU3N5WlZKMEwzbHhUMDh3VWxjeVZWZGxWMmhvUVZGWFdtVlROMnd6Y2xacGJFSlJRVXhMTVVSVWNsVXpkd3BwUm5seFNqUlBXRVZ5YzFCNlkwRXZRMjFKTnpSU2FVOVFXaXR4ZUZCNlpuRlNSMkpWUnpkWVpXVXplR2xWWnk5cVpsVlJZVlJKVVhGYU1qRnVWek14Q2pGdlEzWjJjV3RpU3pSRFkybFVXSFpSTTBaUFEyUkJVRlV6VlZSMVVIQTVabmR4VVVSV01HeFBSR2RwT0c5WmNEQk5Mek5VVjBSbWJuWnBVbWhVVFUwS2RWb3hla0Z2U1VKQlNGQkhiWFpGUlhwcE9FSjZiak5NTHpKUFpUbFhURTVHVUhGV2FUWkdOWFJVU2xZd05IUjVWVUZsV1ZwM2FUQnlUWGhsYlc5SE5ncFpNRWhoWlRWdmJGYzRTV2hLUm1KRlUycDRjR1Z4UlZscGIwSjBORVZtTlRSYWQzb3ZNVlkxTVZneFNUbDFVV3QxYm1Sd2VrdE9VREV2VDFoMWRXaHFDamsxVUdKU1puVldVekJZVVROWFJuSlNORkpJUm1Oc1ZVOUtaSFZTZG1GcGRWSnJaMVpIYWtSbFdVRkdZUzlEZWtkeGMxSXlhbkZaZG05WFFVUlRMMjBLTWt0UVIzcHZSR1YwVGtWdVltdGtkREpyVkRCa1MwZFBaelYxVUUwelFXOWhSVGhEWlZKcVdsSlBUMXBzZEM5eWIzUTVSa3R3VUdGVGVYZE9kR0poYkFweWJrNW5TbFF2VjFwMk0yRkdSMHMwZEhvclRGcENlVnBtUkZwbGRVNVZUa2xCUkRSR04yRk9RbVpFTkZwdWJDOXFkMGhqT0V4RlJqQTJUMDlsYkROcENrTjVRbE5IWm1GMmRVdzJNbU5xVTNVM2VucFZaelZKYWtWaFZqSnpORVZEWjJkRlFVWndXWFJSYVZWbWMwVkxOV0ZUVms0cldHWlFUMmhWYlV0VlFrUUtVVkprY2tObmFISXJja2hDUnpjdmJESkhSMDlTY1RCMkwydEJjMHR5TldSUllXOXBMMHBaUnpOeVMwTTJZMmcyVm1sck5GQllOUzlhVEVWRlZsWkROQXB3VFZwVmNWbDNWMDh4Y0VGeFUza3JNMjUwYjNoSmRFUk5NWEZ2ZEc4eFVuVnVPV1kwTVROWWFVNDFXbkpEUVRWTmFYbzBiR2hwV2xoaWREQkRkalUyQ2toWE4xbG5WRWRXZDBsWVltSnFTM05MTmtadVJIQkJlbkZwYVZNd1NHZFNWMFp6UWtVd1JWSmlTekpWYTNOMmFVdEhVV3AzUlVaU1ZrMXNjR3BzUTFRS2MxUmxaalJIYkd0V2NraDFiVkkzZDNobWFUVmpORmhMVWxoR1RuZEZObkoxZUdoa1RrZHFUV3BsTW5wTE5uSnZSVlpMVTNCdWFXSjBTMHBKWVZsVmN3cEVla1ZGZUZRM2VIRTRVR3RMY2tSMmJWWlZVbGQxWjFoV2NuQTNObGRUTW5vemNHUkhibFZRU1dkVFdraHJjM3BhTVdGS01HZEpOWEJSUFQwS0xTMHRMUzFGVGtRZ1VsTkJJRkJTU1ZaQlZFVWdTMFZaTFMwdExTMEsKY29udGV4dHM6Ci0gY29udGV4dDoKICAgIGNsdXN0ZXI6IGNsdXN0ZXIKICAgIHVzZXI6IGNmZS1tYXN0ZXIKICBuYW1lOiBkZWZhdWx0Q29udGV4dApjdXJyZW50LWNvbnRleHQ6IGRlZmF1bHRDb250ZXh0Cg==","storages":"[{\"username\": \"abc\",\"password\": \"123\",\"ip\": \"8.40.111.70\",\"port\": 8088}, {\"username\": \"abc\",\"password\": \"123\",\"ip\": \"127.0.0.1\",\"port\": 8088}]"})";
    ApplicationResource volume01 = restoreJob->restoreSubObjects.at(0);
    volume01.extendInfo = R"({"pv":"{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-2-1-s-1\",\"size\":\"85Gi\",\"storageUrl\":\"https://127.0.0.1:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"}"})";
    restoreJob->restoreSubObjects.at(0) = volume01;

    int ret;
    // ret = m_k8sRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    // EXPECT_EQ(ret, Module::SUCCESS);
    VolPair volumePair;
    VolInfo copyVol;
    copyVol.m_uuid = "64cf55b1009769b4150f7d5e00000023";
    VolMatchPairInfo volPairs;
    // ret = m_k8sRestoreProtectEngineHandler->GenVolPair(vmInfo, copyVol, volume01, volPairs);
    // EXPECT_EQ(ret, Module::SUCCESS);
    copyVol.m_uuid = "123";
    // ret = m_k8sRestoreProtectEngineHandler->GenVolPair(vmInfo, copyVol, volume01, volPairs);
    // EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：恢复前检查失败
 * 前置条件：存储无法创建；
 * Check点：CheckBeforeRecovery调用失败。
 */
TEST_F(KubernetesProtectEngineTest, CheckBeforeRecovery_Failed_when_storage_cannot_connect) {
    Stub stub;
    stub.set(ADDR(StorageClient, Create), Stub_StorageClientCreate2);
    stub.set(ADDR(StorageClient, GetLunInfoData), Stub_StorageClient_GetLunInfoData);
    stub.set(ADDR(StorageClient, GetDeviceBaseInfo), Stub_StorageClient_GetDeviceBaseInfo);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);
    VMInfo vmInfo;
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetEnv.auth.extendInfo = R"({"config":"a2luZDogQ29uZmlnCmFwaVZlcnNpb246IHYxCmNsdXN0ZXJzOgotIGNsdXN0ZXI6CiAgICBjZXJ0aWZpY2F0ZS1hdXRob3JpdHktZGF0YTogTFMwdExTMUNSVWRKVGlCRFJWSlVTVVpKUTBGVVJTMHRMUzB0Q2sxSlNVaE9la05EUWxJclowRjNTVUpCWjBsS1FVMXZhblpFUVZWYVJrWnJUVUV3UjBOVGNVZFRTV0l6UkZGRlFrTjNWVUZOU1Vjd1RWRnpkME5SV1VRS1ZsRlJSMFYzU2tSVWFrVlRUVUpCUjBFeFZVVkRRWGRLVWpOV2FHSnRaRVZpTWpWdVRWSkZkMFIzV1VSV1VWRklSRUZvVkdGSFZuVlhiV2hzWW1wRmJBcE5RMDFIUVRGVlJVTm5kMk5UU0Zab1pESldjRWxHVW14Wk1taDFZako0ZGxveWJHeGplVUpFWW5rMGMwbEZlREJhUkVWb1RVSTRSMEV4VlVWRGQzZFpDbFF4VGxSSlExbG5WVEpXZVdSdGJHcGFVMEpWWWpJNWMyTjVRa1ZhV0VJd1RWSkpkMFZCV1VSV1VWRkVSRUZzVUZVeFRYcE1ha0ZuVVRCRmVFbEVRV1VLUW1kcmNXaHJhVWM1ZHpCQ1ExRkZWMFZYT1hwamVrNXFXVlZDYjJSWFJqTmFWMnQxV1RJNWRFMUNORmhFVkVsNVRVUlpkMDFxUVhwT1JHY3hUbXh2V0FwRVZFMTVUVVJWZWsxRVFYcE9SR2N4VG14dmQyZGlVWGhEZWtGS1FtZE9Wa0pCV1ZSQmEwNVBUVkpKZDBWQldVUldVVkZKUkVGc1NHUlhSblZhTUZKMkNtSnRZM2hGVkVGUVFtZE9Wa0pCWTAxRFJrNXZXbGMxWVdGSFZuVk5VMVYzU1hkWlJGWlJVVXRFUW5oSlpGZEdNMXBYYTJkV1IxWnFZVWMxZG1KSE9XNEtZVmRXZWtsRlRuWk1hWGRuVkVoU2EwMVRSWGRJZDFsRVZsRlJURVJDYUZCVk1VMW5TbWxDVkZwWVNqSmhWMDVzU1VaU2RtSXllSHBKUlZKc1kwaFJlQXBGYWtGUlFtZE9Wa0pCVFUxRFZUbFVWWHBOZFUxRFFrUlJWRVZuVFVJMFIwTlRjVWRUU1dJelJGRkZTa0ZTV1ZKaU0wNTZUVEpPYUZGSGFERlpXR1JzQ21GVE5XcGlNakIzWjJkSmFVMUJNRWREVTNGSFUwbGlNMFJSUlVKQlVWVkJRVFJKUTBSM1FYZG5aMGxMUVc5SlEwRlJSRk5LUkhsd2QzbFlURWhDTHpVS2JYSkZhR2gwV1RWalQzbGxUbVF2VVVWNGJuWlJZV1JhUW5KSGQwRjVNVzQ1YjNvM2JGTTVWREpHUm01NmJVbFlVMDVLWlZkVVFYVk5PV1F5V0c1RGFBbzRXVE5oYVdoemIxSTBORmhWTDNWd015dE5ZWFJPVG1kMlpuTXpORGhYYm05M2RHdFRkREpHVW1FcmNuaEZWbWQxZDIwNFNubHVkV2xDWmpaMVZ6WjBDbFpuYUVSSVMzSXJSbVVyYVV0ek5uaGlOWGR4YXpGWlRuaE1VRVkxVEdVeWNWVkdlVEJVY1ZKSU9YTmFUazlwWkZOdk9XUk1aR0pNVEROa2RreHNjRzhLUTBoVFF6QmpPUzkzTmxaVU5YVlRkMVY2V1VkT2MwUnlaeXRxVWtGVFdtaE5SVnBWVGtoNVJXeGxkWGQwUzBwNVlsTjJSbmhwV0hKa2RHdDRWMVphYndwcFRrOW5WbkZOZGpCWU5HOVJha1J3YlRWdlNYQnZhR2hYV0VWUE5VVndhblpZZEdkYU5rSnRiVlZVTURFclpHMWFkMjFpVXpadmRGSTFWemhPTjBoTUNrcHhTVFpRYVRWRlIwNUlSakppTkROR1VGUnZNblo0UjBSclNFRjFORXhPU0RkS05rRm5lRVZ1TW1rMlJtOTNXRWd6ZWtwd1FqQmFaVzVaYW5WMlZUWUtOMFE1UVdjcmFFUnVjRzkxUW5CSlVXaENSR28zVVdKSk5YUXhjVzFNVDFNMU1FMTNTWEFyWmxwWGFsVmtNMmxsYml0TVNqaGlRWFpZWTNKa1NYbENWd28yTmxweVV6STVPR0paYlVkWFdUTnJhVWhuUVZsalptWnBMMkkwWXpaNk1uWnNWWG8yYmxWMmJ5OWpXbHBaYTFOR1dHOVNWWE5uUzNJNWMzcFZNblppQ2s1VmRIbFVkekZqVFRKTk1qaDRNVWQwTDNkWVpsTlZlVXNyVjJKMVYzZHpkMFJRYTBOcE9HVk1RVTloU0dZeVFUaEdNa1prYUVkVk1FaGlVV1p6TVVrS1ZrTlJLMkpWWkdWVGMxWm1PVUY1VURGU2RHSnNWMjF1YzNkTU1FYzRiemRtY3pjeWRrNXRTVVIyY2pGc1pXZHRLMHh0VVVKRVpEVkVUVTFYWlVOcE9RcFVVbGRVUXpseE1tWmpVbHAxTldWeU0wOVJSVmdyZFVKT05IZEVkRkZKUkVGUlFVSnZORWxDVTBSRFEwRlZVWGRJVVZsRVZsSXdUMEpDV1VWR1RucFZDbkp2Wm5kR1RHOTJaR0pIZFhnd1F6ZHViRTAyVjBZNFpFMUpTSEJDWjA1V1NGTk5SV2RsUlhkblpEWkJSazU2VlhKdlpuZEdURzkyWkdKSGRYZ3dRemNLYm14Tk5sZEdPR1J2V1VjMmNFbEhNMDFKUnpCTlVYTjNRMUZaUkZaUlVVZEZkMHBFVkdwRlUwMUNRVWRCTVZWRlEwRjNTbEl6Vm1oaWJXUkZZakkxYmdwTlVrVjNSSGRaUkZaUlVVaEVRV2hVWVVkV2RWZHRhR3hpYWtWc1RVTk5SMEV4VlVWRFozZGpVMGhXYUdReVZuQkpSbEpzV1RKb2RXSXllSFphTW14c0NtTjVRa1JpZVRSelNVVjRNRnBFUldoTlFqaEhRVEZWUlVOM2QxbFVNVTVVU1VOWloxVXlWbmxrYld4cVdsTkNWV0l5T1hOamVVSkZXbGhDTUUxU1NYY0tSVUZaUkZaUlVVUkVRV3hRVlRGTmVreHFRV2RSTUVWNFNVUkJaVUpuYTNGb2EybEhPWGN3UWtOUlJWZEZWemw2WTNwT2FsbFZRbTlrVjBZeldsZHJkUXBaTWpsMFoyZHJRWGxwVHpoTlFsSnJWVmRSZDBSQldVUldVakJVUWtGVmQwRjNSVUl2ZWtGTVFtZE9Wa2hST0VWQ1FVMURRVkZaZDBoQldVUldVakJTQ2tKQ1ZYZEZORVZTWWpOT2VrMHlUbWhSUjJneFdWaGtiR0ZUTldwaU1qQjNSRkZaU2t0dldrbG9kbU5PUVZGRlRFSlJRVVJuWjBsQ1FVTlVTWGxFVkdzS05YQlBRVVJrVldzemJVdGpUR1Z5V2pGR1NVMTVhMlkxUzNScFFVNTBZMHhyUjJZNVdGTjVURmw2WW5OQ2FUWklhbTVUTjJ0MFoxZFFhRWQ2WlN0b2RRcENhMGwzYkhkNlJEVmlkbU0wSzJWV1Uyd3ZUa0Y1YzBsSmFuQlNialI1Y0cxRmVYQkNRM3B6TVdwV1FuVndiak5SVERWYVoyTkZNSFJWZEZvemJVbHNDblJxTVdwSVJqQmtVSFoxY3padk0yc3pNaXN5TjIwMlRVeHFOeXRFYW1GTGRUSkJlalY1YkROdWVTdFBVVGs1VkZKemRWQk9XblYxUjNWSGJuUndORUVLUml0MGREWjBlbTB5VkRnd1ptZG1WVmxQTDNGMVNHdEJTVGRoWlRSQk5FWkxOSGhZYldKeldWRmxaR0Z2VmxoMU1VVldiRFExTkZCWVIxaG5aMjB2ZVFwT01qUlpRbXNyYzFSWWRFNUpUMjVyU21Rd1NFZE5VRzlYT1U1SlRFUk5aM2tyYTB0VmRFWkJVakV5T1Rad1lsaHJUM3A1ZVRWSk5IQjJSV3BzTlRSV0NuVkNWRzF2WW1aRlJFUlFNbk5YV2sxa1RrbFNlRlZUVVdwV2JHRm9Na3B5TVhGTFdFOWtOVlpJTTBaVU1ucDRlbXRJZGpkMGJrZHVWblpSZEc5cU1Xb0tOVzEzT1cxS0sydFRhMDFEY1VoRFZEZElXRGRuVWxkeWRERmlTekZSTkRoMlVYbHZObGx5VG01MmVrZExaMEZPZUhZNWNrZE9Na3RPWm5wclowSjNTQXA2T1RGU2QwOVZUbnBDVUhsc00yMUZRUzh5Wm5OUlVrWXlPRVZ6YURrMFprOUdTVmRYUnpOeVdHTjBRa1UyWm5oMWNUVmlhVUYwZG01SFoxVkthbWhYQ2tkcWREVTNVREpFZWtkNU5rMTBWM2RSUzJJMlRsZ3haMVZpZFZsQldYZHZOelpRU1RsaWVuWlFlVU53UTJFeWFXaEplR1YzYTNOWlNHSndVMWhDUkdVS0syZHZhR05KY21oNlYybE9hamMwUXpCNVRXMXZURzl4WkhWNFpHVk9OM2RrYWpnNVJWTlpTbXRaZG5kU2F6RkZRWHAwWVZOeFlXMUNOV2RoYzA5UFRRb3dUVVp5TTA1a2FISkpVVFpVUmxOQ1VFOU5SV2xYTUU5bE9VdFRTbEZ3TW5oWlJuRUtMUzB0TFMxRlRrUWdRMFZTVkVsR1NVTkJWRVV0TFMwdExRbz0KICAgIHNlcnZlcjogaHR0cHM6Ly84LjQwLjEzNy43OjU0NDMKICBuYW1lOiBjbHVzdGVyCnVzZXJzOgotIG5hbWU6IGNmZS1tYXN0ZXIKICB1c2VyOgogICAgY2xpZW50LWNlcnRpZmljYXRlLWRhdGE6IExTMHRMUzFDUlVkSlRpQkRSVkpVU1VaSlEwRlVSUzB0TFMwdENrMUpTVWN3ZWtORFFreDFaMEYzU1VKQlowbEpWRmRYUTBsUlpqZ3ZWa2wzUkZGWlNrdHZXa2xvZG1OT1FWRkZURUpSUVhkbllsRjRRM3BCU2tKblRsWUtRa0ZaVkVGclRrOU5Va2wzUlVGWlJGWlJVVWxFUVd4SVpGZEdkVm93VW5aaWJXTjRSVlJCVUVKblRsWkNRV05OUTBaT2IxcFhOV0ZoUjFaMVRWTlZkd3BKZDFsRVZsRlJTMFJDZUVsa1YwWXpXbGRyWjFaSFZtcGhSelYyWWtjNWJtRlhWbnBKUlU1MlRHbDNaMVJJVW10TlUwVjNTSGRaUkZaUlVVeEVRbWhRQ2xVeFRXZEthVUpVV2xoS01tRlhUbXhKUmxKMllqSjRla2xGVW14alNGRjRSV3BCVVVKblRsWkNRVTFOUTFVNVZGVjZUWFZOUTBKRVVWUkZaMDFDTkVjS1ExTnhSMU5KWWpORVVVVktRVkpaVW1JelRucE5NazVvVVVkb01WbFlaR3hoVXpWcVlqSXdkMGxDWTA1TmFrbDNUbXBCZVUxRVZYZE5hbEV5VjJoblVBcE5ha0V6VFZSQk1rMUVTWGRPVkVGNVRrUmFZVTFKUjNCTlVYTjNRMUZaUkZaUlVVZEZkMHBFVkdwRlUwMUNRVWRCTVZWRlEwSk5TbEl6Vm1oaWJXUkZDbUl5Tlc1TlVrVjNSSGRaUkZaUlVVaEZkMmhVWVVkV2RWZHRhR3hpYWtWc1RVTk5SMEV4VlVWRGFFMWpVMGhXYUdReVZuQkpSbEpzV1RKb2RXSXllSFlLV2pKc2JHTjVRa1JpZVRSelNVVjRNRnBFUldoTlFqaEhRVEZWUlVOM2QxbFVNVTVVU1VOWloxVXlWbmxrYld4cVdsTkNWV0l5T1hOamVVSkZXbGhDTUFwTlVrbDNSVUZaUkZaUlVVUkZkMnhRVlRGTmVreHFRV2RSTUVWNFJsUkJWRUpuVFhCQlVVVlVSRWhDYUZsWVRYUmlNakIwV1RJNWVWcFVRME5CYVVsM0NrUlJXVXBMYjFwSmFIWmpUa0ZSUlVKQ1VVRkVaMmRKVUVGRVEwTkJaMjlEWjJkSlFrRk9WV05hU1VacWQwWTNablU0YWpkMlJtdG9WbUZIVlhORVdsSUtkMGx2WnpOb1JISmhWemxEVFRaaVFqWmhaRXAxVEVOeWEzSnBSM1p3VGt0cFZWUm1hVkZZVFhaeVZWWk1iMmROUVdsUmJrazRSR1JoZGtGcmVYZDViZ294T0hCM2NGRjZZME5hYm01SVR6Rllaamw0VEN0SFdHTkhUV013Tm5OaFNVUk9lbmRzUTNacE9URlhNemgwVlVod2EwRjVibFpuVm0weFpuUm9OVFUxQ21OaFRXSXhWVkZEV1RGNGNpOVRTRk13WlVOaFNsUkVOQ3RqZVZwTlNuWjNSRzg0VlhoblRuRTFhMU5LVlZwTVEybEhlbUZ1V1dKdVNGVmtLeXR1ZWxJS2VHOVNPR05LVlhseGVtWXhlVmRqTjAxNGRUbDFaR0YwUm1WMll5czVURk5KTlRaM1MwYzFjbFE1WkVkUU5VNWlkelF6TVhKdVJFaFJURTkxYUZCTE5BcGFRamd5UXk5R2NHWnRZelJZVG05c1dEQTVXVUp1WTBSV1lrTnVTMlJFUVc1SWVtcGlWbnAwVVVGUWExUm9ORVYxV1hsMVNGQlhURFJDU1RkWE5sVjBDbWt3Y21weFZFTTBaVEZuYlRKNlpuTjJZa3MxVG1ocFVsaHdUblpETlVwbk1GSnRVR05CTjFsT05YRnBiMHhNTDFjMGJqbHZRa3htYzBkVGJuRkJia0lLZDBGTFMwSnlSRzUzV2xjelVVcEdWalp5UldsWk9FZFBUVk5aWjB4cFVuSnplVXhNZURGV05pc3dVbkpyUm0xRFpGUlRlVmxvTVZGQlJUSllRbGhVVVFwVVQwdFFSMlZYYjJndlFVbE9Vbk5NVDNSMWNVMUllVmQzUTFSaFIyazBTa2hXUVcxc0wxTTVaRzEzVjFrMU4xQnpTek5qUjIxS1NtUllWRWxFUmpKU0NsbG5iRGN3VVRWWWNFRmxRems1Tkc5WGRFWmtVMGRuTW5VdmNXdFJhbmR2UjNoSFpqUlNTemR2YzB4ME5rdEJUVUpTTjFJclJuaHlkMEUyYzJoRWFVUUtORXBsUkdZM2FUaFpZWFZFYW01T2NGRXpMMDV2WlVSS1lsRkZabFphYzNkSmQzcHFVVk5KVDFwelpubEJORUZNYUVOeWNqZEpialJtTDBKcFpteG9UQXBLYkUxUGJHTjBhSEUzWXpjdlRtNHpRV2ROUWtGQlIycG5aVGgzWjJWM2QwUm5XVVJXVWpCUVFWRklMMEpCVVVSQlowdHJUVUl3UjBFeFZXUktVVkZYQ2sxQ1VVZERRM05IUVZGVlJrSjNUVU5DWjJkeVFtZEZSa0pSWTBSQlZFRk5RbWRPVmtoU1RVSkJaamhGUVdwQlFVMUNPRWRCTVZWa1NYZFJXVTFDWVVFS1JrNTZWWEp2Wm5kR1RHOTJaR0pIZFhnd1F6ZHViRTAyVjBZNFpFMUpSMHhDWjA1V1NGSkZSV2RaVFhkbldVTkRRak5DYUZsWVRYUmlNakpEUVVsSllncExhVFZyV2xkYWFHUlhlREJNYms0eVdYazFhbUpJVm5wa1IxWjVURzE0ZGxreVJuTm5hRmx4VEcwNWRFeHVUakpaZVRWcVlraFdlbVJIVm5sTWJYaDJDbGt5Um5ObmFHOXhURzB4YUdKdFJtNWFVelY2WkcxTmRWa3llREZqTTFKc1kyazFjMkl5VG1oaVNXTkZjMEozUVVGWlkwVnpRbmRCUVc5alJXWjNRVUVLUVZsalJVTkRhVXBDTkdORlEwTnBTa0kwWTBWRFEybEtRbnBCVGtKbmEzRm9hMmxIT1hjd1FrRlJjMFpCUVU5RFFXZEZRVWRNVkdSUVVXbFBha1l6THdwWUt5OUZibm94T0RKVUt6bG1TMGsyYlhvclMwVjJOME42WTNKaFJ5OTVhM0I0YmxsT2JWaHhlbEYyS3preVYwOXhTemczY1VoR2VHNU1SME4yTDAxMENqSlJkRGtyVGtVeWRsbDZWRkpZWVZkdFVuaDVUVmNyVkVWTmN6bEdWVU5sYldobFJqbFpSVTF2TW1OTVJHeHZVbkZzZURseVEyVnJiRmhHTkZSNVJUQUtZVEIzVUZKTk9VVlZlVmxFVlRod01uaFZWSGRrVWt4NGJHVjBWVU13YVhWSmFYQldhVmRYT0dwM1NqRlNWQzh2TmxoRVFWVnpkVklyV1dOT1dFTnBkQW93VVZWNVRVaGxkRVk1TVRsRFlXazVNamhDUzJ4NFpHRjJZMHR5UTBwU2FXRkZUV3REYms4M1dFMXVRa1Z6WTBORE9HVk1LMFo0Um1WcmFuTnZjMDQ1Q2xJM09IbHhURXBuU1U5NFpEUTNTRWhtUVc4MlpVWk1TMGs0U1hCMVJYVXdTSFI1T0cxcVMxRjJNMHRwYXpab05YbFBVM2hrYzNwaVpERTJjamhEWkRJS1ptaE1LMDAyYUdVd2FTOHlWMFExZUZOWGQweElNMGRxY3pjeVVIcG1aMnR3U0dvdlRHaHlhbmxEV0ZKR2FHbDRaMUJMZDJ4TVp6TlRMekZzUTA1alNBcEthekZWWTBGSmRESTVhVWdyWlZSQlRXZHRUR1E0U0hwc1drWm5kemxQZUhoT2NFWm9XVzV6UTBwQ1EyOXVaVGhaVkc5SGFDdDFiMU5MVXpCWFFtZ3JDazFDUms5bmRXUXpPRGNyTmpnMVVIZ3dWa2RDVldWd2NuUmlUbWtyUkVaT09FbENibUUxV2s1b1NUSXJaMnBKVDJack9XdFdUbkEyZEdOdFNYSlVkVE1LVWtkNlJTOHZiazA0VUc1alRHNWFaVWRWVFRaMmF6YzVZM3BKVUROa2IzUlFSMWhNUzNCYVNXRm9RVE5VYVVaVE5WTlBNVWRrVjAwdk9Ib3djMjlST0FvckwwWXlibVJzZEhSUVVrc3pSM1JsZWxOWk1uQnRhVUpsVjFvM01IVkVOMlpLTnpKUE5qUkhVSE5sVDBzMU5VOUdSSGhpZEZWNVVHOHZiblk1VjFkSUNqSjBhVlpVZGxnMlpGcEZkR2hrYWs5Qk4xYzRaRXRSYmxoT2JrWkhVMk05Q2kwdExTMHRSVTVFSUVORlVsUkpSa2xEUVZSRkxTMHRMUzBLCiAgICBjbGllbnQta2V5LWRhdGE6IExTMHRMUzFDUlVkSlRpQlNVMEVnVUZKSlZrRlVSU0JMUlZrdExTMHRMUXBOU1VsS1NuZEpRa0ZCUzBOQlowVkJNVko0YTJkWFVFRllkQ3MzZVZCMU9GZFRSbFp2V2xOM1RteElRV2xwUkdWRlQzUndZakJKZW5CelNIQndNRzAwQ25OTGRWTjFTV0VyYXpCeFNsSk9LMHBDWTNrcmRGSlZkV2xCZDBOS1EyTnFkMDR4Y1RoRFZFeEVTMlpZZVc1RGJFUk9kMHB0WldOak4xWmtMek5GZGpRS1dtUjNXWGg2VkhGNGIyZE5NMUJEVlVzclRETldZbVo1TVZGbGJWRkVTMlJYUWxkaVZpc3lTRzV1YkhodmVIWldVa0ZLYWxoSGRqbEpaRXhTTkVwdmJBcE5VR28xZWtwcmQyMHZRVTlxZUZSSFFUSnliVkpKYkZKcmMwdEpZazV4WkdoMVkyUlNNemMyWms1SVIyaEllSGRzVkV0eVRpOVlTbHA2YzNwSE56STFDakZ4TUZZMk9YbzNNSFJKYW01eVFXOWliWFJRTVRCWkwyc3hka1JxWmxkMVkwMWtRWE0yTmtVNGNtaHJTSHBaVERoWGJDdGFlbWhqTW1sV1psUXhaMGNLWkhkT1ZuTkxZM0F3VFVOalprOU9kRmhQTVVGQksxSlBTR2RUTldwTE5HTTVXWFpuUldwMFluQlRNa3hUZFU5d1RVeG9OMWREWW1KT0szazVjM0pyTWdwSFNrWmxhekk0VEd0dFJGSkhXVGwzUkhSbk0yMXhTMmR6ZGpsaWFXWXlaMFYwSzNkYVMyVnZRMk5JUVVGdmIwZHpUMlpDYkdKa1FXdFdXSEZ6VTBwcUNuZFpOSGhLYVVGMVNrZDFla2x6ZGtoV1dISTNVa2QxVVZkWlNqRk9URXBwU0ZaQlFWUmFZMFprVGtKTk5HODRXalZoYVVnNFFXY3hSM2R6TmpJMmIzY0taa3BpUVVwT2IyRk1aMnRrVlVOaFdEbE1NVEppUWxwcWJuTXJkM0prZDJGWmEyd3haRTFuVFZoYVJtbERXSFpTUkd4bGEwSTBURE16YVdoaE1GWXhTUXBoUkdFM0szRlNRMUJEWjJKRldpOW9SWEoxYVhkMU0yOXZRWGRHU0hSSU5GaEhka0ZFY1hsRlQwbFFaMncwVGk5MVRIaG9jVFJQVDJNeWJFUm1PREpvQ2pSTmJIUkJVamxXYlhwQmFrUlBUa0pKWnpWdGVDOUpSR2RCZFVWTGRYWnphV1pvTHpoSFNpdFhSWE50VlhjMlZua3lSM0owZW5ZNE1tWmpRMEYzUlVFS1FWRkxRMEZuUW1nd2NWZHFObmxyVEc4NFIzTk5WRzB3TlVOT1RtZ3hXVmh4VWtWeWJHOXBOREZaWWxKU2JEZ3hVbUlySzFKWk0yTnpkbFZ1WkU1eFdBb3JPV1pyTkVsTmFWRlhhWE00UzNOeFZXWkVUbkphVjNjeWVrcFRVakpHV2xSMFdUQkhPVGN6WkhreVJrVjBlSEJoWjJaTlJrdGtVMEZ0VkhoV1F6Uk5Dbk5pYWs0eGFsUTVja1JMUTJreWVWSk5ka041WlZWc1YyWktlVkpGVUhneU1YbEpPV292U1cxU1REVmtWakJwWW5SUk1GZ3ZNRzVvU0M5b2VWaFZUVGtLTkZWVFpHdDRhVTU1ZHpCdFIzRTFlVWx2TDBNdk9YSmtZVTF6T0ROWFIxZ3lRMk0yS3pOQmJWaEdjbEl4YVZnNFJpdFhUemN4YTFZelRpczJTUzlqU1FwUUswOVFNMU5pUTBGTmNHMWpPVEpPUVRoMFVrbzFlRXROYm5oV2RtdzVVMng1WlVkWFExaEdVeTl3TURSU09XMUdNQ3RZZFZReFIyUnBXbTlpTWswekNtRnhTM0ZvVXpkWVVVTlNjRnBQZDJsNFYwcDVNMEl4TkRkU2MwOUVUemhvWXk5clFVaDNOMDFuYTBwUlkyOXhNM2ROYWs1dll6Wk5MMm9yVkZCUGNXc0tkR0pRTUZaRVltSnRjbkZaUmpOaE5DdFRTMnRrVGtzNFptOVFhMnRLYldzMmNDdE1RM0ExUW5oa05sTXpjRkUxZW1Gc1MxTm9RMlpXYUUwMVZGVmhWZ3BrUTNWalJ6ZFdPRzFqTW5CelRFZEVhM0JvUVRGUmVXZ3piMVprY2tOeVlWaDBUamhzVWtJelVsQjFZeTlLWm5vemJtbG9SMjlDY2tONVNIRXJXbGxrQ2xJMlprTjFNVmhSWmpCWVYwMUtZVlJPTTJ0eGVUWlpPRlJuYURsRVVrNXROVEZ4VDFCMFZVNU1kWEpOUlhnNGEzb3dReXRsS3pWVGNVMTRhMWg0ZWtrS2VtSTVkekoxUjI1dmFtdzBOMFUzY0V4aU5IZDZiMmR5ZW1aNmJTOU9VbUV5VVZONWNIcHhRM2hJWXpCcWRuZDVaMGxMU0hJclNUWkhkM2Q1ZGpSc1RncEtOa2wyT0cxRUszRllUVWxZVWs5b2VVWm1Ta1JOV0RVcmVrSmpkRzlHVjBkNWMweExTSEJLTlRrMFdXMDNWVUYzVVV0RFFWRkZRVGRrU3k5UmJXUlRDbGw2U0ZscVZtRlRVbVJ6VldoTU1uUjNTMFZWVUM5TGN6Tlhjbk15VmxGelJtaDViMWx2TDNkQ2JuTkpNbFZhWTFwWGFYWTRjVWt4ZUc4MlVuQjZOVllLZFRWWFZtMWxiMUZGYmtNd1pYWlZiVk5xU2xoRlNXNU1kR2RCVWtoUWEwaEllRGd2UnpoSk1raHZOalZuT1UweVFWcElkRkIwZDBKM1pGVTFUemQxVkFvck5WWTNOME5uYW1VNU5Ia3JjRzFLUkhod1RYY3Ziak5TY2tGa1RYQlFiRTFEU0ROVllucDJjelZ1ZFVKT2JWY3ZLMkp2VFcxaFlXdHNkMVptYVd0UkNsUnZha3hpTld3cmQwWndUR2hyUkROT1ZFRllRbEZrU2l0NlpqTm9VM3BvUjI5NmVYTmxaWGx2ZGxVNGVrcFJibkozVEZOaGJGZFRTamc0ZWxWUE5YTUtiM0prV0V0b1kySjZPR1l2WlhZeWJtWmlhRzQ1VldoNlRtRkViRGt4Tm1aeFFrVTBha3RxWjNWcE9HWk5OV2xNTW5kMlZtMU5VRVpsWTBGM09FTlhlZ3BvYkhoMVlWRkVOMmhTYW5CV2QwdERRVkZGUVRWWFdXWnpkbXBZYjAxVlozRktPRGRQVEhkMFRuSnNRbmxSYkU5RGEwZ3JhVFZaY1dKSVlUVlNaMEp6Q2xCalRFOVJZMmxFVWsxM0syZG9TaXRoVWxWU1NHTnZaV3hxWmt4MmVVOUJjRFZOWkUwMmVIRnBNVlZQWjFWUmNrTXhOelZhYURoMFZsTkthV1V3TDFFS1JGcG9RVXhJVW1zM1YwdExNRm94VWtsdWRWVlhWVWRCZUhKTmVYSlRlVUpEYkUxeWRHZ3JRU3N6V2tzd05sZFZjRWxRYmtsMFNVOWxWMjUyTURCVGRBcFdPSEY1Wm5aMVdDdHFXQzlLTUdobFlrTlpkbVpZUzJwQ1dFMXRWVmR0VGsxRmIxbERhM3BXU0dWbE4xWkNUa0kwUTBaeVJVSTBjMWRHUTBReVNUYzNDbkJKV0dJNE1pOHJSRmRoVEdOd1IwUklTSEpIVHpSTWVGcE1MMGRsZFRFM00yVXdkVFYwY0djM2F6VTVVSFJGUzFoR2N6bGpRMkpTWlRkd1dXNWFlV3NLVFU1bFEzZERXR2QwUVdWbWNrbE9XV2hLVWxvMVNrSnRXQ3QwTlU4NEsxWlVaMjFCY3pKRlVWbFJTME5CVVVGbWRtUkJVRWxrUlhGTmFETjNRakI2ZHdwcEsxQTJURFJNUVZGbU9UZzFXSFpUT1ZwR01VWkVibkZJV25OUWVFcDRTbXRUVWtkTk5VUnFZa0Y1U0hWelUyOXZjVU52UkV4aVQyMWFSbXhCU1hSNUNtMXdOVTFDZFc4NFdIVnRiRFpITlVzNVZXeFVZalV4VjNSM2JuRlFOSEZRYlRSalFrcEVRME5HZVV4MVJEbHJaMDFOUVRWNU4yaGtha3c0UldKMmMwWUtZbXQxY0dwT1NuaE9TbHB2Vm1KV0swNXpWU3N5WlZKMEwzbHhUMDh3VWxjeVZWZGxWMmhvUVZGWFdtVlROMnd6Y2xacGJFSlJRVXhMTVVSVWNsVXpkd3BwUm5seFNqUlBXRVZ5YzFCNlkwRXZRMjFKTnpSU2FVOVFXaXR4ZUZCNlpuRlNSMkpWUnpkWVpXVXplR2xWWnk5cVpsVlJZVlJKVVhGYU1qRnVWek14Q2pGdlEzWjJjV3RpU3pSRFkybFVXSFpSTTBaUFEyUkJVRlV6VlZSMVVIQTVabmR4VVVSV01HeFBSR2RwT0c5WmNEQk5Mek5VVjBSbWJuWnBVbWhVVFUwS2RWb3hla0Z2U1VKQlNGQkhiWFpGUlhwcE9FSjZiak5NTHpKUFpUbFhURTVHVUhGV2FUWkdOWFJVU2xZd05IUjVWVUZsV1ZwM2FUQnlUWGhsYlc5SE5ncFpNRWhoWlRWdmJGYzRTV2hLUm1KRlUycDRjR1Z4UlZscGIwSjBORVZtTlRSYWQzb3ZNVlkxTVZneFNUbDFVV3QxYm1Sd2VrdE9VREV2VDFoMWRXaHFDamsxVUdKU1puVldVekJZVVROWFJuSlNORkpJUm1Oc1ZVOUtaSFZTZG1GcGRWSnJaMVpIYWtSbFdVRkdZUzlEZWtkeGMxSXlhbkZaZG05WFFVUlRMMjBLTWt0UVIzcHZSR1YwVGtWdVltdGtkREpyVkRCa1MwZFBaelYxVUUwelFXOWhSVGhEWlZKcVdsSlBUMXBzZEM5eWIzUTVSa3R3VUdGVGVYZE9kR0poYkFweWJrNW5TbFF2VjFwMk0yRkdSMHMwZEhvclRGcENlVnBtUkZwbGRVNVZUa2xCUkRSR04yRk9RbVpFTkZwdWJDOXFkMGhqT0V4RlJqQTJUMDlsYkROcENrTjVRbE5IWm1GMmRVdzJNbU5xVTNVM2VucFZaelZKYWtWaFZqSnpORVZEWjJkRlFVWndXWFJSYVZWbWMwVkxOV0ZUVms0cldHWlFUMmhWYlV0VlFrUUtVVkprY2tObmFISXJja2hDUnpjdmJESkhSMDlTY1RCMkwydEJjMHR5TldSUllXOXBMMHBaUnpOeVMwTTJZMmcyVm1sck5GQllOUzlhVEVWRlZsWkROQXB3VFZwVmNWbDNWMDh4Y0VGeFUza3JNMjUwYjNoSmRFUk5NWEZ2ZEc4eFVuVnVPV1kwTVROWWFVNDFXbkpEUVRWTmFYbzBiR2hwV2xoaWREQkRkalUyQ2toWE4xbG5WRWRXZDBsWVltSnFTM05MTmtadVJIQkJlbkZwYVZNd1NHZFNWMFp6UWtVd1JWSmlTekpWYTNOMmFVdEhVV3AzUlVaU1ZrMXNjR3BzUTFRS2MxUmxaalJIYkd0V2NraDFiVkkzZDNobWFUVmpORmhMVWxoR1RuZEZObkoxZUdoa1RrZHFUV3BsTW5wTE5uSnZSVlpMVTNCdWFXSjBTMHBKWVZsVmN3cEVla1ZGZUZRM2VIRTRVR3RMY2tSMmJWWlZVbGQxWjFoV2NuQTNObGRUTW5vemNHUkhibFZRU1dkVFdraHJjM3BhTVdGS01HZEpOWEJSUFQwS0xTMHRMUzFGVGtRZ1VsTkJJRkJTU1ZaQlZFVWdTMFZaTFMwdExTMEsKY29udGV4dHM6Ci0gY29udGV4dDoKICAgIGNsdXN0ZXI6IGNsdXN0ZXIKICAgIHVzZXI6IGNmZS1tYXN0ZXIKICBuYW1lOiBkZWZhdWx0Q29udGV4dApjdXJyZW50LWNvbnRleHQ6IGRlZmF1bHRDb250ZXh0Cg==","storages":"[{\"username\": \"abc\",\"password\": \"123\",\"ip\": \"192.0.0.1\",\"port\": 8088}]"})";

    int ret = m_k8sRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：恢复前检查失败
 * 前置条件：入参subobjects数量为空；
 * Check点：CheckBeforeRecovery调用失败。
 */
TEST_F(KubernetesProtectEngineTest, CheckBeforeRecovery_Failed_when_subobjects_empty) {
    Stub stub;
    stub.set(ADDR(StorageClient, Create), Stub_StorageClientCreate2);
    stub.set(ADDR(StorageClient, GetLunInfoData), Stub_StorageClient_GetLunInfoData);
    stub.set(ADDR(StorageClient, GetDeviceBaseInfo), Stub_StorageClient_GetDeviceBaseInfo);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);
    VMInfo vmInfo;
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());

    SetMockRestore(restoreJob);
    restoreJob->restoreSubObjects.clear();
    int ret = m_k8sRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例： 调用CheckBeforeRecovery外部函数失败
 * 前置条件： 生成错误的volmap对。
 * Check点： 返回失败。
 */
TEST_F(KubernetesProtectEngineTest, CheckBeforeRecovery_Failed_when_volume_unmatch)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Create), Stub_StorageClientCreate2);
    stub.set(ADDR(StorageClient, GetLunInfoData), Stub_StorageClient_GetLunInfoData);
    stub.set(ADDR(StorageClient, GetDeviceBaseInfo), Stub_StorageClient_GetDeviceBaseInfo);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);

    VMInfo vmInfo;
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->restoreSubObjects.at(0).name = "123";
    // auto ret = m_k8sRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    // EXPECT_EQ(ret, Module::FAILED);
    m_k8sRestoreProtectEngineHandler->m_restoreVolMap.clear();
}

/**
 * 测试用例： 调用CheckBeforeRecovery外部函数失败
 * 前置条件： 输入目标环境auth.extendInfo为空
 * Check点： 返回失败。
 */
TEST_F(KubernetesProtectEngineTest, CheckBeforeRecovery_Failed_When_auth_extendInfo_is_invalid)
{
    Stub stub;
    stub.set(ADDR(StorageClient, Create), Stub_StorageClientCreate2);
    stub.set(ADDR(StorageClient, GetLunInfoData), Stub_StorageClient_GetLunInfoData);
    stub.set(ADDR(StorageClient, GetDeviceBaseInfo), Stub_StorageClient_GetDeviceBaseInfo);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);

    VMInfo vmInfo;
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());

    SetMockRestore(restoreJob);
    restoreJob->targetEnv.auth.extendInfo = "illegal";

    auto ret = m_k8sRestoreProtectEngineHandler->CheckBeforeRecover(vmInfo);
    EXPECT_EQ(ret, Module::FAILED);
    m_k8sRestoreProtectEngineHandler->m_restoreVolMap.clear();
}

TEST_F(KubernetesProtectEngineTest, CheckBeforeBackup_Succ)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);
    stub.set(ADDR(VirtPlugin::VirtualizationBasicJob, ReportLog2Agent), ReportLog2AgentSuccess);

    VirtPlugin::VirtualizationBasicJob virtualizationBasicJob;
    std::function<void(const VirtPlugin::ApplicationLabelType &)> handler = std::bind(&VirtPlugin::VirtualizationBasicJob::ReportApplicationLabels, &virtualizationBasicJob, std::placeholders::_1);
    m_k8sProtectEngineHandler->SetReportJobDetailHandler(handler);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;

    backupJob->extendInfo = "{\"available_capacity_threshold\":\"20\"}";
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088},{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.116\\\",\\\"port\\\": 8088}]\"}";

    int ret = m_k8sProtectEngineHandler->CheckBeforeBackup();
    EXPECT_EQ(ret, Module::SUCCESS);
}

TEST_F(KubernetesProtectEngineTest, CheckBeforeBackup_Fail1)
{
    Stub stub;
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(ADDR(StorageClient, Send), SendStorageClientApiSuccess);
    stub.set(ADDR(VirtPlugin::VirtualizationBasicJob, ReportLog2Agent), ReportLog2AgentSuccess);

    VirtPlugin::VirtualizationBasicJob virtualizationBasicJob;
    std::function<void(const VirtPlugin::ApplicationLabelType &)> handler = std::bind(&VirtPlugin::VirtualizationBasicJob::ReportApplicationLabels, &virtualizationBasicJob, std::placeholders::_1);
    m_k8sProtectEngineHandler->SetReportJobDetailHandler(handler);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::BackupJob> backupJob = std::dynamic_pointer_cast<AppProtect::BackupJob>(
            jobCommonInfo->GetJobInfo());

    g_logSize = 0;

    backupJob->extendInfo = "{\"available_capacity_threshold\":\"22\"}";
    backupJob->protectObject.id = "statefulset ID";
    backupJob->protectObject.name = "invmdb-1-1-m";
    backupJob->protectObject.type = "StatefulSet";
    backupJob->protectObject.subType = "KubernetesStatefulSet";
    backupJob->protectObject.parentId = "namespace ID";
    backupJob->protectObject.parentName = "ns000000000000000000001";
    backupJob->protectObject.extendInfo = "{\"volumeNames\":\"[\\\"gmdbredo\\\", \\\"gmdbdata\\\", \\\"gmdblredo\\\"]\"}";

    backupJob->protectEnv.id = "kubernetes cluster ID";
    backupJob->protectEnv.type = "VirtualPlatform";
    backupJob->protectEnv.subType = "Kubernetes";
    backupJob->protectEnv.auth.extendInfo = "{\"config\":\"" + KubernetesTestData::t_codedContents + "\",\"storages\":\"[{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.115\\\",\\\"port\\\": 8088},{\\\"username\\\": \\\"admin\\\",\\\"password\\\": \\\"Admin@123\\\",\\\"ip\\\": \\\"8.40.102.116\\\",\\\"port\\\": 8088}]\"}";

    int ret = m_k8sProtectEngineHandler->CheckBeforeBackup();
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：恢复后置任务未执行脚本
 * 前置条件：1. 恢复任务成功；2. 无恢复后置脚本；
 * Check点：恢复任务成功，后置任务成功
 */
TEST_F(KubernetesProtectEngineTest, Restore_success_PostHook_success)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());

    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\",\"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":""})";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    execHookParam.jobExecRet = Module::SUCCESS;

    int ret = m_k8sRestoreProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::SUCCESS);
}

unsigned int Stub_sleep (unsigned int __seconds) {
    return 0;
}

/**
 * 测试用例：恢复后置任务执行脚本
 * 前置条件：1. 恢复任务成功；2. 恢复任务中有后置脚本；
 * Check点：恢复后置任务成功
 */
TEST_F(KubernetesProtectEngineTest, Restore_Success_PostHook_withPostScript_success)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(sleep, Stub_sleep);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());

    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\",\"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"/home/test.sh","failed_script":""})";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    execHookParam.jobExecRet = Module::SUCCESS;

    int ret = m_k8sRestoreProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：恢复后置失败任务未执行脚本
 * 前置条件：1. 恢复任务失败；2. 无恢复失败脚本；
 * Check点：恢复失败后，后置任务成功。
 */
TEST_F(KubernetesProtectEngineTest, Restore_Failed_PostHook_success)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());

    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\",\"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":""})";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    execHookParam.jobExecRet = Module::FAILED;
    int ret = m_k8sRestoreProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：恢复后置任务执行脚本
 * 前置条件：1. 恢复任务失败；2. 有恢复失败脚本；
 * Check点：恢复失败后，后置任务成功，执行恢复失败脚本成功。
 */
TEST_F(KubernetesProtectEngineTest, Restore_Failed_PostHook_withFailedScript_success)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, KubeExec), KubeExecSuccess);
    stub.set(ADDR(KubeClient, Send), SendKubeClientApiSuccess);
    stub.set(sleep, Stub_sleep);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());

    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\",\"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":"/home/test.sh"})";

    VirtPlugin::ExecHookParam execHookParam;
    execHookParam.stage = VirtPlugin::JobStage::POST_JOB;
    execHookParam.hookType = VirtPlugin::HookType::POST_HOOK;
    execHookParam.jobExecRet = Module::FAILED;

    int ret = m_k8sRestoreProtectEngineHandler->PostHook(execHookParam);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：调用PowerOff函数成功
 * 前置条件：1. 参数填写正确, 参数中statefulset的replicasNum已经为0.
 * Check点： 返回值成功
 */
TEST_F(KubernetesProtectEngineTest, PowerOff_StateFulSet_WhenReplicasNumZero_Success)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\", \"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":0,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":""})";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：调用PowerOff函数成功
 * 前置条件：1. 参数填写正确, 参数中statefulset的replicasNum已经为1。2. 底层StopStateFulSet返回成功
 * Check点： 返回值成功
 */
TEST_F(KubernetesProtectEngineTest, PowerOff_StateFulSet_Success)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, StopStateFulSet), Stub_StopStateFulSet_Success);
    stub.set(ADDR(KubernetesApi, ListPods), Stub_ListPods_AfterSetReplicas_Success);
    stub.set(sleep, Stub_sleep);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\", \"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":""})";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：调用PowerOff函数成功
 * 前置条件：1. 参数填写正确, 参数中statefulset的replicasNum已经为1。2. 底层StopStateFulSet返回成功
 * Check点： 返回值成功
 */
TEST_F(KubernetesProtectEngineTest, PowerOff_StateFulSet_Failed_WhenListPodsTimeOut)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, StopStateFulSet), Stub_StopStateFulSet_Success);
    stub.set(ADDR(KubernetesApi, ListPods), Stub_ListPods_AlwaysOne);
    stub.set(sleep, Stub_sleep);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\", \"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":""})";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：调用PowerOff函数失败
 * 前置条件：1. 参数填写正确, 参数中statefulset的replicasNum已经为1。2. 底层StopStateFulSet返回失败
 * Check点：返回值失败
 */
TEST_F(KubernetesProtectEngineTest, PowerOff_StateFulSet_Failed_WhenStopStateFulSetFailed)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, StopStateFulSet), Stub_StopStateFulSet_Failed);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\", \"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":""})";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：调用PowerOff函数失败
 * 前置条件：1. 参数填写错误，restoreJob->targetObject.extendInfo为空
 * Check点：返回值失败
 */
TEST_F(KubernetesProtectEngineTest, PowerOff_StateFulSet_Failed_WhenInputTargetExtendInfoVoid)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = "";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：调用PowerOff函数失败
 * 前置条件：1. 参数填写错误，restoreJob->targetEnv.auth.extendInfo为空
 * Check点：返回值失败
 */
TEST_F(KubernetesProtectEngineTest, PowerOff_StateFulSet_Failed_WhenInputExtendInfoVoid)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetEnv.auth.extendInfo = "";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOffMachine(vmInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：调用PowerOn函数成功
 * 前置条件：参数填写正确, 参数中statefulset的replicasNum已经为0。
 * Check点：返回值成功
 */
TEST_F(KubernetesProtectEngineTest, PowerOn_StateFulSet_Success_WhenReplicasNumIsZero)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\", \"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":0,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":""})";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：调用PowerOn函数失败
 * 前置条件：1. 参数填写正确, 参数中statefulset的replicasNum已经为1。2. 底层RestoreStateFulSet返回失败
 * Check点：返回值失败
 */
TEST_F(KubernetesProtectEngineTest, PowerOn_StateFulSet_Failed_WhenRestoreStateFulSetFailed)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, RestoreStateFulSet), Stub_RestoreStateFulSet_Failed);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\", \"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":""})";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：调用PowerOn函数失败
 * 前置条件：1. targetObject.extendInfo参数填写错误
 * Check点：返回值失败
 */
TEST_F(KubernetesProtectEngineTest, PowerOn_StateFulSet_Failed_WhenInputStsExtendInfoInvalid)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = "Invalid";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：调用PowerOn函数失败
 * 前置条件：参数填写错误,restoreJob->targetEnv.auth.extendInfo中的config值不正确导致k8s接口无法创建。
 * Check点：返回值成功
 */
TEST_F(KubernetesProtectEngineTest, PowerOn_StateFulSet_Faild_WhenInputAuthExtendInfoConfigIsInvalid)
{
    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetEnv.auth.extendInfo = R"({"config":"invalid","storages":"[{\"username\": \"abc\",\"password\": \"123\",\"ip\": \"127.0.0.1\",\"port\": 8088}]"})";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, Module::FAILED);
}

/**
 * 测试用例：调用PowerOn函数成功
 * 前置条件：1. 参数填写正确, 参数中statefulset的replicasNum已经为1。2. 底层RestoreStateFulSet返回成功，ListPods成功
 * Check点：返回值成功
 */
TEST_F(KubernetesProtectEngineTest, PowerOn_StateFulSet_Success)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, RestoreStateFulSet), Stub_RestoreStateFulSet_Success);
    stub.set(ADDR(KubernetesApi, ListPods), Stub_ListPods_AfterRestoreReplicas_Success);
    stub.set(sleep, Stub_sleep);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\", \"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":""})";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
 * 测试用例：调用PowerOn函数成功
 * 前置条件：1. 参数填写正确, 参数中statefulset的replicasNum已经为1。2. 底层RestoreStateFulSet返回成功，ListPods成功
 * Check点：返回值成功
 */
TEST_F(KubernetesProtectEngineTest, PowerOn_StateFulSet_Success_WhenListPodsAlwaysTimeout)
{
    Stub stub;
    stub.set(ADDR(KubernetesApi, RestoreStateFulSet), Stub_RestoreStateFulSet_Success);
    stub.set(ADDR(KubernetesApi, ListPods), Stub_ListPods_AlwaysZero);
    stub.set(sleep, Stub_sleep);

    std::shared_ptr<JobCommonInfo> jobCommonInfo = m_k8sRestoreProtectEngineHandler->GetJobHandle()->GetJobCommonInfo();
    std::shared_ptr<AppProtect::RestoreJob> restoreJob = std::dynamic_pointer_cast<AppProtect::RestoreJob>(
            jobCommonInfo->GetJobInfo());
    SetMockRestore(restoreJob);
    restoreJob->targetObject.extendInfo = R"({"sts":"{\"id\":\"3bb6d270-0f6d-4f0b-bea1-bccc5332163b\",\"name\":\"invmdb-1-1-m\",\"nameSpace\":\"ns000000000000000000001\", \"pods\":[{\"name\":\"adaptermdb-1-1-m-0\",\"pvs\":[{\"lunName\":\"1-adaptermdb-1-1-m-0-redo\",\"name\":\"pv-adaptermdb-1-1-m-0-redo\",\"pvcName\":\"gmdbredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbredo\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-data\",\"name\":\"pv-adaptermdb-1-1-m-0-data\",\"pvcName\":\"gmdbdata-adaptermdb-1-1-m-0\",\"size\":\"80Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdbdata\"},{\"lunName\":\"1-adaptermdb-1-1-m-0-lredo\",\"name\":\"pv-adaptermdb-1-1-m-0-lredo\",\"pvcName\":\"gmdblredo-adaptermdb-1-1-m-0\",\"size\":\"85Gi\",\"storageUrl\":\"https://8.40.111.70:8088/deviceManager/rest\",\"volumeName\":\"gmdblredo\"}]}],\"replicasNum\":1,\"volumeNames\":[\"gmdbredo\",\"gmdbdata\",\"gmdblredo\"]}","pre_script":"","post_script":"","failed_script":""})";

    VMInfo vmInfo;
    int ret = m_k8sRestoreProtectEngineHandler->PowerOnMachine(vmInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
}
/**
*测试用例：调用AllowRestoreInLocalNode函数成功
*前置条件：1.检查与K8s的连通性成功2.检查插件与生产存储的连通性成功
*Check点：返回值成功
*/
TEST_F(KubernetesProtectEngineTest,AllowRestoreInLocalNode_Success)
{
    Stub stub;
    stub.set(ADDR(KubernetesProtectEngine, CheckProtectEnvConn), Stub_CheckProtectEnvConn_Success);
    stub.set(ADDR(KubernetesProtectEngine, CheckAllStorageConnection),
             Stub_CheckAllStorageConnection_Success);
    int32_t errorCode;
    AppProtect::RestoreJob job;
    int ret = m_k8sRestoreProtectEngineHandler->AllowRestoreInLocalNode(job, errorCode);
    EXPECT_EQ(ret,Module::SUCCESS);
}
/**
*测试用例：调用AllowRestoreInLocalNode函数成功
*前置条件：1.检查与K8s的连通性成功2.检查插件与生产存储的连通性成功
*Check点：返回值成功
*/
TEST_F(KubernetesProtectEngineTest, AllowBackupInLocalNode_Success)
{
    Stub stub;
    stub.set(ADDR(KubernetesProtectEngine, CheckProtectEnvConn), Stub_CheckProtectEnvConn_Success);
    stub.set(ADDR(KubernetesProtectEngine, CheckAllStorageConnection),
             Stub_CheckAllStorageConnection_Success);
    int32_t errorCode;
    AppProtect::BackupJob job;
    int ret = m_k8sProtectEngineHandler->AllowBackupInLocalNode(job, errorCode);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
*测试用例：调用AllowBackupSubJobInLocalNode函数成功
*前置条件：1.检查与K8s的连通性成功2.子任务类型是post_sub_job
*Check点：返回值成功
*/
TEST_F(KubernetesProtectEngineTest,AllowBackupSubJobInLocalNode_WhenSubJobTypeIsPost_Success)
{
    Stub stub;
    stub.set(ADDR(KubernetesProtectEngine,CheckProtectEnvConn),Stub_CheckProtectEnvConn_Success);
    AppProtect::BackupJob job;
    AppProtect::SubJob subJob;
    subJob.jobType = SubJobType::type::POST_SUB_JOB;
    int32_t errorCode;	
    int ret = m_k8sProtectEngineHandler->AllowBackupSubJobInLocalNode(job, subJob, errorCode);
    EXPECT_EQ(ret,Module::SUCCESS);
}

/**
*测试用例：调用AllowRestoreSubJobInLocalNode函数成功
*前置条件：1.检查与K8s的连通性成功2.子任务类型是post_sub_job
*Check点：返回值成功
*/
TEST_F(KubernetesProtectEngineTest,AllowRestoreSubJobInLocalNode_WhenSubJobTypeIsPost_Success)
{
    Stub stub;
    stub.set(ADDR(KubernetesProtectEngine,CheckProtectEnvConn),Stub_CheckProtectEnvConn_Success);
    AppProtect::RestoreJob job;
    AppProtect::SubJob subJob;
    subJob.jobType = SubJobType::type::POST_SUB_JOB;
    int32_t errorCode;	
    int ret = m_k8sRestoreProtectEngineHandler->AllowRestoreSubJobInLocalNode(job, subJob, errorCode);
    EXPECT_EQ(ret,Module::SUCCESS);
}

/**
*测试用例：调用CheckProtectEnvConn函数成功
*前置条件：1.检查与K8s的连通性成功2.获取NameSpace成功
*Check点：返回值成功
*/
// TEST_F(KubernetesProtectEngineTest, CheckProtectEnvConn_Success)
// {
//     typedef std::pair<int, std::vector<ApplicationResource>> (*lPtr)();
//     lPtr ListNameSpacesPtr = (lPtr)(&KubernetesApi::ListNameSpaces);

//     Stub stub;
//     stub.set(ListNameSpacesPtr, Stub_ListNameSpaceSuccess);
//     AppProtect::ApplicationEnvironment env;
//     std::string authExtendInfo = R"({"config":"a2luZDogQ29uZmlnCmFwaVZlcnNpb246IHYxCmNsdXN0ZXJzOgotIGNsdXN0ZXI6CiAgICBjZXJ0aWZpY2F0ZS1hdXRob3JpdHktZGF0YTogTFMwdExTMUNSVWRKVGlCRFJWSlVTVVpKUTBGVVJTMHRMUzB0Q2sxSlNVaE9la05EUWxJclowRjNTVUpCWjBsS1FVMXZhblpFUVZWYVJrWnJUVUV3UjBOVGNVZFRTV0l6UkZGRlFrTjNWVUZOU1Vjd1RWRnpkME5SV1VRS1ZsRlJSMFYzU2tSVWFrVlRUVUpCUjBFeFZVVkRRWGRLVWpOV2FHSnRaRVZpTWpWdVRWSkZkMFIzV1VSV1VWRklSRUZvVkdGSFZuVlhiV2hzWW1wRmJBcE5RMDFIUVRGVlJVTm5kMk5UU0Zab1pESldjRWxHVW14Wk1taDFZako0ZGxveWJHeGplVUpFWW5rMGMwbEZlREJhUkVWb1RVSTRSMEV4VlVWRGQzZFpDbFF4VGxSSlExbG5WVEpXZVdSdGJHcGFVMEpWWWpJNWMyTjVRa1ZhV0VJd1RWSkpkMFZCV1VSV1VWRkVSRUZzVUZVeFRYcE1ha0ZuVVRCRmVFbEVRV1VLUW1kcmNXaHJhVWM1ZHpCQ1ExRkZWMFZYT1hwamVrNXFXVlZDYjJSWFJqTmFWMnQxV1RJNWRFMUNORmhFVkVsNVRVUlpkMDFxUVhwT1JHY3hUbXh2V0FwRVZFMTVUVVJWZWsxRVFYcE9SR2N4VG14dmQyZGlVWGhEZWtGS1FtZE9Wa0pCV1ZSQmEwNVBUVkpKZDBWQldVUldVVkZKUkVGc1NHUlhSblZhTUZKMkNtSnRZM2hGVkVGUVFtZE9Wa0pCWTAxRFJrNXZXbGMxWVdGSFZuVk5VMVYzU1hkWlJGWlJVVXRFUW5oSlpGZEdNMXBYYTJkV1IxWnFZVWMxZG1KSE9XNEtZVmRXZWtsRlRuWk1hWGRuVkVoU2EwMVRSWGRJZDFsRVZsRlJURVJDYUZCVk1VMW5TbWxDVkZwWVNqSmhWMDVzU1VaU2RtSXllSHBKUlZKc1kwaFJlQXBGYWtGUlFtZE9Wa0pCVFUxRFZUbFVWWHBOZFUxRFFrUlJWRVZuVFVJMFIwTlRjVWRUU1dJelJGRkZTa0ZTV1ZKaU0wNTZUVEpPYUZGSGFERlpXR1JzQ21GVE5XcGlNakIzWjJkSmFVMUJNRWREVTNGSFUwbGlNMFJSUlVKQlVWVkJRVFJKUTBSM1FYZG5aMGxMUVc5SlEwRlJSRk5LUkhsd2QzbFlURWhDTHpVS2JYSkZhR2gwV1RWalQzbGxUbVF2VVVWNGJuWlJZV1JhUW5KSGQwRjVNVzQ1YjNvM2JGTTVWREpHUm01NmJVbFlVMDVLWlZkVVFYVk5PV1F5V0c1RGFBbzRXVE5oYVdoemIxSTBORmhWTDNWd015dE5ZWFJPVG1kMlpuTXpORGhYYm05M2RHdFRkREpHVW1FcmNuaEZWbWQxZDIwNFNubHVkV2xDWmpaMVZ6WjBDbFpuYUVSSVMzSXJSbVVyYVV0ek5uaGlOWGR4YXpGWlRuaE1VRVkxVEdVeWNWVkdlVEJVY1ZKSU9YTmFUazlwWkZOdk9XUk1aR0pNVEROa2RreHNjRzhLUTBoVFF6QmpPUzkzTmxaVU5YVlRkMVY2V1VkT2MwUnlaeXRxVWtGVFdtaE5SVnBWVGtoNVJXeGxkWGQwUzBwNVlsTjJSbmhwV0hKa2RHdDRWMVphYndwcFRrOW5WbkZOZGpCWU5HOVJha1J3YlRWdlNYQnZhR2hYV0VWUE5VVndhblpZZEdkYU5rSnRiVlZVTURFclpHMWFkMjFpVXpadmRGSTFWemhPTjBoTUNrcHhTVFpRYVRWRlIwNUlSakppTkROR1VGUnZNblo0UjBSclNFRjFORXhPU0RkS05rRm5lRVZ1TW1rMlJtOTNXRWd6ZWtwd1FqQmFaVzVaYW5WMlZUWUtOMFE1UVdjcmFFUnVjRzkxUW5CSlVXaENSR28zVVdKSk5YUXhjVzFNVDFNMU1FMTNTWEFyWmxwWGFsVmtNMmxsYml0TVNqaGlRWFpZWTNKa1NYbENWd28yTmxweVV6STVPR0paYlVkWFdUTnJhVWhuUVZsalptWnBMMkkwWXpaNk1uWnNWWG8yYmxWMmJ5OWpXbHBaYTFOR1dHOVNWWE5uUzNJNWMzcFZNblppQ2s1VmRIbFVkekZqVFRKTk1qaDRNVWQwTDNkWVpsTlZlVXNyVjJKMVYzZHpkMFJRYTBOcE9HVk1RVTloU0dZeVFUaEdNa1prYUVkVk1FaGlVV1p6TVVrS1ZrTlJLMkpWWkdWVGMxWm1PVUY1VURGU2RHSnNWMjF1YzNkTU1FYzRiemRtY3pjeWRrNXRTVVIyY2pGc1pXZHRLMHh0VVVKRVpEVkVUVTFYWlVOcE9RcFVVbGRVUXpseE1tWmpVbHAxTldWeU0wOVJSVmdyZFVKT05IZEVkRkZKUkVGUlFVSnZORWxDVTBSRFEwRlZVWGRJVVZsRVZsSXdUMEpDV1VWR1RucFZDbkp2Wm5kR1RHOTJaR0pIZFhnd1F6ZHViRTAyVjBZNFpFMUpTSEJDWjA1V1NGTk5SV2RsUlhkblpEWkJSazU2VlhKdlpuZEdURzkyWkdKSGRYZ3dRemNLYm14Tk5sZEdPR1J2V1VjMmNFbEhNMDFKUnpCTlVYTjNRMUZaUkZaUlVVZEZkMHBFVkdwRlUwMUNRVWRCTVZWRlEwRjNTbEl6Vm1oaWJXUkZZakkxYmdwTlVrVjNSSGRaUkZaUlVVaEVRV2hVWVVkV2RWZHRhR3hpYWtWc1RVTk5SMEV4VlVWRFozZGpVMGhXYUdReVZuQkpSbEpzV1RKb2RXSXllSFphTW14c0NtTjVRa1JpZVRSelNVVjRNRnBFUldoTlFqaEhRVEZWUlVOM2QxbFVNVTVVU1VOWloxVXlWbmxrYld4cVdsTkNWV0l5T1hOamVVSkZXbGhDTUUxU1NYY0tSVUZaUkZaUlVVUkVRV3hRVlRGTmVreHFRV2RSTUVWNFNVUkJaVUpuYTNGb2EybEhPWGN3UWtOUlJWZEZWemw2WTNwT2FsbFZRbTlrVjBZeldsZHJkUXBaTWpsMFoyZHJRWGxwVHpoTlFsSnJWVmRSZDBSQldVUldVakJVUWtGVmQwRjNSVUl2ZWtGTVFtZE9Wa2hST0VWQ1FVMURRVkZaZDBoQldVUldVakJTQ2tKQ1ZYZEZORVZTWWpOT2VrMHlUbWhSUjJneFdWaGtiR0ZUTldwaU1qQjNSRkZaU2t0dldrbG9kbU5PUVZGRlRFSlJRVVJuWjBsQ1FVTlVTWGxFVkdzS05YQlBRVVJrVldzemJVdGpUR1Z5V2pGR1NVMTVhMlkxUzNScFFVNTBZMHhyUjJZNVdGTjVURmw2WW5OQ2FUWklhbTVUTjJ0MFoxZFFhRWQ2WlN0b2RRcENhMGwzYkhkNlJEVmlkbU0wSzJWV1Uyd3ZUa0Y1YzBsSmFuQlNialI1Y0cxRmVYQkNRM3B6TVdwV1FuVndiak5SVERWYVoyTkZNSFJWZEZvemJVbHNDblJxTVdwSVJqQmtVSFoxY3padk0yc3pNaXN5TjIwMlRVeHFOeXRFYW1GTGRUSkJlalY1YkROdWVTdFBVVGs1VkZKemRWQk9XblYxUjNWSGJuUndORUVLUml0MGREWjBlbTB5VkRnd1ptZG1WVmxQTDNGMVNHdEJTVGRoWlRSQk5FWkxOSGhZYldKeldWRmxaR0Z2VmxoMU1VVldiRFExTkZCWVIxaG5aMjB2ZVFwT01qUlpRbXNyYzFSWWRFNUpUMjVyU21Rd1NFZE5VRzlYT1U1SlRFUk5aM2tyYTB0VmRFWkJVakV5T1Rad1lsaHJUM3A1ZVRWSk5IQjJSV3BzTlRSV0NuVkNWRzF2WW1aRlJFUlFNbk5YV2sxa1RrbFNlRlZUVVdwV2JHRm9Na3B5TVhGTFdFOWtOVlpJTTBaVU1ucDRlbXRJZGpkMGJrZHVWblpSZEc5cU1Xb0tOVzEzT1cxS0sydFRhMDFEY1VoRFZEZElXRGRuVWxkeWRERmlTekZSTkRoMlVYbHZObGx5VG01MmVrZExaMEZPZUhZNWNrZE9Na3RPWm5wclowSjNTQXA2T1RGU2QwOVZUbnBDVUhsc00yMUZRUzh5Wm5OUlVrWXlPRVZ6YURrMFprOUdTVmRYUnpOeVdHTjBRa1UyWm5oMWNUVmlhVUYwZG01SFoxVkthbWhYQ2tkcWREVTNVREpFZWtkNU5rMTBWM2RSUzJJMlRsZ3haMVZpZFZsQldYZHZOelpRU1RsaWVuWlFlVU53UTJFeWFXaEplR1YzYTNOWlNHSndVMWhDUkdVS0syZHZhR05KY21oNlYybE9hamMwUXpCNVRXMXZURzl4WkhWNFpHVk9OM2RrYWpnNVJWTlpTbXRaZG5kU2F6RkZRWHAwWVZOeFlXMUNOV2RoYzA5UFRRb3dUVVp5TTA1a2FISkpVVFpVUmxOQ1VFOU5SV2xYTUU5bE9VdFRTbEZ3TW5oWlJuRUtMUzB0TFMxRlRrUWdRMFZTVkVsR1NVTkJWRVV0TFMwdExRbz0KICAgIHNlcnZlcjogaHR0cHM6Ly84LjQwLjEzNy43OjU0NDMKICBuYW1lOiBjbHVzdGVyCnVzZXJzOgotIG5hbWU6IGNmZS1tYXN0ZXIKICB1c2VyOgogICAgY2xpZW50LWNlcnRpZmljYXRlLWRhdGE6IExTMHRMUzFDUlVkSlRpQkRSVkpVU1VaSlEwRlVSUzB0TFMwdENrMUpTVWN3ZWtORFFreDFaMEYzU1VKQlowbEpWRmRYUTBsUlpqZ3ZWa2wzUkZGWlNrdHZXa2xvZG1OT1FWRkZURUpSUVhkbllsRjRRM3BCU2tKblRsWUtRa0ZaVkVGclRrOU5Va2wzUlVGWlJGWlJVVWxFUVd4SVpGZEdkVm93VW5aaWJXTjRSVlJCVUVKblRsWkNRV05OUTBaT2IxcFhOV0ZoUjFaMVRWTlZkd3BKZDFsRVZsRlJTMFJDZUVsa1YwWXpXbGRyWjFaSFZtcGhSelYyWWtjNWJtRlhWbnBKUlU1MlRHbDNaMVJJVW10TlUwVjNTSGRaUkZaUlVVeEVRbWhRQ2xVeFRXZEthVUpVV2xoS01tRlhUbXhKUmxKMllqSjRla2xGVW14alNGRjRSV3BCVVVKblRsWkNRVTFOUTFVNVZGVjZUWFZOUTBKRVVWUkZaMDFDTkVjS1ExTnhSMU5KWWpORVVVVktRVkpaVW1JelRucE5NazVvVVVkb01WbFlaR3hoVXpWcVlqSXdkMGxDWTA1TmFrbDNUbXBCZVUxRVZYZE5hbEV5VjJoblVBcE5ha0V6VFZSQk1rMUVTWGRPVkVGNVRrUmFZVTFKUjNCTlVYTjNRMUZaUkZaUlVVZEZkMHBFVkdwRlUwMUNRVWRCTVZWRlEwSk5TbEl6Vm1oaWJXUkZDbUl5Tlc1TlVrVjNSSGRaUkZaUlVVaEZkMmhVWVVkV2RWZHRhR3hpYWtWc1RVTk5SMEV4VlVWRGFFMWpVMGhXYUdReVZuQkpSbEpzV1RKb2RXSXllSFlLV2pKc2JHTjVRa1JpZVRSelNVVjRNRnBFUldoTlFqaEhRVEZWUlVOM2QxbFVNVTVVU1VOWloxVXlWbmxrYld4cVdsTkNWV0l5T1hOamVVSkZXbGhDTUFwTlVrbDNSVUZaUkZaUlVVUkZkMnhRVlRGTmVreHFRV2RSTUVWNFJsUkJWRUpuVFhCQlVVVlVSRWhDYUZsWVRYUmlNakIwV1RJNWVWcFVRME5CYVVsM0NrUlJXVXBMYjFwSmFIWmpUa0ZSUlVKQ1VVRkVaMmRKVUVGRVEwTkJaMjlEWjJkSlFrRk9WV05hU1VacWQwWTNablU0YWpkMlJtdG9WbUZIVlhORVdsSUtkMGx2WnpOb1JISmhWemxEVFRaaVFqWmhaRXAxVEVOeWEzSnBSM1p3VGt0cFZWUm1hVkZZVFhaeVZWWk1iMmROUVdsUmJrazRSR1JoZGtGcmVYZDViZ294T0hCM2NGRjZZME5hYm01SVR6Rllaamw0VEN0SFdHTkhUV013Tm5OaFNVUk9lbmRzUTNacE9URlhNemgwVlVod2EwRjVibFpuVm0weFpuUm9OVFUxQ21OaFRXSXhWVkZEV1RGNGNpOVRTRk13WlVOaFNsUkVOQ3RqZVZwTlNuWjNSRzg0VlhoblRuRTFhMU5LVlZwTVEybEhlbUZ1V1dKdVNGVmtLeXR1ZWxJS2VHOVNPR05LVlhseGVtWXhlVmRqTjAxNGRUbDFaR0YwUm1WMll5czVURk5KTlRaM1MwYzFjbFE1WkVkUU5VNWlkelF6TVhKdVJFaFJURTkxYUZCTE5BcGFRamd5UXk5R2NHWnRZelJZVG05c1dEQTVXVUp1WTBSV1lrTnVTMlJFUVc1SWVtcGlWbnAwVVVGUWExUm9ORVYxV1hsMVNGQlhURFJDU1RkWE5sVjBDbWt3Y21weFZFTTBaVEZuYlRKNlpuTjJZa3MxVG1ocFVsaHdUblpETlVwbk1GSnRVR05CTjFsT05YRnBiMHhNTDFjMGJqbHZRa3htYzBkVGJuRkJia0lLZDBGTFMwSnlSRzUzV2xjelVVcEdWalp5UldsWk9FZFBUVk5aWjB4cFVuSnplVXhNZURGV05pc3dVbkpyUm0xRFpGUlRlVmxvTVZGQlJUSllRbGhVVVFwVVQwdFFSMlZYYjJndlFVbE9Vbk5NVDNSMWNVMUllVmQzUTFSaFIyazBTa2hXUVcxc0wxTTVaRzEzVjFrMU4xQnpTek5qUjIxS1NtUllWRWxFUmpKU0NsbG5iRGN3VVRWWWNFRmxRems1Tkc5WGRFWmtVMGRuTW5VdmNXdFJhbmR2UjNoSFpqUlNTemR2YzB4ME5rdEJUVUpTTjFJclJuaHlkMEUyYzJoRWFVUUtORXBsUkdZM2FUaFpZWFZFYW01T2NGRXpMMDV2WlVSS1lsRkZabFphYzNkSmQzcHFVVk5KVDFwelpubEJORUZNYUVOeWNqZEpialJtTDBKcFpteG9UQXBLYkUxUGJHTjBhSEUzWXpjdlRtNHpRV2ROUWtGQlIycG5aVGgzWjJWM2QwUm5XVVJXVWpCUVFWRklMMEpCVVVSQlowdHJUVUl3UjBFeFZXUktVVkZYQ2sxQ1VVZERRM05IUVZGVlJrSjNUVU5DWjJkeVFtZEZSa0pSWTBSQlZFRk5RbWRPVmtoU1RVSkJaamhGUVdwQlFVMUNPRWRCTVZWa1NYZFJXVTFDWVVFS1JrNTZWWEp2Wm5kR1RHOTJaR0pIZFhnd1F6ZHViRTAyVjBZNFpFMUpSMHhDWjA1V1NGSkZSV2RaVFhkbldVTkRRak5DYUZsWVRYUmlNakpEUVVsSllncExhVFZyV2xkYWFHUlhlREJNYms0eVdYazFhbUpJVm5wa1IxWjVURzE0ZGxreVJuTm5hRmx4VEcwNWRFeHVUakpaZVRWcVlraFdlbVJIVm5sTWJYaDJDbGt5Um5ObmFHOXhURzB4YUdKdFJtNWFVelY2WkcxTmRWa3llREZqTTFKc1kyazFjMkl5VG1oaVNXTkZjMEozUVVGWlkwVnpRbmRCUVc5alJXWjNRVUVLUVZsalJVTkRhVXBDTkdORlEwTnBTa0kwWTBWRFEybEtRbnBCVGtKbmEzRm9hMmxIT1hjd1FrRlJjMFpCUVU5RFFXZEZRVWRNVkdSUVVXbFBha1l6THdwWUt5OUZibm94T0RKVUt6bG1TMGsyYlhvclMwVjJOME42WTNKaFJ5OTVhM0I0YmxsT2JWaHhlbEYyS3preVYwOXhTemczY1VoR2VHNU1SME4yTDAxMENqSlJkRGtyVGtVeWRsbDZWRkpZWVZkdFVuaDVUVmNyVkVWTmN6bEdWVU5sYldobFJqbFpSVTF2TW1OTVJHeHZVbkZzZURseVEyVnJiRmhHTkZSNVJUQUtZVEIzVUZKTk9VVlZlVmxFVlRod01uaFZWSGRrVWt4NGJHVjBWVU13YVhWSmFYQldhVmRYT0dwM1NqRlNWQzh2TmxoRVFWVnpkVklyV1dOT1dFTnBkQW93VVZWNVRVaGxkRVk1TVRsRFlXazVNamhDUzJ4NFpHRjJZMHR5UTBwU2FXRkZUV3REYms4M1dFMXVRa1Z6WTBORE9HVk1LMFo0Um1WcmFuTnZjMDQ1Q2xJM09IbHhURXBuU1U5NFpEUTNTRWhtUVc4MlpVWk1TMGs0U1hCMVJYVXdTSFI1T0cxcVMxRjJNMHRwYXpab05YbFBVM2hrYzNwaVpERTJjamhEWkRJS1ptaE1LMDAyYUdVd2FTOHlWMFExZUZOWGQweElNMGRxY3pjeVVIcG1aMnR3U0dvdlRHaHlhbmxEV0ZKR2FHbDRaMUJMZDJ4TVp6TlRMekZzUTA1alNBcEthekZWWTBGSmRESTVhVWdyWlZSQlRXZHRUR1E0U0hwc1drWm5kemxQZUhoT2NFWm9XVzV6UTBwQ1EyOXVaVGhaVkc5SGFDdDFiMU5MVXpCWFFtZ3JDazFDUms5bmRXUXpPRGNyTmpnMVVIZ3dWa2RDVldWd2NuUmlUbWtyUkVaT09FbENibUUxV2s1b1NUSXJaMnBKVDJack9XdFdUbkEyZEdOdFNYSlVkVE1LVWtkNlJTOHZiazA0VUc1alRHNWFaVWRWVFRaMmF6YzVZM3BKVUROa2IzUlFSMWhNUzNCYVNXRm9RVE5VYVVaVE5WTlBNVWRrVjAwdk9Ib3djMjlST0FvckwwWXlibVJzZEhSUVVrc3pSM1JsZWxOWk1uQnRhVUpsVjFvM01IVkVOMlpLTnpKUE5qUkhVSE5sVDBzMU5VOUdSSGhpZEZWNVVHOHZiblk1VjFkSUNqSjBhVlpVZGxnMlpGcEZkR2hrYWs5Qk4xYzRaRXRSYmxoT2JrWkhVMk05Q2kwdExTMHRSVTVFSUVORlVsUkpSa2xEUVZSRkxTMHRMUzBLCiAgICBjbGllbnQta2V5LWRhdGE6IExTMHRMUzFDUlVkSlRpQlNVMEVnVUZKSlZrRlVSU0JMUlZrdExTMHRMUXBOU1VsS1NuZEpRa0ZCUzBOQlowVkJNVko0YTJkWFVFRllkQ3MzZVZCMU9GZFRSbFp2V2xOM1RteElRV2xwUkdWRlQzUndZakJKZW5CelNIQndNRzAwQ25OTGRWTjFTV0VyYXpCeFNsSk9LMHBDWTNrcmRGSlZkV2xCZDBOS1EyTnFkMDR4Y1RoRFZFeEVTMlpZZVc1RGJFUk9kMHB0WldOak4xWmtMek5GZGpRS1dtUjNXWGg2VkhGNGIyZE5NMUJEVlVzclRETldZbVo1TVZGbGJWRkVTMlJYUWxkaVZpc3lTRzV1YkhodmVIWldVa0ZLYWxoSGRqbEpaRXhTTkVwdmJBcE5VR28xZWtwcmQyMHZRVTlxZUZSSFFUSnliVkpKYkZKcmMwdEpZazV4WkdoMVkyUlNNemMyWms1SVIyaEllSGRzVkV0eVRpOVlTbHA2YzNwSE56STFDakZ4TUZZMk9YbzNNSFJKYW01eVFXOWliWFJRTVRCWkwyc3hka1JxWmxkMVkwMWtRWE0yTmtVNGNtaHJTSHBaVERoWGJDdGFlbWhqTW1sV1psUXhaMGNLWkhkT1ZuTkxZM0F3VFVOalprOU9kRmhQTVVGQksxSlBTR2RUTldwTE5HTTVXWFpuUldwMFluQlRNa3hUZFU5d1RVeG9OMWREWW1KT0szazVjM0pyTWdwSFNrWmxhekk0VEd0dFJGSkhXVGwzUkhSbk0yMXhTMmR6ZGpsaWFXWXlaMFYwSzNkYVMyVnZRMk5JUVVGdmIwZHpUMlpDYkdKa1FXdFdXSEZ6VTBwcUNuZFpOSGhLYVVGMVNrZDFla2x6ZGtoV1dISTNVa2QxVVZkWlNqRk9URXBwU0ZaQlFWUmFZMFprVGtKTk5HODRXalZoYVVnNFFXY3hSM2R6TmpJMmIzY0taa3BpUVVwT2IyRk1aMnRrVlVOaFdEbE1NVEppUWxwcWJuTXJkM0prZDJGWmEyd3haRTFuVFZoYVJtbERXSFpTUkd4bGEwSTBURE16YVdoaE1GWXhTUXBoUkdFM0szRlNRMUJEWjJKRldpOW9SWEoxYVhkMU0yOXZRWGRHU0hSSU5GaEhka0ZFY1hsRlQwbFFaMncwVGk5MVRIaG9jVFJQVDJNeWJFUm1PREpvQ2pSTmJIUkJVamxXYlhwQmFrUlBUa0pKWnpWdGVDOUpSR2RCZFVWTGRYWnphV1pvTHpoSFNpdFhSWE50VlhjMlZua3lSM0owZW5ZNE1tWmpRMEYzUlVFS1FWRkxRMEZuUW1nd2NWZHFObmxyVEc4NFIzTk5WRzB3TlVOT1RtZ3hXVmh4VWtWeWJHOXBOREZaWWxKU2JEZ3hVbUlySzFKWk0yTnpkbFZ1WkU1eFdBb3JPV1pyTkVsTmFWRlhhWE00UzNOeFZXWkVUbkphVjNjeWVrcFRVakpHV2xSMFdUQkhPVGN6WkhreVJrVjBlSEJoWjJaTlJrdGtVMEZ0VkhoV1F6Uk5Dbk5pYWs0eGFsUTVja1JMUTJreWVWSk5ka041WlZWc1YyWktlVkpGVUhneU1YbEpPV292U1cxU1REVmtWakJwWW5SUk1GZ3ZNRzVvU0M5b2VWaFZUVGtLTkZWVFpHdDRhVTU1ZHpCdFIzRTFlVWx2TDBNdk9YSmtZVTF6T0ROWFIxZ3lRMk0yS3pOQmJWaEdjbEl4YVZnNFJpdFhUemN4YTFZelRpczJTUzlqU1FwUUswOVFNMU5pUTBGTmNHMWpPVEpPUVRoMFVrbzFlRXROYm5oV2RtdzVVMng1WlVkWFExaEdVeTl3TURSU09XMUdNQ3RZZFZReFIyUnBXbTlpTWswekNtRnhTM0ZvVXpkWVVVTlNjRnBQZDJsNFYwcDVNMEl4TkRkU2MwOUVUemhvWXk5clFVaDNOMDFuYTBwUlkyOXhNM2ROYWs1dll6Wk5MMm9yVkZCUGNXc0tkR0pRTUZaRVltSnRjbkZaUmpOaE5DdFRTMnRrVGtzNFptOVFhMnRLYldzMmNDdE1RM0ExUW5oa05sTXpjRkUxZW1Gc1MxTm9RMlpXYUUwMVZGVmhWZ3BrUTNWalJ6ZFdPRzFqTW5CelRFZEVhM0JvUVRGUmVXZ3piMVprY2tOeVlWaDBUamhzVWtJelVsQjFZeTlLWm5vemJtbG9SMjlDY2tONVNIRXJXbGxrQ2xJMlprTjFNVmhSWmpCWVYwMUtZVlJPTTJ0eGVUWlpPRlJuYURsRVVrNXROVEZ4VDFCMFZVNU1kWEpOUlhnNGEzb3dReXRsS3pWVGNVMTRhMWg0ZWtrS2VtSTVkekoxUjI1dmFtdzBOMFUzY0V4aU5IZDZiMmR5ZW1aNmJTOU9VbUV5VVZONWNIcHhRM2hJWXpCcWRuZDVaMGxMU0hJclNUWkhkM2Q1ZGpSc1RncEtOa2wyT0cxRUszRllUVWxZVWs5b2VVWm1Ta1JOV0RVcmVrSmpkRzlHVjBkNWMweExTSEJLTlRrMFdXMDNWVUYzVVV0RFFWRkZRVGRrU3k5UmJXUlRDbGw2U0ZscVZtRlRVbVJ6VldoTU1uUjNTMFZWVUM5TGN6Tlhjbk15VmxGelJtaDViMWx2TDNkQ2JuTkpNbFZhWTFwWGFYWTRjVWt4ZUc4MlVuQjZOVllLZFRWWFZtMWxiMUZGYmtNd1pYWlZiVk5xU2xoRlNXNU1kR2RCVWtoUWEwaEllRGd2UnpoSk1raHZOalZuT1UweVFWcElkRkIwZDBKM1pGVTFUemQxVkFvck5WWTNOME5uYW1VNU5Ia3JjRzFLUkhod1RYY3Ziak5TY2tGa1RYQlFiRTFEU0ROVllucDJjelZ1ZFVKT2JWY3ZLMkp2VFcxaFlXdHNkMVptYVd0UkNsUnZha3hpTld3cmQwWndUR2hyUkROT1ZFRllRbEZrU2l0NlpqTm9VM3BvUjI5NmVYTmxaWGx2ZGxVNGVrcFJibkozVEZOaGJGZFRTamc0ZWxWUE5YTUtiM0prV0V0b1kySjZPR1l2WlhZeWJtWmlhRzQ1VldoNlRtRkViRGt4Tm1aeFFrVTBha3RxWjNWcE9HWk5OV2xNTW5kMlZtMU5VRVpsWTBGM09FTlhlZ3BvYkhoMVlWRkVOMmhTYW5CV2QwdERRVkZGUVRWWFdXWnpkbXBZYjAxVlozRktPRGRQVEhkMFRuSnNRbmxSYkU5RGEwZ3JhVFZaY1dKSVlUVlNaMEp6Q2xCalRFOVJZMmxFVWsxM0syZG9TaXRoVWxWU1NHTnZaV3hxWmt4MmVVOUJjRFZOWkUwMmVIRnBNVlZQWjFWUmNrTXhOelZhYURoMFZsTkthV1V3TDFFS1JGcG9RVXhJVW1zM1YwdExNRm94VWtsdWRWVlhWVWRCZUhKTmVYSlRlVUpEYkUxeWRHZ3JRU3N6V2tzd05sZFZjRWxRYmtsMFNVOWxWMjUyTURCVGRBcFdPSEY1Wm5aMVdDdHFXQzlLTUdobFlrTlpkbVpZUzJwQ1dFMXRWVmR0VGsxRmIxbERhM3BXU0dWbE4xWkNUa0kwUTBaeVJVSTBjMWRHUTBReVNUYzNDbkJKV0dJNE1pOHJSRmRoVEdOd1IwUklTSEpIVHpSTWVGcE1MMGRsZFRFM00yVXdkVFYwY0djM2F6VTVVSFJGUzFoR2N6bGpRMkpTWlRkd1dXNWFlV3NLVFU1bFEzZERXR2QwUVdWbWNrbE9XV2hLVWxvMVNrSnRXQ3QwTlU4NEsxWlVaMjFCY3pKRlVWbFJTME5CVVVGbWRtUkJVRWxrUlhGTmFETjNRakI2ZHdwcEsxQTJURFJNUVZGbU9UZzFXSFpUT1ZwR01VWkVibkZJV25OUWVFcDRTbXRUVWtkTk5VUnFZa0Y1U0hWelUyOXZjVU52UkV4aVQyMWFSbXhCU1hSNUNtMXdOVTFDZFc4NFdIVnRiRFpITlVzNVZXeFVZalV4VjNSM2JuRlFOSEZRYlRSalFrcEVRME5HZVV4MVJEbHJaMDFOUVRWNU4yaGtha3c0UldKMmMwWUtZbXQxY0dwT1NuaE9TbHB2Vm1KV0swNXpWU3N5WlZKMEwzbHhUMDh3VWxjeVZWZGxWMmhvUVZGWFdtVlROMnd6Y2xacGJFSlJRVXhMTVVSVWNsVXpkd3BwUm5seFNqUlBXRVZ5YzFCNlkwRXZRMjFKTnpSU2FVOVFXaXR4ZUZCNlpuRlNSMkpWUnpkWVpXVXplR2xWWnk5cVpsVlJZVlJKVVhGYU1qRnVWek14Q2pGdlEzWjJjV3RpU3pSRFkybFVXSFpSTTBaUFEyUkJVRlV6VlZSMVVIQTVabmR4VVVSV01HeFBSR2RwT0c5WmNEQk5Mek5VVjBSbWJuWnBVbWhVVFUwS2RWb3hla0Z2U1VKQlNGQkhiWFpGUlhwcE9FSjZiak5NTHpKUFpUbFhURTVHVUhGV2FUWkdOWFJVU2xZd05IUjVWVUZsV1ZwM2FUQnlUWGhsYlc5SE5ncFpNRWhoWlRWdmJGYzRTV2hLUm1KRlUycDRjR1Z4UlZscGIwSjBORVZtTlRSYWQzb3ZNVlkxTVZneFNUbDFVV3QxYm1Sd2VrdE9VREV2VDFoMWRXaHFDamsxVUdKU1puVldVekJZVVROWFJuSlNORkpJUm1Oc1ZVOUtaSFZTZG1GcGRWSnJaMVpIYWtSbFdVRkdZUzlEZWtkeGMxSXlhbkZaZG05WFFVUlRMMjBLTWt0UVIzcHZSR1YwVGtWdVltdGtkREpyVkRCa1MwZFBaelYxVUUwelFXOWhSVGhEWlZKcVdsSlBUMXBzZEM5eWIzUTVSa3R3VUdGVGVYZE9kR0poYkFweWJrNW5TbFF2VjFwMk0yRkdSMHMwZEhvclRGcENlVnBtUkZwbGRVNVZUa2xCUkRSR04yRk9RbVpFTkZwdWJDOXFkMGhqT0V4RlJqQTJUMDlsYkROcENrTjVRbE5IWm1GMmRVdzJNbU5xVTNVM2VucFZaelZKYWtWaFZqSnpORVZEWjJkRlFVWndXWFJSYVZWbWMwVkxOV0ZUVms0cldHWlFUMmhWYlV0VlFrUUtVVkprY2tObmFISXJja2hDUnpjdmJESkhSMDlTY1RCMkwydEJjMHR5TldSUllXOXBMMHBaUnpOeVMwTTJZMmcyVm1sck5GQllOUzlhVEVWRlZsWkROQXB3VFZwVmNWbDNWMDh4Y0VGeFUza3JNMjUwYjNoSmRFUk5NWEZ2ZEc4eFVuVnVPV1kwTVROWWFVNDFXbkpEUVRWTmFYbzBiR2hwV2xoaWREQkRkalUyQ2toWE4xbG5WRWRXZDBsWVltSnFTM05MTmtadVJIQkJlbkZwYVZNd1NHZFNWMFp6UWtVd1JWSmlTekpWYTNOMmFVdEhVV3AzUlVaU1ZrMXNjR3BzUTFRS2MxUmxaalJIYkd0V2NraDFiVkkzZDNobWFUVmpORmhMVWxoR1RuZEZObkoxZUdoa1RrZHFUV3BsTW5wTE5uSnZSVlpMVTNCdWFXSjBTMHBKWVZsVmN3cEVla1ZGZUZRM2VIRTRVR3RMY2tSMmJWWlZVbGQxWjFoV2NuQTNObGRUTW5vemNHUkhibFZRU1dkVFdraHJjM3BhTVdGS01HZEpOWEJSUFQwS0xTMHRMUzFGVGtRZ1VsTkJJRkJTU1ZaQlZFVWdTMFZaTFMwdExTMEsKY29udGV4dHM6Ci0gY29udGV4dDoKICAgIGNsdXN0ZXI6IGNsdXN0ZXIKICAgIHVzZXI6IGNmZS1tYXN0ZXIKICBuYW1lOiBkZWZhdWx0Q29udGV4dApjdXJyZW50LWNvbnRleHQ6IGRlZmF1bHRDb250ZXh0Cg==","storages":"[{\"username\": \"test\",\"password\": \"test\",\"ip\": \"8.40.111.70\",\"port\": 8088},{\"username\": \"test\",\"password\": \"test\",\"ip\": \"127.0.0.1\",\"port\": 8088}]"})";
//     env.auth.extendInfo = authExtendInfo;
//     int ret = m_k8sRestoreProtectEngineHandler->CheckProtectEnvConn(env);
//     EXPECT_EQ(ret, Module::SUCCESS);
// }

/**
*测试用例：调用CheckProtectEnvConn函数成功
*前置条件：1.检查与生产存储的连通性成功2.获取NameSpace成功
*Check点：返回值成功
*/
TEST_F(KubernetesProtectEngineTest, CheckStorageConnection_Success)
{
    typedef int32_t (*fptr)(VirtPlugin::OceanStorVolumeHandler*,const std::string&);
    fptr OceanStorVolumeHandler_TestDeviceConnection = (fptr)(&VirtPlugin::OceanStorVolumeHandler::TestDeviceConnection);
    Stub stub;
    stub.set(OceanStorVolumeHandler_TestDeviceConnection,StubTestDeviceConnectionSuccess);

    std::string authExtendInfo = R"({"config":"123","storages":"[{\"username\": \"test\",\"password\": \"test\",\"ip\": \"8.40.111.70\",\"port\": 8088,\"sn\": \"12345\"},{\"username\": \"test\",\"password\": \"test\",\"ip\": \"127.0.0.1\",\"port\": 8088,\"sn\": \"123456\"}]"})";
    VolInfo volInfo;
    int ret = m_k8sProtectEngineHandler->CheckStorageConnection(volInfo,authExtendInfo);
    EXPECT_EQ(ret, Module::SUCCESS);
}

/**
*测试用例：调用CheckAllStorageConnection函数成功
*前置条件：1.检查与生产存储的连通性成功
*Check点：返回值成功
*/
TEST_F(KubernetesProtectEngineTest, CheckAllStorageConnection_Success)
{
    std::string authExtendInfo = R"({"config":"123","storages":"[{\"username\": \"test\",\"password\": \"test\",\"ip\": \"8.40.111.70\",\"ipList\": \"8.40.111.70\",\"port\": 8088,\"sn\": \"12345\"}]"})";
    int ret = m_k8sProtectEngineHandler->CheckAllStorageConnection(authExtendInfo);
    EXPECT_EQ(ret, -1);
}

/**
 * 测试用例：调用函数成功
 * 前置条件：输入正常
 * Check点：无异常抛出
 */
TEST_F(KubernetesProtectEngineTest, NonTrivalFunctionTest) {
    std::vector<ApplicationResource> appResourceList;
    std::vector<Application> appList;
    Application app;
    ApplicationEnvironment env;
    ApplicationResource appResource;

    Json::Value testJsonVal;
    int32_t retInt;
    VMInfo vmInfo;
    retInt = m_k8sRestoreProtectEngineHandler->RenameMachine(vmInfo, "abc");
    EXPECT_EQ(retInt, Module::SUCCESS);
    retInt = m_k8sRestoreProtectEngineHandler->DeleteMachine(vmInfo);
    EXPECT_EQ(retInt, Module::SUCCESS);
    retInt = m_k8sRestoreProtectEngineHandler->CreateMachine(vmInfo);
    EXPECT_EQ(retInt, Module::SUCCESS);

    VolInfo volInfo;
    retInt = m_k8sRestoreProtectEngineHandler->ReplaceVolume(volInfo);
    EXPECT_EQ(retInt, Module::SUCCESS);
    retInt = m_k8sRestoreProtectEngineHandler->DeleteVolume(volInfo);
    EXPECT_EQ(retInt, Module::SUCCESS);
    retInt = m_k8sRestoreProtectEngineHandler->DetachVolume(volInfo);
    EXPECT_EQ(retInt, Module::SUCCESS);
    retInt = m_k8sRestoreProtectEngineHandler->AttachVolume(volInfo);
    EXPECT_EQ(retInt, Module::SUCCESS);
    DatastoreInfo datastoreInfo;
    retInt = m_k8sRestoreProtectEngineHandler->CreateVolume(volInfo, "abc", "vminfo", datastoreInfo, volInfo);
    EXPECT_EQ(retInt, Module::SUCCESS);
    std::unordered_map<std::string, std::string> testMap;
    retInt = m_k8sRestoreProtectEngineHandler->GetVolumesMetadata(vmInfo, testMap);
    EXPECT_EQ(retInt, Module::SUCCESS);

    m_k8sRestoreProtectEngineHandler->DiscoverHostCluster(env, env);
    m_k8sRestoreProtectEngineHandler->ListApplicationResource(appResourceList, env, app, appResource);
}

}