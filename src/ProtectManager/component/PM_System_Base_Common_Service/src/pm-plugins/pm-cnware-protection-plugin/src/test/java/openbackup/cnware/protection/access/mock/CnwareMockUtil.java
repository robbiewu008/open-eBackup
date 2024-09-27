/*
 * Copyright (c) Huawei Technologies Co., Ltd. 2023-2023. All rights reserved.
 */

package openbackup.cnware.protection.access.mock;

import openbackup.cnware.protection.access.constant.CnwareConstant;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnv;
import openbackup.data.access.client.sdk.api.framework.agent.dto.AppEnvResponse;
import openbackup.data.access.client.sdk.api.framework.agent.dto.ListResourceV2Req;
import openbackup.data.protection.access.provider.sdk.base.Authentication;
import openbackup.data.protection.access.provider.sdk.base.PageListResponse;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedEnvironment;
import openbackup.data.protection.access.provider.sdk.resource.ProtectedResource;
import openbackup.system.base.common.constants.Constants;
import openbackup.system.base.common.constants.TokenBo;

import com.alibaba.fastjson.JSON;
import com.google.common.collect.Lists;

import org.powermock.api.mockito.PowerMockito;

import java.util.ArrayList;
import java.util.Collections;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

/**
 * CNware data Mock
 *
 * @author z30047175
 * @version [OceanProtect DataBackup 1.6.0]
 * @since 2023-12-21
 */
public class CnwareMockUtil {
    /**
     * check方法中 environment的数据
     */
    public static ProtectedEnvironment mockEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setName("test");
        environment.setType("CNware");
        environment.setSubType("CNware");
        environment.setSourceType("register");
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("enableCert", "1");
        extendInfo.put("certName", "wincenter.pem");
        extendInfo.put("certSize", "1.3KB");
        extendInfo.put("crlName", "");
        extendInfo.put("crlSize", "");
        environment.setExtendInfo(extendInfo);
        environment.setEndpoint("8.40.162.56");
        environment.setPort(443);
        Authentication auth = new Authentication();
        auth.setAuthType(1);
        auth.setAuthKey("admin");
        auth.setAuthPwd("passw0rd");
        Map<String, String> extendInfoOfAuth = new HashMap<>();
        extendInfoOfAuth.put("enableCert", "1");
        extendInfoOfAuth.put("certification", "-----BEGIN CERTIFICATE-----\r\nMIIDnjCCAoagAwIBAgIBATANBgkqhkiG9w0BAQsFADCBhzELMAkGA1UEBhMCQ04x\r\nCzAJBgNVBAgMAkdEMQswCQYDVQQHDAJHWjELMAkGA1UECgwCd2gxCzAJBgNVBAsM\r\nAndoMRkwFwYDVQQDDBB3aW5jZW50ZXItY2EuY29tMSkwJwYJKoZIhvcNAQkBFhp3\r\naW5jZW50ZXItY2FAd2luY2VudGVyLmNvbTAeFw0xODEyMTcwNTQ0MzNaFw0yODEy\r\nMTQwNTQ0MzNaMHUxCzAJBgNVBAYTAkNOMQswCQYDVQQIDAJHRDELMAkGA1UECgwC\r\nd2gxCzAJBgNVBAsMAndoMRYwFAYDVQQDDA13aW5jZW50ZXIuY29tMScwJQYJKoZI\r\nhvcNAQkBFhh3aW5jZW50ZXJAd2luY2VudGVyLmNvbSAwggEiMA0GCSqGSIb3DQEB\r\nAQUAA4IBDwAwggEKAoIBAQDGlSgZwnfO8gvqfSx5dzQgzTwZ2PV1YLUit3ZWauJo\r\nS024LO2HSmhSmXe+P1mn8JTSkRzIp1IXYuheBStTsGV9kpZgHuPmhvXdhftlCblL\r\nAP5L8bqpKUeesuraOn9UCnu5ql/bwasV+i7kVSQIJnsoXcBEuuBkvpr21zJBffTT\r\n6cl+yy1xUgkxKhVGDx8LoDGNvqxC0jEqBy/maJW4sZWQ4EjlfM5v6C12LOtag5pi\r\nD6EPuIPGN4cWTR73mF3FxLz6kJeCA1v3YLMTjRvYB2ayV1FKFJY0oLPaPxsaMKpZ\r\nX4T6OmeMdzcBZWV742mi50JtEWQ7sUneV5szTxjxssptAgMBAAGjJjAkMCIGA1Ud\r\nEQQbMBmHBMCozc+CEXd3dy53aW5jZW50ZXIuY29tMA0GCSqGSIb3DQEBCwUAA4IB\r\nAQDa3c/5kkEdz++0YnfJX7HEQrSGaw+DFsiqvfDOoLktOtbvPSpeQA4n2IANyv18\r\n/QPOTWHxqab0FNsw11wOnRfqemC6HHf0x/6NbjheQKVBLV6HvHS9jpB16z/AkwYQ\r\nwFWU3FwQHTTI4KLcPAusbU+FkEuyaZfyrRdLfA9xGEJnz/kg+IjmSP/LDQCxnMXB\r\nrgVxwF7jERYOVf+9r2CCxuxiVZszIz/XrSyfOKxtiUUTdlCxudOgYMX7HihGOZvK\r\neqc/+PhLy22lUKl0hp9QC/ijH8WvnJ/ArED3C4HxpV6JCrRfMWuzTvBZxSxHAamY\r\nNXOhsfOq0sV8U+MBMsLt8pov\r\n-----END CERTIFICATE-----\r\n");
        extendInfoOfAuth.put("revocationList", "");
        auth.setExtendInfo(extendInfoOfAuth);
        environment.setAuth(auth);
        environment.setScanInterval(3600);
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> resourceArrayList = new ArrayList<>();
        ProtectedResource protectedResource = new ProtectedResource();
        protectedResource.setUuid("de4572a6-5d23-4787-a8b3-673da3747ce9");
        resourceArrayList.add(protectedResource);
        dependencies.put("agents", resourceArrayList);
        environment.setDependencies(dependencies);
        return environment;
    }

    /**
     * getAllAgents方法中 collectConnectableResources的返回值
     */
    public static Map<ProtectedResource, List<ProtectedEnvironment>> mockCollectConnectableResources() {
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setUuid("de4572a6-5d23-4787-a8b3-673da3747ce9");
        agentEnvironment.setName("localhost.localdomain");
        agentEnvironment.setType("Host");
        agentEnvironment.setSubType("UBackupAgent");
        agentEnvironment.setPath("");
        agentEnvironment.setCreatedTime("2023-12-12 03:27:33.0");
        agentEnvironment.setRootUuid("de4572a6-5d23-4787-a8b3-673da3747ce9");
        agentEnvironment.setSourceType("");
        agentEnvironment.setVersion("1.6.RC1.032");
        agentEnvironment.setProtectionStatus(0);
        Map<String, String> extendInfoOfAgent = new HashMap<>();
        extendInfoOfAgent.put("pushRegister", "true");
        extendInfoOfAgent.put("agentIpList", "192.168.101.103,8.40.101.103,fe80::29b1:8b3b:663d:77c2,fe80::5e9c:6700:b1d0:caaa");
        extendInfoOfAgent.put("agent_domain_available_ip", "{\"2102354PBB10MC100006\":\"protectengine-0.protectengine.dpa.svc.cluster.local\"}");
        extendInfoOfAgent.put("src_deduption", "true");
        extendInfoOfAgent.put("install_path", "/opt");
        extendInfoOfAgent.put("connection_result", "{\"2102354PBB10MC100006\":{\"end_point\":\"8.40.97.155,8.40.97.156\",\"link_status\":1,\"cluster_name\":\"BackupStorage\"}}");
        extendInfoOfAgent.put("agent_last_update_time_2102354PBB10MC100006", "1703141579851");
        extendInfoOfAgent.put("availableZone", "");
        extendInfoOfAgent.put("$citations_agents_22b387189ce1477badf46ae0dbf93feb", "fb02a492b5e34ca8a17afaff6f91c01e");
        extendInfoOfAgent.put("agent_applications", "{\"menus\":[{\"applications\":[{\"appDesc\":\"\",\"appLabel\":\"FusionCompute\",\"appValue\":\"FusionCompute\",\"isChosen\":true,\"pluginName\":\"FusionComputePlugin,VirtualizationPlugin\"},{\"appDesc\":\"\",\"appLabel\":\"CNware\",\"appValue\":\"CNware,CNwareHostPool,CNwareCluster,CNwareHost,CNwareVm,CNwareDisk\",\"isChosen\":true,\"pluginName\":\"VirtualizationPlugin\"}],\"isChosen\":true,\"menuDesc\":\"\",\"menuLabel\":\"agent.application.menu.virtualization\",\"menuValue\":\"Virtualization\"},{\"applications\":[{\"appDesc\":\"\",\"appLabel\":\"agent.application.huaweicloudstack\",\"appValue\":\"HCScontainer,HCSCloudHost,HCSProject,HCSTenant\",\"isChosen\":true,\"pluginName\":\"VirtualizationPlugin\"},{\"appDesc\":\"\",\"appLabel\":\"OpenStack\",\"appValue\":\"OpenStackContainer,OpenStackProject,OpenStackCloudServer\",\"isChosen\":true,\"pluginName\":\"VirtualizationPlugin\"},{\"appDesc\":\"\",\"appLabel\":\"agent.application.huaweicloudstackgaussdb\",\"appValue\":\"HCSGaussDBProject,HCSGaussDBInstance\",\"isChosen\":true,\"pluginName\":\"GeneralDBPlugin\"},{\"appDesc\":\"\",\"appLabel\":\"agent.application.tpopsgaussdb\",\"appValue\":\"TPOPSGaussDBProject,TPOPSGaussDBInstance\",\"isChosen\":true,\"pluginName\":\"GeneralDBPlugin\"},{\"appDesc\":\"\",\"appLabel\":\"agent.application.objectstorage\",\"appValue\":\"ObjectStorage\",\"isChosen\":true,\"pluginName\":\"ObsPlugin\"}],\"isChosen\":true,\"menuDesc\":\"\",\"menuLabel\":\"agent.application.menu.cloudplatform\",\"menuValue\":\"Cloud Platform\"},{\"applications\":[{\"appDesc\":\"\",\"appLabel\":\"Kubernetes FlexVolume\",\"appValue\":\"Kubernetes,KubernetesNamespace,KubernetesStatefulSet,KubernetesPVC\",\"isChosen\":true,\"pluginName\":\"VirtualizationPlugin\"}],\"isChosen\":true,\"menuDesc\":\"\",\"menuLabel\":\"agent.application.menu.containers\",\"menuValue\":\"Containers\"}],\"pluginNames\":[\"GeneralDBPlugin\",\"ObsPlugin\",\"VirtualizationPlugin\",\"FusionComputePlugin\"]}\n");
        extendInfoOfAgent.put("scenario", "0");
        extendInfoOfAgent.put("register_user_id", "88a94c476f12a21e016f12a246e50009");
        extendInfoOfAgent.put("internal_agent_esn", "");
        agentEnvironment.setExtendInfo(extendInfoOfAgent);
        agentEnvironment.setEndpoint("192.168.101.103");
        agentEnvironment.setPort(59521);
        agentEnvironment.setLinkStatus("1");
        agentEnvironment.setUsername("");
        agentEnvironment.setLocation("");
        agentEnvironment.setOsType("linux");
        agentEnvironment.setOsName("linux");
        agentEnvironment.setScanInterval(3600);
        agentEnvironment.setCluster(false);
        List<ProtectedEnvironment> protectedEnvironmentList = new ArrayList<>();
        protectedEnvironmentList.add(agentEnvironment);
        Map<ProtectedResource, List<ProtectedEnvironment>> map = new HashMap<>();
        ProtectedEnvironment protectedResource = new ProtectedEnvironment();
        protectedResource.setEndpoint("8.40.162.56");
        protectedResource.setPort(443);
        protectedResource.setLinkStatus("1");
        protectedResource.setUsername(null);
        protectedResource.setLocation(null);
        protectedResource.setOsType(null);
        protectedResource.setOsName(null);
        protectedResource.setCluster(false);
        protectedResource.setRegisterType(null);
        protectedResource.setScanInterval(3600);
        protectedResource.setStartDate(null);
        map.put(protectedResource, protectedEnvironmentList);
        return map;
    }

    /**
     * scan方法中 environment的值
     */
    public static ProtectedEnvironment mockScanEnvironment() {
        ProtectedEnvironment environment = new ProtectedEnvironment();
        environment.setUuid("65fae57fb3914cd1b9e659074a9f0364");
        environment.setName("test");
        environment.setType("CNware");
        environment.setSubType("CNware");
        environment.setPath("8.40.162.56");
        environment.setCreatedTime("2023-12-15 11:09:46.406");
        environment.setRootUuid("65fae57fb3914cd1b9e659074a9f0364");
        environment.setSourceType("register");
        environment.setVersion("8.2.3");
        environment.setProtectionStatus(0);
        environment.setExtendInfo(mockExtendInfo());
        environment.setEndpoint("8.40.162.56");
        environment.setPort(443);
        environment.setAuth(mockAuth());
        Map<String, List<ProtectedResource>> dependencies = new HashMap<>();
        List<ProtectedResource> resourceArrayList = new ArrayList<>();
        ProtectedEnvironment agentEnvironment = new ProtectedEnvironment();
        agentEnvironment.setUuid("de4572a6-5d23-4787-a8b3-673da3747ce9");
        agentEnvironment.setName("localhost.localdomain");
        agentEnvironment.setType("HOST");
        agentEnvironment.setSubType("UBackupAgent");
        agentEnvironment.setPath("");
        agentEnvironment.setCreatedTime("2023-12-12 03:27:33.0");
        agentEnvironment.setRootUuid("de4572a6-5d23-4787-a8b3-673da3747ce9");
        agentEnvironment.setSourceType("");
        agentEnvironment.setVersion("1.6.RC1.032");
        agentEnvironment.setProtectionStatus(0);
        Map<String, String> extendInfoOfAgent = new HashMap<>();
        extendInfoOfAgent.put("pushRegister", "true");
        extendInfoOfAgent.put("agentIpList", "192.168.101.103,8.40.101.103,fe80::29b1:8b3b:663d:77c2,fe80::5e9c:6700:b1d0:caaa");
        extendInfoOfAgent.put("agent_domain_available_ip", "{\"2102354PBB10MC100006\":\"protectengine-1.protectengine.dpa.svc.cluster.local,protectengine-0.protectengine.dpa.svc.cluster.local\"}");
        extendInfoOfAgent.put("src_deduption", "true");
        extendInfoOfAgent.put("install_path", "/opt");
        extendInfoOfAgent.put("connection_result", "{\"2102354PBB10MC100006\":{\"end_point\":\"8.40.97.155,8.40.97.156\",\"link_status\":1,\"cluster_name\":\"BackupStorage\"}}");
        extendInfoOfAgent.put("agent_last_update_time_2102354PBB10MC100006", "1702909781438");
        extendInfoOfAgent.put("availableZone", "");
        extendInfoOfAgent.put("$citations_agents_ecdfb173fe04434b83bd5d3d92b977af", "65fae57fb3914cd1b9e659074a9f0364");
        extendInfoOfAgent.put("agent_applications", "{\"menus\":[{\"applications\":[{\"appDesc\":\"\",\"appLabel\":\"agent.application.huaweicloudstack\",\"appValue\":\"HCScontainer,HCSCloudHost,HCSProject,HCSTenant\",\"isChosen\":true,\"pluginName\":\"VirtualizationPlugin\"},{\"appDesc\":\"\",\"appLabel\":\"OpenStack\",\"appValue\":\"OpenStackContainer,OpenStackProject,OpenStackCloudServer\",\"isChosen\":true,\"pluginName\":\"VirtualizationPlugin\"},{\"appDesc\":\"CNware\",\"appLabel\":\"CNware\",\"appValue\":\"CNware,CNwareHostPool,CNwareCluster,CNwareHost,CNwareVm,CNwareDisk\",\"pluginName\":\"VirtualizationPlugin\",\"isChosen\":true}],\"isChosen\":false,\"menuDesc\":\"\",\"menuLabel\":\"agent.application.menu.cloudplatform\",\"menuValue\":\"Cloud Platform\"}],\"pluginNames\":[\"VirtualizationPlugin\"]}+");
        extendInfoOfAgent.put("scenario", "0");
        extendInfoOfAgent.put("register_user_id", "88a94c476f12a21e016f12a246e50009");
        extendInfoOfAgent.put("internal_agent_esn", "");
        agentEnvironment.setExtendInfo(extendInfoOfAgent);
        agentEnvironment.setEndpoint("192.168.101.103");
        agentEnvironment.setPort(59524);
        agentEnvironment.setLinkStatus("1");
        agentEnvironment.setUsername("");
        agentEnvironment.setLocation("");
        agentEnvironment.setOsType("linux");
        agentEnvironment.setOsName("linux");
        agentEnvironment.setScanInterval(3600);
        agentEnvironment.setCluster(false);
        resourceArrayList.add(agentEnvironment);
        dependencies.put("agents", resourceArrayList);
        environment.setDependencies(dependencies);
        environment.setLinkStatus("1");
        environment.setScanInterval(3600);
        environment.setCluster(false);
        return environment;
    }

    /**
     * 环境信息中 auth的值
     */
    private static Authentication mockAuth() {
        Authentication auth = new Authentication();
        auth.setAuthType(1);
        auth.setAuthKey("admin");
        auth.setAuthPwd("passw0rd");
        Map<String, String> extendInfoOfAuth = new HashMap<>();
        extendInfoOfAuth.put("enableCert", "0");
        auth.setExtendInfo(extendInfoOfAuth);
        return auth;
    }

    /**
     * doScanResources方法中 getDetailPageList的request参数
     */
    public static ListResourceV2Req mockRequestCnwareHostPool() {
        ListResourceV2Req request = new ListResourceV2Req();
        request.setPageSize(1000);
        request.setPageNo(0);
        request.setAppEnv(mockAppEnv());
        request.setApplications(Lists.newArrayList());
        Map<String, String> conditions = new HashMap<>();
        conditions.put(CnwareConstant.RESOURCE_TYPE_KEY, CnwareConstant.RES_TYPE_CNWARE_HOST_POOL);
        conditions.put(CnwareConstant.IS_TREE, String.valueOf(CnwareConstant.ZERO));
        request.setConditions(JSON.toJSONString(conditions));
        return request;
    }

    /**
     * setVersion方法中 queryClusterInfo的返回值
     */
    public static AppEnvResponse mockAppEnvResponse() {
        AppEnvResponse appEnvResponse = new AppEnvResponse();
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("productVersion", "8.2.3");
        appEnvResponse.setExtendInfo(extendInfo);
        return appEnvResponse;
    }

    /**
     * 生成user token
     *
     * @param userType user type
     */
    public static void mockTokenBo(String userType) {
        PowerMockito.mockStatic(TokenBo.class);
        TokenBo mockToken = PowerMockito.mock(TokenBo.class);
        PowerMockito.when(TokenBo.get()).thenReturn(mockToken);
        TokenBo.UserBo userBo = new TokenBo.UserBo();
        TokenBo.RoleBo roleBo = new TokenBo.RoleBo();
        userBo.setUserType(userType);
        userBo.setId("userIdCon");
        roleBo.setName(Constants.Builtin.ROLE_SYS_ADMIN);
        userBo.setRoles(Collections.singletonList(roleBo));
        PowerMockito.when(mockToken.getUser()).thenReturn(userBo);
    }

    /**
     * doScanResources方法中 getDetailPageList的返回值
     */
    public static PageListResponse<ProtectedResource> mockResponseCnwareHostPool() {
        PageListResponse<ProtectedResource> response = new PageListResponse<>();
        response.setTotalCount(2);
        List<ProtectedResource> resources = new ArrayList<>();
        ProtectedResource protectedResource1 = new ProtectedResource();
        protectedResource1.setUuid("283cd310-47b6-4256-b832-06fe422a2004");
        protectedResource1.setName("Op-CNware-2");
        protectedResource1.setType("CNware");
        protectedResource1.setSubType("CNwareHostPool");
        protectedResource1.setParentName("test-155");
        protectedResource1.setParentUuid("fb02a492b5e34ca8a17afaff6f91c01e");
        Map<String, String> extendInfo1 = new HashMap<>();
        extendInfo1.put("details", "{\"remark\":\"\"}");
        protectedResource1.setExtendInfo(extendInfo1);
        resources.add(protectedResource1);
        ProtectedResource protectedResource2 = new ProtectedResource();
        protectedResource2.setUuid("3626d9a1-3e59-4c23-bd5d-57d4dbeb81be");
        protectedResource2.setName("Op-CNware-1");
        protectedResource2.setType("CNware");
        protectedResource1.setSubType("CNwareHostPool");
        protectedResource2.setParentName("test-155");
        protectedResource2.setParentUuid("fb02a492b5e34ca8a17afaff6f91c01e");
        Map<String, String> extendInfo2 = new HashMap<>();
        extendInfo2.put("details", "{\"remark\":\"\"}");
        protectedResource2.setExtendInfo(extendInfo2);
        resources.add(protectedResource2);
        response.setRecords(resources);
        return response;
    }

    /**
     * 环境信息中 extendInfo的值
     */
    private static Map<String, String> mockExtendInfo() {
        Map<String, String> extendInfo = new HashMap<>();
        extendInfo.put("enableCert", "0");
        extendInfo.put("certName", "");
        extendInfo.put("certSize", "");
        extendInfo.put("crlName", "");
        extendInfo.put("crlSize", "");
        extendInfo.put("connection_result", "{\"2102354PBB10MC100006\":{\"end_point\":\"8.40.97.155,8.40.97.156\",\"link_status\":1,\"cluster_name\":\"BackupStorage\"}}");
        return extendInfo;
    }

    /**
     * 环境信息中 AppEnv的值
     */
    public static AppEnv mockAppEnv() {
        AppEnv appEnv = new AppEnv();
        appEnv.setUuid("65fae57fb3914cd1b9e659074a9f0364");
        appEnv.setName("test");
        appEnv.setType("CNware");
        appEnv.setSubType("CNware");
        appEnv.setEndpoint("8.40.162.56");
        appEnv.setAuth(mockAuth());
        appEnv.setExtendInfo(mockExtendInfo());
        return appEnv;
    }
}