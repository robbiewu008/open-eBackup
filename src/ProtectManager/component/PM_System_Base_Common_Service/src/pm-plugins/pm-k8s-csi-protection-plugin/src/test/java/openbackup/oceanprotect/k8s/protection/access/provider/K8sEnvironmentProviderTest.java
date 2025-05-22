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
package openbackup.oceanprotect.k8s.protection.access.provider;

import static org.mockito.ArgumentMatchers.any;
import static org.mockito.ArgumentMatchers.anyBoolean;
import static org.mockito.ArgumentMatchers.anyInt;
import static org.mockito.ArgumentMatchers.anyMap;

import openbackup.access.framework.resource.service.provider.UnifiedConnectionCheckProvider;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.BrowseEnvironmentResourceConditions;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.data.protection.access.provider.sdk.resource.ResourceExtendInfoService;
import openbackup.data.protection.access.provider.sdk.resource.ResourceService;
import openbackup.oceanprotect.k8s.protection.access.common.K8sQueryParam;
import openbackup.oceanprotect.k8s.protection.access.constant.K8sConstant;
import openbackup.oceanprotect.k8s.protection.access.constant.K8sExtendInfoKey;
import openbackup.oceanprotect.k8s.protection.access.provider.K8sEnvironmentProvider;
import openbackup.oceanprotect.k8s.protection.access.service.K8sCommonService;
import openbackup.system.base.common.constants.CommonErrorCode;
import openbackup.system.base.common.exception.LegoCheckedException;
import openbackup.system.base.common.scurity.TokenVerificationService;
import openbackup.system.base.common.utils.json.JsonUtil;
import openbackup.system.base.sdk.resource.enums.LinkStatusEnum;
import openbackup.system.base.sdk.resource.model.ResourceSubTypeEnum;
import openbackup.system.base.util.MessageTemplate;

import org.junit.Assert;
import org.junit.Test;
import org.mockito.Mockito;

import java.util.ArrayList;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Optional;

/**
 * 功能描述: K8sEnvironmentProviderTest
 *
 */
public class K8sEnvironmentProviderTest {
    private static final int K8S_CLUSTER_MAX_COUNT = 8;

    private final ResourceService resourceService = Mockito.mock(ResourceService.class);
    private final K8sCommonService commonService = Mockito.mock(K8sCommonService.class);
    private final MessageTemplate messageTemplate = Mockito.mock(MessageTemplate.class);
    private final ResourceExtendInfoService resourceExtendInfoService = Mockito.mock(ResourceExtendInfoService.class);
    private final UnifiedConnectionCheckProvider unifiedConnectionCheckProvider = Mockito
            .mock(UnifiedConnectionCheckProvider.class);
    private final TokenVerificationService tokenVerificationService = Mockito
            .mock(TokenVerificationService.class);
    private final K8sEnvironmentProvider provider = new K8sEnvironmentProvider(resourceService, commonService,
            messageTemplate, resourceExtendInfoService, unifiedConnectionCheckProvider, tokenVerificationService);

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: 参数检查正确，无异常抛出
     */
    @Test
    public void test_check_success() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        AppEnvResponse appEnvResponse = mockClusterInfo();
        Mockito.when(commonService.queryClusterInfo(any())).thenReturn(appEnvResponse);
        provider.register(k8sCluster);
        Mockito.verify(commonService, Mockito.times(1)).checkConnectivity(k8sCluster);
        Assert.assertEquals(k8sCluster.getEndpoint(), k8sCluster.getPath());
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), k8sCluster.getLinkStatus());
    }

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: timeout为空，直接返回
     */
    @Test
    public void check_timeout_is_null_and_return() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getExtendInfo().put(K8sExtendInfoKey.TASK_TIMEOUT, null);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        AppEnvResponse appEnvResponse = mockClusterInfo();
        Mockito.when(commonService.queryClusterInfo(any())).thenReturn(appEnvResponse);
        provider.register(k8sCluster);
        Mockito.verify(commonService, Mockito.times(1)).checkConnectivity(k8sCluster);
        Assert.assertEquals(k8sCluster.getEndpoint(), k8sCluster.getPath());
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), k8sCluster.getLinkStatus());
    }

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: 抛出参数错误异常
     */
    @Test
    public void should_throw_exception_when_check_timeout_is_illegal() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getExtendInfo().put(K8sExtendInfoKey.TASK_TIMEOUT, "{'days': 1, 'hours': 0.23, 'minutes': 0, " +
                "'seconds': 1}");
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        AppEnvResponse appEnvResponse = mockClusterInfo();
        Mockito.when(commonService.queryClusterInfo(any())).thenReturn(appEnvResponse);
        Assert.assertThrows("k8s TASK_TIMEOUT is illegal", LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        k8sCluster.getExtendInfo().put(K8sExtendInfoKey.TASK_TIMEOUT, "{'days': 1, 'hours': 90, 'minutes': 0, " +
                "'seconds': 1}");
        Assert.assertThrows("k8s TASK_TIMEOUT is illegal", LegoCheckedException.class,
                () -> provider.register(k8sCluster));
    }


    /**
     * 用例场景：注册流程检查K8S集群参数JobNumOnSingleNode
     * 前置条件：无
     * 检查点: 参数检查正确，无异常抛出
     */
    @Test
    public void test_check_job_num_on_single_node_success() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.setExtendInfoByKey(K8sExtendInfoKey.JOB_NUM_ON_SINGLE_NODE, "2");
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        AppEnvResponse appEnvResponse = mockClusterInfo();
        Mockito.when(commonService.queryClusterInfo(any())).thenReturn(appEnvResponse);
        provider.register(k8sCluster);
        Mockito.verify(commonService, Mockito.times(1)).checkConnectivity(k8sCluster);
        Assert.assertEquals(k8sCluster.getEndpoint(), k8sCluster.getPath());
        Assert.assertEquals(LinkStatusEnum.ONLINE.getStatus().toString(), k8sCluster.getLinkStatus());
    }

    /**
     * 用例场景：注册流程检查K8S集群参数JobNumOnSingleNode
     * 前置条件：无
     * 检查点: 参数检查失败，异常抛出
     */
    @Test
    public void test_check_job_num_on_single_node_error() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.setExtendInfoByKey(K8sExtendInfoKey.JOB_NUM_ON_SINGLE_NODE, "9");
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
        k8sCluster.setExtendInfoByKey(K8sExtendInfoKey.JOB_NUM_ON_SINGLE_NODE, "aaa");
        exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
    }

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: Config文件内参数检查正确，无异常抛出
     */
    @Test
    public void test_check_config_success() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getAuth().setAuthType(Authentication.OTHER);
        Map<String, String> extenInfo = new HashMap<>();
        extenInfo.put(K8sExtendInfoKey.CONFIG, "YXBpVmVyc2lvbjogdjEKa2luZDogQ29uZmlnCnByZWZlcmVuY2VzOiB7fQpjbHVzd" +
                "GVyczoKICAtIGNsdXN0ZXI6CiAgICAgIGNlcnRpZmljYXRlLWF1dGhvcml0eS1kYXRhOiBMUzB0TFMxQ1JVZEpUaUJEUlZKVVNVW" +
                "kpRMEZVUlMwdExTMHRDazFKU1VRdmFrTkRRVzFoWjBGM1NVSkJaMGxDUVVSQlRrSm5hM0ZvYTJsSE9YY3dRa0ZSYzBaQlJFRldUV" +
                "kpOZDBWUldVUldVVkZFUlhkd2NtUlhTbXdLWTIwMWJHUkhWbnBOUWpSWVJGUkplazFFWTNkT2FrRTFUV3BKZUU1V2IxaEVWRTE2V" +
                "FVSamQwMTZRVFZOYWtsNFRsWnZkMFpVUlZSTlFrVkhRVEZWUlFwQmVFMUxZVE5XYVZwWVNuVmFXRkpzWTNwRFEwRmhTWGRFVVZsS" +
                "1MyOWFTV2gyWTA1QlVVVkNRbEZCUkdkblIxQkJSRU5EUVZsdlEyZG5SMEpCVFRobENrUnJaREJ0Ynl0VVRUWlVZMWRUZFdoeGJVe" +
                "GljbWRFU0U5NE9XOVplVXhRWjJsVGJGQlNWMUJwZFZCc2JVSkRSakUwYldObFpuRllUMFJuUzFacGNqSUtkVll5V1RsWmNtOTNTV" +
                "mRPVGtweFVEZGhaVkJrU1daa2MyTXZSVTV4V2pSdWVrdzVUSGx0Ym1SYWVEaENMMVI0YmxKVU1HazFhMmRpZGpWSU5VTlRSQXBqU" +
                "VRSNGNFVnRjRkI0TTJoWU9GcG9Lek41UTFsWFlsSmhiMlI1VWxsdFRsWnVZVVp5WjBoNGRFcHRTakpHUlhSQ1kydG1OVkoxWVVsM" +
                "1dGQnhjV1ZLQ2pseVlrTlNUVVpYVmpCWmQxUTRXWGswWlVWVGFWVXhNVkpXVjA4M2NFWTNWMjVrWjFORU5GcHVjR0pzY2k4eldHU" +
                "nJUSFpRTkdOR09XMDFVR2xIYVZvS2RXeE5lUzlyTWtWUlJuaFZXRUptVVZaaFZUQk1VMk52WTNadlJ6TkVabFZxZWxCNGRXSmlNa" +
                "mxpYTNSU1dXVkhka3hxVlUxUVVEZHllbmsxYWpCTFJnb3JkbTAxYkhscWNIUlZVVzR4TVhsS1N6Qm9LM1lyYVZCelNYWm1SMWhhW" +
                "W5jNVdrSjRPVEZtY21RMVlUaE5jVUV3U25FNFRVOTFaVkpMVm5sUFIwOWpDbXBxWlhwQ1ZESTRjM0V3UjJoQ04ydEZhbHBsVG5Gd" +
                "2JVdzBSRlJDYTBsVGEzWkpNVFp0YTJWUldXODBiREZNTTBkaldHOWtaR0ZYTUVKa09YbDFVVGNLVUd0WmNVcFdNRlpFVTFReldIR" +
                "nFSR1ZVU2xWTVYyYzRkRXhuU1dFeU16QkZSMnRPT1RKbGVXRnVaMlpWWkdreE9XcFBSWGhsSzBaeldtUmlOVkZKUkFwQlVVRkNie" +
                "kZyZDFaNlFVOUNaMDVXU0ZFNFFrRm1PRVZDUVUxRFFYRlJkMFIzV1VSV1VqQlVRVkZJTDBKQlZYZEJkMFZDTDNwQlpFSm5UbFpJV" +
                "VRSRkNrWm5VVlZ3U0ZselRtSklLM0kzU0dSRFFqQXhOR1JOTkVjNVQyNVhhVzkzUmxGWlJGWlNNRkpDUVRSM1JFbEpTMkV6Vm1sY" +
                "VdFcDFXbGhTYkdONlFVNEtRbWRyY1docmFVYzVkekJDUVZGelJrRkJUME5CV1VWQldEZDBPVVJVWlZjNU15dDJUV052Tld4RWRua" +
                "ENWblpMTVN0dk5sVkdUWFp1WkV4dmVITXpWQXAyTVRoTU5tcHZOVzFOUVVSdE5EbFFTbWxKYm10MFQwdFNWbWRJWVdkR2NFSnFWR" +
                "nBHZG1kRVRrRlFkR1J4V0VWUFNFcHZNa1ZpVkdOT2VXSm1kVmxRQ2k5dU0ydFNaRTVsVFZKbVJHaDZVMUZqWWxKNFIwVlVkRXB0V" +
                "EdSc2FDc3JTMWsxVjJkb056ZFRVM013WW5scmJtbGhOMnd4ZFdGc2FYcFVUSHB3VWtVS2ExZHFXakpNUlM4eEwyWlFOVlZaVTAwM" +
                "2FYTjVUR0pLUlcxSVNGWXJTell3U20xdmRXdFlkRTFwYVVOSWVFWk5kV040UW1sTGVqZHJORTkwWmt3d1ZncHBWbmhuYkdkUk5tU" +
                "jZPRXBFTVRCM05EbG9iemxDVFRSWFoySm1NelpHVkdaNE4xRllVRTVPTW1OV1pWbDFWMmxzYjBZME9GQXlNMVpHY1dacUt6VXhDb" +
                "TR6VlNzNE1rWmpWRmMyUjI1NFJ6bHdXR2c1ZDJaUGExSkVZVEEwZW5KelNYaFBjVnBrTWt4M1ZWcFZiRk5CZUVkNFVXVTVReTl1V" +
                "lRGak9HcHdiRUlLUjBsSWFsWklURUZoYVRKbmNraDNPR05LUjIxSlZreEJTMnhUT0hkMFJtazBZeTl4ZUZvckt5dEJia2hVY1doT" +
                "U1TdHdkRk50ZURCdVJWSlJSbXRaWndwemFsVjJWRWQzVEVOd1QycDJMMEZMTVVsbFoxaGhiWFkwY1ZkT2RtOVRZa0ZxUTJObVZHa" +
                "E5iR0poYWtWSVRXODNjSEJYTVU1UFlWQXpSSE5CUWs5ckNrUnRRVGRNYW1OMmRtTlJMekkyVjJoS01sRjRSalpSZEFvdExTMHRMV" +
                "VZPUkNCRFJWSlVTVVpKUTBGVVJTMHRMUzB0Q2c9PQogICAgICBzZXJ2ZXI6IGh0dHBzOi8vOC40MC4xNDUuMTMzOjE2NDQzCiAgI" +
                "CBuYW1lOiBzb2Z0LXdhcmUtZGV2CmNvbnRleHRzOgogIC0gY29udGV4dDoKICAgICAgY2x1c3Rlcjogc29mdC13YXJlLWRldgogI" +
                "CAgICB1c2VyOiBzeXNhZG1pbgogICAgbmFtZTogc3lzYWRtaW5Ac29mdC13YXJlLWRldgpjdXJyZW50LWNvbnRleHQ6IHN5c2Fkb" +
                "WluQHNvZnQtd2FyZS1kZXYKdXNlcnM6CiAgLSBuYW1lOiBzeXNhZG1pbgogICAgdXNlcjoKICAgICAgdG9rZW46IGtybTpBQUFBQ" +
                "VFBQUFBRUFBQUFBQUFBQUJ3QUFBQUlBQUFBQkIyUDBxMlBVeTIzcHIzNytPaTJJd2t4M0pyRGlLQXJNTytqczYvTDhDakVBQUFCZ" +
                "0FBQUFBQUFBQUFCVFUzUWlZdXNiaWhzT0hnM1VoM0lIMzRYRGdadTE5VVZpVXBLdERwUThqd09jU2IvRUQwd0EvTEV4VXNRbW1lO" +
                "Tk2eW8vS0ducmFJV2tmOXRSbk83VGxGYWxpZExGK20yeEVEQnlqZ3p1NTJ2ZUNGcHN6bjNQazhrZ3VjMmpUcWdBQUFBQkFBQUFBQ" +
                "UFBQ0FRQUFBQUNBQUFBQVViUWY0c2ZvVGF0YzhaZFhhR0M5WTBBQUFBQUFBQUFBRHUzSVlpdTk1MkZ6VlREVG8wNHVDdEpDdTlhS" +
                "nkrcnVCMmZGeUhpM2lFMQ==");

        k8sCluster.getAuth().setExtendInfo(extenInfo);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        AppEnvResponse appEnvResponse = mockClusterInfo ();
        Mockito.when(commonService.queryClusterInfo(any())).thenReturn(appEnvResponse);
        provider.register(k8sCluster);
        Mockito.verify(commonService, Mockito.times(1)).checkConnectivity(k8sCluster);
    }

    /**
     * 用例场景：注册流程检查K8S集群参数,CCE存在内网外网区别，文件内可能会有两个以上ip
     * 前置条件：无
     * 检查点: Config文件内参数检查正确，无异常抛出
     */
    @Test
    public void test_check_config_for_CCE_success() {
        String kubeconfigs = "eyJraW5kIjoiQ29uZmlnIiwiYXBpVmVyc2lvbiI6InYxIiwicHJlZmVyZW5jZXMiOnt9LCJjbHVzdGVycyI6W3s" +
                "ibmFtZSI6ImludGVybmFsQ2x1c3RlciIsImNsdXN0ZXIiOnsic2VydmVyIjoiaHR0cHM6Ly8xMC4xMC4xMDcuMjA2OjU0NDMiLCJ" +
                "jZXJ0aWZpY2F0ZS1hdXRob3JpdHktZGF0YSI6IkxTMHRMUzFDUlVkSlRpQkRSVkpVU1VaSlEwRlVSUzB0TFMwdENrMUpTVVJFZWt" +
                "ORFFXWmxaMEYzU1VKQlowbENRVVJCVGtKbmEzRm9hMmxIT1hjd1FrRlJjMFpCUkVGd1RWSnJkMFozV1VSV1VWRkxSWGhDUkZFd1Z" +
                "XY0tWa2RXYW1GSE5YWmlSemx1WVZkV2VrMVJkM2REWjFsRVZsRlJSRVYzVGtSUk1GVjNTR2hqVGsxcVRYZFBWRUV6VFVSUmVVOVV" +
                "RWGxYYUdOT1RrUk5kd3BQVkVFelRVUlJlVTlVUVhsWGFrRndUVkpyZDBaM1dVUldVVkZMUlhoQ1JGRXdWV2RXUjFacVlVYzFkbUp" +
                "IT1c1aFYxWjZUVkYzZDBObldVUldVVkZFQ2tWM1RrUlJNRlYzWjJkRmFVMUJNRWREVTNGSFUwbGlNMFJSUlVKQlVWVkJRVFJKUWt" +
                "SM1FYZG5aMFZMUVc5SlFrRlJSRTVoWWs1NmJESlhVRU5GSzFZS1JrWkRiWFprVEZKcFdsUlZRbXA1UjJNMmFVVldRemw2WlZGbVY" +
                "zVTJRV2xOU21FclJHdEpORGRvU0RReFNWTTNUMVoxYkVSMGRFbHVjbWhDUkhVNE53b3dSR3hqZUhSWVUxSTRabU5TTlZoTWFVRlZ" +
                "SM2htTTBoSFRVTjNaMmwzV2pSV1lVVlhMMDVsVWk5Q00wVXZieXR0YjBNNE5EVlNSalJ6TlRjM1JFeGlDbkJwYm01alJtNDJhRmx" +
                "uUzJnd1VqbGhhVXgxVVRSVlJFaEdRbEJ6TkhGS05XVm5OMDFCUzJwSWRFWkZhMU5ETlRRck5WRnpTa0ZUUzBkWlFuTmpWaklLYm5" +
                "oNFZrZzNUMEkwZG1vcll6VlVaVlZESzNCNE0zRkRXRlV6U1dWTlIwbE1WSEJJTUVaWmVESkNTVTVaTWt4SVVXNWFTbGxMVlU1UFp" +
                "uWnJZVGRHVHdvemNsZHNORGRaUzNGQksyOU1jVGgyY2tWS2EzRjJjbEZKWVV0RlYwMW9PRzl2UkVadFMxSjRTR1JXYmxoMFQybFh" +
                "aV1ZWTWtsbVNHaGhiR2s0T1ZSSkNuZFJlams0TUhGaVFXZE5Ra0ZCUjJwUmFrSkJUVUUwUjBFeFZXUkVkMFZDTDNkUlJVRjNTVU5" +
                "3UkVGUVFtZE9Wa2hTVFVKQlpqaEZRbFJCUkVGUlNDOEtUVUl3UjBFeFZXUkVaMUZYUWtKVVNrc3pia3RaSzAxTFFrdFJlREYwVFR" +
                "odVVESXJjblk0Wm1sNlFVNUNaMnR4YUd0cFJ6bDNNRUpCVVhOR1FVRlBRd3BCVVVWQllqTnJOa2QyY0RkVE9ETllOVmxQVFZGT2R" +
                "FUTFRVUZIUTNkVk5ubDJhelZKVXlzckwwWm9iVVptVVZKVFNHbzVUR0ZSVlRFNFIyeElVRmhFQ213eVVHeFRNa0Y0Y3k5bGRtWnp" +
                "XV1JNY2xoT2IyTXdZV3BHVVcxU05tVlpZVTlEVFZkRlZGZDFkazRyYW05NllURmhVRFZ2UlhSbWFXa3hOMlpxUmpVS1EwSjZWR3h" +
                "JUlVoRVRIaHpLMG8yTTBsTWFub3JURmN4UmxFelYyUmtaek54VjI4clNHRkdUbEZGU1ZCakx6azBVMFEwTUZCaEsyTkRkSGcxY25" +
                "CeFdnbzJaWEE1UVRNNWJUWlBNazgyVVdodmVHNXZia2gwTURObFIzbFdiMEk1YlN0c1kyaG9SRTR5YlZwVFQzZFdNSFpOTUd0bks" +
                "yeDZTVTV5ZWxRd1NXeFBDa0lyZW1SaVVYRXlUMVZpVEdOSlYxbENia3R0WkZWbU9FOVhOelp4UTB4dWJFNTZUQzlMWkhGNWRGa3l" +
                "TSGt5UkhFMGJHWlJiM3BLZUU5RmFWWnZMMVVLUTJkTFNIRktZMjVRVWpNME1UUXJlbTB3V25GYVJXWkZNVUU5UFFvdExTMHRMVVZ" +
                "PUkNCRFJWSlVTVVpKUTBGVVJTMHRMUzB0Q2c9PSJ9fSx7Im5hbWUiOiJleHRlcm5hbENsdXN0ZXIiLCJjbHVzdGVyIjp7InNlcnZ" +
                "lciI6Imh0dHBzOi8vMzMuNTAuMTAwLjE1Njo1NDQzIiwiaW5zZWN1cmUtc2tpcC10bHMtdmVyaWZ5Ijp0cnVlfX0seyJuYW1lIjo" +
                "iZXh0ZXJuYWxDbHVzdGVyVExTVmVyaWZ5IiwiY2x1c3RlciI6eyJzZXJ2ZXIiOiJodHRwczovLzMzLjUwLjEwMC4xNTY6NTQ0MyI" +
                "sImNlcnRpZmljYXRlLWF1dGhvcml0eS1kYXRhIjoiTFMwdExTMUNSVWRKVGlCRFJWSlVTVVpKUTBGVVJTMHRMUzB0Q2sxSlNVUkV" +
                "la05EUVdabFowRjNTVUpCWjBsQ1FVUkJUa0puYTNGb2EybEhPWGN3UWtGUmMwWkJSRUZ3VFZKcmQwWjNXVVJXVVZGTFJYaENSRkV" +
                "3VldjS1ZrZFdhbUZITlhaaVJ6bHVZVmRXZWsxUmQzZERaMWxFVmxGUlJFVjNUa1JSTUZWM1NHaGpUazFxVFhkUFZFRXpUVVJSZVU" +
                "5VVFYbFhhR05PVGtSTmR3cFBWRUV6VFVSUmVVOVVRWGxYYWtGd1RWSnJkMFozV1VSV1VWRkxSWGhDUkZFd1ZXZFdSMVpxWVVjMWR" +
                "tSkhPVzVoVjFaNlRWRjNkME5uV1VSV1VWRkVDa1YzVGtSUk1GVjNaMmRGYVUxQk1FZERVM0ZIVTBsaU0wUlJSVUpCVVZWQlFUUkp" +
                "Ra1IzUVhkblowVkxRVzlKUWtGUlJFNWhZazU2YkRKWFVFTkZLMVlLUmtaRGJYWmtURkpwV2xSVlFtcDVSMk0yYVVWV1F6bDZaVkZ" +
                "tVjNVMlFXbE5TbUVyUkd0Sk5EZG9TRFF4U1ZNM1QxWjFiRVIwZEVsdWNtaENSSFU0Tndvd1JHeGplSFJZVTFJNFptTlNOVmhNYVV" +
                "GVlIzaG1NMGhIVFVOM1oybDNXalJXWVVWWEwwNWxVaTlDTTBVdmJ5dHRiME00TkRWU1JqUnpOVGMzUkV4aUNuQnBibTVqUm00MmF" +
                "GbG5TMmd3VWpsaGFVeDFVVFJWUkVoR1FsQnpOSEZLTldWbk4wMUJTMnBJZEVaRmExTkROVFFyTlZGelNrRlRTMGRaUW5OalZqSUt" +
                "ibmg0VmtnM1QwSTBkbW9yWXpWVVpWVkRLM0I0TTNGRFdGVXpTV1ZOUjBsTVZIQklNRVpaZURKQ1NVNVpNa3hJVVc1YVNsbExWVTV" +
                "QWm5acllUZEdUd296Y2xkc05EZFpTM0ZCSzI5TWNUaDJja1ZLYTNGMmNsRkpZVXRGVjAxb09HOXZSRVp0UzFKNFNHUldibGgwVDJ" +
                "sWFpXVlZNa2xtU0doaGJHazRPVlJKQ25kUmVqazRNSEZpUVdkTlFrRkJSMnBSYWtKQlRVRTBSMEV4VldSRWQwVkNMM2RSUlVGM1N" +
                "VTndSRUZRUW1kT1ZraFNUVUpCWmpoRlFsUkJSRUZSU0M4S1RVSXdSMEV4VldSRVoxRlhRa0pVU2tzemJrdFpLMDFMUWt0UmVERjB" +
                "UVGh1VURJcmNuWTRabWw2UVU1Q1oydHhhR3RwUnpsM01FSkJVWE5HUVVGUFF3cEJVVVZCWWpOck5rZDJjRGRUT0ROWU5WbFBUVkZ" +
                "PZEVRMVFVRkhRM2RWTm5sMmF6VkpVeXNyTDBab2JVWm1VVkpUU0dvNVRHRlJWVEU0UjJ4SVVGaEVDbXd5VUd4VE1rRjRjeTlsZG1" +
                "aeldXUk1jbGhPYjJNd1lXcEdVVzFTTm1WWllVOURUVmRGVkZkMWRrNHJhbTk2WVRGaFVEVnZSWFJtYVdreE4yWnFSalVLUTBKNlZ" +
                "HeElSVWhFVEhoekswbzJNMGxNYW5vclRGY3hSbEV6VjJSa1p6TnhWMjhyU0dGR1RsRkZTVkJqTHprMFUwUTBNRkJoSzJORGRIZzF" +
                "jbkJ4V2dvMlpYQTVRVE01YlRaUE1rODJVV2h2ZUc1dmJraDBNRE5sUjNsV2IwSTViU3RzWTJob1JFNHliVnBUVDNkV01IWk5NR3R" +
                "uSzJ4NlNVNXllbFF3U1d4UENrSXJlbVJpVVhFeVQxVmlUR05KVjFsQ2JrdHRaRlZtT0U5WE56WnhRMHh1YkU1NlRDOUxaSEY1ZEZ" +
                "reVNIa3lSSEUwYkdaUmIzcEtlRTlGYVZadkwxVUtRMmRMU0hGS1kyNVFVak0wTVRRcmVtMHdXbkZhUldaRk1VRTlQUW90TFMwdEx" +
                "VVk9SQ0JEUlZKVVNVWkpRMEZVUlMwdExTMHRDZz09In19XSwidXNlcnMiOlt7Im5hbWUiOiJ1c2VyIiwidXNlciI6eyJjbGllbnQ" +
                "tY2VydGlmaWNhdGUtZGF0YSI6IkxTMHRMUzFDUlVkSlRpQkRSVkpVU1VaSlEwRlVSUzB0TFMwdENrMUpTVVIwUkVORFFYQjVaMEY" +
                "zU1VKQlowbEpWMVo1VEZRMllXNDVTVVYzUkZGWlNrdHZXa2xvZG1OT1FWRkZURUpSUVhkTFZFVmFUVUpqUjBFeFZVVUtRMmhOVVZ" +
                "Fd1RrWkpSbEpzV1RKb2RXSXllSFphTW14c1kzcEZUVTFCYjBkQk1WVkZRWGhOUkZFd1RrWk5RalJZUkZSSmVrMUVhM2xOUkVGNlR" +
                "rUkZkd3BQUm05WVJGUkpNRTFFYTNoUFZFRjZUa1JGZDA5R2IzZG5ZMEY0WjFwSmQwWlJXVVJXVVZGTFJYYzFlbVZZVGpCYVZ6QTJ" +
                "ZbGRHZW1SSFZubGpla0Z1Q2tKblRsWkNRVzlVU1VSUk5GcFVSWGxOZWxwb1RXcGpORTVxVVhoT01scHBUbXBrYVUxWFRYaE5SMFV" +
                "3V1cxR2JWbFVhRzFOUTJOSFFURlZSVU5vVFdjS1RrUnJlbHBxYXpKWk1rVjNUbXBaZDA1RVFUSlpNa1V3VFhwb2JGbFhWbWhPUjB" +
                "wcFRrUldiVTlFUlhkS2QxbEVWbEZSUzBWNVFtMU9iVTEzVG1wc2F3cGFWRnByVGtkUk1FNXRSWGhQVkUwMFdsUnNhMDFxWXpGTmF" +
                "sRXdUbTFGTlZwcVJYQk5RMk5IUVRGVlJVRjRUV2RPZW1OM1QwUlZOVTE2VW1sT1ZFcHJDazVFWjNwT1YwcHRUVWRTYTA5RVl6Vk9" +
                "SRTB4VFcxUk1VMXFaM2RuWjBWcFRVRXdSME5UY1VkVFNXSXpSRkZGUWtGUlZVRkJORWxDUkhkQmQyZG5SVXNLUVc5SlFrRlJSRTV" +
                "oWWs1NmJESlhVRU5GSzFaR1JrTnRkbVJNVW1sYVZGVkNhbmxIWXpacFJWWkRPWHBsVVdaWGRUWkJhVTFLWVN0RWEwazBOMmhJTkF" +
                "veFNWTTNUMVoxYkVSMGRFbHVjbWhDUkhVNE56QkViR040ZEZoVFVqaG1ZMUkxV0V4cFFWVkhlR1l6U0VkTlEzZG5hWGRhTkZaaFJ" +
                "WY3ZUbVZTTDBJekNrVXZieXR0YjBNNE5EVlNSalJ6TlRjM1JFeGljR2x1Ym1OR2JqWm9XV2RMYURCU09XRnBUSFZSTkZWRVNFWkN" +
                "VSE0wY1VvMVpXYzNUVUZMYWtoMFJrVUthMU5ETlRRck5WRnpTa0ZUUzBkWlFuTmpWakp1ZUhoV1NEZFBRalIyYWl0ak5WUmxWVU1" +
                "yY0hnemNVTllWVE5KWlUxSFNVeFVjRWd3UmxsNE1rSkpUZ3BaTWt4SVVXNWFTbGxMVlU1UFpuWnJZVGRHVHpOeVYydzBOMWxMY1V" +
                "FcmIweHhPSFp5UlVwcmNYWnlVVWxoUzBWWFRXZzRiMjlFUm0xTFVuaElaRlp1Q2xoMFQybFhaV1ZWTWtsbVNHaGhiR2s0T1ZSSmQ" +
                "xRjZPVGd3Y1dKQlowMUNRVUZIYWxORVFrZE5RVFJIUVRGVlpFUjNSVUl2ZDFGRlFYZEpSbTlFUVZRS1FtZE9Wa2hUVlVWRVJFRkx" +
                "RbWRuY2tKblJVWkNVV05FUVdwQlprSm5UbFpJVTAxRlIwUkJWMmRDVkVwTE0yNUxXU3ROUzBKTFVYZ3hkRTA0YmxBeUt3cHlkamh" +
                "tYVhwQlRrSm5hM0ZvYTJsSE9YY3dRa0ZSYzBaQlFVOURRVkZGUVVoWmJESkVhRzg0YTA5c1NXUjFlR3RrUm5sNVJqSkRjVXg2UjF" +
                "BMVl5dFNDbEV4TTA4elYwWmpOVEkyVmtSUFZERlVSalJXUlN0SWNHVnpkU3RqYVhnMmNuWjNVMFZuWXpsRFVVd3hXbHA2ZFdWcU1" +
                "rMXBhbm94V21vMmJXVlFiQzhLZHpCU05YZFpXRU00ZVhsalowOTNOazk1VUZKVVYwZFNSbU52VFhOVE5XNXROVkJ6TkRCYVFVeEx" +
                "SazlOUzA5TVExSndUR3M0VlRCRFRHSm5ZVmxpTmdvek5tOXFOMlkzTkVKVlYxUkJORVp5VjNSYVRUbGxWWEpDTURsRFRrdERkamx" +
                "KVW14NVJHbDNOa2QxUzNVdk5FZFZkMk5zVTNoR2VXTjFlaTlpUm5CTUNrSnFUazlxUTFablRsRmlkVlZoTkdSek5GaERaMUJYWWx" +
                "KeGEwRXZVREZFWW5kMVNUbFFVbWxwV2xkc2FtbDBZWEYyVnk5aFVGUXJVRmxwTDBSNFNYY0tSMVZwY1dnMVZFaERUbE5OYlhKNUt" +
                "6UlBjMGxuYzNaSldVcFFaelp2UW1JM1QzaENkMUJCZUcxV05IRlROV2hzU25wV1EzbG5QVDBLTFMwdExTMUZUa1FnUTBWU1ZFbEd" +
                "TVU5CVkVVdExTMHRMUW89IiwiY2xpZW50LWtleS1kYXRhIjoiTFMwdExTMUNSVWRKVGlCU1UwRWdVRkpKVmtGVVJTQkxSVmt0TFM" +
                "wdExRcE5TVWxGY0VGSlFrRkJTME5CVVVWQmVsZHRlbU0xWkd4cWQyaFFiRkpTVVhCeU0xTXdXVzFWTVVGWk9HaHVUMjlvUmxGMll" +
                "6TnJTREZ5ZFdkSmFrTlhDblpuTlVOUFR6UlNLMDVUUlhWNmJHSndVVGRpVTBvMk5GRlJOM1pQT1VFMVdFMWlWakJyWmtnelJXVld" +
                "lVFJuUmtKeldEbDRlR3BCYzBsSmMwZGxSbGNLYUVaMmVsaHJabmRrZUZBMlVIQnhRWFpQVDFWU1pVeFBaU3QzZVRJMldYQTFNMEp" +
                "hSzI5WFNVTnZaRVZtVjI5cE4ydFBSa0Y0ZUZGVU4wOUxhV1ZZYndwUGVrRkRiM2czVWxKS1JXZDFaVkIxVlV4RFVVVnBhRzFCWWt" +
                "oR1pIQTRZMVpTSzNwblpVdzBMMjVQVlROc1FYWnhZMlEyWjJ3eFRubElha0pwUXpBMkNsSTVRbGROWkdkVFJGZE9hWGd3U2pKVFY" +
                "wTnNSRlJ1TnpWSGRYaFVkRFl4Y0dWUE1rTnhaMUJ4UXpaMlREWjRRMXBMY2pZd1EwZHBhRVpxU1daTFMwRUtlRnBwYTJOU00xWmF" +
                "NVGRVYjJ4dWJteE9hVWg0TkZkd1dYWlFWWGxOUlUwdlprNUxiWGRKUkVGUlFVSkJiMGxDUVVoalNGRlJNVU5PV0RsclpWaHJVZ3B" +
                "vU1hGaGFFbGpXRU5xTm5sUWJrSkxjRTlRU2xKS1dFNUdSbGhDT0RGRVkyaDJNSEZzWjFWWVRIbE1XbEYyUTJGc1FtaGFUelJuYUU" +
                "xUVVFTkROamRHQ2xGbVoxa3pPVFI0Tm1kWmFrWlNTVlJvTlZsa1RrdHJhVVp6V1hoTFFYb3pTRzlOYTBwVGIwVXhZbkZ4ZDA0NU1" +
                "TdENUamhpU0dKNWNtMVFkM2R4WlRBS05tRk9Tek14YW1vNU5tcHBTMVJZYWxGemNDdEtXakZtTUhOdVlsaFVhVU50VW14dlJtTXZ" +
                "Za0pRYkRsdGVYWmFWamhxV1VOcFZsSnpjekZwTWpkQ2FBcG5ZbXcyYjBVMlFsaHdRWFZST0Nzd2IzbGpVMFZ3ZFRoblZIUXpTM05" +
                "xUlVWSmEwbFBVSGxYWVRGTFdXWkNjalZLYWpBelRYUnRXRmxOVkdSTFJucG1Dak12V0c4eFExRkJOalpxUm1oNlZGTnFibEp1ZVR" +
                "ocE1WRm1SbXBTUlhZMlZEbHdSamRPV0cxNU5EbHlkRXhQT1c1M2NGbFRXRmxWWWtOUUwyWkhaQzhLY1dkc1pIazRSVU5uV1VWQk1" +
                "HOXdNMlpOVG00eWRtUlBZemx6YlVSWE1FeFZXa1pPZERGeFRraDFiVlZST1ZkTGRWWklUMGRVVVhKUFRVbHdiMDVxVFFvd1pHdEN" +
                "Ta3RqT0dkNmIyTXdTR3gzWjFVMGQwOVRZV1pKVDJOaGMxWXZVR3BvVDBKS2FtVm9VSHB4ZWs1cVZUTjJTazQyUjBGc05XMTViblY" +
                "1V2tSbENrRm5XbHBPU0doSmRHaDJhekpGV2tsT00zQklaVkZCVWpWNlZEZE1hVkZ1V0VWdlVuQnBNelJVT0VvclF6QldiVWREVEV" +
                "weWNHTkRaMWxGUVN0alVFb0tha2hKUVRWTVJXeHpOVEJSYWxOMGFGQnJNakJzVW5VMU9XdzFNRkpWVEZWMlNrZE9kVXRvY1V0emJ" +
                "sSjRaRGxIVkV3dlFXWXJPWEJUY2pZM1FqWmtOQXB3T1dodFkzcERUR28zT1U4MU1EbEJNbXg1WTJScFYwSkRPWE5WVm5neWVDOTF" +
                "iRU5qTTI1ck1rUkplblJaTTA5YVEwWlhaSGcxVFZobU1pOUpXRkpVQ21oUVdtczJaVkoxUWtZcmFqbExiMGR2U21ka2FFOW5MMjQ" +
                "yTVhWMVJVNURjbHAyTVdsS01FTm5XVVZCZEhZdmMwRXJTRVUzTm5CME5HNHpiR2RXTXpZS2NrcGthblZoT1ZoWmFuTmplVU5zWjF" +
                "CMGVFRkRLMEYzWjFaRmFUVmtRVXhzTkhKTVZqVlRWa2xIY25WcFRGcEdaRGs0UTFFeWNYbG1aVXhwWWt4SlpRcHphSFExT0d0SlF" +
                "WRm1heTkwUTNCc2JUa3JSVVJGYVZGRWNrcFFMelYxVUc1TGNuWTViemx0YUVGUFV6Qm1iV3MwTjFCQ1ZrRmpabWxLZFdkb0syRmp" +
                "Dak0zWW5oNVJWQkZlRmM0UlVvM0wxZHJZbmRNUjFaclEyZFpRbFJSU25sek9FMDRNV0UwY25GdVRqaEpabm8wZUhVd2MwaE1TeTl" +
                "ITTBkTllrNTRVMk1LUzJ4Uk5FTm5UV2hqYUZrelVUVTBNazVRUW1ka1NtWXZUSGxaSzJabFkzTTBNbVkyTWpKTFduSnhXVXBYYUZ" +
                "Jck9VMUhLMkVyTDJjeVRYbHNORk5OVlFwdFowaHdXbVZqVHpOWFZEVkViVk5QV1dOS2JHZEVZbkZYV1hSa1FtdzFWV2R4UkU5cFl" +
                "reFhVMFkzY0ZSalpGcDJibkZSWXpZelNtUjBSR2xKUzBaWkNrcDRWVzU2VVV0Q1oxRkRaVlZWUkhWMVpWcFdVVFp4U0c5dlQycEt" +
                "WRU4xY21WU2R6aExVa1lyVUhoV1FTOXZkMmwxU2taa04xTlVOelZ1ZW01bllTOEtjVWRxYnpnMWNHbFdiWGxaTlV0U05qSkVWemx" +
                "uWkZRNWVrODNabWhCTkd4RmNXMVdlRkJqVUVFM0wyUXdOWGRRT1ZKMlJpOW9iMGRUTW5KeFZIQnlVUXBxUjFWUVlXaEVjMndyZEh" +
                "nMU15OW5aUzh3T1dneVkxQTJkMk5KTTBsTGNGcElhR1pYVWl0TlZFMXFTa0ZWVjNNelRISktTMUU5UFFvdExTMHRMVVZPUkNCU1U" +
                "wRWdVRkpKVmtGVVJTQkxSVmt0TFMwdExRbz0ifX1dLCJjb250ZXh0cyI6W3sibmFtZSI6ImludGVybmFsIiwiY29udGV4dCI6eyJ" +
                "jbHVzdGVyIjoiaW50ZXJuYWxDbHVzdGVyIiwidXNlciI6InVzZXIifX0seyJuYW1lIjoiZXh0ZXJuYWwiLCJjb250ZXh0Ijp7ImN" +
                "sdXN0ZXIiOiJleHRlcm5hbENsdXN0ZXIiLCJ1c2VyIjoidXNlciJ9fSx7Im5hbWUiOiJleHRlcm5hbFRMU1ZlcmlmeSIsImNvbnR" +
                "leHQiOnsiY2x1c3RlciI6ImV4dGVybmFsQ2x1c3RlclRMU1ZlcmlmeSIsInVzZXIiOiJ1c2VyIn19XSwiY3VycmVudC1jb250ZXh" +
                "0IjoiZXh0ZXJuYWwifQ==";
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getAuth().setAuthType(Authentication.OTHER);
        Map<String, String> extenInfo = new HashMap<>();
        extenInfo.put(K8sExtendInfoKey.CONFIG,kubeconfigs);
        k8sCluster.getAuth().setExtendInfo(extenInfo);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        AppEnvResponse appEnvResponse = mockClusterInfo ();
        Mockito.when(commonService.queryClusterInfo(any())).thenReturn(appEnvResponse);
        provider.register(k8sCluster);
        Assert.assertTrue(true);
    }

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: Config文件内参数缺少端口参数，抛出异常
     */
    @Test
    public void test_check_cce_config_prot_is_null() {
        String config = getConfig();
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getAuth().setAuthType(Authentication.OTHER);
        Map<String, String> extenInfo = k8sCluster.getExtendInfo();
        extenInfo.put(K8sExtendInfoKey.CONFIG, config);
        k8sCluster.getAuth().setExtendInfo(extenInfo);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        Mockito.when(commonService.queryClusterInfo(any())).thenReturn(appEnvResponse);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.KUBE_CONFIG_ERROR, exception.getErrorCode());
    }

    private static String getConfig() {
        String config = "ew0KICAgICJraW5kIjogIkNvbmZpZyIsDQogICAgImFwaVZlcnNpb24iOiAidjEiLA0KICAgICJwcmVmZXJlbmNlcyI6I" +
                "Ht9LA0KICAgICJjbHVzdGVycyI6IFsNCiAgICAgICAgew0KICAgICAgICAgICAgIm5hbWUiOiAiaW50ZXJuYWxDbHVzdGVyIiwNC" +
                "iAgICAgICAgICAgICJjbHVzdGVyIjogew0KICAgICAgICAgICAgICAgICJzZXJ2ZXIiOiAiaHR0cHM6Ly8xMC4xMC4xMDcuMjA2O" +
                "jU0NDMiLA0KICAgICAgICAgICAgICAgICJjZXJ0aWZpY2F0ZS1hdXRob3JpdHktZGF0YSI6ICJMUzB0TFMxQ1JVZEpUaUJEUlZKV" +
                "VNVWkpRMEZVUlMwdExTMHRDazFKU1VSRWVrTkRRV1psWjBGM1NVSkJaMGxDUVVSQlRrSm5hM0ZvYTJsSE9YY3dRa0ZSYzBaQlJFR" +
                "ndUVkpyZDBaM1dVUldVVkZMUlhoQ1JGRXdWV2NLVmtkV2FtRkhOWFppUnpsdVlWZFdlazFSZDNkRFoxbEVWbEZSUkVWM1RrUlJNR" +
                "lYzU0doalRrMXFUWGRQVkVFelRVUlJlVTlVUVhsWGFHTk9Ua1JOZHdwUFZFRXpUVVJSZVU5VVFYbFhha0Z3VFZKcmQwWjNXVVJXV" +
                "VZGTFJYaENSRkV3VldkV1IxWnFZVWMxZG1KSE9XNWhWMVo2VFZGM2QwTm5XVVJXVVZGRUNrVjNUa1JSTUZWM1oyZEZhVTFCTUVkR" +
                "FUzRkhVMGxpTTBSUlJVSkJVVlZCUVRSSlFrUjNRWGRuWjBWTFFXOUpRa0ZSUkU1aFlrNTZiREpYVUVORksxWUtSa1pEYlhaa1RGS" +
                "nBXbFJWUW1wNVIyTTJhVVZXUXpsNlpWRm1WM1UyUVdsTlNtRXJSR3RKTkRkb1NEUXhTVk0zVDFaMWJFUjBkRWx1Y21oQ1JIVTROd" +
                "293Ukd4amVIUllVMUk0Wm1OU05WaE1hVUZWUjNobU0waEhUVU4zWjJsM1dqUldZVVZYTDA1bFVpOUNNMFV2Ynl0dGIwTTRORFZTU" +
                "mpSek5UYzNSRXhpQ25CcGJtNWpSbTQyYUZsblMyZ3dVamxoYVV4MVVUUlZSRWhHUWxCek5IRktOV1ZuTjAxQlMycElkRVpGYTFOR" +
                "E5UUXJOVkZ6U2tGVFMwZFpRbk5qVmpJS2JuaDRWa2czVDBJMGRtb3JZelZVWlZWREszQjRNM0ZEV0ZVelNXVk5SMGxNVkhCSU1FW" +
                "lplREpDU1U1Wk1reElVVzVhU2xsTFZVNVBablpyWVRkR1R3b3pjbGRzTkRkWlMzRkJLMjlNY1RoMmNrVkthM0YyY2xGSllVdEZWM" +
                "DFvT0c5dlJFWnRTMUo0U0dSV2JsaDBUMmxYWldWVk1rbG1TR2hoYkdrNE9WUkpDbmRSZWprNE1IRmlRV2ROUWtGQlIycFJha0pCV" +
                "FVFMFIwRXhWV1JFZDBWQ0wzZFJSVUYzU1VOd1JFRlFRbWRPVmtoU1RVSkJaamhGUWxSQlJFRlJTQzhLVFVJd1IwRXhWV1JFWjFGW" +
                "FFrSlVTa3N6Ymt0WkswMUxRa3RSZURGMFRUaHVVRElyY25ZNFptbDZRVTVDWjJ0eGFHdHBSemwzTUVKQlVYTkdRVUZQUXdwQlVVV" +
                "kJZak5yTmtkMmNEZFRPRE5ZTlZsUFRWRk9kRVExUVVGSFEzZFZObmwyYXpWSlV5c3JMMFpvYlVabVVWSlRTR281VEdGUlZURTRSM" +
                "nhJVUZoRUNtd3lVR3hUTWtGNGN5OWxkbVp6V1dSTWNsaE9iMk13WVdwR1VXMVNObVZaWVU5RFRWZEZWRmQxZGs0cmFtOTZZVEZoV" +
                "URWdlJYUm1hV2t4TjJacVJqVUtRMEo2Vkd4SVJVaEVUSGh6SzBvMk0wbE1hbm9yVEZjeFJsRXpWMlJrWnpOeFYyOHJTR0ZHVGxGR" +
                "lNWQmpMemswVTBRME1GQmhLMk5EZEhnMWNuQnhXZ28yWlhBNVFUTTViVFpQTWs4MlVXaHZlRzV2YmtoME1ETmxSM2xXYjBJNWJTd" +
                "HNZMmhvUkU0eWJWcFRUM2RXTUhaTk1HdG5LMng2U1U1eWVsUXdTV3hQQ2tJcmVtUmlVWEV5VDFWaVRHTkpWMWxDYmt0dFpGVm1PR" +
                "TlYTnpaeFEweHViRTU2VEM5TFpIRjVkRmt5U0hreVJIRTBiR1pSYjNwS2VFOUZhVlp2TDFVS1EyZExTSEZLWTI1UVVqTTBNVFFyZ" +
                "W0wd1duRmFSV1pGTVVFOVBRb3RMUzB0TFVWT1JDQkRSVkpVU1VaSlEwRlVSUzB0TFMwdENnPT0iDQogICAgICAgICAgICB9DQogI" +
                "CAgICAgIH0sDQogICAgICAgIHsNCiAgICAgICAgICAgICJuYW1lIjogImV4dGVybmFsQ2x1c3RlciIsDQogICAgICAgICAgICAiY" +
                "2x1c3RlciI6IHsNCiAgICAgICAgICAgICAgICAic2VydmVyIjogImh0dHBzOi8vMzMuNTAuMTAwLjE1NjoiLA0KICAgICAgICAgI" +
                "CAgICAgICJpbnNlY3VyZS1za2lwLXRscy12ZXJpZnkiOiB0cnVlDQogICAgICAgICAgICB9DQogICAgICAgIH0sDQogICAgICAgI" +
                "HsNCiAgICAgICAgICAgICJuYW1lIjogImV4dGVybmFsQ2x1c3RlclRMU1ZlcmlmeSIsDQogICAgICAgICAgICAiY2x1c3RlciI6I" +
                "HsNCiAgICAgICAgICAgICAgICAic2VydmVyIjogImh0dHBzOi8vMzMuNTAuMTAwLjE1Njo1NDQzIiwNCiAgICAgICAgICAgICAgI" +
                "CAiY2VydGlmaWNhdGUtYXV0aG9yaXR5LWRhdGEiOiAiTFMwdExTMUNSVWRKVGlCRFJWSlVTVVpKUTBGVVJTMHRMUzB0Q2sxSlNVU" +
                "kVla05EUVdabFowRjNTVUpCWjBsQ1FVUkJUa0puYTNGb2EybEhPWGN3UWtGUmMwWkJSRUZ3VFZKcmQwWjNXVVJXVVZGTFJYaENSR" +
                "kV3VldjS1ZrZFdhbUZITlhaaVJ6bHVZVmRXZWsxUmQzZERaMWxFVmxGUlJFVjNUa1JSTUZWM1NHaGpUazFxVFhkUFZFRXpUVVJSZ" +
                "VU5VVFYbFhhR05PVGtSTmR3cFBWRUV6VFVSUmVVOVVRWGxYYWtGd1RWSnJkMFozV1VSV1VWRkxSWGhDUkZFd1ZXZFdSMVpxWVVjM" +
                "WRtSkhPVzVoVjFaNlRWRjNkME5uV1VSV1VWRkVDa1YzVGtSUk1GVjNaMmRGYVUxQk1FZERVM0ZIVTBsaU0wUlJSVUpCVVZWQlFUU" +
                "kpRa1IzUVhkblowVkxRVzlKUWtGUlJFNWhZazU2YkRKWFVFTkZLMVlLUmtaRGJYWmtURkpwV2xSVlFtcDVSMk0yYVVWV1F6bDZaV" +
                "kZtVjNVMlFXbE5TbUVyUkd0Sk5EZG9TRFF4U1ZNM1QxWjFiRVIwZEVsdWNtaENSSFU0Tndvd1JHeGplSFJZVTFJNFptTlNOVmhNY" +
                "VVGVlIzaG1NMGhIVFVOM1oybDNXalJXWVVWWEwwNWxVaTlDTTBVdmJ5dHRiME00TkRWU1JqUnpOVGMzUkV4aUNuQnBibTVqUm00M" +
                "mFGbG5TMmd3VWpsaGFVeDFVVFJWUkVoR1FsQnpOSEZLTldWbk4wMUJTMnBJZEVaRmExTkROVFFyTlZGelNrRlRTMGRaUW5OalZqS" +
                "Utibmg0VmtnM1QwSTBkbW9yWXpWVVpWVkRLM0I0TTNGRFdGVXpTV1ZOUjBsTVZIQklNRVpaZURKQ1NVNVpNa3hJVVc1YVNsbExWV" +
                "TVQWm5acllUZEdUd296Y2xkc05EZFpTM0ZCSzI5TWNUaDJja1ZLYTNGMmNsRkpZVXRGVjAxb09HOXZSRVp0UzFKNFNHUldibGgwV" +
                "DJsWFpXVlZNa2xtU0doaGJHazRPVlJKQ25kUmVqazRNSEZpUVdkTlFrRkJSMnBSYWtKQlRVRTBSMEV4VldSRWQwVkNMM2RSUlVGM" +
                "1NVTndSRUZRUW1kT1ZraFNUVUpCWmpoRlFsUkJSRUZSU0M4S1RVSXdSMEV4VldSRVoxRlhRa0pVU2tzemJrdFpLMDFMUWt0UmVER" +
                "jBUVGh1VURJcmNuWTRabWw2UVU1Q1oydHhhR3RwUnpsM01FSkJVWE5HUVVGUFF3cEJVVVZCWWpOck5rZDJjRGRUT0ROWU5WbFBUV" +
                "kZPZEVRMVFVRkhRM2RWTm5sMmF6VkpVeXNyTDBab2JVWm1VVkpUU0dvNVRHRlJWVEU0UjJ4SVVGaEVDbXd5VUd4VE1rRjRjeTlsZ" +
                "G1aeldXUk1jbGhPYjJNd1lXcEdVVzFTTm1WWllVOURUVmRGVkZkMWRrNHJhbTk2WVRGaFVEVnZSWFJtYVdreE4yWnFSalVLUTBKN" +
                "lZHeElSVWhFVEhoekswbzJNMGxNYW5vclRGY3hSbEV6VjJSa1p6TnhWMjhyU0dGR1RsRkZTVkJqTHprMFUwUTBNRkJoSzJORGRIZ" +
                "zFjbkJ4V2dvMlpYQTVRVE01YlRaUE1rODJVV2h2ZUc1dmJraDBNRE5sUjNsV2IwSTViU3RzWTJob1JFNHliVnBUVDNkV01IWk5NR" +
                "3RuSzJ4NlNVNXllbFF3U1d4UENrSXJlbVJpVVhFeVQxVmlUR05KVjFsQ2JrdHRaRlZtT0U5WE56WnhRMHh1YkU1NlRDOUxaSEY1Z" +
                "EZreVNIa3lSSEUwYkdaUmIzcEtlRTlGYVZadkwxVUtRMmRMU0hGS1kyNVFVak0wTVRRcmVtMHdXbkZhUldaRk1VRTlQUW90TFMwd" +
                "ExVVk9SQ0JEUlZKVVNVWkpRMEZVUlMwdExTMHRDZz09Ig0KICAgICAgICAgICAgfQ0KICAgICAgICB9DQogICAgXSwNCiAgICAid" +
                "XNlcnMiOiBbDQogICAgICAgIHsNCiAgICAgICAgICAgICJuYW1lIjogInVzZXIiLA0KICAgICAgICAgICAgInVzZXIiOiB7DQogI" +
                "CAgICAgICAgICAgICAgImNsaWVudC1jZXJ0aWZpY2F0ZS1kYXRhIjogIkxTMHRMUzFDUlVkSlRpQkRSVkpVU1VaSlEwRlVSUzB0T" +
                "FMwdENrMUpTVVIwUkVORFFYQjVaMEYzU1VKQlowbEpTelZQUTJFcmIxUlJTVlYzUkZGWlNrdHZXa2xvZG1OT1FWRkZURUpSUVhkT" +
                "FZFVmFUVUpqUjBFeFZVVUtRMmhOVVZFd1RrWkpSbEpzV1RKb2RXSXllSFphTW14c1kzcEZUVTFCYjBkQk1WVkZRWGhOUkZFd1RrW" +
                "k5RalJZUkZSSmVrMUVhM2xOYWtFd1RsUlJNQXBOTVc5WVJGUkpNRTFFVFhsTlJFRXdUbFJSTUUweGIzZG5ZMEY0WjFwSmQwWlJXV" +
                "VJXVVZGTFJYYzFlbVZZVGpCYVZ6QTJZbGRHZW1SSFZubGpla0Z1Q2tKblRsWkNRVzlVU1VSUk5GcFVSWGxOZWxwb1RXcGpORTVxV" +
                "VhoT01scHBUbXBrYVUxWFRYaE5SMFV3V1cxR2JWbFVhRzFOUTJOSFFURlZSVU5vVFdjS1RrUnJlbHBxYXpKWk1rVjNUbXBaZDA1R" +
                "VFUSlpNa1V3VFhwb2JGbFhWbWhPUjBwcFRrUldiVTlFUlhkS2QxbEVWbEZSUzBWNVFtMU9iVTEzVG1wc2F3cGFWRnByVGtkUk1FN" +
                "XRSWGhQVkUwMFdsUnNhMDFxWXpGTmFsRXdUbTFGTlZwcVJYQk5RMk5IUVRGVlJVRjRUV2RPZW1OM1QwUlZOVTE2VW1sT1ZFcHJDa" +
                "zVFWjNwT1YwcHRUVWRTYTA5RVl6Vk9SRTB4VFcxUk1VMXFaM2RuWjBWcFRVRXdSME5UY1VkVFNXSXpSRkZGUWtGUlZVRkJORWxDU" +
                "khkQmQyZG5SVXNLUVc5SlFrRlJSRTVoWWs1NmJESlhVRU5GSzFaR1JrTnRkbVJNVW1sYVZGVkNhbmxIWXpacFJWWkRPWHBsVVdaW" +
                "GRUWkJhVTFLWVN0RWEwazBOMmhJTkFveFNWTTNUMVoxYkVSMGRFbHVjbWhDUkhVNE56QkViR040ZEZoVFVqaG1ZMUkxV0V4cFFWV" +
                "khlR1l6U0VkTlEzZG5hWGRhTkZaaFJWY3ZUbVZTTDBJekNrVXZieXR0YjBNNE5EVlNSalJ6TlRjM1JFeGljR2x1Ym1OR2JqWm9XV" +
                "2RMYURCU09XRnBUSFZSTkZWRVNFWkNVSE0wY1VvMVpXYzNUVUZMYWtoMFJrVUthMU5ETlRRck5WRnpTa0ZUUzBkWlFuTmpWakp1Z" +
                "UhoV1NEZFBRalIyYWl0ak5WUmxWVU1yY0hnemNVTllWVE5KWlUxSFNVeFVjRWd3UmxsNE1rSkpUZ3BaTWt4SVVXNWFTbGxMVlU1U" +
                "FpuWnJZVGRHVHpOeVYydzBOMWxMY1VFcmIweHhPSFp5UlVwcmNYWnlVVWxoUzBWWFRXZzRiMjlFUm0xTFVuaElaRlp1Q2xoMFQyb" +
                "FhaV1ZWTWtsbVNHaGhiR2s0T1ZSSmQxRjZPVGd3Y1dKQlowMUNRVUZIYWxORVFrZE5RVFJIUVRGVlpFUjNSVUl2ZDFGRlFYZEpSb" +
                "TlFUVZRS1FtZE9Wa2hUVlVWRVJFRkxRbWRuY2tKblJVWkNVV05FUVdwQlprSm5UbFpJVTAxRlIwUkJWMmRDVkVwTE0yNUxXU3ROU" +
                "zBKTFVYZ3hkRTA0YmxBeUt3cHlkamhtYVhwQlRrSm5hM0ZvYTJsSE9YY3dRa0ZSYzBaQlFVOURRVkZGUVc1RlNUbHFSVFV3YzNsU" +
                "VpFTkhUMlYzTjB0d2FIWTBSWFZIZDBRMmN6UTFDalZ5UkRaWllUZEplV2hOUjJaMFlVNXNiRE5CWjFRNU9URllVWEJQZGxRNU9EV" +
                "XJSRk0wZWpWWE5FZDVWMkY1YXprM2MzbHJNMWh1WjBKQ1JqVm9jVmdLYVhGSVdubHZXR2h5Y0VKMWFUbG5kVlJ0UmxCRmFXWkRia" +
                "zQwTlZjNGIxRmhlRWc1VVhWdWJuRnJSbFZvY1U0ck5FZGFWM00yTm5KVmNWUjVUek42UlFvdk1HdHNiMjRyTWtablZFeERVMlZLY" +
                "2s0d0wwUklRVU5zVFdSdFZrb3lTemRLZW14SVRUSjNVVzh3VDJORVdsVlRVamg1T0drMmNEaDRVVWRUUVdST0NtOWhhVmxsU1U1c" +
                "U5XRk5NMjVETldoUE4xUlBNR3hhT1ZSaFZXbFFaRWRsYjBSRVVHZ3llRXBMSzAxNk9XUXpUMXBLTTFoTVlWaFlZMVZOUVhJd1VFR" +
                "UtWME13UTFwWVJXMXFXblp6YVdNd2JEQnVaREZTYUhWWFQxTkpSMVJVYzFsUWVTdHNWbVpFWTFOSmNraHlPR3BqUm5Oa1JVWm5QV" +
                "DBLTFMwdExTMUZUa1FnUTBWU1ZFbEdTVU5CVkVVdExTMHRMUW89IiwNCiAgICAgICAgICAgICAgICAiY2xpZW50LWtleS1kYXRhI" +
                "jogIkxTMHRMUzFDUlVkSlRpQlNVMEVnVUZKSlZrRlVSU0JMUlZrdExTMHRMUXBOU1VsRmNFRkpRa0ZCUzBOQlVVVkJlbGR0ZW1NM" +
                "VpHeHFkMmhRYkZKU1VYQnlNMU13V1cxVk1VRlpPR2h1VDI5b1JsRjJZek5yU0RGeWRXZEpha05YQ25abk5VTlBUelJTSzA1VFJYV" +
                "jZiR0p3VVRkaVUwbzJORkZSTjNaUE9VRTFXRTFpVmpCclprZ3pSV1ZXZVRSblJrSnpXRGw0ZUdwQmMwbEpjMGRsUmxjS2FFWjJlb" +
                "GhyWm5ka2VGQTJVSEJ4UVhaUFQxVlNaVXhQWlN0M2VUSTJXWEExTTBKYUsyOVhTVU52WkVWbVYyOXBOMnRQUmtGNGVGRlVOMDlMY" +
                "VdWWWJ3cFBla0ZEYjNnM1VsSktSV2QxWlZCMVZVeERVVVZwYUcxQllraEdaSEE0WTFaU0szcG5aVXcwTDI1UFZUTnNRWFp4WTJRM" +
                "loyd3hUbmxJYWtKcFF6QTJDbEk1UWxkTlpHZFRSRmRPYVhnd1NqSlRWME5zUkZSdU56VkhkWGhVZERZeGNHVlBNa054WjFCeFF6W" +
                "jJURFo0UTFwTGNqWXdRMGRwYUVacVNXWkxTMEVLZUZwcGEyTlNNMVphTVRkVWIyeHVibXhPYVVoNE5GZHdXWFpRVlhsTlJVMHZaaz" +
                "VMYlhkSlJFRlJRVUpCYjBsQ1FVaGpTRkZSTVVOT1dEbHJaVmhyVWdwb1NYRmhhRWxqV0VOcU5ubFFia0pMY0U5UVNsSktXRTVHUmx" +
                "oQ09ERkVZMmgyTUhGc1oxVllUSGxNV2xGMlEyRnNRbWhhVHpSbmFFMVFVRU5ETmpkR0NsRm1aMWt6T1RSNE5tZFpha1pTU1ZSb05W" +
                "bGtUa3RyYVVaeldYaExRWG96U0c5TmEwcFRiMFV4WW5GeGQwNDVNU3RDVGpoaVNHSjVjbTFRZDNkeFpUQUtObUZPU3pNeGFtbzVOb" +
                "XBwUzFSWWFsRnpjQ3RLV2pGbU1ITnVZbGhVYVVOdFVteHZSbU12WWtKUWJEbHRlWFphVmpocVdVTnBWbEp6Y3pGcE1qZENhQXBuWW" +
                "13MmIwVTJRbGh3UVhWUk9Dc3diM2xqVTBWd2RUaG5WSFF6UzNOcVJVVkphMGxQVUhsWFlURkxXV1pDY2pWS2FqQXpUWFJ0V0ZsTl" +
                "ZHUkxSbnBtQ2pNdldHOHhRMUZCTmpacVJtaDZWRk5xYmxKdWVUaHBNVkZtUm1wU1JYWTJWRGx3UmpkT1dHMTVORGx5ZEV4UE9XNT" +
                "NjRmxUV0ZsVllrTlFMMlpIWkM4S2NXZHNaSGs0UlVObldVVkJNRzl3TTJaTlRtNHlkbVJQWXpsemJVUlhNRXhWV2taT2RERnhUa2" +
                "gxYlZWUk9WZExkVlpJVDBkVVVYSlBUVWx3YjA1cVRRb3daR3RDU2t0ak9HZDZiMk13U0d4M1oxVTBkMDlUWVdaSlQyTmhjMVl2VU" +
                "dwb1QwSkthbVZvVUhweGVrNXFWVE4yU2s0MlIwRnNOVzE1Ym5WNVdrUmxDa0ZuV2xwT1NHaEpkR2gyYXpKRldrbE9NM0JJWlZGQl" +
                "VqVjZWRGRNYVZGdVdFVnZVbkJwTXpSVU9Fb3JRekJXYlVkRFRFcHljR05EWjFsRlFTdGpVRW9LYWtoSlFUVk1SV3h6TlRCUmFsTj" +
                "BhRkJyTWpCc1VuVTFPV3cxTUZKVlRGVjJTa2RPZFV0b2NVdHpibEo0WkRsSFZFd3ZRV1lyT1hCVGNqWTNRalprTkFwd09XaHRZM3" +
                "BEVEdvM09VODFNRGxCTW14NVkyUnBWMEpET1hOVlZuZ3llQzkxYkVOak0yNXJNa1JKZW5SWk0wOWFRMFpYWkhnMVRWaG1NaTlKV0" +
                "ZKVUNtaFFXbXMyWlZKMVFrWXJhamxMYjBkdlNtZGthRTluTDI0Mk1YVjFSVTVEY2xwMk1XbEtNRU5uV1VWQmRIWXZjMEVyU0VVM0" +
                "5uQjBORzR6YkdkV016WUtja3BrYW5WaE9WaFphbk5qZVVOc1oxQjBlRUZESzBGM1oxWkZhVFZrUVV4c05ISk1WalZUVmtsSGNuVn" +
                "BURnBHWkRrNFExRXljWGxtWlV4cFlreEpaUXB6YUhRMU9HdEpRVkZtYXk5MFEzQnNiVGtyUlVSRmFWRkVja3BRTHpWMVVHNUxjbl" +
                "k1YnpsdGFFRlBVekJtYldzME4xQkNWa0ZqWm1sS2RXZG9LMkZqQ2pNM1luaDVSVkJGZUZjNFJVbzNMMWRyWW5kTVIxWnJRMmRaUW" +
                "xSUlNubHpPRTA0TVdFMGNuRnVUamhKWm5vMGVIVXdjMGhNU3k5SE0wZE5ZazU0VTJNS1MyeFJORU5uVFdoamFGa3pVVFUwTWs1UV" +
                "FtZGtTbVl2VEhsWksyWmxZM00wTW1ZMk1qSkxXbkp4V1VwWGFGSXJPVTFISzJFckwyY3lUWGxzTkZOTlZRcHRaMGh3V21WalR6Tl" +
                "hWRFZFYlZOUFdXTktiR2RFWW5GWFdYUmtRbXcxVldkeFJFOXBZa3hYVTBZM2NGUmpaRnAyYm5GUll6WXpTbVIwUkdsSlMwWlpDa3" +
                "A0Vlc1NlVVdENaMUZEWlZWVlJIVjFaVnBXVVRaeFNHOXZUMnBLVkVOMWNtVlNkemhMVWtZclVIaFdRUzl2ZDJsMVNrWmtOMU5VTn" +
                "pWdWVtNW5ZUzhLY1VkcWJ6ZzFjR2xXYlhsWk5VdFNOakpFVnpsblpGUTVlazgzWm1oQk5HeEZjVzFXZUZCalVFRTNMMlF3TlhkUU" +
                "9WSjJSaTlvYjBkVE1uSnhWSEJ5VVFwcVIxVlFZV2hFYzJ3cmRIZzFNeTluWlM4d09XZ3lZMUEyZDJOSk0wbExjRnBJYUdaWFVpdE" +
                "5WRTFxU2tGVlYzTXpUSEpLUzFFOVBRb3RMUzB0TFVWT1JDQlNVMEVnVUZKSlZrRlVSU0JMUlZrdExTMHRMUW89Ig0KICAgICAgIC" +
                "AgICAgfQ0KICAgICAgICB9DQogICAgXSwNCiAgICAiY29udGV4dHMiOiBbDQogICAgICAgIHsNCiAgICAgICAgICAgICJuYW1lIj" +
                "ogImludGVybmFsIiwNCiAgICAgICAgICAgICJjb250ZXh0Ijogew0KICAgICAgICAgICAgICAgICJjbHVzdGVyIjogImludGVybm" +
                "FsQ2x1c3RlciIsDQogICAgICAgICAgICAgICAgInVzZXIiOiAidXNlciINCiAgICAgICAgICAgIH0NCiAgICAgICAgfSwNCiAgIC" +
                "AgICAgew0KICAgICAgICAgICAgIm5hbWUiOiAiZXh0ZXJuYWwiLA0KICAgICAgICAgICAgImNvbnRleHQiOiB7DQogICAgICAgIC" +
                "AgICAgICAgImNsdXN0ZXIiOiAiZXh0ZXJuYWxDbHVzdGVyIiwNCiAgICAgICAgICAgICAgICAidXNlciI6ICJ1c2VyIg0KICAgIC" +
                "AgICAgICAgfQ0KICAgICAgICB9LA0KICAgICAgICB7DQogICAgICAgICAgICAibmFtZSI6ICJleHRlcm5hbFRMU1ZlcmlmeSIsDQ" +
                "ogICAgICAgICAgICAiY29udGV4dCI6IHsNCiAgICAgICAgICAgICAgICAiY2x1c3RlciI6ICJleHRlcm5hbENsdXN0ZXJUTFNWZX" +
                "JpZnkiLA0KICAgICAgICAgICAgICAgICJ1c2VyIjogInVzZXIiDQogICAgICAgICAgICB9DQogICAgICAgIH0NCiAgICBdLA0KIC" +
                "AgICJjdXJyZW50LWNvbnRleHQiOiAiZXh0ZXJuYWwiDQp9";
        return config;
    }

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: Config文件内参数缺少端口参数，抛出异常
     */
    @Test
    public void test_check_config_port_is_null() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getAuth().setAuthType(Authentication.OTHER);
        Map<String, String> extenInfo = k8sCluster.getExtendInfo();
        String config = getConfigString();
        extenInfo.put(K8sExtendInfoKey.CONFIG, config);

        k8sCluster.getAuth().setExtendInfo(extenInfo);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        Mockito.when(commonService.queryClusterInfo(any())).thenReturn(appEnvResponse);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.KUBE_CONFIG_ERROR, exception.getErrorCode());
    }

    private static String getConfigString() {
        String config = "YXBpVmVyc2lvbjogdjEKa2luZDogQ29uZmlnCnByZWZlcmVuY2VzOiB7fQpjbHVzdGVy" +
            "czoKICAtIGNsdXN0ZXI6CiAgICAgIGNlcnRpZmljYXRlLWF1dGhvcml0eS1kYXRhOiBMUzB0TFMxQ1JVZEpUaUJEUlZKVVNVWkp" +
            "RMEZVUlMwdExTMHRDazFKU1VRdmFrTkRRVzFoWjBGM1NVSkJaMGxDUVVSQlRrSm5hM0ZvYTJsSE9YY3dRa0ZSYzBaQlJFRldUVk" +
            "pOZDBWUldVUldVVkZFUlhkd2NtUlhTbXdLWTIwMWJHUkhWbnBOUWpSWVJGUkplazFFWTNkT2FrRTFUV3BKZUU1V2IxaEVWRTE2V" +
            "FVSamQwMTZRVFZOYWtsNFRsWnZkMFpVUlZSTlFrVkhRVEZWUlFwQmVFMUxZVE5XYVZwWVNuVmFXRkpzWTNwRFEwRmhTWGRFVVZs" +
            "S1MyOWFTV2gyWTA1QlVVVkNRbEZCUkdkblIxQkJSRU5EUVZsdlEyZG5SMEpCVFRobENrUnJaREJ0Ynl0VVRUWlVZMWRUZFdoeGJ" +
            "VeGljbWRFU0U5NE9XOVplVXhRWjJsVGJGQlNWMUJwZFZCc2JVSkRSakUwYldObFpuRllUMFJuUzFacGNqSUtkVll5V1RsWmNtOT" +
            "NTVmRPVGtweFVEZGhaVkJrU1daa2MyTXZSVTV4V2pSdWVrdzVUSGx0Ym1SYWVEaENMMVI0YmxKVU1HazFhMmRpZGpWSU5VTlRSQ" +
            "XBqUVRSNGNFVnRjRkI0TTJoWU9GcG9Lek41UTFsWFlsSmhiMlI1VWxsdFRsWnVZVVp5WjBoNGRFcHRTakpHUlhSQ1kydG1OVkox" +
            "WVVsM1dGQnhjV1ZLQ2pseVlrTlNUVVpYVmpCWmQxUTRXWGswWlVWVGFWVXhNVkpXVjA4M2NFWTNWMjVrWjFORU5GcHVjR0pzY2k" +
            "4eldHUnJUSFpRTkdOR09XMDFVR2xIYVZvS2RXeE5lUzlyTWtWUlJuaFZXRUptVVZaaFZUQk1VMk52WTNadlJ6TkVabFZxZWxCNG" +
            "RXSmlNamxpYTNSU1dXVkhka3hxVlUxUVVEZHllbmsxYWpCTFJnb3JkbTAxYkhscWNIUlZVVzR4TVhsS1N6Qm9LM1lyYVZCelNYW" +
            "m1SMWhhWW5jNVdrSjRPVEZtY21RMVlUaE5jVUV3U25FNFRVOTFaVkpMVm5sUFIwOWpDbXBxWlhwQ1ZESTRjM0V3UjJoQ04ydEZh" +
            "bHBsVG5Gd2JVdzBSRlJDYTBsVGEzWkpNVFp0YTJWUldXODBiREZNTTBkaldHOWtaR0ZYTUVKa09YbDFVVGNLVUd0WmNVcFdNRlp" +
            "FVTFReldIRnFSR1ZVU2xWTVYyYzRkRXhuU1dFeU16QkZSMnRPT1RKbGVXRnVaMlpWWkdreE9XcFBSWGhsSzBaeldtUmlOVkZKUk" +
            "FwQlVVRkNiekZyZDFaNlFVOUNaMDVXU0ZFNFFrRm1PRVZDUVUxRFFYRlJkMFIzV1VSV1VqQlVRVkZJTDBKQlZYZEJkMFZDTDNwQ" +
            "lpFSm5UbFpJVVRSRkNrWm5VVlZ3U0ZselRtSklLM0kzU0dSRFFqQXhOR1JOTkVjNVQyNVhhVzkzUmxGWlJGWlNNRkpDUVRSM1JF" +
            "bEpTMkV6Vm1sYVdFcDFXbGhTYkdONlFVNEtRbWRyY1docmFVYzVkekJDUVZGelJrRkJUME5CV1VWQldEZDBPVVJVWlZjNU15dDJ" +
            "UV052Tld4RWRuaENWblpMTVN0dk5sVkdUWFp1WkV4dmVITXpWQXAyTVRoTU5tcHZOVzFOUVVSdE5EbFFTbWxKYm10MFQwdFNWbW" +
            "RJWVdkR2NFSnFWRnBHZG1kRVRrRlFkR1J4V0VWUFNFcHZNa1ZpVkdOT2VXSm1kVmxRQ2k5dU0ydFNaRTVsVFZKbVJHaDZVMUZqW" +
            "WxKNFIwVlVkRXB0VEdSc2FDc3JTMWsxVjJkb056ZFRVM013WW5scmJtbGhOMnd4ZFdGc2FYcFVUSHB3VWtVS2ExZHFXakpNUlM4" +
            "eEwyWlFOVlZaVTAwM2FYTjVUR0pLUlcxSVNGWXJTell3U20xdmRXdFlkRTFwYVVOSWVFWk5kV040UW1sTGVqZHJORTkwWmt3d1Z" +
            "ncHBWbmhuYkdkUk5tUjZPRXBFTVRCM05EbG9iemxDVFRSWFoySm1NelpHVkdaNE4xRllVRTVPTW1OV1pWbDFWMmxzYjBZME9GQX" +
            "lNMVpHY1dacUt6VXhDbTR6VlNzNE1rWmpWRmMyUjI1NFJ6bHdXR2c1ZDJaUGExSkVZVEEwZW5KelNYaFBjVnBrTWt4M1ZWcFZiR" +
            "k5CZUVkNFVXVTVReTl1VlRGak9HcHdiRUlLUjBsSWFsWklURUZoYVRKbmNraDNPR05LUjIxSlZreEJTMnhUT0hkMFJtazBZeTl4" +
            "ZUZvckt5dEJia2hVY1doTU1TdHdkRk50ZURCdVJWSlJSbXRaWndwemFsVjJWRWQzVEVOd1QycDJMMEZMTVVsbFoxaGhiWFkwY1Z" +
            "kT2RtOVRZa0ZxUTJObVZHaE5iR0poYWtWSVRXODNjSEJYTVU1UFlWQXpSSE5CUWs5ckNrUnRRVGRNYW1OMmRtTlJMekkyVjJoS0" +
            "1sRjRSalpSZEFvdExTMHRMVVZPUkNCRFJWSlVTVVpKUTBGVVJTMHRMUzB0Q2c9PQogICAgICBzZXJ2ZXI6IGh0dHBzOi8vOC40M" +
            "C4xNDUuMTMzOgogICAgbmFtZTogc29mdC13YXJlLWRldgpjb250ZXh0czoKICAtIGNvbnRleHQ6CiAgICAgIGNsdXN0ZXI6IHNv" +
            "ZnQtd2FyZS1kZXYKICAgICAgdXNlcjogc3lzYWRtaW4KICAgIG5hbWU6IHN5c2FkbWluQHNvZnQtd2FyZS1kZXYKY3VycmVudC1" +
            "jb250ZXh0OiBzeXNhZG1pbkBzb2Z0LXdhcmUtZGV2CnVzZXJzOgogIC0gbmFtZTogc3lzYWRtaW4KICAgIHVzZXI6CiAgICAgIH" +
            "Rva2VuOiBrcm06QUFBQUFRQUFBQUVBQUFBQUFBQUFCd0FBQUFJQUFBQUJvT3BndWRxanJUREVESjNrVFhqVE8yUGRaQ1pTYkkvO" +
            "E0yWmI0bnhFcGs0QUFBQmdBQUFBQUFBQUFBRGRubjhyYmRYSTlaODFZaGxTUGxMK0taUnJxTWpMNkphRHdvbldlMlFnb0c0LzVQ" +
            "ZXlzci91czg3QkU2REEzS3grNEpoSjJlS0pTR1ZCcFFGZkRQb2hhWWhXVVU3Q2pWUk5ZeFRtbVo3S1c4OUJkZFE0S3hMOGplN2N" +
            "kOWZMUmhNQUFBQUJBQUFBQUFBQUNBUUFBQUFDQUFBQUFScDBhNWFuTE1YSFdwVmp5TEtMai9FQUFBQUFBQUFBQU9nRS9nWXJqV2" +
            "8rbkZLU1JVV3N5eG54dUFYRWVtYytpZGNBTEhabVBHUEM=";
        return config;
    }

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: Endpoint，port为空，抛出异常
     */
    @Test
    public void should_throw_exception_when_endpoint_is_empty() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getAuth().setAuthType(Authentication.TOKEN);
        k8sCluster.setEndpoint(null);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
    }

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: Endpoint，port为空，抛出异常
     */
    @Test
    public void should_throw_exception_when_port_is_empty() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getAuth().setAuthType(Authentication.TOKEN);
        k8sCluster.setPort(null);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
    }

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: AuthType为OTHER，且扩展信息得不到对应config参数
     */
    @Test
    public void should_throw_exception_when_authtype_is_other_and_extendinfo_is_null() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getAuth().setAuthType(Authentication.OTHER);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
    }
    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: AuthType不是TOKEN或者OTHER（以无认证为例）
     */
    @Test
    public void should_throw_exception_when_authtype_is_no_auth(){
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getAuth().setAuthType(Authentication.NO_AUTH);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
    }
    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: AuthType为Invalid，抛出异常
     */
    @Test
    public void should_throw_exception_when_authtype_is_invalid(){
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getAuth().setAuthType(111);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
    }

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: existedCount大于等于K8S_CLUSTER_MAX_COUNT，抛出异常
     */
    @Test
    public void should_throw_exception_when_existedcount_above_k8s_cluster_max_count(){
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        PageListResponse<ProtectedResource> query = new PageListResponse<>();
        query.setTotalCount(K8S_CLUSTER_MAX_COUNT);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(query);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ENV_COUNT_OVER_LIMIT, exception.getErrorCode());
    }

    /**
     * 用例场景：已注册情况下检查K8S集群参数
     * 前置条件：无
     * 检查点: version版本过低或为空，抛出异常
     */
    @Test
    public void should_throw_exception_when_k8s_cluster_version_is_out_date(){
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.setUuid("login");
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        Map<String, String> extendInfo=new HashMap<>();
        appEnvResponse.setExtendInfo(extendInfo);
        Mockito.when(commonService.queryClusterInfo(any())).thenReturn(appEnvResponse);
        Mockito.when(resourceService.getBasicResourceById(any())).thenReturn(Optional.of(k8sCluster));
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());

        extendInfo.put("version","v1.10");
        appEnvResponse.setExtendInfo(extendInfo);
        exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
    }

    /**
     * 用例场景：已注册情况下检查K8S集群参数
     * 前置条件：无
     * 检查点: 修改ip
     */
    @Test
    public void should_throw_exception_when_k8s_cluster_ip_is_wrong(){
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.setUuid("login");
        ProtectedResource resource = new ProtectedResource();
        resource.setEndpoint("0");
        resource.setPort(0);
        Mockito.when(resourceService.getBasicResourceById(any())).thenReturn(Optional.of(resource));
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
        Assert.assertEquals("k8s cluster ip cant change", exception.getMessage());
    }

    /**
     * 用例场景：注册流程检查k8s的备份Pod镜像名称和Tag
     * 前置条件：无
     * 检查点: 备份Pod镜像名称和Tag格式不正确
     */
    @Test
    public void should_throw_exception_when_k8s_cluster_image_name_and_tag_is_wrong(){
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        Map<String, String> extendInfo = k8sCluster.getExtendInfo();
        String test_no_pre = ":V";
        String test_end_wrong = "K:V_";

        extendInfo.put(K8sExtendInfoKey.IMAGE_NAME_AND_TAG, null);
        k8sCluster.setExtendInfo(extendInfo);
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
        Assert.assertEquals("K8s imageNameAndTag is empty" , exception.getMessage());

        extendInfo.put(K8sExtendInfoKey.IMAGE_NAME_AND_TAG,test_no_pre);
        k8sCluster.setExtendInfo(extendInfo);
        exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
        Assert.assertEquals("K8s imageNameAndTag is illegal" , exception.getMessage());

        extendInfo.put(K8sExtendInfoKey.IMAGE_NAME_AND_TAG,test_end_wrong);
        k8sCluster.setExtendInfo(extendInfo);
        exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.ILLEGAL_PARAM, exception.getErrorCode());
        Assert.assertEquals("K8s tag is illegal" , exception.getMessage());

    }

    /**
     * 用例场景：注册流程检查K8S集群参数
     * 前置条件：无
     * 检查点: 从Config文件读出ip，抛出异常
     */
    @Test
    public void should_throw_exception_when_ip_from_config_is_wrong(){
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        k8sCluster.getAuth().setAuthType(Authentication.OTHER);
        Map<String,String> extentInfo=new HashMap<>();
        extentInfo.put(K8sExtendInfoKey.CONFIG, "config");
        k8sCluster.getAuth().setExtendInfo(extentInfo);
        Mockito.when(resourceService.query(anyInt(), anyInt(), anyMap())).thenReturn(new PageListResponse<>());
        LegoCheckedException exception = Assert.assertThrows(LegoCheckedException.class,
                () -> provider.register(k8sCluster));
        Assert.assertEquals(CommonErrorCode.KUBE_CONFIG_ERROR, exception.getErrorCode());
    }



    /**
     * 用例场景：扫描集群下namespace
     * 前置条件：无
     * 检查点: 扫描成功，无异常抛出
     */
    @Test
    public void test_scan_success(){
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        ProtectedResource ns = new ProtectedResource();
        ns.setUuid("001");
        ns.setName("test-ns");
        response.getRecords().add(ns);
        ProtectedResource systemNs = new ProtectedResource();
        systemNs.setUuid("003");
        systemNs.setName("kube-system");
        response.getRecords().add(systemNs);
        Mockito.when(commonService.queryResource(anyInt(), anyInt(), any(), any())).thenReturn(response);
        PageListResponse<ProtectedResource> olDataset = new PageListResponse<>();
        ns = new ProtectedResource();
        ns.setParentUuid("2360306e-fee3-375e-a814-cb3bcb100e15");
        ns.setName("test-ns");
        ns.setUuid("abc");
        olDataset.setRecords(new ArrayList<>());
        olDataset.getRecords().add(ns);
        ProtectedResource two = new ProtectedResource();
        two.setParentUuid("2360306e-fee3-375e-a814-cb3bcb100e12");
        two.setName("test-ns");
        two.setUuid("bdb");
        olDataset.getRecords().add(two);
        Mockito.when(resourceService.basicQuery(anyBoolean(), anyInt(), anyInt(), anyMap())).thenReturn(olDataset);
        List<ProtectedResource> resourceList = provider.scan(k8sCluster);
        Assert.assertEquals(1, resourceList.size());
    }

    /**
     * 用例场景：浏览集群下资源
     * 前置条件：无
     * 检查点: 浏览成功，无异常抛出
     */
    @Test
    public void test_browse_success() {
        ProtectedEnvironment k8sCluster = mockK8sCluster();
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setRecords(new ArrayList<>());
        response.getRecords().add(new ProtectedResource());
        response.getRecords().add(new ProtectedResource());
        response.setTotalCount(20);
        Mockito.when(commonService.queryResource(anyInt(), anyInt(), any(), any())).thenReturn(response);
        BrowseEnvironmentResourceConditions conditions = new BrowseEnvironmentResourceConditions();
        conditions.setConditions(JsonUtil.json(new K8sQueryParam()));
        PageListResponse<ProtectedResource> browse = provider.browse(k8sCluster, conditions);
        Assert.assertEquals(browse.getRecords().size(), 2);
        Assert.assertEquals(browse.getTotalCount(), 20);
    }

    @Test
    public void test_applicable_success() {
        Assert.assertTrue(provider.applicable(ResourceSubTypeEnum.KUBERNETES_CLUSTER_COMMON.getType()));
    }

    private ProtectedEnvironment mockK8sCluster() {
        ProtectedEnvironment k8sCluster = new ProtectedEnvironment();
        Authentication auth = new Authentication();
        auth.setAuthType(Authentication.TOKEN);
        auth.setExtendInfo(new HashMap<>());
        auth.getExtendInfo().put(K8sExtendInfoKey.TOKEN, "test-token");
        auth.getExtendInfo().put(K8sExtendInfoKey.CERTIFICATE_AUTHORITY_DATA, "test-CERTIFICATE_AUTHORITY_DATA");
        k8sCluster.setAuth(auth);
        k8sCluster.setEndpoint("127.0.0.1");
        k8sCluster.setPort(8888);
        k8sCluster.setName("test");
        Map<String,String> extendInfo =  new HashMap<>();
        extendInfo.put(K8sExtendInfoKey.IS_VERIFY_SSL,"0");
        extendInfo.put(K8sExtendInfoKey.IMAGE_NAME_AND_TAG, "Hello:World");
        extendInfo.put(K8sExtendInfoKey.TASK_TIMEOUT, "{'days': 1, 'hours': 1, 'minutes': 0, 'seconds': 1}");
        k8sCluster.setExtendInfo(extendInfo);
        return k8sCluster;
    }

    private AppEnvResponse mockClusterInfo() {
        //模拟从agent返回的参数
        AppEnvResponse ClusterInfo = new AppEnvResponse ();
        Map<String,String> extendInfo = new HashMap<> ();
        extendInfo.put ("version",K8sConstant.K8S_MIN_VERSION);
        ClusterInfo.setExtendInfo (extendInfo);
        return ClusterInfo;
    }

}